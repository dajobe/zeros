/*
 * progress.c - TTY progress reporting and summary output
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#include "progress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "format.h"

struct progress_state {
  char progname[64];
  char filename[512];
  uint64_t chunk_size;
  int quiet;
  int is_tty;
  int tty_line_active;
  struct timeval start;
  struct timeval last_update;
};

static double timeval_secs(const struct timeval *tv) {
  return (double)tv->tv_sec + (double)tv->tv_usec / 1.0e6;
}

static double elapsed_since(const struct timeval *start,
                            const struct timeval *now) {
  return timeval_secs(now) - timeval_secs(start);
}

progress_state *progress_create(const char *progname, const char *filename,
                                uint64_t chunk_size, int quiet) {
  progress_state *ps;

  ps = calloc(1, sizeof(*ps));
  if (!ps)
    return NULL;
  strncpy(ps->progname, progname, sizeof(ps->progname) - 1);
  strncpy(ps->filename, filename, sizeof(ps->filename) - 1);
  ps->chunk_size = chunk_size;
  ps->quiet = quiet;
  ps->is_tty = isatty(STDERR_FILENO);
  ps->tty_line_active = 0;
  gettimeofday(&ps->start, NULL);
  ps->last_update = ps->start;
  return ps;
}

void progress_destroy(progress_state *ps) { free(ps); }

void progress_print_method(progress_state *ps, zerofill_method method,
                           const char *fallback_reason) {
  const char *name;
  char chunk_buf[32];

  if (ps->quiet)
    return;

  name = zerofill_method_name(method);
  format_bytes(ps->chunk_size, chunk_buf, sizeof(chunk_buf));
  fprintf(stderr, "%s: filling %s via %s, %s buffer\n", ps->progname,
          ps->filename, name, chunk_buf);
  if (fallback_reason && fallback_reason[0])
    fprintf(stderr, "%s: note: %s\n", ps->progname, fallback_reason);
}

static int progress_emit(progress_state *ps, uint64_t done, uint64_t total,
                         int force_newline) {
  struct timeval now;
  char done_buf[32];
  char total_buf[32];
  char rate_buf[32];
  char eta_buf[32];
  double elapsed;
  double rate;
  double pct;
  double eta;
  double since_last;

  gettimeofday(&now, NULL);
  elapsed = elapsed_since(&ps->start, &now);
  since_last = elapsed_since(&ps->last_update, &now);

  if (!force_newline && ps->is_tty && since_last < 0.1)
    return 0;

  ps->last_update = now;

  if (total == 0) {
    pct = 100.0;
    eta = 0;
    rate = 0;
  } else {
    pct = 100.0 * (double)done / (double)total;
    rate = (elapsed > 0) ? (double)done / elapsed : 0;
    eta = (rate > 0) ? (double)(total - done) / rate : 0;
  }

  format_bytes_pair(done, total, done_buf, total_buf, sizeof(done_buf));
  format_rate(rate, rate_buf, sizeof(rate_buf));
  format_duration(eta, eta_buf, sizeof(eta_buf));

  if (ps->is_tty && !force_newline) {
    fprintf(stderr,
            "\r%s: %s  %.1f%%  %s/%s  %s  ETA %s\033[K", ps->progname,
            ps->filename, pct, done_buf, total_buf, rate_buf, eta_buf);
    ps->tty_line_active = 1;
  } else {
    fprintf(stderr, "%s: %s  %.1f%%  %s/%s  %s  ETA %s\n", ps->progname,
            ps->filename, pct, done_buf, total_buf, rate_buf, eta_buf);
  }
  fflush(stderr);
  return 1;
}

void progress_update(progress_state *ps, uint64_t done, uint64_t total) {
  if (ps->quiet)
    return;
  progress_emit(ps, done, total, 0);
}

void progress_finish(progress_state *ps, uint64_t total, double elapsed_secs,
                     zerofill_method method) {
  char size_buf[32];
  char rate_buf[32];
  double rate;
  const char *method_name;

  if (!ps->quiet && ps->is_tty && ps->tty_line_active)
    fprintf(stderr, "\n");

  format_bytes(total, size_buf, sizeof(size_buf));
  method_name = zerofill_method_name(method);

  if (elapsed_secs < 0.05) {
    fprintf(stderr, "%s: wrote %s  %s in <0.1s  (instant, %s)\n", ps->progname,
            ps->filename, size_buf, method_name);
  } else {
    rate = (double)total / elapsed_secs;
    format_rate(rate, rate_buf, sizeof(rate_buf));
    fprintf(stderr, "%s: wrote %s  %s in %.1fs  (%s avg, %s)\n", ps->progname,
            ps->filename, size_buf, elapsed_secs, rate_buf, method_name);
  }
  fflush(stderr);
}
