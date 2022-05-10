/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 2 Memory Allocation
    File:       mymalloc.c
    Purpose:    Implements the functions in mymalloc.h
    History:    4/08/22 Wrote all code without testing anything, past 12
                        hours spent debugging (hail gdb).
                4/11/22 Everything is broken, commence existential crisis.
                4/12/22 Core functionality wrapping up: Init, Malloc, Free, and
                        Print working, still need to complete paranoid error
                        checking everywhere. (Also wrote a bunch of tests)
                4/13/22 Paranoid error checking everywhere, Wrote way more
                        tests, added style-guide comment headers, wrote
                        my_validate.
*******************************************************************************/
#include "mymalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HEAP_SIZE 4096
#define MIN_FRAG 16
#define BUSY_HASH 0x7FE9ED8E
#define FREE_HASH 0x3D497112
#define FREE_BUSY_DIFF ((2 * sizeof(free_header)) - sizeof(busy_header))
#define FSIZE sizeof(free_header)
#define BSIZE sizeof(busy_header)

typedef struct busy_header_s
{
    int hash;
    int size;
} busy_header;

// Header refers to low address
// Footer refers to high address
typedef struct free_header_s
{
    int hash;
    int size;
    struct free_header_s *next_free;
} free_header;

// external data, pointer to memory and 2 pointers to free list allowed
static char *g_mem_start;
static free_header *g_first_free;

static void free_list_add(free_header *);
static void free_list_remove(free_header *);
static void copy_header_to_footer(free_header *);
static int header_is_invalid(free_header *, int);

//************************************
// Initialize the memory allocator.
// This function must be called before any other memory allocator function
// Returns: nothing. If this function fails, it will print an error message
// and terminate the process.
// Thread safety: None
void my_mem_init()
{
    g_first_free = NULL;
    g_mem_start = (char *)malloc(HEAP_SIZE);

    if (g_mem_start == NULL)
    {
        fprintf(stderr, "[FATAL ERROR] Real malloc returned NULL,"
            " could not continue\n");
        exit(1);
    }

    free_header *header = (free_header *)g_mem_start; // rel addr 0
    header->hash = FREE_HASH;
    header->size = HEAP_SIZE - (2 * FSIZE);

    free_list_add(header);
}

//************************************
// Request a block for 'size' bytes.
// Returns a pointer on success, NULL on failure.
// Thread safety: None
void *my_malloc(int size)
{
    // make size multiple of 8 and make room for headers
    int actual_size = size + BSIZE;
    actual_size = ((actual_size + 7) & (-8));

    if (size <= 0 || actual_size > HEAP_SIZE || g_first_free == NULL)
    {
        return NULL;
    }

    // find valid block (best fit)
    free_header *travel = g_first_free;
    free_header *bestfit = NULL;
    int smallest_waste = 0x7FFFFFFF;
    while (travel != NULL && smallest_waste != 0)
    {
        if (travel->size + (2 * FSIZE) >= actual_size) // big enough
        {
            int waste = travel->size + (2 * FSIZE) - actual_size;

            if (waste < 0) // something broke in really weird way
            {
                fprintf(stderr, "[FATAL ERROR] If you are seeing this error"
                    " I'm already dead, could not continue\n");
                exit(1);
            }

            if (waste < smallest_waste)
            {
                smallest_waste = waste;
                bestfit = travel;
            }
        }

        travel = travel->next_free;
    }

    if (bestfit == NULL) // no block available
    {
        return NULL;
    }

    // make busy block
    int total_block_size = bestfit->size + (2 * FSIZE);
    int remainder = total_block_size - actual_size;
    if (remainder < 2 * FSIZE || remainder < MIN_FRAG + BSIZE) // use entire blk
    {
        free_list_remove(bestfit);

        // clear memory, innefficient but guarantees no lingering free footer
        // hint: don't dock any points if this isn't allowed please and thanks
        memset(bestfit, 0, total_block_size);

        busy_header *new_header = (busy_header *)bestfit;
        new_header->size = total_block_size - BSIZE;
        new_header->hash = BUSY_HASH;

        return (void *)(((uintptr_t)new_header) + BSIZE);
    }
    else // worth splitting into two blocks
    {
        // clear bits in busy block (I do this so the free footer is removed)
        memset((void *)(((uintptr_t)bestfit) + remainder), 0, actual_size);

        // adjust size of free block and copy to bottom
        bestfit->size = bestfit->size - actual_size;
        copy_header_to_footer(bestfit);

        // place busy header
        busy_header *new_header = (busy_header *)(((uintptr_t)bestfit) + 
            remainder);
        new_header->size = actual_size - BSIZE;
        new_header->hash = BUSY_HASH;

        return (void *)(((uintptr_t)new_header) + BSIZE);
    }
}

//************************************
// Free a previously allocated block.
// If my_free detects an error, it will likely print an error message and then
// terminate the process.
// Returns: nothing. This function terminates the process on failure.
// Thread safety: None
void my_free(void *ptr)
{
    if (ptr == NULL)
    {
        // per design specs
        fprintf(stderr, "[FATAL ERROR] The address passed to my_free did not "
            "correspond to a valid busy header, could not continue\n");
        exit(1); 
    }

    busy_header *header = (busy_header *)(((uintptr_t)ptr) - BSIZE);

    if (header_is_invalid((free_header *)header, BUSY_HASH))
    {
        // per design specs
        fprintf(stderr, "[FATAL ERROR] The address passed to my_free did not "
            "correspond to a valid busy header, could not continue\n");
        exit(1); 
    }

    int block_size = header->size;

    free_header *high_header = (free_header *)(((uintptr_t)header) + BSIZE + 
        block_size);
    free_header *low_footer = (free_header *)(((uintptr_t)header) - FSIZE);

    if (low_footer >= (free_header *)g_mem_start && low_footer->hash == 
        FREE_HASH)
    {
        free_header *lower_header = (free_header *)(((uintptr_t)low_footer) - 
            FSIZE - low_footer->size);

        if (lower_header->hash != FREE_HASH)
        {
            fprintf(stderr, "[FATAL ERROR] my_free noticed a missing header,"
                " memory is corrupt, could not continue\n");
            exit(1);
        }

        if ((uintptr_t)high_header < ((uintptr_t)g_mem_start + HEAP_SIZE) 
            && high_header->hash == FREE_HASH) // high and low blocks are free
        {
            // Coalesce with free blocks above and below
            free_list_remove(high_header);
            free_list_remove(lower_header);
            lower_header->size = lower_header->size + block_size 
                + high_header->size + (2 * FSIZE) + BSIZE;
            memset((void *)(((uintptr_t)lower_header) + FSIZE), 0,
                lower_header->size);
            free_list_add(lower_header);
        }
        else // Only low block is free
        {
            // Coalesce with free block below
            free_list_remove(lower_header);
            lower_header->size = lower_header->size + block_size + BSIZE;
            memset((void *)(((uintptr_t)lower_header) + FSIZE), 0, 
                lower_header->size);
            free_list_add(lower_header);
        }
    }
    else if ((uintptr_t)high_header < ((uintptr_t)g_mem_start) + HEAP_SIZE && 
        high_header->hash == FREE_HASH) // Only high block is free
    {
        // Coalesce with free block above
        free_list_remove(high_header);
        free_header *new_header = (free_header *)header;
        new_header->hash = FREE_HASH;
        new_header->size = block_size + BSIZE + high_header->size;
        memset((void *)(((uintptr_t)new_header) + FSIZE), 0, new_header->size);
        free_list_add(new_header);
    }
    else // Neither Above or Below are free
    {
        // place new free header
        free_header *new_header = (free_header *)header;
        int new_size = block_size + BSIZE - (2 * FSIZE);

        new_header->size = new_size;
        new_header->hash = FREE_HASH;
        memset((void *)(((uintptr_t)new_header) + FSIZE), 0, new_header->size);
        free_list_add(new_header);
    }
}

//************************************
// Scan the meta data for corruption.
// my_validate will print information if any corruption is detected.
// If no corruption is found, my_validate silently does nothing.
// Returns: nothing. This function never fails.
// Thread safety: None
void my_validate()
{
    // validate free_list
    int free_list_count = 0;
    free_header *travel = g_first_free;
    while (travel != NULL)
    {
        if (header_is_invalid(travel, FREE_HASH))
        {
            fprintf(stderr, "[MEMORY VALIDATION ERROR] Invalid free header "
                "detected at relative memory location: 0x%04x\n", 
                (unsigned int)((uintptr_t)travel - (uintptr_t)g_mem_start));
        }

        free_header *footer = (free_header *)(((uintptr_t)travel) + 
            travel->size + FSIZE);

        if (header_is_invalid(footer, FREE_HASH))
        {
            fprintf(stderr, "[MEMORY VALIDATION ERROR] Invalid free footer "
                "detected at relative memory location: 0x%04x belonging to "
                "header: 0x%04x\n", (unsigned int)((uintptr_t)footer - 
                (uintptr_t)g_mem_start), (unsigned int)((uintptr_t)travel - 
                (uintptr_t)g_mem_start));
        }

        if (travel->size != footer->size)
        {
            fprintf(stderr, "[MEMORY VALIDATION ERROR] Header and footer size "
                "mismatch at relative memory location: header: 0x%04x footer: "
                "0x%04x\n", (unsigned int)((uintptr_t)travel - 
                (uintptr_t)g_mem_start), (unsigned int)((uintptr_t)travel - 
                (uintptr_t)g_mem_start));
        }

        free_list_count++;
        travel = travel->next_free;
    }

    // validate blocks
    int free_block_count = 0;
    travel = (free_header *)g_mem_start;

    while (travel != (free_header *)(((uintptr_t)g_mem_start) + HEAP_SIZE))
    {
        if (header_is_invalid(travel, 0))
        {
            fprintf(stderr, "[MEMORY VALIDATION ERROR] Header at relative "
                "address: 0x%04x is invalid, unable to continue\n", 
                (unsigned int)((uintptr_t)travel - (uintptr_t)g_mem_start));
            break;
        }

        int bytes_to_next_header = 0;
        if (travel->hash == FREE_HASH)
        {
            bytes_to_next_header = travel->size + (2 * FSIZE);

            free_header *footer = (free_header *)(((uintptr_t)travel) + 
                travel->size + FSIZE);

            if (header_is_invalid(footer, FREE_HASH))
            {
                fprintf(stderr, "[MEMORY VALIDATION ERROR] Invalid free footer"
                    " detected at relative memory location: 0x%04x belonging "
                    "to header: 0x%04x, This free block was likely not in the "
                    "free list!\n", (unsigned int)((uintptr_t)footer - 
                    (uintptr_t)g_mem_start), (unsigned int)((uintptr_t)travel - 
                    (uintptr_t)g_mem_start));
            }

            if (travel->size != footer->size)
            {
                fprintf(stderr, "[MEMORY VALIDATION ERROR] Header and footer "
                    "size mismatch at relative memory location: header: 0x%04x"
                    " footer: 0x%04x\n, This free block was likely not in the "
                    "free list!", (unsigned int)((uintptr_t)travel - 
                    (uintptr_t)g_mem_start), (unsigned int)((uintptr_t)travel - 
                    (uintptr_t)g_mem_start));
            }

            free_block_count++;
        }
        else if (travel->hash == BUSY_HASH)
        {
            bytes_to_next_header = travel->size + BSIZE;
        }
        else
        {
            // header validation function is broken
            // this should not be reachable
            fprintf(stderr, "[MEMORY VALIDATION ERROR] Inexplicable error, "
                "unable to continue\n");
            break;
        }

        travel = (free_header *)(((uintptr_t)travel) + bytes_to_next_header);
    }

    if (travel == (free_header *)(((uintptr_t)g_mem_start) + HEAP_SIZE))
    {
        if (free_list_count != free_block_count)
        {
            fprintf(stderr, "[MEMORY VALIDATION ERROR] Number of free blocks "
                "parsed does not match number of free blocks in free list\n");
        }
    }
    else
    {
        fprintf(stderr, "[MEMORY VALIDATION ERROR] Unable to reach end of "
            "memory region, stopped at relative address: 0x%04x\n", 
            (unsigned int)((uintptr_t)travel - (uintptr_t)g_mem_start));
    }
}

//************************************
// Prints information about the state of the memory. All free and busy blocks
// should be included in the output.
// Returns: nothing. This function never fails.
// Thread safety: None
void my_print_mem()
{
    free_header *travel_header = (free_header *)g_mem_start;
    char *cur_addr = g_mem_start;

    fprintf(stdout, "%-13s%-10s%-10snext free\n", "Address", "size", "busy");

    while (cur_addr - g_mem_start < HEAP_SIZE && (travel_header->hash == 
        FREE_HASH || travel_header->hash == BUSY_HASH))
    {
        if (travel_header->hash == FREE_HASH)
        {
            fprintf(stdout, "0x%04x %#10x %9s",
                (unsigned int)(cur_addr - g_mem_start),
                (unsigned int)(travel_header->size + FREE_BUSY_DIFF), "no");

            if (travel_header->next_free == NULL)
            {
                fprintf(stdout, "%15s\n", "NULL");
            }
            else
            {
                fprintf(stdout, "%11s%04x\n", "0x", 
                    (unsigned int)((uintptr_t)travel_header->next_free - 
                    (uintptr_t)g_mem_start));
            }

            cur_addr = cur_addr + travel_header->size + (2 * FSIZE);
            travel_header = (free_header *)((uintptr_t)travel_header + 
                travel_header->size + (2 * FSIZE));
        }
        else if (travel_header->hash == BUSY_HASH)
        {
            fprintf(stdout, "0x%04x %#10x %9s\n",
                (unsigned int)(cur_addr - g_mem_start),
                (unsigned int)travel_header->size, "yes");

            cur_addr = cur_addr + travel_header->size + BSIZE;
            travel_header = (free_header *)((uintptr_t)travel_header + 
                travel_header->size + BSIZE);
        }
        else
        {
            fprintf(stderr, "[ERROR] my_print_mem encountered a should-be "
                "header with an invalid hash\n");
        }
    }
}

/*******************************************************************************
function:       free_list_add
purpose:        adds a free header to the freelist, makes that header the
                first_free and sets its next_free to the previous
                first_free. This function copies the entirety of the
                updated header to the footer of the free block.
                Updates first_free as necessary.
arguments:      free_header *header         the header to add to the
                                            free list
return:         none
thread safety:  none
*******************************************************************************/
static void free_list_add(free_header *header)
{
    if (header_is_invalid(header, FREE_HASH))
    {
        fprintf(stderr, "[FATAL ERROR] Header supplied to free_list_add was "
            "invalid, could not continue as this is indicative of a larger "
            "problem\n");
        exit(1);
    }

    if (g_first_free == NULL) // list is empty
    {
        g_first_free = header;
        header->next_free = NULL;

        copy_header_to_footer(header);
    }
    else
    {
        header->next_free = g_first_free;
        copy_header_to_footer(header);
        if (header->next_free != NULL)
        {
            copy_header_to_footer(header->next_free);
        }

        g_first_free = header;
    }
}

/*******************************************************************************
function:       free_list_remove
purpose:        removes a free header from the freelist,
                This function also cleans up any pointers pointing to the
                supplied header by either terminating them will NULL or
                redirecting them to the supplied header's next_free header,
                Updates first_free as necessary.
arguments:      free_header *header         the header to remove from the
                                            free list
return:         none
thread safety:  none
*******************************************************************************/
static void free_list_remove(free_header *header)
{
    if (header_is_invalid(header, FREE_HASH))
    {
        fprintf(stderr, "[FATAL ERROR] Header supplied to free_list_remove "
            "was invalid, could not continue as this is indicative of a larger "
            "problem\n");
        exit(1);
    }

    if (g_first_free == NULL) // empty list
    {
        fprintf(stderr, "[FATAL ERROR] free_list_remove called with an empty "
            "free_list, could not continue as this is indicative of a larger "
            "problem\n");
        exit(1);
    }

    if (header == g_first_free && g_first_free->next_free == NULL) // one elem
    {
        g_first_free = NULL;
    }
    else if (header == g_first_free && g_first_free->next_free != NULL)
    {
        g_first_free = header->next_free;
    }
    else // header is not first and more than one elem in list
    {
        free_header *travel = g_first_free;
        free_header *trail = NULL;

        while (travel != header && travel != NULL) // look for header
        {
            trail = travel;
            travel = travel->next_free;
        }

        if (travel == NULL) // header not in free list
        {
            fprintf(stderr, "[FATAL ERROR] Header supplied to free_list_remove"
                " is not in the free_list, could not continue as this is "
                "indicative of a larger problem\n");
            exit(1);
        }

        trail->next_free = travel->next_free;
    }
}

/*******************************************************************************
function:       copy_header_to_footer
purpose:        This function simply copies the contents of a free header
                to the contents of the corresponding footer to that header.
arguments:      free_header *header         the header that will have its
                                            copied into its footer
return:         none
thread safety:  none
*******************************************************************************/
static void copy_header_to_footer(free_header *header)
{
    if (header_is_invalid(header, FREE_HASH))
    {
        fprintf(stderr, "[FATAL ERROR] header supplied to "
            "copy_header_to_footer is invalid, could not continue as this "
            "is indicative of a larger problem\n");
        exit(1);
    }

    free_header *footer = (free_header *)(((uintptr_t)header) + FSIZE + 
        header->size);

    memcpy(footer, header, FSIZE);

    if (header_is_invalid(footer, FREE_HASH))
    {
        fprintf(stderr, "[FATAL ERROR] copy_header_to_footer failed, could not"
            " continue\n");
        exit(1);
    }
}

/*******************************************************************************
function:       header_is_invalid
purpose:        This function verifies that a free or busy header is valid by
                ensuring its address is in range and that its hash is correct
arguments:      free_header *header         the header that will be verified
                int hash    When supplied with FREE_HASH or BUSY_HASH this
                            function will flag the header as invalid if the
                            supplied header's hash does not match, if anything
                            other than a valid HASH is supplied here this
                            function will only flag the header as invalid
                            if it has neither a valid free or busy hash.
                            (or has another issue)
return:         0   the header is valid
                1   the header is invalid
thread safety:  none
*******************************************************************************/
static int header_is_invalid(free_header *header, int hash)
{
    if (header == NULL)
    {
        return 1;
    }

    // under range
    if ((uintptr_t)header < (uintptr_t)g_mem_start)
    {
        return 1;
    }

    // over range
    if ((uintptr_t)header > (uintptr_t)g_mem_start + HEAP_SIZE)
    {
        return 1;
    }

    // invalid hash
    if (hash == FREE_HASH)
    {
        if (header->hash != FREE_HASH)
        {
            return 1;
        }
    }
    else if (hash == BUSY_HASH)
    {
        if (header->hash != BUSY_HASH)
        {
            return 1;
        }
    }
    else
    {
        if ((header->hash != FREE_HASH && header->hash != BUSY_HASH))
        {
            return 1;
        }
    }

    return 0;
}