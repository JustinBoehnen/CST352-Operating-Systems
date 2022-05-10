/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 4 Interrupts
    File:       system_def.h
    Purpose:    Defines constants for syscall op codes and other system
                variables.
    History:    4/25/22 created for lab 4 resubmission based on feedback from
                lab 3.
*******************************************************************************/
const int HALT_OPCODE = 0;
const int PRINTS_OPCODE = 1;
const int PRINTI_OPCODE = 2;
const int GETS_OPCODE = 3;
const int GETI_OPCODE = 4;

const int STRING_MAX = 256;