/*************************************************************
* Author:        Philip Howard
* Filename:      ts_queue.h
* Purpose:       Thread Safe Queue declaration
**************************************************************/

// The opaque type for a thread safe queue
typedef void * queue_t;

/********************************************************************** 
* Purpose: This function initializes a queue. 
* It returns an opaque pointer to the queue. The queue must be destroyed when
* no longer needed by calling Q_Destroy()
*
* Precondition: 
*     None
*
* Postcondition: 
*      Queue has been created.
*      Returns NULL on failure
*
* Thread safety: Thread safe.
*
************************************************************************/
queue_t Q_Init();

/********************************************************************** 
* Purpose: This function cleans up any memory occupied by the queue. 
* It should only be called when the queue is no longer needed, and is no
* longer being accessed by any other threads.
*
* Precondition: 
*     The queue is a valid queue that has been closed and emptied.
*     No other threads are currently accessing the queue, and none will in
*     the future.
*
* Postcondition: 
*      Queue has been destroyed; all memory has been reclaimed.
*      Returns zero on success and non-zero on failure
*
* Thread safety: 
*      Only one thread can call this function on the specified queue.
*      No other threads can be accessing the queue at the same time
*      as this function call.
*      
* Returns zero on success, non-zero on failure
*
************************************************************************/
int Q_Destroy(queue_t queue);

/********************************************************************** 
* Purpose: This markes the queue as closed. Dequeue operations are allowed
* after a queue is marked as closed, but no further enqueue operations should
* be performed.
*
* Precondition: 
*     Queue is a valid queue.
*
* Postcondition: 
*      Queue has been marked as closed.
*      Returns zero on success, non-zero on failure
*
* Thread safety: Thread safe.
*
************************************************************************/
int Q_Close(queue_t queue);

/********************************************************************** 
* Purpose: Places a new element into the queue.
*
* Precondition: 
*     Queue is a valid queue that has not been marked as closed.
*
* Postcondition: 
*      Queue contains one additional element
*      Returns zero on success, non-zero on failure
*
* Thread safety: Thread safe.
*
************************************************************************/
int Q_Enqueue(queue_t queue, char *buffer);

/********************************************************************** 
* Purpose: Removes an element from the queue
*
* Precondition: 
*     Queue is a valid queue
*
* Postcondition: 
*      If the queue was not empty, it contains one less element
*      Returns a pointer to the string stored in the queue.
*      Returns NULL if the queue is empty.
*      NOTE: This behavior will be changed as part of this lab
*
* Thread safety: Thread safe.
*
************************************************************************/
char *Q_Dequeue(queue_t queue);

/********************************************************************** 
* Purpose: Indicates whether the queue is closed
*
* Precondition: 
*     Queue is a valid queue
*
* Postcondition: 
*      Returns zero if either the queue has not been marked as closed OR
*                             the queue is not empty
*      Returns non-zero if BOTH: the queue has been marked as closed AND
*                                the queue is empty
*
* Thread safety: Thread safe.
*
************************************************************************/
int Q_Is_Closed(queue_t queue);

