#ifndef SIZE_H
#define SIZE_H

#include <sys/types.h> /* For off_t, size_t */

/* Parse size string (e.g., "10G", "512M", "2048K", "1024") with case
 * insensitive suffix (K, M, G, T, P). Suffixes represent powers of 2.
 *
 * @param size_str String containing the size to parse.
 * @param max_size The maximum allowed value for the parsed size.
 * @return Size in bytes as long long, or <0 on error.
 */
long long parse_size(const char *size_str, long long max_size);

#endif /* SIZE_H */
