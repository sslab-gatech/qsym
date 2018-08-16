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
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>

/**
 * This test targets a specific edge case that happened when detaching while
 * a thread was inside a (potentially) blocking syscall. Pin has special
 * handling for this case which can cause issues depending on how Pin's libc
 * behaves. While this happens even with one thread, we use several to stress
 * the mechanism/
 */

#define THRNUM (10)
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_t thr[THRNUM];

// This global array is used to let the main app thread know when it's time
// to tell Pin to detach. The accesses to this array aren't synchronized, but
// it's safe because only one thread writes to each array member, and reading
// will repeat until all writes are seen anyway.
int pass[THRNUM] = {0};

void* thread_func(void *arg)
{
    size_t index = (size_t)arg;
    pass[index] = 1;
    // The main thread owns the lock, so this creates a blocking syscall.
    pthread_mutex_lock(&mut);
    pthread_mutex_unlock(&mut);
    return NULL;
}

void DetachPin(unsigned int num)
{
    // Pin should replace this function.
    assert(0);
}

int main ()
{
    int check = 0;
    size_t i = 0;
    // All your lock are belong to us.
    pthread_mutex_lock(&mut);
    for (i = 0; i < THRNUM; ++i)
    {
        pthread_create(&thr[i], 0, thread_func, (void*)i);
    }
    while (!check)
    {
        check = 1;
        for (i = 0; i < THRNUM; ++i)
        {
            check &= pass[i];
        }
        sched_yield();
    }
    // allow all threads to make it to the lock.
    sleep(1);
    DetachPin(THRNUM+1);
    pthread_mutex_unlock(&mut);
    for (i = 0; i < THRNUM; ++i)
    {
        pthread_join(thr[i], 0);
    }
    return 0;
}

