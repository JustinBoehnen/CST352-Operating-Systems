//************************************************
// Author: Phil Howard
//
// Purpose:
//    Initial test of user mode threading system
//    Tests the ability to start and switch to a thread.
//
#include <stdio.h>
#include <stdlib.h>

#include "sched.h"

// *****************************************
// function we are going to run as a thread
//
// Thread Safety: None
// Returns: does not return. Calls exit()
//
void *thread_test(void *arg)
{
    printf("Thread is running!\n");
    exit(0);

    return NULL;
}

// *****************************************
// Main function which starts a thread
//
// Thread Safety: none
// Returns zero on success 1 on failure
int main()
{
    mythread_init();

    mythread_create(thread_test, 0);

    mythread_yield();

    printf("main is going away\n");

    return 1;
}
