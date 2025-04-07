#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


/* Parse size string (e.g., "10G", "512M", "2048K", "1024") with case
 * insensitive suffix (K, M, G, T, P).
 *
 * @param size_str String containing the size to parse.
 * @param max_size The maximum allowed value for the parsed size.
 * Returns size in bytes as long long, or <0 on error.
 */
long long
parse_size(const char *size_str, long long max_size)
{
  char *endptr;
  long long size;

  errno = 0;

  size = strtoll(size_str, &endptr, 10);
  /* Check for conversion errors, non-numeric input, or negative values */
  if (errno != 0 || endptr == size_str || size < 0) {
    fprintf(stderr, "Invalid size number: %s\n", size_str);
    return -1;
  }

  /* Check for suffixes (case-insensitive) */
  if (*endptr) {
    char suffix = toupper((unsigned char)*endptr);
    int shift = 0;
    /* Ensure only one suffix character */
    if (!endptr[1]) {
      if (suffix == 'K') {
        shift = 10;
      } else if (suffix == 'M') {
        shift = 20;
      } else if (suffix == 'G') {
        shift = 30;
      } else if (suffix == 'T') {
        shift = 40;
      } else if (suffix == 'P') {
        shift = 50;
      } else {
        fprintf(stderr, "Invalid size suffix: %c (use K, M, G, T or P)\n",
                *endptr);
        return -1;
      }
      /* Check for overflow before shifting */
      if (size > (max_size >> shift)) {
        fprintf(stderr, "Size value with suffix '%c' exceeds maximum allowed: %s\n", *endptr, size_str);
         return -1;
      }
    } else {
      fprintf(stderr, "Invalid characters after size suffix: %s\n", endptr);
      return -1;
    }

    /* Apply suffix multiplier */
    size <<= shift;
  }

  /* Check if the final size exceeds the provided maximum */
  if (size > max_size) {
    fprintf(stderr, "Size value exceeds maximum allowed: %lld\n", size);
    return -1;
  }

  return size;
}
