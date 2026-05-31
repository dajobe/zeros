/*
 * zerofill.c - shared physical zero-fill (fallocate and write loop)
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#ifdef __linux__
#define _GNU_SOURCE
#endif

#include "zerofill.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef USE_MMAP
#include <sys/mman.h>
#endif

#ifdef __linux__
#ifndef FALLOC_FL_ZERO_RANGE
#define FALLOC_FL_ZERO_RANGE 0x10
#endif
#endif

static void info_clear(zerofill_info *info) {
  if (!info)
    return;
  info->method = ZEROFILL_WRITE;
  info->fallback_reason[0] = '\0';
}

#ifdef __linux__
static void set_fallback(zerofill_info *info, const char *msg) {
  if (!info || !msg)
    return;
  strncpy(info->fallback_reason, msg, sizeof(info->fallback_reason) - 1);
  info->fallback_reason[sizeof(info->fallback_reason) - 1] = '\0';
}

static int reset_file(int fd) {
  if (ftruncate(fd, 0) != 0)
    return -1;
  if (lseek(fd, 0, SEEK_SET) < 0)
    return -1;
  return 0;
}
#endif

const char *zerofill_method_name(zerofill_method method) {
  switch (method) {
  case ZEROFILL_FALLOCATE_ZERO_RANGE:
    return "fallocate zero-range";
  case ZEROFILL_WRITE:
    return "write loop";
  default:
    return "unknown";
  }
}

static int call_progress(zerofill_progress_fn progress, void *ctx, uint64_t done,
                         uint64_t total) {
  if (progress)
    return progress(ctx, done, total);
  return 0;
}

static void notify_method(zerofill_method_fn method_cb, void *method_ctx,
                          zerofill_info *info) {
  if (method_cb)
    method_cb(method_ctx, info->method,
              info->fallback_reason[0] ? info->fallback_reason : NULL);
}

static int verify_file_size(int fd, uint64_t expected) {
  struct stat st;

  if (fstat(fd, &st) != 0)
    return -1;
  return ((uint64_t)st.st_size == expected) ? 0 : -1;
}

#ifdef __linux__
static int try_fallocate_zero_range(int fd, uint64_t total_size,
                                    uint64_t chunk_size,
                                    zerofill_progress_fn progress,
                                    void *progress_ctx,
                                    zerofill_method_fn method_cb,
                                    void *method_ctx, zerofill_info *info) {
  uint64_t done = 0;
  int method_announced = 0;

  while (done < total_size) {
    uint64_t len = chunk_size;
    if (len > total_size - done)
      len = total_size - done;

    if (fallocate(fd, FALLOC_FL_ZERO_RANGE, (off_t)done, (off_t)len) != 0) {
      if (done == 0 &&
          (errno == EOPNOTSUPP || errno == ENOTSUP || errno == EINVAL)) {
        set_fallback(info, "fallocate zero-range unavailable on this filesystem");
        return -1;
      }
      return -1;
    }

    if (!method_announced) {
      if (info)
        info->method = ZEROFILL_FALLOCATE_ZERO_RANGE;
      notify_method(method_cb, method_ctx, info);
      method_announced = 1;
    }

    done += len;
    if (call_progress(progress, progress_ctx, done, total_size) != 0)
      return -1;
  }

  return 0;
}
#endif

#ifdef USE_MMAP
static int try_write_loop(int fd, uint64_t total_size, uint64_t chunk_size,
                          zerofill_progress_fn progress, void *progress_ctx,
                          zerofill_method_fn method_cb, void *method_ctx,
                          zerofill_info *info) {
  void *buffer;
  uint64_t done = 0;
  size_t buf_size;
  int method_announced = 0;

  if (chunk_size == 0)
    chunk_size = 1;
  buf_size = (size_t)chunk_size;
  if ((uint64_t)buf_size != chunk_size)
    buf_size = (size_t)(1ULL << 20);

  buffer = mmap(NULL, buf_size, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (buffer == MAP_FAILED)
    return -1;

  while (done < total_size) {
    uint64_t len = chunk_size;
    size_t to_write;
    ssize_t written;

    if (len > total_size - done)
      len = total_size - done;
    to_write = (size_t)len;

    written = write(fd, buffer, to_write);
    if (written < 0 || (size_t)written != to_write) {
      munmap(buffer, buf_size);
      return -1;
    }

    if (!method_announced) {
      if (info)
        info->method = ZEROFILL_WRITE;
      notify_method(method_cb, method_ctx, info);
      method_announced = 1;
    }

    done += len;
    if (call_progress(progress, progress_ctx, done, total_size) != 0) {
      munmap(buffer, buf_size);
      return -1;
    }
  }

  munmap(buffer, buf_size);
  return 0;
}
#else
static int try_write_loop(int fd, uint64_t total_size, uint64_t chunk_size,
                          zerofill_progress_fn progress, void *progress_ctx,
                          zerofill_method_fn method_cb, void *method_ctx,
                          zerofill_info *info) {
  char *buffer;
  uint64_t done = 0;
  size_t buf_size;
  int method_announced = 0;

  if (chunk_size == 0)
    chunk_size = 1;
  buf_size = (size_t)chunk_size;
  if ((uint64_t)buf_size != chunk_size)
    buf_size = (size_t)(1ULL << 20);

  buffer = calloc(1, buf_size);
  if (!buffer)
    return -1;

  while (done < total_size) {
    uint64_t len = chunk_size;
    size_t to_write;
    ssize_t written;

    if (len > total_size - done)
      len = total_size - done;
    to_write = (size_t)len;

    written = write(fd, buffer, to_write);
    if (written < 0 || (size_t)written != to_write) {
      free(buffer);
      return -1;
    }

    if (!method_announced) {
      if (info)
        info->method = ZEROFILL_WRITE;
      notify_method(method_cb, method_ctx, info);
      method_announced = 1;
    }

    done += len;
    if (call_progress(progress, progress_ctx, done, total_size) != 0) {
      free(buffer);
      return -1;
    }
  }

  free(buffer);
  return 0;
}
#endif

int zerofill(int fd, uint64_t total_size, uint64_t chunk_size,
             zerofill_progress_fn progress, void *progress_ctx,
             zerofill_method_fn method_cb, void *method_ctx,
             zerofill_info *info) {
  info_clear(info);

  if (total_size == 0) {
    if (ftruncate(fd, 0) != 0)
      return -1;
    if (info)
      info->method = ZEROFILL_WRITE;
    notify_method(method_cb, method_ctx, info);
    if (call_progress(progress, progress_ctx, 0, 0) != 0)
      return -1;
    return 0;
  }

  if (chunk_size == 0)
    chunk_size = 1ULL << 30;

#ifdef __linux__
  if (try_fallocate_zero_range(fd, total_size, chunk_size, progress,
                               progress_ctx, method_cb, method_ctx, info) == 0) {
    if (verify_file_size(fd, total_size) == 0)
      return 0;
    if (reset_file(fd) != 0)
      return -1;
    set_fallback(info, "fallocate did not produce expected size; using write loop");
  }
#endif

  if (try_write_loop(fd, total_size, chunk_size, progress, progress_ctx,
                     method_cb, method_ctx, info) != 0)
    return -1;

  if (verify_file_size(fd, total_size) != 0) {
    errno = EIO;
    return -1;
  }

  return 0;
}
