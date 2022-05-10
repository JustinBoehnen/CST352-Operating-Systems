/*************************************************************
Author:         Philip Howard
Modifier:       Justin Boehnen
Filename:       ts_queue.c
Purpose:        Implementation of a thread safe queue
Modifications:  5/5/2022    Added mutex locking to ensure basic thread
                            safety.
                5/6/2022    Added a semaphore to eliminate consumer
                            busy-waiting and become thread-efficient.
                            Ran multitude of tests with both testfile
                            and validate.py (largest test n=100000)
                            "found 27409 prod interleavings, 5976 cons
                            interleavings, and 100000 records".
                5/7/2022    Cleaned up code and documented.
                5/8/2022    Testing
                5/10/2022   Made sure that all waiting consumers are released
                            when the queue is closed!
**************************************************************/
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "ts_queue.h"

// an element on the thread safe queue
typedef struct item_s
{
    void *data;
    struct item_s *next;
} item_t;

// Type of the queue (visible version of opaque type)
typedef struct
{
    int closed;
    item_t *first;
    item_t *last;
    pthread_mutex_t lock;
    sem_t sem;
    int waiting;
} my_queue_t;

/**********************************************************************
Purpose: This function initializes a queue.

See details in the header file
************************************************************************/
queue_t Q_Init()
{
    my_queue_t *queue = (my_queue_t *)malloc(sizeof(my_queue_t));

    if (queue == NULL) return NULL;

    queue->first = NULL;
    queue->last = NULL;
    queue->closed = 0;
    queue->waiting = 0;

    if (pthread_mutex_init(&(queue->lock), NULL) != 0)
    {
        free(queue);
        return NULL;
    }

    if (sem_init(&(queue->sem), 0, 0) == -1)
    {
        free(queue);
        return NULL;
    }

    return (queue_t)queue;
}

/**********************************************************************
Purpose: This function cleans up any memory occupied by the queue.
It should only be called when the queue is no longer needed, and is no
longer being accessed by any other threads.

See details in the header file
************************************************************************/
int Q_Destroy(queue_t q)
{
    my_queue_t *queue = (my_queue_t *)q;

    if (pthread_mutex_destroy(&(queue->lock)) != 0) return 1;

    if (sem_destroy(&(queue->sem)) != 0) return 1;

    free(queue);

    return 0;
}

/**********************************************************************
Purpose: This marks the queue as closed. Dequeue operations are allowed
after a queue is marked as closed, but no further enqueue operations should
be performed.

See details in the header file
************************************************************************/
int Q_Close(queue_t q)
{
    my_queue_t *queue = (my_queue_t *)q;

    queue->closed = 1;

    while (queue->waiting > 0)
        if (sem_post(&(queue->sem)) != 0) return 1;

    return 0;
}

/**********************************************************************
Purpose: Places a new element into the queue.

See details in the header file
************************************************************************/
int Q_Enqueue(queue_t q, char *buffer)
{
    my_queue_t *queue = (my_queue_t *)q;

    if (pthread_mutex_lock(&(queue->lock)) != 0) return 1;

    item_t *item = (item_t *)malloc(sizeof(item_t));
    if (item == NULL) return 1;

    item->data = buffer;
    item->next = NULL;

    if (queue->first == NULL)
    {
        queue->first = item;
        queue->last = item;
    }
    else
    {
        queue->last->next = item;
        queue->last = item;
    }

    if (sem_post(&(queue->sem)) != 0) return 1;

    if (pthread_mutex_unlock(&(queue->lock)) != 0) return 1;

    return 0;
}

/**********************************************************************
Purpose: Removes an element from the queue

See details in the header file
************************************************************************/
char *Q_Dequeue(queue_t q)
{
    my_queue_t *queue = (my_queue_t *)q;

    if (pthread_mutex_lock(&(queue->lock)) != 0) return NULL;
    queue->waiting++;
    if (pthread_mutex_unlock(&(queue->lock)) != 0) return NULL;

    if (sem_wait(&(queue->sem)) != 0) return NULL;

    if (pthread_mutex_lock(&(queue->lock)) != 0) return NULL;
    queue->waiting--;

    char *buffer = NULL;
    item_t *item;

    if (queue->first != NULL)
    {
        item = queue->first;
        queue->first = item->next;
        if (queue->first == NULL) queue->last = NULL;

        buffer = (char *)item->data;
        free(item);
    }

    if (pthread_mutex_unlock(&(queue->lock)) != 0) return NULL;

    return buffer;
}

/**********************************************************************
Purpose: Indicates whether the queue is open

See details in the header file
************************************************************************/
int Q_Is_Closed(queue_t q)
{
    my_queue_t *queue = (my_queue_t *)q;
    int result = 0;

    if (pthread_mutex_lock(&(queue->lock)) != 0)
    {}

    if (queue->closed && queue->first == NULL) result = 1;

    if (pthread_mutex_unlock(&(queue->lock)) != 0)
    {}

    return result;
}