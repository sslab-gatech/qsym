/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2015 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 * gcc_thread_local_vars_app.c
 * This example code checks GCC's __thread variable:
 * 1. Each of the threads writes a different value to its own __thread variable
 * 2. All threads are synchronized
 * 3. Each thread reads the value of the __thread variable and returns it back
 *        to the main thread
 * 4. The main thread prints the values of each thread's variable in order
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#define THREADS_COUNT 10

__thread int tid_local[16] = {0};
int barrier = THREADS_COUNT;
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_threshold_cv;

void Fail(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

void AddToThreadLocalVar(int i)
{
    tid_local[5] += i;
}


void *thread_main(void *ptr)
{
    AddToThreadLocalVar(*(int*)ptr);
    AddToThreadLocalVar(*(int*)ptr);
    pthread_mutex_lock(&barrier_mutex);
    if (--barrier <= 0)
    {
        pthread_cond_signal(&barrier_threshold_cv);
    }
    pthread_mutex_unlock(&barrier_mutex);

    *(int*)ptr = tid_local[5];
    return NULL;
}

int main()
{
    pthread_t threads[THREADS_COUNT] = {0};
    int results[THREADS_COUNT] = {0};
    int res = -1;
    int i = 0;

    res = pthread_mutex_init(&barrier_mutex, NULL);
    if (0 != res)
    {
        Fail("Failed to initialize mutex (error %d)\n", res);
    }
    res = pthread_cond_init (&barrier_threshold_cv, NULL);
    if (0 != res)
    {
        Fail("Failed to initialize condition variable (error %d)\n", res);
    }
    for (i = 0; i < THREADS_COUNT; i++)
    {
        results[i] = i;
        res = pthread_create( &threads[i], NULL, thread_main, (void*)&results[i]);
        if (0 != res)
        {
            Fail("Failed to create thread number %d (error %d)\n", i, res);
        }
    }

    pthread_mutex_lock(&barrier_mutex);
    /*
     * Waits for all the threads to write to their '__thread' variables
     */
    while (barrier > 0)
    {
        pthread_cond_wait(&barrier_threshold_cv, &barrier_mutex);
    }
    pthread_mutex_unlock(&barrier_mutex);

    /*
     * Waits for all the threads to terminate, reading their '__thread' variables before
     */
    for (i = 0; i < THREADS_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
        printf("TLS%d=%d\n", i, results[i]);
    }

    pthread_mutex_destroy(&barrier_mutex);
    pthread_cond_destroy(&barrier_threshold_cv);
    return 0;
}
