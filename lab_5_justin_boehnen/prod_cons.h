/*************************************************************
* Author:        Philip Howard
* Filename:      prod_cons.h
* Purpose:       Declaration of thread-safe producer/consumer
**************************************************************/

#include "ts_queue.h"

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
*
* Thread safety: Thread safe
************************************************************************/
void *producer(queue_t queue, char *filename);

/********************************************************************** 
* Purpose: Acts as a consumer that reads from a queue and writes the lines 
* pulled from the queue to stdout.
*
*
* Precondition: 
*     Queue is a valid queue
*
* Postcondition: 
*      The queue has been closed and emptied.
*
* Thread safety: Thread safe
************************************************************************/
void *consumer(queue_t queue);

