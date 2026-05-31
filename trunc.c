/*
 * trunc.c - legacy wrapper to create a zero-filled file
 *
 * USAGE:
 *   trunc FILENAME SIZE[K|M|G|T|P]
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
#include <sys/types.h>
#include <unistd.h>

#include "cli.h"
#include "progress.h"
#include "size.h"
#include "zerofill.h"

static double elapsed_secs(const struct timeval *start,
                           const struct timeval *end) {
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_usec - start->tv_usec) / 1.0e6;
}

int main(int argc, char *argv[]) {
  char *filename;
  int fd = -1;
  off_t target_size;
  long long max_off_t;
  int opt;
  int rc = 0;
  progress_state *progress = NULL;
  zerofill_info zinfo;
  struct timeval t_start;
  struct timeval t_end;
  double elapsed;

  if ((off_t)-1 > 0)
    max_off_t = (long long)((off_t)-1);
  else
    max_off_t =
        (long long)(~((off_t)1 << (sizeof(off_t) * 8 - 1)));

  while ((opt = getopt(argc, argv, "hV")) != -1) {
    switch (opt) {
    case 'h':
      trunc_print_help(argv[0]);
      return 0;
    case 'V':
      print_version(argv[0]);
      return 0;
    default:
      trunc_print_help(argv[0]);
      return 2;
    }
  }

  if (optind + 2 != argc) {
    trunc_print_help(argv[0]);
    return 2;
  }

  filename = argv[optind];
  target_size = (off_t)parse_size(argv[optind + 1], max_off_t);
  if (target_size < 0)
    return 1;

  progress = progress_create(argv[0], filename, 1ULL << 30, 1);
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

  if (zerofill(fd, (uint64_t)target_size, 1ULL << 30, NULL, NULL, NULL, NULL,
               &zinfo) != 0) {
    fprintf(stderr, "%s: error: failed to fill %s - %s\n", argv[0], filename,
            strerror(errno));
    rc = 1;
    goto tidy;
  }

  gettimeofday(&t_end, NULL);
  elapsed = elapsed_secs(&t_start, &t_end);
  progress_finish(progress, (uint64_t)target_size, elapsed, zinfo.method);

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
