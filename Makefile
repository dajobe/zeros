FILES= trunc zero
CPPFLAGS= -Wall -DUSE_MMAP

all: $(FILES)

clean:
	rm -f $(FILES) *~

trunc: trunc.c size.c

zero: zero.c size.c
