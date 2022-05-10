CC=gcc
CFLAGS=-Wall -g -c -O0 -std=c99 -o
OBJS = mymalloc.o \
       main.o

all: main

clean:
	rm -f *.o
	rm -f main

mymalloc.o: mymalloc.h mymalloc.c
	gcc $(CFLAGS) mymalloc.o mymalloc.c

main.o: main.c
	gcc $(CFLAGS) main.o main.c

main: $(OBJS)
	gcc $(OBJS) -o main