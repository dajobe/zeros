/*
 * zero.c - create a file filled with zeros
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "cli.h"
#include "progress.h"
#include "size.h"
#include "zerofill.h"

static int progress_cb(void *ctx, uint64_t done, uint64_t total) {
  progress_update((progress_state *)ctx, done, total);
  return 0;
}

static void method_cb(void *ctx, zerofill_method method,
                      const char *fallback_reason) {
  progress_print_method((progress_state *)ctx, method, fallback_reason);
}

static double elapsed_secs(const struct timeval *start,
                           const struct timeval *end) {
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_usec - start->tv_usec) / 1.0e6;
}

int main(int argc, char *argv[]) {
  char *filename;
  int fd = -1;
  size_t chunk_size = 1ULL << 30;
  size_t total_size = 0;
  int total_size_changed = 0;
  int quiet = 0;
  int opt;
  long long max_size_t = (long long)((size_t)-1);
  int rc = 0;
  progress_state *progress = NULL;
  zerofill_info zinfo;
  struct timeval t_start;
  struct timeval t_end;
  double elapsed;

  while ((opt = getopt(argc, argv, "b:ht:qV")) != -1) {
    long long parsed_size;
    switch (opt) {
    case 'b':
      parsed_size = parse_size(optarg, max_size_t);
      if (parsed_size <= 0)
        return 1;
      chunk_size = (size_t)parsed_size;
      break;

    case 'h':
      zero_print_help(argv[0]);
      return 0;

    case 't':
      parsed_size = parse_size(optarg, max_size_t);
      if (parsed_size < 0)
        return 1;
      total_size = (size_t)parsed_size;
      total_size_changed = 1;
      break;

    case 'q':
      quiet = 1;
      break;

    case 'V':
      print_version(argv[0]);
      return 0;

    default:
      zero_print_brief(argv[0]);
      return 2;
    }
  }

  if (optind >= argc) {
    zero_print_brief(argv[0]);
    return 2;
  }

  filename = argv[optind];

  if (!total_size_changed)
    total_size = chunk_size;

  progress = progress_create(argv[0], filename, (uint64_t)chunk_size, quiet);
  if (!progress) {
    fprintf(stderr, "%s: error: out of memory\n", argv[0]);
    return 1;
  }

  fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd < 0) {
    fprintf(stderr, "%s: error: cannot open %s - %s\n", argv[0], filename,
            strerror(errno));
    rc = 1;
    goto tidy;
  }

  gettimeofday(&t_start, NULL);

  if (zerofill(fd, (uint64_t)total_size, (uint64_t)chunk_size, progress_cb,
               progress, method_cb, progress, &zinfo) != 0) {
    fprintf(stderr, "%s: error: failed to fill %s - %s\n", argv[0], filename,
            strerror(errno));
    rc = 1;
    goto tidy;
  }

  gettimeofday(&t_end, NULL);
  elapsed = elapsed_secs(&t_start, &t_end);

  progress_finish(progress, (uint64_t)total_size, elapsed, zinfo.method);

tidy:
  if (fd >= 0) {
    if (close(fd) != 0 && rc == 0) {
      fprintf(stderr, "%s: error: cannot close %s - %s\n", argv[0], filename,
              strerror(errno));
      rc = 1;
    }
    fd = -1;
  }
  progress_destroy(progress);
  return rc;
}
