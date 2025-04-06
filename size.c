#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


/* Parse size string (e.g., "10G", "512M", "2048K", "1024") with case
 * insensitive suffix.
 *
 * Returns size in bytes as long long, or <0 on error.
 */
long long
parse_size(const char *size_str, long long max_size)
{
  char *endptr;
  long long size;

  errno = 0;

  size = strtoll(size_str, &endptr, 10);
  /* Check for conversion errors or non-numeric input */
  if (errno != 0 || endptr == size_str || size < 0) {
    fprintf(stderr, "Invalid size number: %s\n", size_str);
    return -1;
  }

  /* Check for suffixes (case-insensitive) */
  if (*endptr) {
    char suffix = toupper((unsigned char)*endptr);
    /* Ensure only one suffix character */
    if (!endptr[1]) {
      if (suffix == 'K') {
        size <<= 10;
      } else if (suffix == 'M') {
        size <<= 20;
      } else if (suffix == 'G') {
        size <<= 30;
      } else if (suffix == 'T') {
        size <<= 40;
      } else if (suffix == 'P') {
        size <<= 50;
      } else {
        fprintf(stderr, "Invalid size suffix: %c (use K, M, G, T or P)\n", *endptr);
        return -1;
      }
    } else {
      fprintf(stderr, "Invalid characters after size suffix: %s\n", endptr);
      return -1;
    }
  }
  
  /* Check if the final size exceeds limits */
  if (size > max_size) {
    fprintf(stderr, "Size value exceeds maximum allowed: %lld\n", size);
    return -1;
  }

  return size;
}
