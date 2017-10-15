#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifdef __APPLE__
#include "barrier.h"
#endif

#define NTHREADS 2

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

union routine_param
{
    struct {
        int *conditionMet;
        pthread_cond_t *cond;
        pthread_mutex_t *mutex;
    } condition_params;

    pthread_barrier_t *bp;
};

void checkResults(const char* func_name, int ret_val)
{
    if (ret_val)
    {
        fprintf(stderr, "%s exec with error %d\n", func_name, ret_val); 
    }
}

void *pthread_routine1(void *arg)
{
    routine_param* param = (routine_param*)arg;

    if (param)
    {
        int *conditionMet = param->condition_params.conditionMet;
        pthread_mutex_t *mutex = param->condition_params.mutex;
        pthread_cond_t *cond = param->condition_params.cond;

        if (conditionMet && mutex && cond)
        {
            int rc;
            rc = pthread_mutex_lock(mutex);
            checkResults("pthread_mutex_lock()\n", rc);

            while (!*conditionMet) {

                printf("Thread blocked\n");
                rc = pthread_cond_wait(cond, mutex);
                checkResults("pthread_cond_wait()\n", rc);
            }

            rc = pthread_mutex_unlock(mutex);
            checkResults("pthread_mutex_lock()\n", rc);

        }
        else
        {
            fprintf(stderr, "%s", "pthread_routine1 has bad arg\n"); 
        }

    }
    else
    {
        fprintf(stderr, "%s", "pthread_routine1 has bad arg\n"); 
    }

    return NULL;
}

void *pthread_routine2(void *arg)
{
    routine_param* param = (routine_param*)arg;

    if (param)
    {
        pthread_barrier_t *bp = param->bp;

        if (bp)
        {
            int rc;
            printf("Thread blocked\n");
            rc = pthread_barrier_wait(bp);
#ifndef __APPLE__
            if (rc != PTHREAD_BARRIER_SERIAL_THREAD)
            {
                fprintf(stderr, "%s exec with error %d", "pthread_barrier_wait()\n" , rc); 
            }
#endif
        }
        else
        {
            fprintf(stderr, "%s", "pthread_routine2 has bad arg\n"); 
        }
    }
    else
    {
        fprintf(stderr, "%s", "pthread_routine2 has bad arg\n"); 
    }

    return NULL;
}

int main()
{
    pid_t pid = getpid();
    write_pid(pid);

    int rc;

    routine_param param1;
    param1.condition_params.conditionMet = (int*)calloc(1, sizeof(int));

    param1.condition_params.cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    rc = pthread_cond_init(param1.condition_params.cond, NULL);
    checkResults("pthread_cond_init()\n", rc);

    param1.condition_params.mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    rc = pthread_mutex_init(param1.condition_params.mutex, NULL);
    checkResults("pthread_mutex_init()\n", rc);

    routine_param param2;
    param2.bp = (pthread_barrier_t*)malloc(sizeof(pthread_barrier_t));
    rc = pthread_barrier_init(param2.bp, NULL, 1);
    checkResults("pthread_barrier_init()\n", rc);

    pthread_t* thread_id1 = (pthread_t*) malloc( sizeof(pthread_t) );
    rc = pthread_create(thread_id1, NULL, pthread_routine1, (void*)&param1);
    checkResults("pthread_create()\n", rc);

    pthread_t* thread_id2 = (pthread_t*) malloc( sizeof(pthread_t) );
    rc = pthread_create(thread_id2, NULL, pthread_routine2, (void*)&param2);
    checkResults("pthread_create()\n", rc);

    sigset_t* sigset = (sigset_t*) malloc( sizeof(sigset_t) );
    rc = sigemptyset(sigset);
    checkResults("sigemptyset()\n", rc);
    rc = sigaddset(sigset, SIGTERM);
    checkResults("sigaddset()\n", rc);
    rc = sigaddset(sigset, SIGINT);
    checkResults("sigaddset()\n", rc);
    rc = sigprocmask(SIG_BLOCK, sigset, NULL);
    checkResults("sigprocmask()\n", rc);
    
    int* sig = (int*) malloc( sizeof(int) );

    rc = sigwait(sigset, sig);
    checkResults("sigwait()\n", rc);

    rc = pthread_mutex_lock(param1.condition_params.mutex);
    checkResults("pthread_mutex_lock()\n", rc);

    /* The condition has occured. Set the flag and wake up any waiting threads */
    *param1.condition_params.conditionMet = 1;
    printf("Wake up all waiting threads...\n");
    rc = pthread_cond_broadcast(param1.condition_params.cond);
    checkResults("pthread_cond_broadcast()\n", rc);

    rc = pthread_mutex_unlock(param1.condition_params.mutex);
    checkResults("pthread_mutex_unlock()\n", rc);

    rc = pthread_barrier_wait(param2.bp);
#ifndef __APPLE__
    if (rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        fprintf(stderr, "%s exec with error %d", "pthread_barrier_wait()\n" , rc); 
    }
#endif

    printf("Wait for threads and cleanup\n");

    rc = pthread_join(*thread_id1, NULL);
    checkResults("pthread_join()\n", rc);
    rc = pthread_join(*thread_id2, NULL);
    checkResults("pthread_join()\n", rc);

    pthread_barrier_destroy(param2.bp);
    pthread_cond_destroy(param1.condition_params.cond);
    pthread_mutex_destroy(param1.condition_params.mutex);
    free(param1.condition_params.conditionMet);
    free(sigset);
    free(sig);
    free(thread_id2);
    free(thread_id1);

    printf("Main completed\n");
    return 0;
}
