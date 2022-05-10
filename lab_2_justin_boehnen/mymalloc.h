#pragma once
//*****************************************************
// Declaration of a memory allocator
//
// Author: Phil Howard
//

//************************************
// Initialize the memory allocator.
// This function must be called before any other memory allocator function
// Returns: nothing. If this function fails, it will print an error message
// and terminate the process.
// Thread safety: None
void my_mem_init();

//************************************
// Request a block for 'size' bytes.
// Returns a pointer on success, NULL on failure.
// Thread safety: None
void *my_malloc(int size);

//************************************
// Free a previously allocated block.
// If my_free detects an error, it will likely print an error message and then
// terminate the process.
// Returns: nothing. This function terminates the process on failure.
// Thread safety: None
void my_free(void *ptr);

//************************************
// Scan the meta data for corruption. 
// my_validate will print information if any corruption is detected. 
// If no corruption is found, my_validate silently does nothing.
// Returns: nothing. This function never fails.
// Thread safety: None
void my_validate();

//************************************
// Prints information about the state of the memory. All free and busy blocks
// should be included in the output.
// Returns: nothing. This function never fails.
// Thread safety: None
void my_print_mem();

