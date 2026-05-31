/*
 * cli.h - CLI help and version declarations
 *
 * Copyright (c) 2009-2026 David Beckett
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE for the full license text.
 */

#ifndef CLI_H
#define CLI_H

void zero_print_brief(const char *progname);
void zero_print_help(const char *progname);
void trunc_print_help(const char *progname);
void print_version(const char *progname);

#endif /* CLI_H */
