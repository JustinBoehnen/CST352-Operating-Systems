/*******************************************************************************
    Justin Boehnen
    Lab:        Lab 1 Processes and Pipes
    File:       process.c
    Purpose:    Processes the output of a command with prefilter
                and postfilter.
    History:    3/31/22 File created, inoperable
                4/01/22 Command output sucessfully piped to and
                        processed by prefilter, issue getting output
                        from postfilter
                4/02/22 Optimizations made, troubleshooting postInPipe
                        functionality continues.
                4/03/22 Refactored everything, believe that output is correct
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void closepipe(int fd);

int main(int argc, char *argv[])
{
    if (argc >= 4)
    {
        char *preToken = argv[1];
        char *postToken = argv[2];

        // pointer to start of command args (sub array)
        char **commandArgv = &argv[3];

        int preInPipe[2] = {0};  // command out -> prefilter in
        int postInPipe[2] = {0}; // prefilter out -> postfilter in

        if (pipe(preInPipe) == -1)
        {
            fprintf(stderr, "[ERROR] Unable to create pipe!\n");
            exit(1);
        }
        if (pipe(postInPipe) == -1)
        {
            fprintf(stderr, "[ERROR] Unable to create pipe!\n");
            exit(1);
        }

        int postfilterChild = fork();

        if (postfilterChild == 0) // child (will do postfilter)
        {
            //  postfilterChild needs read end of postInPipe
            //  postfilterChild does not need write end of postInPipe
            //  or either end of preInPipe

            // close write end of postInPipe
            closepipe(postInPipe[1]);

            // close read end of preInPipe
            closepipe(preInPipe[0]);

            // close write end of preInPipe
            closepipe(preInPipe[1]);

            // read from prefilter
            if (dup2(postInPipe[0], 0) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to do pipe stuff!\n");
                exit(1);
            }

            // close read end of postInPipe
            closepipe(postInPipe[0]);

            if (execl("postfilter", "postfilter",
                postToken, (char *)NULL) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to run postfilter!\n");
                exit(1);
            }
        }
        else if (postfilterChild < 0)
        {
            fprintf(stderr, "[ERROR] Failed to fork postfilter child!\n");
            exit(1);
        }

        int prefilterChild = fork();

        if (prefilterChild == 0) // child (will do prefilter)
        {
            //  prefilterChild needs read end of preInPipe
            //  and write end of postInPipe
            //  prefilterChild does not need write end of preInPipe
            //  or read end of postInPipe

            // close write end of preInPipe
            closepipe(preInPipe[1]);

            // close read end of postInPipe
            closepipe(postInPipe[0]);

            // read from commandChild (command output)
            if (dup2(preInPipe[0], 0) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to do pipe stuff!\n");
                exit(1);
            }

            // write to postfilter
            if (dup2(postInPipe[1], 1) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to do pipe stuff!\n");
                exit(1);
            }

            // close read end of preInPipe
            closepipe(preInPipe[0]);

            // close write end of postInPipe
            closepipe(postInPipe[1]);

            if (execl("prefilter", "prefilter", preToken, (char *)NULL) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to run prefilter!\n");
                exit(1);
            }
        }
        else if (prefilterChild < 0)
        {
            fprintf(stderr, "[ERROR] Failed to fork prefilter child!\n");
            exit(1);
        }

        int commandChild = fork();

        if (commandChild == 0) // child (will do command execution)
        {
            //  commandChild needs write end of preInPipe
            //  commandChild does not need read end of preInPipe
            //  or either end of postInPipe

            // close read end of preInPipe
            closepipe(preInPipe[0]);

            // close read end of postInPipe
            closepipe(postInPipe[0]);

            // close write end of postInPipe
            closepipe(postInPipe[1]);

            // write to prefilter
            if (dup2(preInPipe[1], 1) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to do pipe stuff!\n");
                exit(1);
            }

            // close write end of preInPipe
            closepipe(preInPipe[1]);

            if (execvp(commandArgv[0], commandArgv) == -1)
            {
                fprintf(stderr, "[ERROR] Failed to run command!\n");
                exit(1);
            }
        }
        else if (commandChild < 0)
        {
            fprintf(stderr, "[ERROR] Failed to fork command child!\n");
            exit(1);
        }

        // close pipes
        closepipe(preInPipe[0]);
        closepipe(preInPipe[1]);
        closepipe(postInPipe[0]);
        closepipe(postInPipe[1]);

        // wait for child processes to finish
        if (waitpid(postfilterChild, NULL, 0) == -1)
        {
            fprintf(stderr, "[ERROR] Failed to wait for process!\n");
            exit(1);
        }
        if (waitpid(prefilterChild, NULL, 0) == -1)
        {
            fprintf(stderr, "[ERROR] Failed to wait for process!\n");
            exit(1);
        }
        if (waitpid(commandChild, NULL, 0) == -1)
        {
            fprintf(stderr, "[ERROR] Failed to wait for process!\n");
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "[ERROR] Insufficient number of arguments provided!\n"
                        "You must provide two characters to filter the input "
                        "with a command as input\n");
        exit(1);
    }
}

/*******************************************************************************
    function:       closepipe
    purpose:        closes file descriptors associated with pipes while also
                    checking for success of the close call and reacting
                    accordingly if the call fails.
    arguments:      int fd              the file to close
    return:         none
    thread safety:  none
*******************************************************************************/
static void closepipe(int fd)
{
    if (close(fd) == -1)
    {
        fprintf(stderr, "[ERROR] Failed to close pipe!\n");
        exit(1);
    }
}