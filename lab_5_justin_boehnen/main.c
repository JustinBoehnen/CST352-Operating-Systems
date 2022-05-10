/*************************************************************
Author:         Philip Howard
Modifier:       Justin Boehnen
Filename:       main.c
Date Created:   5/4/2016
Modifications:  5/5/2022    Began lab, replaced pseudo-code with actual
                            multi-threading code, verified that there are
                            no memory leaks and that threads are created and
                            waited for correctly.
                5/7/2022    Cleaned up code and documented.
                5/8/2022    Testing
**************************************************************

Lab/Assignment: Lab 5 - Producer Consumer

Overview:
    This program is a producer-consumer application.
    The producers read text files and send the text, one line at a time,
    to the consumers. The consumers print the text along with a
    thread-id to identify which thread printed each line.

Input:
    Command line arguments are used to specify the number of consumers and
    the filenames of the files to be read by producers. One producer will
    be started for each filename.

Output:
    The lines of text from the collected input files.
    Note: If there are multiple consumers, there is no guarantee as to the
    order of the output lines.

NOTE: this is PSEUDO-CODE, and it will have to be turned into real code
to complete this lab.
************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "prod_cons.h"
#include "ts_queue.h"

// used to pass args to producer thread
typedef struct prod_args_s
{
    queue_t queue;
    char *filename;
} prod_args_t;

static void *producer_routine(void *);
static void *consumer_routine(void *);
static void fatal_error(int, char *);

int main(int argc, char **argv)
{
    int n_consumers;
    int n_producers = (argc - 2);
    queue_t queue;

    if (argc < 3)
        fatal_error(
            1, "Must specify the number of threads and at least one file");

    n_consumers = atoi(argv[1]);
    if (n_consumers < 1 || n_consumers > 20)
        fatal_error(
            1, "Number of threads must be greater than 0 and less than 20");

    queue = Q_Init();
    if (queue == NULL) fatal_error(1, "Failed to initialize queue!");

    // list of producer threads (used for thread cleanup later)
    pthread_t *producers = malloc(sizeof(pthread_t) * n_producers);
    if (producers == NULL)
        fatal_error(1, "Failed to allocate space for producer threads!");

    // list of consumer threads (used for thread cleanup later)
    pthread_t *consumers = malloc(sizeof(pthread_t) * n_consumers);
    if (consumers == NULL)
        fatal_error(1, "Failed to allocate space for consumer threads!");

    // start consumers on job (consumer), num of threads specificed by arg[1]
    for (int ii = 0; ii < n_consumers; ii++)
    {
        if (pthread_create(&consumers[ii], NULL, &consumer_routine, queue) != 0)
            fatal_error(1, "Failed to create a queue consumer thread!");
    }

    // list of structs containing the arguments passed to the producer routine.
    prod_args_t *prod_args = malloc(sizeof(prod_args_t) * n_producers);
    if (prod_args == NULL)
        fatal_error(1, "Failed to allocate space for producer args!");

    // populate prod_args struct with appropriate arguments and then start
    // producers on job (produce). 1 prod thread per input file
    for (int ii = 0; (ii + 2) < argc; ii++)
    {
        prod_args[ii].queue = queue;
        prod_args[ii].filename = argv[ii + 2];

        // clang-format off
        if(pthread_create(
            &producers[ii], NULL, &producer_routine, &(prod_args[ii])) != 0)
            fatal_error(1, "Failed to create a file producer thread!");
        // clang-format on
    }

    // wait for all producers to return (all input files have been read)
    for (int ii = 0; ii < n_producers; ii++)
    {
        if (pthread_join(producers[ii], NULL) != 0)
            fatal_error(1, "Failed to wait for a producer thread to finish!");
    }

    // since input to the queue is complete we may close the queue
    if (Q_Close(queue) != 0)
        fatal_error(1, "Failed to close queue after producers finished!");

    // wait for consumers to empty the queue
    for (int ii = 0; ii < n_consumers; ii++)
    {
        if (pthread_join(consumers[ii], NULL) != 0)
            fatal_error(1, "Failed to wait for a consumer thread to finish!");
    }

    // verify that producers and consumers did their job correctly by ensuring
    // that the queue is both empty and closed (completed).
    if (Q_Is_Closed(queue) == 0)
        fatal_error(
            1, "Queue was not empty or closed after consumers finished!");

    // since the queue is complete we may discard it
    if (Q_Destroy(queue) != 0)
        fatal_error(1, "Failed to destroy queue after consumers finished!");

    // memory cleanup
    free(producers);
    free(consumers);
    free(prod_args);

    return 0;
}

/*******************************************************************************
function:       producer_routine
purpose:        This function is the routine passed to producer threads.
                It is a wrapper for the producer function and is required
                as to meet the requirements of a routine passed to a thread
                (void* return type and void* argument type)
arguments:      void * -> prod_args_t* _prod_args
                    a struct for passing two arguments to the producer thread
                    routine. specifically: queue_t queue (the queue to enqueue
                    file line onto), and char *filename (the name of the file
                    to read lines from)
return:         NULL
thread safety:  MT-Safe
*******************************************************************************/
static void *producer_routine(void *_prod_args)
{
    prod_args_t *prod_args = (prod_args_t *)_prod_args;

    return producer(prod_args->queue, prod_args->filename);
}

/*******************************************************************************
function:       consumer_routine
purpose:        This function is the routine passed to consumer threads.
                It is a wrapper for the consumer function and is required
                as to meet the requirements of a routine passed to a thread
                (void* return type and void* argument type)
arguments:      void * -> queue_t* _prod_args
                    the queue to dequeue files from
return:         NULL
thread safety:  MT-Safe
*******************************************************************************/
static void *consumer_routine(void *_queue)
{
    queue_t queue = (queue_t)_queue;

    return consumer(queue);
}

/*******************************************************************************
function:       fatal_error
purpose:        Small helper function designed to reduce some of the redundancy
                of error handling
arguments:      void * -> prod_args_t* _prod_args
                    a struct for passing two arguments to the producer thread
                    routine. specifically: queue_t queue (the queue to enqueue
                    file line onto), and char *filename (the name of the file
                    to read lines from)
return:         none
thread safety:  MT-Safe
*******************************************************************************/
static void fatal_error(int status, char *msg)
{
    fprintf(stderr, "\e[1;31m[Fatal Error]\e[0m %s\n", msg);
    exit(status);
}