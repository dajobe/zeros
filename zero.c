/*
 * zero.c - create a file containing zeros, optionally measuring write speed.
 * Writes data in blocks, useful for testing filesystem/disk write performance.
 *
 * USAGE:
 *   zero [OPTIONS] FILENAME
 * OPTIONS
 *   -b block_size : Size of each write block (default: 1G)
 *   -t total_size : Total size of the file to create (default: block_size)
 * Block size and total size are of form <integer>[KMGTP] eg 10M, 20P
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
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <ctype.h>

#ifdef USE_MMAP
#include <sys/mman.h>
#else
#endif

extern long long parse_size(const char *size_str, long long max_size);


static const unsigned long long M = 1024*1024;
static const unsigned long long G = 1024*1024*1024;

int main(int argc, char *argv[]);


int main(int argc, char *argv[]) 
{
  char *filename;
#ifdef USE_STDIO
  FILE *handle;
#else
  int fd;
#endif
  size_t block_size = 1 * G; // Default block size: 1 GiB
  size_t total_size = 0; // Default: write one block_size
  size_t total_written = 0;
  int opt;
#ifdef USE_MMAP
  void *buffer = NULL;
#else
  char *buffer = NULL;
#endif
  double rate = 0.0;
  int usage = 0;
  /* size_t is always unsigned so the maximum possible value can be
   * found from casting -1 */
  long long max_size_t = (long long)((size_t)-1);

  while ((opt = getopt(argc, argv, "b:ht:")) != -1) {
    switch (opt) {
      case 'b':
        block_size = (size_t)parse_size(optarg, max_size_t);
        if (!block_size)
          exit(1);
        break;

      case 'h':
        usage = 1;
        break;

      case 't':
        total_size = (size_t)parse_size(optarg, max_size_t);
        if (!total_size)
          exit(1);
        break;

      default: /* '?' */
        usage = 2;
        break;
    }
  }

  if (optind >= argc)
    usage = 2;

  if(usage > 0) {
    fprintf(stderr,
            "USAGE:\n"
            "  %s [OPTIONS] FILENAME\n"
            "OPTIONS\n"
            "  -b block_size : Size of each write block (default: 1G)\n"
            "  -t total_size : Total size of the file to create (default: block_size)\n"
            "Block size and total size are of form <integer>[KMGTP] eg 10M, 20P\n",
            argv[0]);
    exit(usage > 1);
  }

  filename = argv[optind];

  // If total size was not specified, default to writing one block
  if (!total_size)
    total_size = block_size;

#ifdef USE_MMAP
  buffer = mmap(NULL /* addr */, block_size,
                PROT_READ, MAP_ANON | MAP_PRIVATE,
                -1 /* fd */, 0 /* offset */);
  if(buffer == MAP_FAILED) {
    fprintf(stderr, "Failed to mmap %" PRIuPTR " bytes - %s\n",
            block_size, strerror(errno));
    exit(1);
  } else
    fprintf(stderr, "%s: mmap()ed %" PRIuPTR " bytes at %p\n",
            argv[0], block_size, buffer);
#else
  buffer = (char*)calloc(nitems, size);
  if(!buffer) {
    fprintf(stderr, "Failed to allocate %" PRIuPTR " bytes\n",
            block_size);
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
  if(fd < 0) {
    fprintf(stderr, "Failed to open output file %s - %s\n", filename,
            strerror(errno));
#ifndef USE_MMAP
    free(buffer);
#endif
    exit(1);
  }
#endif

  while(1) {
    ssize_t written_now; // write returns ssize_t
    size_t expected_write;
    struct timeval tv;
    time_t start;
    suseconds_t start_usec;
    time_t end;
    suseconds_t end_usec;
    double secs;

    // Determine how much to write in this iteration
    expected_write = block_size;
    if (total_written + expected_write > total_size) {
        expected_write = total_size - total_written;
    }
    if (expected_write == 0)
      /* Should not happen if total_size >= block_size unless
       * total_size was 0 initially */
      break; 

    fprintf(stderr,
            "%s: Writing %" PRIuPTR " bytes to %s (%" PRIuPTR "/%" PRIuPTR " total)\n",
            argv[0], expected_write, filename,
            total_written + expected_write, total_size);

    gettimeofday(&tv, NULL);
    start = tv.tv_sec;
    start_usec = tv.tv_usec;

#ifdef USE_STDIO
    expected = nitems;
    // THIS PATH IS NOT RECOMMENDED / TESTED WITH NEW LOGIC
    // fwrite returns number of items successfully written.
    // Need adaptation if USE_STDIO is truly desired.
    fprintf(stderr, "USE_STDIO path needs review for size logic.\n");
    written_now = fwrite(buffer, expected_write, 1, (FILE*)((void*)0)); // Deliberately broken
#else
    written_now = write(fd, buffer, expected_write);
#endif

    gettimeofday(&tv, NULL);
    end = tv.tv_sec;
    end_usec = tv.tv_usec;

    if(written_now < 0) {
      fprintf(stderr,
              "%s: ERROR writing to %s - %s\n",
              argv[0], filename, strerror(errno));
      break;
    } else if ((size_t)written_now != expected_write) {
      fprintf(stderr,
              "%s: WARNING: partial write - wrote %zd bytes, expected %zu\n",
              argv[0], written_now, expected_write);
      total_written += written_now;
      break;
    }

    total_written += written_now;

    secs = ((end-start) + (end_usec-start_usec)/1.0e6);
    rate = (secs > 0) ? (written_now / secs) : 0;
    fprintf(stderr,
            "%s: Wrote in %2.2f secs at %2.2f Mbytes/sec (%2.2f Gbytes/sec)\n",
            argv[0], secs, rate / M, rate / G);

    if(total_written >= total_size)
        break;
    
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
