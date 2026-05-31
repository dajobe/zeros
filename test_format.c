/*
 * test_format.c - unit tests for format.c
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"

#define ASSERT_STR(test_name, expected, actual)                                \
  if (strcmp((expected), (actual)) != 0) {                                     \
    fprintf(stderr, "FAIL: %s: expected '%s', got '%s'\n", (test_name),      \
            (expected), (actual));                                             \
    exit(1);                                                                   \
  }

int main(void) {
  char buf[64];

  printf("Running format tests...\n");

  format_bytes(0, buf, sizeof(buf));
  ASSERT_STR("0 bytes", "0 B", buf);

  format_bytes(1023, buf, sizeof(buf));
  ASSERT_STR("1023 B", "1023 B", buf);

  format_bytes(1536, buf, sizeof(buf));
  ASSERT_STR("1536 -> KiB", "1.50 KiB", buf);

  format_bytes(1ULL << 30, buf, sizeof(buf));
  ASSERT_STR("1 GiB", "1.00 GiB", buf);

  format_bytes(3ULL << 40, buf, sizeof(buf));
  ASSERT_STR("3 TiB", "3.00 TiB", buf);

  {
    char done_buf[32];
    char total_buf[32];
    format_bytes_pair(5ULL << 30, 10ULL << 30, done_buf, total_buf,
                      sizeof(done_buf));
    ASSERT_STR("pair done", "5.00 GiB", done_buf);
    ASSERT_STR("pair total", "10.00 GiB", total_buf);
  }

  format_rate(892.0 * 1024 * 1024, buf, sizeof(buf));
  ASSERT_STR("rate MiB/s", "892.00 MiB/s", buf);

  format_rate(1.5e12, buf, sizeof(buf));
  ASSERT_STR("rate TiB/s", "1.36 TiB/s", buf);

  format_duration(6.2, buf, sizeof(buf));
  ASSERT_STR("duration seconds", "6s", buf);

  format_duration(134.0, buf, sizeof(buf));
  ASSERT_STR("duration minutes", "2m 14s", buf);

  format_duration(3661.0, buf, sizeof(buf));
  ASSERT_STR("duration hours", "1h 01m", buf);

  printf("All format tests passed.\n");
  return 0;
}
