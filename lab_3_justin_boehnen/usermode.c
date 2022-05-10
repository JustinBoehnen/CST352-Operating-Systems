/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 3 Syscalls
    File:       usermode.c
    Purpose:    Implements usermode.h library functions, this is the public
                interface that the user uses to interact with the OS
    History:    4/19/22 Wrote all function definitions.
                4/20/22 Fixed all function definitions.
*******************************************************************************/
#pragma feature inp

#include <string.h>
#include <syscodes.h>
#include <machine_def.h>
#include "usermode.h"

#define HALT_OPCODE 0
#define PRINTS_OPCODE 1
#define PRINTI_OPCODE 2
#define GETS_OPCODE 3
#define GETI_OPCODE 4
#define STRING_MAX 256

int syscall(int code, char *str, int value);
int validate_args(int code, char *str, int value);

/*******************************************************************************
function:       prints
purpose:        Prints the string to stdout and returns 0 indicating success
                or a non-zero value indicating failure.
arguments:      char *string            address of string buffer to print
return:         0                       success
                non-zero                failure
thread safety:  none
*******************************************************************************/
int prints(char *string)
{
    return syscall(PRINTS_OPCODE, string, strlen(string));
}

/*******************************************************************************
function:       printi
purpose:        Prints the integer value to stdout and returns 0 indicating
                success or a non-zero value indicating failure.
arguments:      int value               integer to print
return:         0                       success
                non-zero                failure
thread safety:  none
*******************************************************************************/
int printi(int value)
{
    char buffer[16];
    if (itostr(value, buffer) == &buffer)
        return syscall(PRINTI_OPCODE, buffer, strlen(buffer));
    else
        return -1;
}

/*******************************************************************************
function:       geti
purpose:        Reads an integer value from stdin and returns the integer.
                Note: this function cannot return an error.
arguments:      none
return:         integer read from stdin
thread safety:  none
*******************************************************************************/
int geti()
{
    int *value;
    syscall(GETI_OPCODE, (char *)value, 0);

    return *value;
}

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
int gets(char *buff)
{
    return syscall(GETS_OPCODE, buff, STRING_MAX);
}

/*******************************************************************************
function:       halt
purpose:        This function should cause the CPU to halt
                (thus terminating the stackl interpreter).
arguments:      none
return:         anything                failure
thread safety:  none
*******************************************************************************/
int halt()
{
    return syscall(HALT_OPCODE, 0, 0);
}

/*******************************************************************************
function:       syscall
purpose:        single syscall function for all user functions, this functions
                executed the 'trap' instruction.
arguments:      int code        the opcode of the instruction to execute.
                char *str       a string buffer for applicable opcodes.
                int value       an integer value for providing buffer lengths
                                or captured integer values where applicable.
return:         0               success
                non-zero        failure
thread safety:  none
*******************************************************************************/
int syscall(int code, char *str, int value)
{
    if (validate_args(code, str, value) != 0)
        return -1;

    asm("TRAP");

    return 0;
}

/*******************************************************************************
function:       validate_args
purpose:        This function validates that a valid opcode was provided to
                syscall and that all arguments needed by that opcode are
                are provided.
arguments:      int code        the opcode of the instruction to execute.
                char *str       a string buffer for applicable opcodes.
                int value       an integer value for providing buffer lengths
                                or captured integer values where applicable.
return:         0               success
                non-zero        failure
thread safety:  none
*******************************************************************************/
int validate_args(int code, char *str, int value)
{
    if (code == HALT_OPCODE)
    {
        if (value != 0)
            return -1;
        if (str != 0)
            return -1;
        return 0;
    }
    else if (code == PRINTS_OPCODE)
    {
        if (value == 0)
            return -1;
        if (str == 0)
            return -1;
        return 0;
    }
    else if (code == PRINTI_OPCODE)
    {
        if (value == 0)
            return -1;
        if (str == 0)
            return -1;
        return 0;
    }
    else if (code == GETS_OPCODE)
    {
        if (str == 0)
            return -1;
        if (value > STRING_MAX)
            return -1;
        return 0;
    }
    else if (code == GETI_OPCODE)
    {
        if (str == 0)
            return -1;
        if (value != 0)
            return -1;
        return 0;
    }
    else
        return -1;
}