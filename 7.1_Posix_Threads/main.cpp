#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void* routine(void *value)
{
    size_t* seconds = (size_t*)value;

    sleep(*seconds);

    return value;
}

int main()
{
    pid_t pid = getpid();
    FILE* f_pid = fopen("main.pid", "w");
    fprintf(f_pid, "%d\n", pid);
    fclose(f_pid);

    pthread_t* thread = (pthread_t*)malloc(sizeof(pthread_t));

    size_t seconds = 1000;

    pthread_create(thread, NULL, routine, (void*)&seconds);

    void *result = malloc(sizeof(size_t));

    pthread_join(*thread, &result);

    printf("result: %lu\n", *((size_t*)(result)));

    free(result);
    free(thread);
    
    return 0;
}
