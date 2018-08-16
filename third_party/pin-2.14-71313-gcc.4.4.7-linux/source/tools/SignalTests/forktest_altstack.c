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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

FILE* log_fd;

void PrintAndCompareAlternateStack(const char* prefix, const stack_t* compare_stack) {
    stack_t sigstk;

    int sigalt_result = sigaltstack(NULL, &sigstk);

    fprintf(log_fd,"%s: sigalt_result == %d {ss_sp=%p; ss_size=%d; ss_flags=%x}\n",
            prefix, sigalt_result, sigstk.ss_sp, (int)sigstk.ss_size, sigstk.ss_flags);
    fflush(log_fd);

    if (sigalt_result < 0){
        perror("ERROR printing alternate stack");
    }
    if (!sigstk.ss_sp) {
        fprintf(stderr,"%s: NO alternate stack\n", prefix);
    }
    if (compare_stack) {
        fprintf(stderr,"%s: current stack %s compare_stack\n",
                prefix,
                ((sigstk.ss_sp == compare_stack->ss_sp) && sigstk.ss_size == compare_stack->ss_size)?
                "equals to":"different from");
    }
    fflush(stderr);
}

int main(int argc, char *argv[]) {
    int err, i;

    if (argc != 2) {
        printf("Usage %s <log_file_name>\n", argv[0]);
        exit(1);
    }

    log_fd = fopen(argv[1], "a");
    if (log_fd < 0 ) {
        perror("Could not open log file");
        exit(1);
    }
    fprintf(log_fd, "Start running forktest_altstack.c\n");

    // set current alternate stack
    stack_t sigstk;
    sigstk.ss_sp = malloc(SIGSTKSZ);
    void * ss_sp = sigstk.ss_sp;

    if (sigstk.ss_sp == NULL) { 
        fprintf(stderr,"Could not allocate ss_sp\n");
        return -1;
    }
    sigstk.ss_size = SIGSTKSZ;
    sigstk.ss_flags = 0;

    int sigalt_result = sigaltstack(&sigstk, NULL);

    PrintAndCompareAlternateStack("Father", NULL);

    int pid = fork();
    if (pid < 0 ) {
        perror("Couldn't fork");
        exit(1);
    } else if (pid == 0) {
        //child process - print sigaltstack
        PrintAndCompareAlternateStack("child process printing" , &sigstk);
        fclose(log_fd);
    } else {
        //pid > 0 => father process (free pointer)
        if (ss_sp) free(ss_sp);
        fclose(log_fd);
    }

    return 0;
}


