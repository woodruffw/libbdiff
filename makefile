CC = gcc
CFLAGS = -Wall -O3 -std=gnu89
CFLAGS += -Wno-attributes -Wno-maybe-uninitialized
CPPFLAGS = -D_FILE_OFFSET_BITS=64
LDFLAGS = -lbz2

SOURCES = $(wildcard ./src/*.c)
OBJS = $(SOURCES:.c=.o)
LIBNAME = libbdiff

all: $(OBJS)
	ar -cq $(LIBNAME).a $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@ $(LDFLAGS)

tests: all
	gcc ./src/tests/hw1.c -o ./hw1
	gcc ./src/tests/hw2.c -o ./hw2
	gcc ./src/tests/bdiff_test.c -L. -lbdiff -lbz2 -o ./bin/bdiff_test
	gcc ./src/tests/bpatch_test.c -L. -lbdiff -lbz2 -o ./bin/bpatch_test
	./bin/bdiff_test
	./bin/bpatch_test
	chmod +x ./hw2.new
	./hw2.new
	rm hw*
	rm test.patch

clean:
	rm -f $(OBJS)
	rm -f $(LIBNAME).a
	rm -f ./bin/*
