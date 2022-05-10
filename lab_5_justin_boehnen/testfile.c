// ******************************************************
// Program to test the output of a producer/consumer
// Author: Phil Howard
//
// Program reads from stdin the output of a run from the test program.

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    int a_count = 0;
    int b_count = 0;
    int last_a = 0;
    int interleaves = 0;
    int fields;

    unsigned long thread;
    char id;
    int seq;
    char text[256];

    char buff[300];

    while (fgets(buff, sizeof(buff), stdin) != NULL)
    {
        fields = sscanf(buff, "%ld %c %d %s", &thread, &id, &seq, text);
        if (fields != 4) 
            fprintf(stderr, "Bad line %s", buff);
        else
        {
            if (strcmp(text, "abcdefghijklmnopqrstuvwxyz") != 0)
                fprintf(stderr, "Invalid text: %s", text);

            if (id == 'A')
            {
                if (a_count != seq)
                    fprintf(stderr, "Bad sequence: %d %d %s", 
                            a_count, seq, buff);
                a_count = seq + 1;
                if (!last_a) interleaves++;
                last_a = 1;
            }
            else
            {
                if (b_count != seq)
                    fprintf(stderr, "Bad sequence: %d %d %s", 
                            b_count, seq, buff);
                b_count = seq + 1;
                if (last_a) interleaves++;
                last_a = 0;
            }
        }
    }

    printf("Found %d A's %d B's and %d interleavings\n", 
            a_count, b_count, interleaves);

    return 0;
}

