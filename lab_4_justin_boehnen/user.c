/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 4 Interrupts
    File:       user.c
    Purpose:    Emulates an OS User (User mode). Runs tests on the OS and
                verifies that all syscalls operate as expected.
    History:    4/19/22 Esnured that functions can be run from this file and
                        that the OS is properly loading this file
                4/20/22 Tests written for all syscalls.
                4/21/22 Modified test for lab 4, output only.
*******************************************************************************/
#include <machine_def.h>
#include "usermode.h"

int main()
{
    prints("This is\n");
    prints("A\n");
    printi(1234567890);
    prints("\n");
    prints("Test\n");
    prints("");

    halt();
    prints("This will not print\n");

    return 0;
}