CC=$(shell which clang)
CFLAGS=-ansi -g
CFLAGS+=-fsanitize=address,undefined
LDFLAGS=$(shell pkg-config --libs-only-L readline)
LDLIBS=-lreadline
SRC=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)

.PHONY: all clean

all: clean pseudo

pseudo: $(SRC) $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(SRC) -o build/$@

clean:
	rm -rf build

run:
	ASAN_OPTIONS=detect_leaks=1 ./build/pseudo test.txt
