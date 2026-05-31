/*
 * format.c - IEC byte, rate, and duration formatting helpers
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#include "format.h"

#include <stdio.h>

static const char *const unit_labels[] = {"B",  "KiB", "MiB",
                                          "GiB", "TiB", "PiB"};
static const int n_units = 6;

static int unit_index(uint64_t n) {
  int idx = 0;
  while (idx < n_units - 1 && n >= 1024ULL) {
    n >>= 10;
    idx++;
  }
  return idx;
}

static void format_with_unit(uint64_t n, int idx, char *buf, size_t buflen) {
  if (idx == 0) {
    snprintf(buf, buflen, "%llu B", (unsigned long long)n);
    return;
  }
  double val = (double)n / (double)(1ULL << (idx * 10));
  snprintf(buf, buflen, "%.2f %s", val, unit_labels[idx]);
}

void format_bytes(uint64_t n, char *buf, size_t buflen) {
  format_with_unit(n, unit_index(n), buf, buflen);
}

void format_bytes_pair(uint64_t done, uint64_t total, char *done_buf,
                       char *total_buf, size_t buflen) {
  int done_idx = unit_index(done);
  int total_idx = unit_index(total);

  if (done_idx == total_idx || total == 0) {
    format_with_unit(done, total_idx, done_buf, buflen);
    format_with_unit(total, total_idx, total_buf, buflen);
  } else {
    format_with_unit(done, done_idx, done_buf, buflen);
    format_with_unit(total, total_idx, total_buf, buflen);
  }
}

void format_rate(double bytes_per_sec, char *buf, size_t buflen) {
  double val;
  int idx;

  if (bytes_per_sec < 0)
    bytes_per_sec = 0;
  val = bytes_per_sec;
  idx = 0;
  while (idx < n_units - 1 && val >= 1024.0) {
    val /= 1024.0;
    idx++;
  }
  if (idx == 0)
    snprintf(buf, buflen, "%.0f B/s", val);
  else
    snprintf(buf, buflen, "%.2f %s/s", val, unit_labels[idx]);
}

void format_duration(double secs, char *buf, size_t buflen) {
  long total;
  long hours;
  long minutes;
  long seconds;

  if (secs < 0)
    secs = 0;
  total = (long)(secs + 0.5);
  hours = total / 3600;
  minutes = (total % 3600) / 60;
  seconds = total % 60;

  if (hours > 0)
    snprintf(buf, buflen, "%ldh %02ldm", hours, minutes);
  else if (minutes > 0)
    snprintf(buf, buflen, "%ldm %02lds", minutes, seconds);
  else
    snprintf(buf, buflen, "%lds", seconds);
}
