//***************************************************
// Test file for user mode threading library.
// Tests ability to switch between threads
//
// Author: Phil Howard
//
#include <stdio.h>
#include <stdlib.h>

#include "sched.h"

//***************************************************
// function we are going to run as a thread
//
// preconditions:
//    mythread_init has already been called
//    function running inside a thread
//
// post condition:
//    program is terminated with exit() call
//
// Returns: never returns. Calls exit()
void *thread_test(void *arg)
{
    long count = (long)arg;

    while (--count)
    {
        printf("Counting down: %ld\n", count);
        mythread_yield();
    }

    printf("Thread is exiting!\n");
    exit(0);

    return NULL;
}

//***************************************************
// main program for test
//
// Thread safety: none
// returns 0 on success, 1 on failure
int main()
{
    mythread_init();

    mythread_create(thread_test, (void *)5);
    mythread_create(thread_test, (void *)10);

    while (1)
    {
        mythread_yield();
    }

    printf("main is going away\n");

    return 1;
}
