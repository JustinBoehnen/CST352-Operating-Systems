/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 2 Memory Allocation
    File:       main.c
    Purpose:    Test suite for lab 2 memory management functions
    History:    4/12/22 Wrote Init and Malloc tests, some fail
                4/13/22 Wrote Free tests, all tests pass
*******************************************************************************/
/*******************************************************************************
TESTS
1. Init Tests
    a. Verify the integrity of the memory space immediately after mem_init
    b. verify mem_init resets the memory space accordingly when called after
        memory space is dirtied
2. Malloc Tests
    a. Allocate maximum number of minimum sized blocks + 1
        (only the final should return NULL)
    b. Allocate block of size 4088 (maximum size of a block)
    c. Allocate block of size 4089 (should return NULL)
    d. Allocate block of size 0 (should return NULL)
    e. Allocate block of size -1
    f. Allocate block of size -4088
    g. Random fill test:
        Run a loop that allocates blocks of random sizes
        (keeping track of what the total memory used should be)
        Ensure that malloc only returns null when the memory space should
        be full.
3. Free Tests
    a. Free block fo size 4088
    b. Free block with invalid address NULL
    c. Free block with invalid address HIGH
    d. Free block with invalid address DUPLICATE (TRYING TO FREE A FREE BLOCK)
    e. Free block with invalid address NON-HEADER
    f. Free a block in which it it forced to coallesce with a block
        ABOVE itself
    g. Free a block in which it it forced to coallesce with a block
        BELOW itself
    h. Free a block in which it it forced to coallesce with a block
        ABOVE AND BELOW itself
    i. Loop free random size blocks
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mymalloc.h"

#define DO_PRINT_MEM 0 // SET TO 1 TO FOR SANITY

int main()
{
    fprintf(stdout, "****************BEGIN TEST****************\n");

    fprintf(stdout, "\n------------TEST 1A: INIT ONLY------------\n");
    my_mem_init();
    my_validate();
    if (DO_PRINT_MEM)
        my_print_mem();

    fprintf(stdout, "\n---------TEST 1B: INIT RESETS MEM---------\n");
    my_mem_init();
    my_malloc(100);
    my_malloc(200);
    char *mem1 = my_malloc(300);
    my_malloc(400);
    char *mem2 = my_malloc(500);
    my_malloc(2000);
    my_malloc(3000);
    my_free(mem1);
    my_free(mem2);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_validate();
    my_mem_init();
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n------------TEST 2A: MAX BLOCKS-----------\n");
    my_mem_init();
    int blocks = 0;
    while (my_malloc(1) != NULL)
    {
        blocks++;
    }
    if (DO_PRINT_MEM)
        my_print_mem();
    fprintf(stdout, "%d blocks created before NULL returned\n", blocks);
    my_validate();

    fprintf(stdout, "\n----------TEST 2B: MAX SIZE BLOCK---------\n");
    my_mem_init();
    my_malloc(4088);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n-----------TEST 2C: BLOCK TOO BIG---------\n");
    my_mem_init();
    my_malloc(4089);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n-----------TEST 2D: 0 SIZE BLOCK----------\n");
    my_mem_init();
    my_malloc(0);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n----------TEST 2E: -1 SIZE BLOCK----------\n");
    my_mem_init();
    my_malloc(-1);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n---------TEST 2F: -4088 SIZE BLOCK--------\n");
    my_mem_init();
    my_malloc(-4088);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n-----------TEST 2G: RANDOM FILL-----------");
    my_mem_init();
    if (!DO_PRINT_MEM)
        fprintf(stdout, "\n");

    int iters = 3;
    for (int i = 0; i < iters; i++)
    {
        int rand_size = rand() % 500;
        while (my_malloc(rand_size) != NULL)
        {
            rand_size = rand() % 500;
        }

        if (DO_PRINT_MEM)
        {
            fprintf(stdout, "\n");
            my_print_mem();
        }

        my_validate();
        my_mem_init();
    }

    fprintf(stdout, "\n----------TEST 3A: MAX SIZE FREE----------\n");
    my_mem_init();
    char *addr0 = my_malloc(4088);
    my_free(addr0);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n+ + + + + TESTS 3B to 3E OMITTED + + + + +\n");
    // the following tests have been emitted as they now prematurely end the
    // test as an invalid/non-busy header being passed to my_free() requires an
    // exit() call per the design specifications

    /*fprintf(stdout, "\n-------TEST 3B: INVALID FREE (NULL)-------\n");
    my_mem_init();
    my_free(NULL);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n-------TEST 3C: INVALID FREE (HIGH)-------\n");
    my_mem_init();
    char *addr1 = my_malloc(100);
    my_free(addr1 + 10000);
    my_free(addr1);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n-------TEST 3D: INVALID FREE (DUPE)-------\n");
    my_mem_init();
    char *addr2 = my_malloc(500);
    my_free(addr2);
    my_free(addr2);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();

    fprintf(stdout, "\n----TEST 3E: INVALID FREE (NON-HEADER)----\n");
    my_mem_init();
    char *addr3 = my_malloc(500);
    // my_free((char *)(long int)addr3 + 9);
    my_free(addr3);
    if (DO_PRINT_MEM)
        my_print_mem();
    my_validate();*/

    fprintf(stdout, "\n---------TEST 3F: COALLESCE ABOVE---------\n");
    my_mem_init();
    char *above1 = my_malloc(256); // above
    char *merge1 = my_malloc(256);
    my_malloc(256); // below
    my_malloc(16);  // seperator between big free block 0x0

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_free(above1);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_free(merge1);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
    }

    fprintf(stdout, "\n---------TEST 3G: COALLESCE BELOW---------\n");
    my_mem_init();
    my_malloc(256); // above
    char *merge2 = my_malloc(256);
    char *below2 = my_malloc(256); // below
    my_malloc(16);                 // seperator between big free block 0x0

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_free(below2);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_free(merge2);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
    }

    fprintf(stdout, "\n--------TEST 3H: COALLESCE BETWEEN--------\n");
    my_mem_init();
    char *above3 = my_malloc(256); // above
    char *merge3 = my_malloc(256);
    char *below3 = my_malloc(256); // below
    my_malloc(16);                 // seperator between big free block 0x0

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_free(above3);
    my_free(below3);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }

    my_free(merge3);

    if (DO_PRINT_MEM)
    {
        my_print_mem();
    }

    // the following test was modified for the same reasons 3B - 3E were
    // omitted

    /*fprintf(stdout, "\n---------TEST 3I: LOOP RANDOM FREE--------\n");
    time_t t;
    srand((unsigned)time(&t));
    my_mem_init();
    int loops = 1000;
    int oversized = 0;
    for (int i = 0; i < loops; i++)
    {
        int rand_size = rand() % 10;
        if (rand_size == 0) // 10% chance to be null from malloc
        {
            oversized++;
            rand_size = 10000;
        }
        else
            rand_size = rand() % 4089;

        char *addr = my_malloc(rand_size);
        my_free(addr);
        my_validate();
    }
    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "Performed %d frees, %d NULL pointers\n", loops, oversized);

    fprintf(stdout, "\n*****************END TEST*****************\n");*/

    fprintf(stdout, "\n---------TEST 3I: LOOP RANDOM FREE--------\n");
    time_t t;
    srand((unsigned)time(&t));
    my_mem_init();
    int loops = 1000;
    for (int i = 0; i < loops; i++)
    {
        int rand_size = rand() % 4087;
        rand_size++;

        char *addr = my_malloc(rand_size);
        my_free(addr);
        my_validate();
    }
    if (DO_PRINT_MEM)
    {
        my_print_mem();
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "Performed %d frees\n", loops);

    fprintf(stdout, "\n*****************END TEST*****************\n");

    return 0;
}