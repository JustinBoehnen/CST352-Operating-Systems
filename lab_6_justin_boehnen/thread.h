const int STATE_RUNNING = 0;
const int STATE_READY = 1;
const int STATE_WAITING = 2;

//******************************************
// Definition of Thread Control Block
typedef struct thread_s
{
    int state;
    unsigned long thread_id;         // Unique thread identifier
    unsigned long sp;                // Stack pointer for thread
    unsigned long fp;                // Frame pointer for thread
    long *stack;                     // ptr to block of memory used for stack
    void *(*start_func)(void *arg);  // function to run in thread
    void *arg;                       // arg to be passed to start_func
    void *return_value;              // pointer to store start_func result
    struct thread_s *who_called;     // pointer to the thread that called this 1
} thread_t;