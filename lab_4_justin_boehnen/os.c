/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 4 Interrupts
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
                4/21/22 Began lab 4 by duplicating the source files of lab3.
                        Implemented part 1 of the lab.
                4/22/22 Finished main lab 4 requirements.
                4/25/22 modified for lab 4 resubmission based on feedback from
                lab 3.
*******************************************************************************/
#pragma feature inp
#pragma feature pio_term
#pragma startup startup
#pragma systrap systrap
#pragma interrupt isr

#include <inp_def.h>
#include <machine_def.h>
#include <pio_term.h>
#include <string.h>
#include "system_def.h"

int systrap(int code, char *str, int value, int *output_status);
int isr();
int process_interrupt();
int validate_args(int code, char *str, int value, int *output_status);
int validate_and_execute(int code, char *str, int value, int *output_status);
char *todo_out_buff;
int *g_output_status;

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
    // enable XMIT interrupt for pio_term
    char *pio_ier = PIO_T_IER;
    *pio_ier |= PIO_T_IE_XMIT;

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
                int *output_status  pointer to an integer that the os will use
                                    to flag the user that that output has either
                                    completed or failed.
return:         none
thread safety:  none
*******************************************************************************/
int systrap(int code, char *str, int value, int *output_status)
{
    validate_and_execute(code, str, value, output_status);
    asm("RTI");
}

/*******************************************************************************
function:       isr
purpose:        this is the interrupt service routine, this function is called
                when a interrupt occurs. Always RTI.
arguments:      none
return:         none
thread safety:  none
*******************************************************************************/
int isr()
{
    process_interrupt();
    asm("RTI");
}

/*******************************************************************************
function:       process_interrupt
purpose:        this is the interrupt handler, isr calls this function to handle
                interrupts because this function is allowed to maintain local
                variables.
                Currently this function is equipped to handle XMIT interrupts
                from the pio_term device, when this interrupt occurs this
                function will output the next character in the todo output
                buffer and then increment the todo pointer to the next
                character, stopping when the null char is reached and
                informing the user code with g_output_status.
arguments:      none
return:         none
thread safety:  none
*******************************************************************************/
int process_interrupt()
{
    char *pio_xdr = PIO_T_XDR;
    char *pio_iir = PIO_T_IIR;

    // pio_term device ready to transmit another character:
    // verify interrupt type and then send next character and move todo ptr
    if ((*pio_iir) & PIO_T_IID_XMIT)
    {
        if (*todo_out_buff != 0)
        {
            *pio_xdr = *todo_out_buff;
            todo_out_buff++;
        }
        else
            *g_output_status = 1;
    }
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
                int *output_status  pointer to an integer that the os will use
                                    to flag the user that that output has either
                                    completed or failed.
return:         0               success
                non-zero        failure
thread safety:  none
*******************************************************************************/
int validate_args(int code, char *str, int value, int *output_status)
{
    int bp;
    int lp;
    bp = asm2("PUSHREG", BP_REG);
    lp = asm2("PUSHREG", LP_REG);

    if (str != 0)
        str += bp;

    if (output_status != 0)
        output_status = (int *)((int)output_status + bp);

    if (code == HALT_OPCODE)
    {
        if (value != 0)
            return -1;
        if (str != 0)
            return -1;
        if (output_status != 0)
            return -1;
        return 0;
    }
    else if (code == PRINTS_OPCODE)
    {
        if (str < bp)
            return -1;
        if (str > lp)
            return -1;
        if (str + value > lp)
            return -1;
        if (value == 0)
            return -1;
        if (str == 0)
            return -1;
        if (output_status == 0)
            return -1;
        if (*output_status != 0)
            return -1;

        return 0;
    }
    else if (code == PRINTI_OPCODE)
    {
        if (str < bp)
            return -1;
        if (str > lp)
            return -1;
        if (str + value > lp)
            return -1;
        if (value == 0)
            return -1;
        if (str == 0)
            return -1;
        if (output_status == 0)
            return -1;
        if (*output_status != 0)
            return -1;
        return 0;
    }
    else if (code == GETS_OPCODE)
    {
        if (str < bp)
            return -1;
        if (str > lp)
            return -1;
        if (str + STRING_MAX > lp)
            return -1;
        if (str == 0)
            return -1;
        if (value > STRING_MAX)
            return -1;
        if (output_status != 0)
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
        if (output_status != 0)
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
                int *output_status  pointer to an integer that the os will use
                                    to flag the user that that output has either
                                    completed or failed.
return:         none
thread safety:  none
*******************************************************************************/
int validate_and_execute(int code, char *str, int value, int *output_status)
{
    if (validate_args(code, str, value, output_status) == 0)
    {
        int bp;
        int lp;
        bp = asm2("PUSHREG", BP_REG);
        lp = asm2("PUSHREG", LP_REG);

        if (str != 0)
            str += bp;

        if (output_status != 0)
            output_status = (int *)((int)output_status + bp);

        g_output_status = output_status;

        if (code == HALT_OPCODE)
        {
            asm("HALT");
        }
        else if (code == PRINTS_OPCODE || code == PRINTI_OPCODE)
        {
            char *pio_xdr = PIO_T_XDR;

            str[value] = 0;

            todo_out_buff = str;

            // send first character and todo next char
            *pio_xdr = *todo_out_buff;
            todo_out_buff++;
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
            g_output_status = -1;
    }
    else
        g_output_status = -1;
}