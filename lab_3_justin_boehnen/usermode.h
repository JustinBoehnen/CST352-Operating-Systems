/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 3 Syscalls
    File:       usermode.h
    Purpose:    Declares a user-OS interface to be implemeted.
    History:    4/19/22 Wrote all function declarations.
*******************************************************************************/
/*******************************************************************************
function:       prints
purpose:        Prints the string to stdout and returns 0 indicating success
                or a non-zero value indicating failure.
arguments:      char *string            address of string buffer to print
return:         0                       success
                non-zero                failure
thread safety:  none
*******************************************************************************/
int prints(char *string);

/*******************************************************************************
function:       printi
purpose:        Prints the integer value to stdout and returns 0 indicating
                success or a non-zero value indicating failure.
arguments:      int value               integer to print
return:         0                       success
                non-zero                failure
thread safety:  none
*******************************************************************************/
int printi(int value);

/*******************************************************************************
function:       geti
purpose:        Reads an integer value from stdin and returns the integer.
                Note: this function cannot return an error.
arguments:      none
return:         integer read from stdin
thread safety:  none
*******************************************************************************/
int geti();

/*******************************************************************************
function:       gets
purpose:        Reads a line of text from stdin. This function should have
                behavior similar to the standard C fgets(buff, 256, stdin)
                function. Returns zero on success and non-zero on failure.
                You can assume a maximum buffer size of 256.
arguments:      char *buff              address of string buffer to store
                                        string read from stdin in
return:         0                       success
                non-zero                failure
thread safety:  none
*******************************************************************************/
int gets(char *buff);

/*******************************************************************************
function:       halt
purpose:        This function should cause the CPU to halt
                (thus terminating the stackl interpreter).
arguments:      none
return:         anything                failure
thread safety:  none
*******************************************************************************/
int halt();