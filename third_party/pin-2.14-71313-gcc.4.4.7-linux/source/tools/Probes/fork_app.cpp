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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

/***************************************************************************/

static void event_on_process_fork_before(void)
{
    fprintf(stdout, "event_on_process_fork_before\n"); fflush(stdout);
}

static void event_on_process_fork_after_in_parent(void)
{
    fprintf(stdout, "event_on_process_fork_after_in_parent\n"); fflush(stdout);
    FILE* fd = fopen("father.log", "w");
    fprintf(fd, "This is temporary log written from within pthread_atfork hook");
    fclose(fd);
}

static void event_on_process_fork_after_in_child(void)
{
    fprintf(stdout, "event_on_process_fork_after_in_child\n"); fflush(stdout);
    FILE* fd = fopen("child.log", "w");
    fprintf(fd, "This is temporary log written from within pthread_atfork hook");
    fclose(fd);
}

/***************************************************************************/

int testFork()
{
	pthread_atfork(event_on_process_fork_before,
		event_on_process_fork_after_in_parent,
		event_on_process_fork_after_in_child);
	if (fork())
	{
		int res = 0;
		
		fprintf(stdout, "after fork before wait of child\n"); fflush(stdout);
		wait(&res);
	}
	else
	{
		fprintf(stdout, "from child before exit\n"); fflush(stdout);
		exit(0);
	}

	return 0;
}

/***************************************************************************/

int main ( )
{

	fprintf(stdout, "main before fork\n"); fflush(stdout);

	testFork();

	fprintf(stdout, "main after fork\n"); fflush(stdout);

	return 0;
}

