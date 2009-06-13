FILES= trunc zero
CPPFLAGS= -Wall

all: $(FILES)

clean:
	rm -f $(FILES) *~

trunc: trunc.c

zero: zero.c
