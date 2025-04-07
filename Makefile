FILES= trunc zero

# Standard flags: Wall enables warnings, O2 optimizes, g adds debug info
CFLAGS= -Wall -O2 -g
# Preprocessor flags: USE_MMAP enables mmap() in zero.c
CPPFLAGS= -DUSE_MMAP
# Linker flags (if needed)
# LDFLAGS=
# Libraries (if needed, e.g., -lm for math library)
# LIBS=


all: $(FILES)

clean:
	$(RM) -f $(FILES) *~

size.o: size.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c size.c -o size.o

trunc: trunc.o size.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) trunc.o size.o -o $@ $(LIBS)

zero: zero.o size.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) zero.o size.o -o $@ $(LIBS)
