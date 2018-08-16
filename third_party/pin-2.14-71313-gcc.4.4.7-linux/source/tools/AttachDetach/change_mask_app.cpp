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
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <cstdlib>
#include <semaphore.h>
#include <sys/syscall.h>

/*
 * This application checks the correctness of the retrieve and alteration
 * of the thread sigmask when a tool retrieve/alter it.
 */


static sem_t mutex, mutex1;
static pthread_t one_tid, two_tid;
static FILE * fd_signals;
static char * FILE_NAME = const_cast<char *>("signal_list.txt");
static int MAX_SIZE = 128; /*maximum line size*/
static int syncPipe[2];
static volatile int iteration = 0;

extern "C" bool WaitChangeSigmask()
{
   return false;
}

void EmptySignalHandler(int param)
{
}

/*
 * block all the signals relevant to this test
 * Note that we don't want to block SIGTERM, SIGINT and such because we want
 * to allow the test termination in case of cancelation
 */
void BlockUserSignals()
{ 
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    sigaddset(&sigmask, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
} 

/*
 * block only the signals in the list: "signalsListToBlock"
 */
void BlockSignals(int signalsListToBlock[] , int len, sigset_t * sigmask)
{
    sigemptyset(sigmask);
    int i;
    for(i=0; i< len; ++i) 
        sigaddset(sigmask, signalsListToBlock[i]);
    pthread_sigmask(SIG_SETMASK, sigmask, NULL);
}

/*
 *  A thread function that processes SIGUSR1 and SIGUSR2 signals sent by the SignalSender thread function
 */
void * SignalReceiver(void *arg)
{ 
    fd_signals= fopen(FILE_NAME, "wt"); 
    int sigList[1] = {SIGUSR2};
    sigset_t sigmask;
    BlockSignals(sigList, 1, &sigmask);
    signal(SIGUSR2, EmptySignalHandler);
    signal(SIGUSR1, EmptySignalHandler);
    sem_post(&mutex);
    while (iteration<10)
    { 
        int numSigReceived=0;
        pthread_sigmask(SIG_SETMASK, NULL, &sigmask);

        time_t start = time(NULL);

        // wait for the signal for at most 2 seconds
        while ((time(NULL) - start) < 2)
        {
            if (0 != sigpending(&sigmask))
            {
                perror("sigpending");
                exit(2);
            }
            if (sigismember(&sigmask, SIGUSR1) || sigismember(&sigmask, SIGUSR2))
            {
                // sigwait will not block since we already have signal pending
                sigwait(&sigmask, &numSigReceived);
                break;
            }
            sched_yield();
        }

        signal(SIGUSR2, EmptySignalHandler);
        signal(SIGUSR1, EmptySignalHandler);

        if(numSigReceived == SIGUSR2)
        {
            iteration++;
            fprintf(fd_signals, "%d", 2 );
        }
        if(numSigReceived == SIGUSR1)
        {
            iteration++;
            fprintf(fd_signals, "%d", 1 );
        }
        sem_post(&mutex);
        sem_wait(&mutex1);
    } 
    fclose(fd_signals);
    return NULL;
} 

/*
 *  Send repeatedly signals (SIGUSR1 and SIGUSR2) to the thread which starts execution by invoking the function
 *  SignalReceiver
 */
void * SignalSender(void *arg)
{
    bool wasSigmaskChanged = false;
    sigset_t sigmask;
    int sigList[2] = {SIGUSR1, SIGUSR2};
    BlockSignals(sigList, 2, &sigmask);
    sem_wait(&mutex);
    do
    {
        pthread_kill(two_tid, SIGUSR2); /* Delivers a signal*/
        pthread_kill(two_tid, SIGUSR1); /* Delivers a signal*/

        if(iteration==2 && !wasSigmaskChanged)
        {
            wasSigmaskChanged = true;
            close(syncPipe[1]); // close the write side - releasing the child process to start PIN
            while(!WaitChangeSigmask()) sched_yield();
        }

        sem_post(&mutex1);
        sem_wait(&mutex);           
    }
    while (pthread_kill(two_tid, 0) == 0); // while two_tid is alive
    return NULL;
}

int main(int argc, char * argv[])
{
    pid_t parentPid = getpid();

    if (argc != 3 && argc != 4)
    {
        fprintf(stderr, "Usage: %s <PIN exe> [-slow_asserts] <Tool name>\n", argv[0]);
        return 1;
    }

    if (0 != pipe(syncPipe))
    {
        perror("pipe");
        return 1;
    }
    pid_t pid = fork();
    BlockUserSignals();
 
    if (pid)
    {
        close(syncPipe[0]); // close the read side of the pipe
        sem_init(&mutex, 0, 0);
        sem_init(&mutex1, 0, 0);
    
        /*
         * create two threads, one which sends siganls to the other threads, which receives them.
         */
        pthread_create(&one_tid, NULL, SignalSender, NULL); 
        pthread_create(&two_tid, NULL, SignalReceiver, NULL);
     
        /*
         * suspended excution until the two threads terminate
         */
        pthread_join(two_tid,NULL);
        sem_post(&mutex); // release mutex as one_tid may wait on it
        pthread_join(one_tid, NULL);

        /*
         * cleanup  
         */
        sem_destroy(&mutex); /* destroy semaphore */
        sem_destroy(&mutex1); /* destroy semaphore */
    }
    else 
    {
        char dummy;
        // inside child
        close(syncPipe[1]); // close the write side of the pipe
        read(syncPipe[0], &dummy, sizeof(dummy)); // wait for parent
        close(syncPipe[0]); // close the read side as we're done
        char attachPid[MAX_SIZE];
        sprintf(attachPid, "%d", parentPid);

        char* args[9];
        int argsco = 0;
        int argsci = 1;
        args[argsco++] = argv[argsci++]; // pin executable
        if (argc == 4)
        {
            char* slow_assert = args[argsco++] = argv[argsci++]; // -slow-assert (if presented)
            if (0 != strcmp(slow_assert, "-slow_asserts"))
            {
                fprintf(stderr, "Expected 2nd argument in command line (%s) to be -slow_asserts\n", slow_assert);
                killpg(0, SIGKILL);
                return 2;
            }
        }
        args[argsco++] = "-probe";
        args[argsco++] = "-pid";
        args[argsco++] = attachPid;
        args[argsco++] = "-t";
        args[argsco++] = argv[argsci++]; // tool name
        args[argsco++] = NULL; // end
          
        /*
         * Pin attach to the parent thread.
         * never return
         */
        execv(args[0], args);
        fprintf(stderr, "execl failed with errno: %d\n", errno);
    }
}
