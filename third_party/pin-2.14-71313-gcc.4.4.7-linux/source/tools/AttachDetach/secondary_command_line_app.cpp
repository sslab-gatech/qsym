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
   Test the scenario where Pin attaches to an application, using the command line,
   which one of its secondary thread is a zombie thread
   Pin wouldn't attach the zombie thread and wouldn't give a thread detach callbacks
   on detach to the zombie thread.
*/

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include <iostream>
#include "zombie_utils.h"

static int pipefd[2];

void* SecondaryThreadMain(void* v)
{
    close(pipefd[0]); // Close child's read end
    write(pipefd[1], const_cast<char *>("NotifyParent"), strlen(const_cast<char *>("NotifyParent")));
    close(pipefd[1]);// Close parent's write end
    pthread_exit(0);
}


// Expected argv arguments:
// [1] pin executable
// [2] -slow_assert
// [3] tool
// [4] output file	
int main(int argc, char *argv[])
{    
    if(argc!=5)
    {
       fprintf(stderr, "Not enough arguments\n" );
       fflush(stderr);
       exit(RES_INVALID_ARGS);
    }
    
    if( pipe(pipefd)== -1)         
    {         
        fprintf(stderr, "Pipe Failed.\n");  
        return RES_PIPE_CREATION_ERROR;
    } 

    // The pipe is used to transfer information from the
    // child process to the secondary thread.
    pid_t child_pid;
    pid_t parentPid = getpid();

    child_pid = fork ();
    
    if (child_pid > 0) 
    {  
        pthread_t tid; 
        pthread_create(&tid, NULL, SecondaryThreadMain, NULL);       
        while(1) sleep(1);
    }
    else 
    {  
        // In child
        char attachPid[MAX_SIZE];
        snprintf(attachPid ,MAX_SIZE , "%d", parentPid);
        
        close(pipefd[1]);// Close parent's write end
        int buf[2];
        if (read(pipefd[0], buf, 1) < 0)
        {
            exit(RES_PIPE_ERROR);
        }

        close(pipefd[0]);// Close parent's write end.

        // Pin attaches to the application.
        execl(argv[1], argv[1], argv[2], "-probe","-pid", attachPid,  "-t",  argv[3], "-o", argv[4], NULL); // never return
        
        exit(RES_EXEC_FAILED);
       
    }

    return RES_SUCCESS;
}
