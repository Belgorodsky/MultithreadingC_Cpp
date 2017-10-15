#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>

#define P_NAME "/test.shm"
#define SH_SIZE 1024 * 1024
#define SH_PR 0666

int main()
{

    int fd = shm_open(P_NAME, O_CREAT | O_RDWR, SH_PR);

    if(fd < 0)
    {
        perror("shm_open");
    }
    else
    {
        ftruncate(fd, SH_SIZE);

        void* addr = mmap(NULL, SH_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED , fd, 0);

        if(MAP_FAILED != addr)
        {
            char* bytep = (char*)addr;
            size_t i = 0;
            for(; i < SH_SIZE; ++i)
            {
                bytep[i] = 13;
            }
        }
        else
        {
            perror("mmap");
        }

        pause();

        munmap(addr, SH_SIZE);

    }

    shm_unlink(P_NAME);

    return 0;
}
