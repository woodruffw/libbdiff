CC = gcc
CFLAGS = -Wall -O3 -std=c99
CFLAGS += -Wno-attributes -Wno-maybe-uninitialized
CPPFLAGS = -D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L

SOURCES = $(wildcard ./src/*.c)
OBJS = $(SOURCES:.c=.o)
LIBNAME = libbdiff

all: $(OBJS)
	mkdir -p libbz2
	ar -x $(shell find /lib /usr/lib /usr/local/lib -name libbz2.a 2> /dev/null)
	mv ./*.o libbz2
	ar -cq $(LIBNAME).a $(OBJS) ./libbz2/*.o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@ $(LDFLAGS)

tests: all
	gcc ./src/tests/hw1.c -o ./hw1
	gcc ./src/tests/hw2.c -o ./hw2
	mkdir -p bin
	gcc ./src/tests/bdiff_test.c -L. -lbdiff -o ./bin/bdiff_test
	gcc ./src/tests/bpatch_test.c -L. -lbdiff -o ./bin/bpatch_test
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
	rm -rf ./libbz2