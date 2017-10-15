#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <string.h>
#include <signal.h>

#define gettid() syscall(SYS_gettid)

#define PID_FILE "main.pid"

//pthread_mutex_t put in thread  as arg


void write_pid(pid_t pid)
{
    FILE* fpid = fopen(PID_FILE, "w");

    if(fpid)
    {
        fprintf(fpid, "%d\n", pid);

        fclose(fpid);

        return;
    }
    
    perror("fopen\n");
}

void cleanup_handler(void *arg)
{
    pid_t* id = (pid_t*)arg;

    if(id)
        printf("Thread %d canceled.\n", *id);
    else
        printf("Thread canceled.\n");
}

void *pthread_routine1(void *arg)
{
    printf("Mutex thread routine.\n");
    pid_t* id = (pid_t*) malloc(sizeof(pid_t));
    *id = gettid();
    pthread_cleanup_push(cleanup_handler, (void*)id);

    pthread_mutex_t* mutex = (pthread_mutex_t*)arg;

    if(mutex)
    { 
        pthread_mutex_lock(mutex);

        printf("Mutex going to unlock\n");

        pthread_mutex_unlock(mutex);
    }
    
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
    free(id);
}

#ifndef __APPLE__
void *pthread_routine2(void *arg)
{
    printf("Spin thread routine.\n");
    pid_t* id = (pid_t*) malloc(sizeof(pid_t));
    *id = gettid();
    pthread_cleanup_push(cleanup_handler, (void*)id);

    pthread_spinlock_t* spin = (pthread_spinlock_t*)arg;

    if(spin)
    {
        pthread_spin_lock(spin);

        printf("Spin going to unlock\n");

        pthread_spin_unlock(spin);
    }
    
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
    free(id);
}
#endif

void *pthread_routine3(void *arg)
{
    printf("Rw read thread routine.\n");
    pid_t* id = (pid_t*) malloc(sizeof(pid_t));
    *id = gettid();
    pthread_cleanup_push(cleanup_handler, (void*)id);

    pthread_rwlock_t* rw = (pthread_rwlock_t*)arg;

    if(rw)
    {
        pthread_rwlock_rdlock(rw);

        printf("Rdlock going to unlock\n");

        pthread_rwlock_unlock(rw);
    }
    
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
    free(id);
}

void *pthread_routine4(void *arg)
{
    printf("Rw write thread started.\n");
    pid_t* id = (pid_t*) malloc(sizeof(pid_t));
    *id = gettid();
    pthread_cleanup_push(cleanup_handler, (void*)id);

    pthread_rwlock_t* rw = (pthread_rwlock_t*)arg;
    
    if(rw)
    {
        pthread_rwlock_wrlock(rw);

        printf("Wrlock going to unlock\n");

        pthread_rwlock_unlock(rw);
    }
    
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
    free(id);
}

int main()
{
    int result;
    pid_t pid = getpid();

    write_pid(pid);

    pthread_mutex_t *mutex = (pthread_mutex_t*) malloc( sizeof(pthread_mutex_t) );
#ifndef __APPLE__
    pthread_spinlock_t *spin = (pthread_spinlock_t*) malloc( sizeof(pthread_spinlock_t) );
#endif
    pthread_rwlock_t *rw = (pthread_rwlock_t*) malloc( sizeof(pthread_rwlock_t) );

    pthread_mutex_init(mutex, NULL);
    pthread_mutex_lock(mutex);

#ifndef __APPLE__
    pthread_spin_init(spin, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_lock(spin);
#endif

    pthread_rwlock_init(rw, NULL);
    //pthread_rwlock_rdlock(rw);
    pthread_rwlock_wrlock(rw);

    pthread_t* thread_id1 = (pthread_t*) malloc( sizeof(pthread_t) );
    pthread_create(thread_id1, NULL, pthread_routine1, (void*)mutex);

#ifndef __APPLE__
    pthread_t* thread_id2 = (pthread_t*) malloc( sizeof(pthread_t) );
    pthread_create(thread_id2, NULL, pthread_routine2, (void*)spin);
#endif

    pthread_t* thread_id3 = (pthread_t*) malloc( sizeof(pthread_t) );
    pthread_create(thread_id3, NULL, pthread_routine3, (void*)rw);

    pthread_t* thread_id4 = (pthread_t*) malloc( sizeof(pthread_t) );
    pthread_create(thread_id4, NULL, pthread_routine4, (void*)rw);

    sigset_t* sigset = (sigset_t*) malloc( sizeof(sigset_t) );
    sigemptyset(sigset);
    sigaddset(sigset, SIGTERM);
    sigaddset(sigset, SIGINT);
    sigprocmask(SIG_BLOCK, sigset, NULL);
    
    int* sig = (int*) malloc( sizeof(int) );

    sigwait(sigset, sig);

    pthread_mutex_unlock(mutex);

#ifndef __APPLE__
    pthread_spin_unlock(spin);
#endif

    pthread_rwlock_unlock(rw);

    pthread_join(*thread_id1, NULL);
#ifndef __APPLE__
    pthread_join(*thread_id2, NULL);
#endif
    pthread_join(*thread_id3, NULL);
    pthread_join(*thread_id4, NULL);

    pthread_mutex_destroy(mutex);
#ifndef __APPLE__
    pthread_spin_destroy(spin);
#endif
    pthread_rwlock_destroy(rw);

    free(sig);
    free(sigset);
    free(rw);
#ifndef __APPLE__
    free(spin);
#endif
    free(mutex);

    return 0;
}
