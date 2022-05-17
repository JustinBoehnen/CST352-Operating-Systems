/******************************************************************************\
    Author:     Phil Howard
    Modifier:   Justin Boehnen
    Filename:   sched.c
    Modified:   5/12/2022 - Began Lab, passed tests 1 and 2
                5/13/2022 - Modified yield function to pass remaining tests
                5/14/2022 - Paranoid Error checking and extensive testing
                5/16/2022 - Retested code and added style guide requirements
\******************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sched.h"
#include "ts_queue.h"

// use a 1M stack
#define STACK_SIZE (1024 * 1024)

// list of detached threads
typedef struct detached_threads_s
{
    int thread_id;
    struct detached_thread_s *next;
} detached_threads;

// list of all threads
typedef struct thread_list_s
{
    thread_t *thread;
    int is_detached;
    struct thread_list_s *next;
} thread_list_t;

// Pointer to currently running thread
static thread_t *Current_Thread;
static thread_list_t *All_Threads;
static queue_t Ready_Queue;
static thread_t *Detatch_Pointer;

static int add_thread(thread_t *);
static int remove_thread(unsigned long);
static thread_list_t *get_thread_by_id(unsigned long);
static void fatal_error(int, const char *);
static int destroy_thread(unsigned long);

/******************************************************************************\
    Purpose:        This function can be used for adding debug statements to
                    your code. Debug statements can then be turned on/off with
                    the Do_Debug flag.
    Precondition:   None
    Postcondition:  Text printed to stderr if Do_Debug is true
\******************************************************************************/
static int Do_Debug = 0;
static void debug_print(const char *fmt, ...)
{
    if (!Do_Debug) return;

    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);
    va_end(args);
}

/******************************************************************************\
    Purpose:        This function can be used for adding debug statements to
                    your code. Debug statements can then be turned on/off with
                    the Do_Debug flag.
    Precondition:   None
    Postcondition:  Text printed to stderr if Do_Debug is true
\******************************************************************************/
void mythread_init()
{
    // Create TCB for whoever is initiating me
    Current_Thread = (thread_t *)malloc(sizeof(thread_t));
    if (Current_Thread == NULL)
        fatal_error(1, "Error mallocing memory in mythread_init()");

    Ready_Queue = Q_Init();
    if (Ready_Queue == NULL)
        fatal_error(1, "Unable to initialize ready queue in mythread_init()");

    memset(Current_Thread, 0, sizeof(thread_t));

    Current_Thread->state = STATE_RUNNING;
    Current_Thread->thread_id = 0;
    Current_Thread->sp = 0;
    Current_Thread->fp = 0;
    Current_Thread->stack = NULL;
    Current_Thread->start_func = NULL;
    Current_Thread->arg = NULL;
    Current_Thread->return_value = NULL;
    Current_Thread->who_called = NULL;

    Detatch_Pointer = NULL;
}

/******************************************************************************\
    Purpose:        This function gets called on thread startup. It calls the
                    user supplied function and then exits the thread.
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
                    Current_Thread points to a newly created TCB
    Postcondition:  User's thread has completed
\******************************************************************************/
static void start_thread_func()
{
    void *result;
    result = Current_Thread->start_func(Current_Thread->arg);

    mythread_exit(result);
}

/******************************************************************************\
    Purpose:        This function will create a new user thread func is the
                    function to run in the thread. Arg is the argument to be
                    passed to the function
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
    Postcondition:  The user's thread has been created
    Return:         The thread_id of the newly created thread is returned
                    returns zero on error
\******************************************************************************/
unsigned long mythread_create(void *(*func)(void *arg), void *arg)
{
    long *mem;
    thread_t *thread;

    // allocate the thread control block
    thread = (thread_t *)malloc(sizeof(thread_t));
    if (thread == NULL)
        fatal_error(
            1, "Failed to allocate thread control block in mythread_create()");

    if (Q_Enqueue(Ready_Queue, thread) != 0)
        fatal_error(
            1, "Failed to add thread to ready queue in mythread_create()");

    if (add_thread(thread) != 0)
        fatal_error(1,
            "Failed to add thread to list of all threads in mythread_create()");

    memset(thread, 0, sizeof(thread_t));

    // init thread
    thread->state = STATE_RUNNING;
    thread->thread_id = (unsigned long)thread;
    thread->return_value = NULL;
    thread->who_called = NULL;

    // Allocate space for the stack
    thread->stack = (long *)malloc(STACK_SIZE * sizeof(long));

    // Remember the user's thread function and argument
    thread->start_func = func;
    thread->arg = arg;

    // Initialize the stack. Pretend we made a call to yield
    // from the beginning of start_thread_func
    //
    // NOTE: The following code, even though written in C is machine specific
    mem = thread->stack;

    mem[STACK_SIZE - 1] = (long)start_thread_func;     // return addr
    mem[STACK_SIZE - 2] = (long)&mem[STACK_SIZE - 2];  // frame ptr
    thread->fp = (long)&mem[STACK_SIZE - 2];           // save FP
    thread->sp = (long)&mem[STACK_SIZE - 2];           // save SP

    // a call to yield will now run this thread
    return thread->thread_id;  // should return thread_id
}

/******************************************************************************\
    Purpose:        This function is called by threads in order to yield the
                    CPU
    NOTE:           This code is CPU and compiler option specific
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
    Postcondition:  none
\******************************************************************************/
void mythread_yield()
{
    debug_print("Thread is yielding\n");

    // save FP, SP and queue current thread in ready queue
    unsigned long reg;
    if (Current_Thread != NULL)
    {
        __asm__ volatile("movq %%rbp, %0" : "=m"(reg) : :);
        Current_Thread->fp = reg;
        __asm__ volatile("movq %%rsp, %0" : "=m"(reg) : :);
        Current_Thread->sp = reg;
    }

    // Place the current thread in the ready queue
    if (Current_Thread != NULL)
        if (Detatch_Pointer == NULL)
            if (Current_Thread->state != STATE_WAITING)
            {
                Current_Thread->state = STATE_READY;
                if (Q_Enqueue(Ready_Queue, Current_Thread) != 0)
                    fatal_error(1,
                        "Unable to enqueue current thread in mythread_yield()");
            }

    // Get the next thread
    Current_Thread = Q_Dequeue(Ready_Queue);
    if (Current_Thread == NULL)
        fatal_error(
            1, "Unable to dequeue from ready queue in mythread_yield()");

    Current_Thread->state = STATE_RUNNING;
    debug_print("Thread is running\n");

    // restore the SP, FP
    reg = Current_Thread->sp;
    __asm__ volatile("movq %0, %%rsp" : : "m"(reg) :);

    reg = Current_Thread->fp;
    __asm__ volatile("movq %0, %%rbp" : : "m"(reg) :);

    if (Detatch_Pointer != NULL) destroy_thread(Detatch_Pointer->thread_id);
    Detatch_Pointer = NULL;

    // return to next thread
    return;
}

/******************************************************************************\
    Purpose:        This function will terminate the thread that called it
    Result:         the return value of the thread
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
    Postcondition:  The user's thread has been terminated.
                    If the thread is detached, the memory for the thread will
                    be
                    cleaned up If the thread is not detached, result is
                    available for a caller to mythread_join.
\******************************************************************************/
void mythread_exit(void *result)
{
    debug_print("Thread is exiting\n");

    unsigned long id = Current_Thread->thread_id;
    thread_list_t *this = get_thread_by_id(id);
    if (this == NULL)
        fatal_error(1,
            "Unable To find current thread in list of all threads in "
            "mythread_exit()");

    this->thread->return_value = result;

    if (this->is_detached)
        Detatch_Pointer = Current_Thread;
    else
    {
        // thread will wait for user to get return
        Current_Thread->state = STATE_WAITING;
        // enqueue the parent so that eventually the
        // return value of current thread will be read
        if (Current_Thread->who_called != NULL)
            if (Q_Enqueue(Ready_Queue, Current_Thread->who_called) != 0)
                fatal_error(1,
                    "Failed to add current thread's invoker to the ready "
                    "queue");
    }

    mythread_yield();
}

/******************************************************************************\
    Purpose:        This function will wait for the specified thread to
                    terminate.
    Id:             the identifier of the thread to wait for
    Result:         location to store the thread's result.
                    if result==NULL, the thread's result will be ignored
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
                    thread_id is the ID of a thread that is not detached and
                    has not already joined.
    Postcondition:  The user's thread has finished
                    The result of the user's thread is in result (if result
                    != NULL)
\******************************************************************************/
void mythread_join(unsigned long thread_id, void **result)
{
    thread_list_t *t = get_thread_by_id(thread_id);
    if (t == NULL)
        fatal_error(
            1, "Failed to find thread with given id in mythread_join()");
    thread_t *thread = t->thread;

    // thread is still running and not ready to join
    // this is blocking
    while (thread->state != STATE_WAITING)
    {
        mythread_yield();
    }

    // set return value if user provided ptr
    if (result != NULL) *result = thread->return_value;

    // cleanup thread
    destroy_thread(thread_id);
}

/******************************************************************************\
    Purpose:        This function will mark a thread a detached meaning that
                    the thread's memory will be cleaned up immediately upon
                    thread_exit()
    Id:             the ID of the thread to be detached
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
                    thread_id has not already been detached and has not already
                    finished
    Postcondition:  The user's thread has been marked as detached
\******************************************************************************/
void mythread_detach(unsigned long thread_id)
{
    thread_list_t *thread = get_thread_by_id(thread_id);
    if (thread != NULL) thread->is_detached = 1;
}

/******************************************************************************\
    Purpose:        This function shut down the user thread system and clean-up
                    any memory used by the threading system
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
    Postcondition:  Memory has been reclaimed.
                    the behavior of any calls to mythread_ functions after this
                    call is undefined
\******************************************************************************/
void mythread_cleanup()
{
    while (All_Threads != NULL)
        if (remove_thread(All_Threads->thread->thread_id) != 0)
            fatal_error(1,
                "Failed to remove thread from list of all threads in "
                "mythread_cleanup()");

    if (Q_Close(Ready_Queue) != 0)
        fatal_error(1, "Failed to close ready queue in mythread_cleanup()");

    while (!Q_Is_Closed(Ready_Queue))
    {
        thread_t *thread = Q_Dequeue(Ready_Queue);
        if (thread == NULL)
            fatal_error(1,
                "Failed to get thread from non-closed ready queue in "
                "mythread_cleanup()");

        free(thread->stack);
        free(thread);
    }

    if (Q_Destroy(Ready_Queue) != 0)
        fatal_error(1, "Failed to destroy ready queue in mythread_cleanup()");

    free(Current_Thread);
}

/******************************************************************************\
    Purpose:        This function returns the thread_id of the currently
                    running thread.
    Precondition:   mythread_init has been called and mythread_cleanup has NOT.
    Postcondition:  The threading system is in the same state as prior to the
                    call.
    Return:         Returns the Thread ID of the current thread.
\******************************************************************************/
unsigned long mythread_self()
{
    return Current_Thread->thread_id;
}

/******************************************************************************\
    Function:       add_thread
    Purpose:        This function adds a thread to the thread lists and
                    allocates space for the tcb
    Arguments:      thread-     the thread control block of the new thread
    Return:         zero-       success
                    nonzero-    failure
    Thread Safety:  none
\******************************************************************************/
static int add_thread(thread_t *thread)
{
    thread_list_t *list_item = (thread_list_t *)malloc(sizeof(thread_list_t));
    if (list_item == NULL) return -1;

    list_item->thread = thread;
    list_item->is_detached = 0;

    if (All_Threads == NULL)
    {
        list_item->next = NULL;
        All_Threads = list_item;
    }
    else
    {
        list_item->next = All_Threads;
        All_Threads = list_item;
    }

    return 0;
}

/******************************************************************************\
    Function:       remove_thread
    Purpose:        This function removes threads from the thread lists and
                    frees the tcb
    Arguments:      thread_id-  id of the thread to remove
    Return:         zero-       success
                    nonzero-    failure
    Thread Safety:  none
\******************************************************************************/
static int remove_thread(unsigned long thread_id)
{
    thread_list_t *travel = All_Threads;
    thread_list_t *trail = NULL;
    while (travel != NULL)
    {
        if (travel->thread->thread_id == thread_id)
        {
            if (trail == NULL)
                All_Threads = travel->next;
            else
                trail->next = travel->next;

            free(travel);

            return 0;  // short-circuit and return to save compute power
        }
        else
        {
            trail = travel;
            travel = travel->next;
        }
    }

    return -1;  // failed to remove thread
}

/******************************************************************************\
    Function:       get_thread_by_id
    Purpose:        This function fetches a thread from the thread list
    Arguments:      thread_id-  the id of the thread to fetch
    Return:         thread node- pointer to the node of the thread
                    null- failed to find thread
    Thread Safety:  none
\******************************************************************************/
static thread_list_t *get_thread_by_id(unsigned long thread_id)
{
    thread_list_t *travel = All_Threads;
    while (travel != NULL)
    {
        if (travel->thread->thread_id == thread_id)
            return travel;  // short-circuit and return to save compute power
        travel = travel->next;
    }

    return NULL;  // failed to find thread
}

/******************************************************************************\
    Function:       fatal_error
    Purpose:        This function will display an error message to the user and
                    exit
    Arguments:      status- the exit status code
                    msg-    the error message
    Return:         none
    Thread Safety:  none
\******************************************************************************/
static void fatal_error(int status, const char *msg)
{
    fprintf(stderr, "\e[1;31m[Fatal Error]\e[0m %s\n", msg);
    exit(status);
}

/******************************************************************************\
    Function:       destroy_thread
    Purpose:        This function cleans up thread memory
    Arguments:      thread_id-  the id of the thread to clean up
    Return:         zero-       success
                    nonzero-    failure
    Thread Safety:  none
\******************************************************************************/
static int destroy_thread(unsigned long thread_id)
{
    thread_list_t *t = get_thread_by_id(thread_id);
    thread_t *thread = t->thread;
    if (t == NULL) return -1;

    if (remove_thread(thread_id) != 0) return -1;

    free(thread->stack);
    free(thread);

    return 0;
}