#include <sys/ipc.h>                                                            
#include <sys/sem.h>                                                             
#include <sys/types.h>                                                          
#include <stdio.h>                                                              
#include <unistd.h>                                                             
#include <fcntl.h>                                                              
#include <errno.h>                                                              
#include <string.h>  

#define S_COUNT 16

int main()
{
    unsigned short array[S_COUNT];
    union semun arg;
    for(int i = 0 ; i < S_COUNT; ++i)
    {
        array[i] = i;
    }

    arg.array = array;

    key_t key = ftok("/tmp/sem.temp", 1);
    int semid = semget(key, S_COUNT, 0666 | IPC_CREAT);

    semctl(semid, S_COUNT, SETALL, arg);
    
    pause();

    semctl(semid, IPC_RMID, 0);                                          
    
    return 0;
}
