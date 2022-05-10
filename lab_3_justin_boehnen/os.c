/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 3 Syscalls
    File:       os.c
    Purpose:    Emulates an OS (priviledged mode), handles trap instructions
                and runs user mode
    History:    4/19/22 Began lab after reading through the stackl machine
                        reference thoroughly and ensuring that stackl is
                        working as expected. startup, syscall, systrap, and the
                        user library were written this day, none of which were
                        100% complete. The largest issue currently is that the
                        program seems to halt after any single user function is
                        called from user.slb
                4/20/22 Prior issue fixed (I was maintainging variables in
                        systrap). Lab is seemingly complete, user and
                        priviledged more are seperated and I believe all
                        necessary syscall validation is completed.
*******************************************************************************/
#pragma feature inp
#pragma startup startup
#pragma systrap systrap

#include <inp_def.h>
#include <machine_def.h>
#include <string.h>

#define HALT_OPCODE 0
#define PRINTS_OPCODE 1
#define PRINTI_OPCODE 2
#define GETS_OPCODE 3
#define GETI_OPCODE 4
#define STRING_MAX 256

int systrap(int code, char *str, int value);
int validate_args(int code, char *str, int value);
void validate_and_execute(int code, char *str, int value);

/*******************************************************************************
function:       startup
purpose:        Entry point of Stackl/the os. pepares memory and runs user.slb.
                This entire function was written by Phil.
arguments:      none
return:         none
thread safety:  none
*******************************************************************************/
int startup()
{
    io_blk_t io_blk;
    int bp;
    int high_mem;
    int *stack_size;
    // Set the BP leaving enough room for our stack
    bp = asm2("PUSHREG", SP_REG);
    bp += 64; // size of stack
    asm2("POPREG", BP_REG, bp);
    // Load user.slb into memory
    io_blk.op = INP_EXEC_CALL;
    io_blk.param1 = "user.slb"; // program to run
    io_blk.param2 = 0;
    asm("INP", &io_blk);
    while (io_blk.op >= 0) // wait for EXEC to finish
    {
    }
    // Set the LP leaving room for stack
    stack_size = io_blk.param2; // EXEC places requested stack size here
    high_mem = io_blk.param2 + *stack_size;
    asm2("POPREG", LP_REG, high_mem);
    // Set SP and FP
    // NOTE: FP must be set only AFTER all variable references
    // NOTE: SP must be set LAST (otherwise you're using the user's stack)
    high_mem = io_blk.param2 + 4 - bp;
    asm("DUP", high_mem);
    asm2("POPREG", FP_REG);
    asm2("POPREG", SP_REG);
    // Execute user.slb
    asm2("JMPUSER", 8);

    asm("HALT");
}

/*******************************************************************************
function:       systrap
purpose:        this function handles the 'TRAP' instruction, as local variables
                are not allowed in this function (learned the hard way), this
                function calls others functions that validates and executes the
                provided syscall.
arguments:      int code        the opcode of the instruction to execute.
                char *str       a string buffer for applicable opcodes.
                int value       an integer value for providing buffer lengths
                                or captured integer values where applicable.
return:         none
thread safety:  none
*******************************************************************************/
int systrap(int code, char *str, int value)
{
    validate_and_execute(code, str, value);
    asm("RTI");
}

/*******************************************************************************
function:       validate_args
purpose:        This function validates that a valid opcode was provided to
                systrap and that all arguments needed by that opcode are
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
    int bp;
    int lp;
    bp = asm2("PUSHREG", BP_REG);
    lp = asm2("PUSHREG", LP_REG);

    if (str != 0)
        str += bp;

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
        if (str < bp)
            return -1;
        if (str + value > lp)
            return -1;
        if (value == 0)
            return -1;
        if (str == 0)
            return -1;
        return 0;
    }
    else if (code == PRINTI_OPCODE)
    {
        if (str < bp)
            return -1;
        if (str + value > lp)
            return -1;
        if (value == 0)
            return -1;
        if (str == 0)
            return -1;
        return 0;
    }
    else if (code == GETS_OPCODE)
    {
        if (str < bp)
            return -1;
        if (str == 0)
            return -1;
        if (value > STRING_MAX)
            return -1;
        return 0;
    }
    else if (code == GETI_OPCODE)
    {
        if (str < bp)
            return -1;
        if (str == 0)
            return -1;
        if (value != 0)
            return -1;
        return 0;
    }
    else
        return -1;
}

/*******************************************************************************
function:       validate_and_execute
purpose:        This function executes operations after validating arguments.
arguments:      int code        the opcode of the instruction to execute.
                char *str       a string buffer for applicable opcodes.
                int value       an integer value for providing buffer lengths
                                or captured integer values where applicable.
return:         none
thread safety:  none
*******************************************************************************/
void validate_and_execute(int code, char *str, int value)
{
    if (validate_args(code, str, value) == 0)
    {
        int bp;
        int lp;
        bp = asm2("PUSHREG", BP_REG);
        lp = asm2("PUSHREG", LP_REG);

        str += bp;

        if (code == HALT_OPCODE)
        {
            asm("HALT");
        }
        else if (code == PRINTS_OPCODE)
        {
            str[value] = 0;
            asm("OUTS", str);
        }
        else if (code == PRINTI_OPCODE)
        {
            str[value] = 0;
            asm("OUTS", str);
        }
        else if (code == GETS_OPCODE)
        {
            io_blk_t io_blk;
            io_blk.op = INP_GETL_CALL;
            io_blk.param1 = (int)str;
            asm("INP", &io_blk);
            while (io_blk.op >= 0)
            {
            }
        }
        else if (code == GETI_OPCODE)
        {
            io_blk_t io_blk;
            io_blk.op = INP_GETI_CALL;
            io_blk.param1 = (int)(int *)str;
            asm("INP", &io_blk);
            while (io_blk.op >= 0)
            {
            }
        }
        else
        {
            asm("OUTS", "invalid code\n");
        }
    }
    else
    {
        asm("OUTS", "Systrap validation failed\n");
    }
}