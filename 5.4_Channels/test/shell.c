#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define IN_BUFF 1024
#define OUT_BUFF 2048

void read_stdin(char* cmd)
{
    if( fgets(cmd, IN_BUFF, stdin) == NULL)
    {
        perror("read command");
        exit(1);
    }
}

void do_command(const char* cmd)
{
    char result[OUT_BUFF];
    FILE* fp = popen(cmd, "r");
    if(fp)
    {
        FILE* fr = fopen("result", "w");
        while(fgets(result, OUT_BUFF, fp) != NULL)
        {
            fprintf(fr, "%s", result);
        }
        fclose(fr);
    }
    else
    {
        perror("run command");
        exit(1);
    }

    pclose(fp);
}

int main()
{
    char cmd[IN_BUFF];
    read_stdin(cmd);
    do_command(cmd);

    return 0;
}
