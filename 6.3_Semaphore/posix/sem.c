#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>                                                              
#include <unistd.h>                                                             
#include <errno.h>                                                              
#include <string.h>                                                             
                                                                                
#define P_NAME "/test.sem"                                                                                

int main ()
{
    sem_t* sem = sem_open(P_NAME, O_CREAT, 0666, 66);

    pause();

    sem_unlink(P_NAME);

    return 0; 
}
