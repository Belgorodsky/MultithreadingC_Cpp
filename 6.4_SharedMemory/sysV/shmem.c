#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <unistd.h>

#define SH_SIZE 1024 * 1024

int main()
{
    key_t key = ftok("/tmp/mem.temp", 1);

    int shmid = shmget(key, SH_SIZE, 0666 | IPC_CREAT);

    void *addr = shmat(shmid, NULL, 0);

    if( (void*)-1 != addr )
    { 
        char* bytep = (char*)addr;
        size_t i = 0;
        for(i; i < SH_SIZE; ++i)
        {
            bytep[i] = 42;
        }
    }
    else
    {
        perror("shmat");
    }

    pause();

    shmdt(addr);

    return 0;
}
