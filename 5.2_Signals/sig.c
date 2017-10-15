#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void sigChildHandler(int signum)
{
    pid_t pid;
    int status;

    waitpid(-1, &status, 0); 
    printf("Child is gone with exit code: %d\n", status );
}

int main()
{

    struct sigaction act;
    sigemptyset(&act.sa_mask);

    act.sa_handler = &sigChildHandler;
    act.sa_flags = SA_NOCLDWAIT;

    sigaction(SIGCHLD, &act, 0);
    pid_t p = fork();

    if(p)
    {
        FILE* childPidFile = fopen("pid_child", "w");
        fprintf(childPidFile, "%d\n", p);
        fclose(childPidFile);
        FILE* parentPidFile = fopen("pid_parent", "w");
        fprintf(parentPidFile, "%d\n", getpid());
        fclose(parentPidFile);

    }
    pause();

    return 0;
}
