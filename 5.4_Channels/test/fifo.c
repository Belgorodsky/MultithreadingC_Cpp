#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER 2048

#define IN_FIFO "in.fifo"
#define OUT_FIFO "out.fifo"

int main()
{
    char buf[BUFFER];
    mkfifo(IN_FIFO, 0666);
    mkfifo(OUT_FIFO, 0666);
    FILE* in_fd = fopen(IN_FIFO, "r");
    FILE* out_fd = fopen(OUT_FIFO, "w");

    while(fgets(buf, BUFFER, in_fd) != NULL)
    {
        if( fputs(buf, out_fd) != -1 )
        {
            printf("You send: %s", buf);
        }
        else
        {
            perror("fputs");
        }
    }

    fclose(in_fd);
    fclose(out_fd);

    return 0;
}
