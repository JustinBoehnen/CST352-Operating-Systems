/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 1 Processes and Pipes
    File:       postfilter.c
    Purpose:    Wrapper for strrchr, but outputs everything before the
                strrchr pointer.
    History:    3/31/22 File created, inoperable
                4/01/22 Core functionality completed
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
        if (memset(line, 0, MAXLINELENGTH) == NULL)
        {
            fprintf(stderr, "[ERROR] Memset error!\n");
            exit(1);
        }

        while (fgets(line, MAXLINELENGTH, stdin) != NULL)
        {
            fprintf(stderr, "%i post %c: %s", procid, token, line);

            char *garbage = strrchr(line, token);
            if (garbage != NULL)
            {
                char result[MAXLINELENGTH];
                if (memset(result, 0, MAXLINELENGTH) == NULL)
                {
                    fprintf(stderr, "[ERROR] Memset error!\n");
                    exit(1);
                }

                int resultLen = (strlen(line) - 1) - (strlen(garbage) - 1);
                if (resultLen > 0)
                {
                    if (resultLen >= MAXLINELENGTH)
                    {
                        resultLen = MAXLINELENGTH - 1;
                    }

                    if (strncpy(result, line, resultLen) == NULL)
                    {
                        fprintf(stderr, "[ERROR] String copy error!\n");
                        exit(1);
                    }
                }
                result[resultLen] = '\n';

                if (result != NULL && resultLen > 0)
                {
                    fprintf(stdout, "%s", result);
                    fprintf(stderr, "%i remaining: %s", procid, result);
                }
                else
                {
                    fprintf(stderr, "%i remaining:\n", procid);
                }
            }
            else
            {
                if (strlen(line) > 1)
                {
                    fprintf(stdout, "%s", line);
                }
                fprintf(stderr, "%i remaining: %s", procid, line);
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