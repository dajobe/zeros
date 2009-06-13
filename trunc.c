/*
 * trunc.c - make a lot of zeros
 * 
 * Copyright (c) 2009 David Beckett
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


static size_t size = 1073741824;
static size_t nitems = 10;


#define USE_STDIO 0

int main(int argc, char *argv[]);


int main(int argc, char *argv[]) 
{
  char *filename;
#if USE_STDIO
  FILE *handle;
#else
  int fd;
#endif
  off_t buffer_size = (off_t)(size * nitems);
  
  if(argc != 2) {
    fprintf(stderr, "USAGE %s: FILENAME\n", argv[0]);
    exit(1);
  }

  filename=argv[1];
  
#if USE_STDIO
  handle = fopen(filename, "wb");
  if(!handle) {
    fprintf(stderr, "Failed to open output file %s - %s\n", filename,
            strerror(errno));
    exit(1);
  }
#else
  fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if(!fd) {
    fprintf(stderr, "Failed to open output file %s - %s\n", filename,
            strerror(errno));
    exit(1);
  }
#endif


#if USE_STDIO
  error cannot
#else
  ftruncate(fd, buffer_size);
#endif

#if USE_STDIO
  fclose(handle);
#else
  close(fd);
#endif

  return 0;
}
