/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 1 Processes and Pipes
    File:       prefilter.c
    Purpose:    Wrapper for strchr
    History:    3/31/22 File created, core functionality completed
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

const int MAXLINELENGTH = 100;

int main(int argc, char **argv)
{
    int procid = getpid();

    if (argc > 1)
    {
        char token = argv[1][0];
        char line[MAXLINELENGTH];

        while (fgets(line, MAXLINELENGTH, stdin) != NULL)
        {
            fprintf(stderr, "%i pre %c: %s", procid, token, line);

            char *result = strchr(line, token);

            if (result != NULL)
            {
                fprintf(stdout, "%s", result);
                fprintf(stderr, "%i rest: %s", procid, result);
            }
            else
            {
                fprintf(stderr, "%i rest:\n", procid);
            }
        }
    }
    else
    {
        fprintf(stderr, "[ERROR] No argument provided!\n"
                        "You must provide a character to filter with\n");
        exit(1);
    }

    return 0;
}