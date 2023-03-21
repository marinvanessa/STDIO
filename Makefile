CC = gcc
CFLAGS = -Wall -fPIC
LIBFLAGS = -shared

.PHONY: build clean

all: build

build: libso_stdio.so 

libso_stdio.so: so_stdio.o
	$(CC) $(LIBFLAGS) so_stdio.o -o libso_stdio.so

so_stdio.o: so_stdio.c
	$(CC) $(CFLAGS) -c so_stdio.c

clean: so_stdio.o libso_stdio.so
		rm so_stdio.o libso_stdio.so
