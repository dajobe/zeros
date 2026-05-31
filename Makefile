PROGS= trunc zero
TEST_PROGS= test_size test_format
CFLAGS= -Wall -O2 -g -D_FILE_OFFSET_BITS=64
CPPFLAGS= -DUSE_MMAP

COMMON_OBJS= size.o format.o zerofill.o cli.o progress.o

all: $(PROGS)

.PHONY: all clean check
.PHONY: test test-size test-format test-trunc test-zero test-progress

clean:
	$(RM) -f $(PROGS) $(TEST_PROGS) *.o *~ test_*.out test_*.img test_*.tmp

size.o: size.c size.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c size.c -o $@

format.o: format.c format.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c format.c -o $@

zerofill.o: zerofill.c zerofill.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c zerofill.c -o $@

cli.o: cli.c cli.h version.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c cli.c -o $@

progress.o: progress.c progress.h zerofill.h format.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c progress.c -o $@

zero.o: zero.c size.h cli.h progress.h zerofill.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c zero.c -o $@

trunc.o: trunc.c size.h cli.h progress.h zerofill.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c trunc.c -o $@

test_size.o: test_size.c size.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c test_size.c -o $@

test_format.o: test_format.c format.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c test_format.c -o $@

trunc: trunc.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) trunc.o $(COMMON_OBJS) -o $@ $(LIBS)

zero: zero.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) zero.o $(COMMON_OBJS) -o $@ $(LIBS)

test_size: test_size.o size.o
	$(CC) $(LDFLAGS) test_size.o size.o -o $@ $(LIBS)

test_format: test_format.o format.o
	$(CC) $(LDFLAGS) test_format.o format.o -o $@ $(LIBS)

test-size: test_size
	./test_size

test-format: test_format
	./test_format

test-trunc: trunc
	@echo "Running trunc tests..."
	@./test_trunc.sh

test-zero: zero
	@echo "Running zero tests..."
	@./test_zero.sh

test-progress: zero
	@echo "Running progress tests..."
	@./test_progress.sh

test: all test-size test-format test-trunc test-zero test-progress
	@echo "All tests completed successfully."

check: test
