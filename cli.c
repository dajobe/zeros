/*
 * cli.c - shared help and version output for zero and trunc
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#include "cli.h"

#include <stdio.h>

#include "version.h"

void zero_print_brief(const char *progname) {
  fprintf(stderr,
          "Usage: %s [-b SIZE] [-t SIZE] [-q] FILENAME\n"
          "Try '%s -h' for full help.\n",
          progname, progname);
}

void zero_print_help(const char *progname) {
  fprintf(stderr,
          "%s - create a file filled with zeros\n"
          "\n"
          "Usage:\n"
          "  %s [OPTIONS] FILENAME\n"
          "\n"
          "Options:\n"
          "  -b SIZE   chunk size for progress updates (default: 1G = 1 GiB)\n"
          "  -t SIZE   total file size (default: same as -b)\n"
          "  -q        quiet; suppress progress and startup method line\n"
          "  -h        show this help and exit\n"
          "  -V        show version and exit\n"
          "\n"
          "Size suffixes: K, M, G, T, P (powers of 1024)\n"
          "  e.g. 1K=1 KiB, 1G=1 GiB, 1T=1 TiB, 1P=1 PiB\n"
          "  Bare integer = bytes (e.g. 4096 = 4096 bytes)\n"
          "\n"
          "Examples:\n"
          "  %s -t 10G disk.img           create a 10 GiB zero-filled file\n"
          "  %s -t 10G -b 1G disk.img     progress every 1 GiB\n"
          "  %s -t 1T big.img             create a 1 TiB file\n"
          "  %s -q -t 1G scratch.bin      no progress output\n"
          "\n"
          "Notes:\n"
          "  Writes physical zero bytes (not sparse holes).\n"
          "  Linux uses fallocate zero-range when available, else write loop.\n"
          "  trunc(1) is a legacy wrapper (FILE SIZE argument order).\n"
          "\n"
          "Exit status: 0 on success, 1 on error, 2 on usage error.\n",
          progname, progname, progname, progname, progname, progname);
}

void trunc_print_help(const char *progname) {
  fprintf(stderr,
          "%s - legacy wrapper to create a zero-filled file\n"
          "\n"
          "Usage:\n"
          "  %s FILENAME SIZE\n"
          "  %s [-hV]\n"
          "\n"
          "Options:\n"
          "  -h        show this help and exit\n"
          "  -V        show version and exit\n"
          "\n"
          "SIZE suffixes: K, M, G, T, P (powers of 1024; 1G = 1 GiB)\n"
          "\n"
          "Prefer: zero -t SIZE FILENAME\n"
          "\n"
          "Exit status: 0 on success, 1 on error, 2 on usage error.\n",
          progname, progname, progname);
}

void print_version(const char *progname) {
  printf("%s %s\n", progname, ZERO_VERSION);
}
