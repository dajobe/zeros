PROGS= trunc zero
TEST_PROGS= test_size
# Standard flags: Wall enables warnings, O2 optimizes, g adds debug info
CFLAGS= -Wall -O2 -g
# Preprocessor flags: USE_MMAP enables mmap() in zero.c
CPPFLAGS= -DUSE_MMAP
# Linker flags (if needed)
# LDFLAGS=
# Libraries (if needed, e.g., -lm for math library)
# LIBS=

all: $(PROGS)

.PHONY: all clean check
.PHONY: test test-size test-trunc test-zero


clean:
	$(RM) -f $(PROGS) $(TEST_PROGS) *.o *~ test_*.out test_*.img test_*.tmp

size.o: size.c size.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c size.c -o $@

trunc.o: trunc.c size.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c trunc.c -o $@

zero.o: zero.c size.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c zero.c -o $@

test_size.o: test_size.c size.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c test_size.c -o $@

trunc: trunc.o size.o
	$(CC) $(LDFLAGS) trunc.o size.o -o $@ $(LIBS)

zero: zero.o size.o
	$(CC) $(LDFLAGS) zero.o size.o -o $@ $(LIBS)

test_size: test_size.o size.o
	$(CC) $(LDFLAGS) test_size.o size.o -o $@ $(LIBS)

# Individual test runners
test-size: test_size
	./test_size

test-trunc: trunc
	@echo "Running trunc tests..."
	@./test_trunc.sh

test-zero: zero
	@echo "Running zero tests..."
	@./test_zero.sh

test: all test_size test-size test-trunc test-zero
	@echo "All tests completed successfully."

check: test

