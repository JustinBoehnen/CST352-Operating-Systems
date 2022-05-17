# ****************************************************
# Makefile to build test code for user mode threading library
#
# Author: Phil Howard
CFLAGS=-Wall -g -O0 -pthread
OBJS = sched.o \
       ts_queue.o

EXES=test1 \
	test2 \
	test3 \
	test4 \
	test5

all: $(OBJS) $(EXES)

clean:
	rm -f $(OBJS)
	rm -f $(EXES)

%: %.c $(OBJS)
	gcc $(CFLAGS) $@.c $(OBJS) -g -o $@

