FILES= trunc zero
CPPFLAGS= -Wall -DUSE_MMAP

all: $(FILES)

clean:
	rm -f $(FILES) *~

trunc: trunc.c

zero: zero.c
