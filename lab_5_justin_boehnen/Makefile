CC = gcc
CFLAGS = -Wall -g -O0 -pthread

OBJS = prod_cons.o \
       ts_queue.o \
       main.o \

all: mytest genfile testfile

clean:
	rm -rf mytest
	rm -rf genfile
	rm -rf testfile
	rm -rf $(OBJS)

mytest: $(OBJS)
	gcc $(OBJS) -o mytest -pthread
