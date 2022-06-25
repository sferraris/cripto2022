CCOMP = gcc
CFLAGS = -pedantic -Wall -std=c99 -g -lcrypto

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES: %.c = %.o)
OUTPUT := stegobmp

all:
	${CCOMP} ${SOURCES} ${CFLAGS} -o ${OUTPUT}

clean:
	rm -rvf *.o ${OUTPUT}
