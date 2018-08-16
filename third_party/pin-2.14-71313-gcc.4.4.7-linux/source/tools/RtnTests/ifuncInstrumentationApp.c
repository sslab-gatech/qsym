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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

int main(int argv, char** argc, char** env){
    struct timeval tiv;
   
    gettimeofday(&tiv, NULL);
    printf("gettimeofday : %d\n", (int) tiv.tv_sec);
 
    gettimeofday(&tiv, NULL);
    printf("gettimeofday : %d\n", (int)tiv.tv_sec);
   

    gettimeofday(&tiv, NULL);
    printf("gettimeofday : %d\n", (int)tiv.tv_sec);

    char a[50] = {'5'};
    char b[50] = {'0'};

    printf("a : %s, b : %s\n", a, b);
    int cmp_result = strcmp(a,b);
    if (cmp_result >0 )
        printf("a>b\n");
    else if (cmp_result <0 )
        printf("a<b\n");
    else
        printf("a==b\n");

    
    return 0;
}
