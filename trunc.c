/*
 * trunc.c - make a lot of zeros
 *
 * USAGE:
 *   trunc FILENAME SIZE[K|M|G]
 * 
 * Copyright (c) 2009-2024 David Beckett
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

extern long long parse_size(const char *size_str, long long max_size);


int main(int argc, char *argv[]);


int main(int argc, char *argv[]) 
{
  char *filename;
  int fd;
  off_t target_size;
  long long max_off_t;

  /* Determine the maximum possible value that can be stored in a
   * variable of type off_t
   */
  if((off_t)-1 > 0)
    /* off_t is unsigned */
    max_off_t = (long long)((off_t)-1);
  else
    /* off_t is signed. want max positive value. In binary: 011....111 */
    max_off_t = (long long)(~((off_t)1 << (sizeof(off_t) * 8 - 1)));

  if(argc != 3) {
    fprintf(stderr, "USAGE: %s FILENAME SIZE[K|M|G|T|P]\n", argv[0]);
    exit(1);
  }

  filename = argv[1];
  target_size = (off_t)parse_size(argv[2], max_off_t);
  if (target_size < 0) {
    exit(1);
  }
  
  fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if(fd < 0) {
    fprintf(stderr, "%s: Failed to open output file %s - %s\n",
            argv[0], filename, strerror(errno));
    exit(1);
  }


  /* ftruncate is the most efficient way to set file size, possibly
   * creating a sparse file if extending.
   */
  if(ftruncate(fd, target_size) == -1) {
    fprintf(stderr, "%s: Failed to truncate file %s to %lld bytes - %s\n",
            argv[0], filename, (long long)target_size, strerror(errno));
    close(fd);
  }
  fprintf(stderr, "%s: Truncated %s to %lld bytes\n",
          argv[0], filename, (long long)target_size);

  close(fd);

  return 0;
}
