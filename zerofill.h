/*
 * zerofill.h - zero-fill API
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#ifndef ZEROFILL_H
#define ZEROFILL_H

#include <stdint.h>

typedef enum {
  ZEROFILL_FALLOCATE_ZERO_RANGE, /* Linux fallocate FALLOC_FL_ZERO_RANGE */
  ZEROFILL_WRITE                 /* mmap + write loop; physically writes zeros */
} zerofill_method;

typedef int (*zerofill_progress_fn)(void *ctx, uint64_t done, uint64_t total);

typedef void (*zerofill_method_fn)(void *ctx, zerofill_method method,
                                   const char *fallback_reason);

typedef struct {
  zerofill_method method;
  char fallback_reason[256];
} zerofill_info;

const char *zerofill_method_name(zerofill_method method);

/* Fill an open file descriptor with physical zeros up to total_size.
 * Returns 0 on success, -1 on error. */
int zerofill(int fd, uint64_t total_size, uint64_t chunk_size,
             zerofill_progress_fn progress, void *progress_ctx,
             zerofill_method_fn method_cb, void *method_ctx,
             zerofill_info *info);

#endif /* ZEROFILL_H */
