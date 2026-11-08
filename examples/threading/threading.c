#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    bool mutex_locked = false;

    thread_func_args->thread_complete_success = false;

    if (usleep((useconds_t)thread_func_args->wait_to_obtain_ms * 1000U) != 0) {
        ERROR_LOG("Unable to complete wait before obtaining mutex");
        return thread_param;
    }

    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Unable to lock mutex");
        return thread_param;
    }
    mutex_locked = true;

    if (usleep((useconds_t)thread_func_args->wait_to_release_ms * 1000U) != 0) {
        ERROR_LOG("Unable to complete wait before releasing mutex");
    } else {
        thread_func_args->thread_complete_success = true;
    }

    if (mutex_locked && pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Unable to unlock mutex");
        thread_func_args->thread_complete_success = false;
    }

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *thread_func_args;

    if (thread == NULL || mutex == NULL || wait_to_obtain_ms < 0 || wait_to_release_ms < 0) {
        return false;
    }

    thread_func_args = malloc(sizeof(*thread_func_args));
    if (thread_func_args == NULL) {
        ERROR_LOG("Unable to allocate thread data");
        return false;
    }

    thread_func_args->mutex = mutex;
    thread_func_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms = wait_to_release_ms;
    thread_func_args->thread_complete_success = false;

    if (pthread_create(thread, NULL, threadfunc, thread_func_args) != 0) {
        ERROR_LOG("Unable to create thread");
        free(thread_func_args);
        return false;
    }

    return true;
}

