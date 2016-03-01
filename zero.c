/*
 * zero.c - make a lot of zeros
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
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_MMAP
#include <sys/mman.h>
#else
#endif

/* static unsigned int K = 1024; */
static unsigned int M = 1024*1024;
static unsigned int G = 1024*1024*1024;


static size_t size = 1073741824;
static size_t nitems = 1;


int main(int argc, char *argv[]);


int main(int argc, char *argv[]) 
{
  char *filename;
#ifdef USE_STDIO
  FILE *handle;
#else
  int fd;
#endif
  size_t block_size = (size_t)(size * nitems);
#ifdef USE_MMAP
  void *buffer = NULL;
#else
  char *buffer = NULL;
#endif
  double rate=0.0;
  
  if(argc != 2) {
    fprintf(stderr, "USAGE %s: FILENAME\n", argv[0]);
    exit(1);
  }

  filename=argv[1];

#ifdef USE_MMAP
  buffer = mmap(NULL /* addr */, block_size,
                PROT_READ, MAP_ANON | MAP_PRIVATE,
                -1 /* fd */, 0 /* offset */);
  if(buffer == MAP_FAILED) {
    fprintf(stderr, "Failed to mmap %lu bytes - %s\n",
            (long unsigned int)(size * nitems), strerror(errno));
    exit(1);
  }
  fprintf(stderr, "mmap()ed %lu bytes at %p\n",
          (long unsigned int)(size * nitems), buffer);
#else
  buffer = (char*)calloc(nitems, size);
  if(!buffer) {
    fprintf(stderr, "Failed to allocate %lu bytes\n",
            (long unsigned int)(size * nitems));
    exit(1);
  }
#endif
  
#ifdef USE_STDIO
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

  while(1) {
    size_t written;
    struct timeval tv;
    time_t start;
    suseconds_t start_usec;
    time_t end;
    suseconds_t end_usec;
    size_t expected;
    
    fprintf(stderr, "writing %d bytes to %s\n", (int)block_size, filename);

    gettimeofday(&tv, NULL);
    start = tv.tv_sec;
    start_usec = tv.tv_usec;

#ifdef USE_STDIO
    expected = nitems;
    written = fwrite(buffer, size , nitems, handle);
#else
    expected = block_size;
    written = write(fd, buffer, block_size);
#endif

    gettimeofday(&tv, NULL);
    end = tv.tv_sec;
    end_usec = tv.tv_usec;

    if(written != expected) {
      fprintf(stderr, "WARNING write wrote %d items expected %d\n",
              (int)written, (int)expected);
      break;
    }
    rate = block_size / ((end-start) + (end_usec-start_usec)/1.0e6);
    fprintf(stderr, "  wrote at %2.2f Mbytes/sec (%2.2f Gbytes/sec)\n",
            rate / M, rate / G);
  }

#ifdef USE_STDIO
  fclose(handle);
#else
  close(fd);
#endif

#ifdef USE_MMAP
  if(buffer)
     munmap(buffer, block_size);
#endif
  
  return 0;
}
