/*
 * format.h - IEC formatting API
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#ifndef FORMAT_H
#define FORMAT_H

#include <stddef.h>
#include <stdint.h>

/* Format byte count using IEC binary units (B, KiB, ... PiB). */
void format_bytes(uint64_t n, char *buf, size_t buflen);

/* Format done/total; uses same unit when magnitudes are close. */
void format_bytes_pair(uint64_t done, uint64_t total, char *done_buf,
                       char *total_buf, size_t buflen);

/* Format transfer rate as IEC units per second. */
void format_rate(double bytes_per_sec, char *buf, size_t buflen);

/* Format duration: 6s, 2m 14s, 1h 05m. */
void format_duration(double secs, char *buf, size_t buflen);

#endif /* FORMAT_H */
