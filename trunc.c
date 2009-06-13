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
