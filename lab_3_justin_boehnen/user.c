/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 3 Syscalls
    File:       user.c
    Purpose:    Emulates an OS User (User mode). Runs tests on the OS and
                verifies that all syscalls operate as expected.
    History:    4/19/22 Esnured that functions can be run from this file and
                        that the OS is properly loading this file
                4/20/22 Tests written for all syscalls.
*******************************************************************************/
#include <machine_def.h>
#include "usermode.h"

int main()
{
    char t1buff[256];
    prints("TEST 1: GETS\n");
    prints("inp: ");
    gets(t1buff);
    prints("got: ");
    prints(t1buff);

    prints("\n");

    prints("TEST 2: GETI AND PRINTI\n");
    prints("inp: ");
    int t2got = geti();
    prints("got: ");
    printi(t2got);

    prints("\n");

    halt();

    prints("This will not print\n");

    return 0;
}