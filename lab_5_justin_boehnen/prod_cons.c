/*************************************************************
* Author:        Philip Howard
* Filename:      prod_cons.c
* Purpose:     Implementation of a thread-safe producer/consumer
* Modifications: 
**************************************************************/
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "prod_cons.h"
#include "ts_queue.h"

#define BUFF_SIZE 256

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

/********************************************************************** 
* Purpose: Acts as a producer that reads from a file and places the data
* from the file into the queue one line at a time.
*
* Precondition: 
*     Queue is a valid queue
*     filename is a valid filename for which the process has read access
*
* Postcondition: 
*      The file has been read and the lines are places into the queue
************************************************************************/
void *producer(queue_t queue, char *filename)
{
    FILE *input;
    char *buff;

    input = fopen(filename, "r");
    if (input == NULL) return (void *)1L;

    // Need to malloc each buffer so each line is in a separate buffer.
    // Consumer needs to free the buffer
    buff = (char *)malloc(BUFF_SIZE);
    assert(buff != NULL);

    while (fgets(buff, BUFF_SIZE, input) != NULL)
    {
        // only queue non-empty lines
        if (strlen(buff) != strspn(buff, " \t\n"))
        {
            // eliminate the newline
            if (buff[ strlen(buff)-1 ] =='\n') buff[ strlen(buff)-1 ] = 0;
            Q_Enqueue(queue, buff);
            buff = (char *)malloc(BUFF_SIZE);
            assert(buff != NULL);
        }
    }

    // malloc'd one extra buffer
    free(buff);
    fclose(input);

    return NULL;
}

/********************************************************************** 
* Purpose: Acts as a consumer that reads from a queue and writes the lines 
* pulled from the queue to stdout.
*
* Precondition: 
*     Queue is a valid queue
*
* Postcondition: 
*      The file has been closed and emptied.
************************************************************************/
void *consumer(queue_t queue)
{
    char *buff;

    while (!Q_Is_Closed(queue))
    {
        buff = Q_Dequeue(queue);
        if (buff != NULL) 
        {
            pthread_mutex_lock(&print_lock);
            printf("%ld %s\n", pthread_self(), buff);
            fflush(stdout);
            pthread_mutex_unlock(&print_lock);
            free(buff);
        }
    }

    return NULL;
}

