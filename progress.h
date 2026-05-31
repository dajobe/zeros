/*
 * progress.h - progress reporting API
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#ifndef PROGRESS_H
#define PROGRESS_H

#include <stddef.h>
#include <stdint.h>

#include "zerofill.h"

typedef struct progress_state progress_state;

progress_state *progress_create(const char *progname, const char *filename,
                                uint64_t chunk_size, int quiet);
void progress_destroy(progress_state *ps);

void progress_print_method(progress_state *ps, zerofill_method method,
                         const char *fallback_reason);

void progress_update(progress_state *ps, uint64_t done, uint64_t total);

void progress_finish(progress_state *ps, uint64_t total, double elapsed_secs,
                     zerofill_method method);

#endif /* PROGRESS_H */
