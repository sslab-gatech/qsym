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

#include "interactive_listener.H"

#if !defined(TARGET_WINDOWS)
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>

#if !defined(SYS_close)
#  include <asm/unistd.h>
#  define SYS_close __NR_close
#endif

#endif

#if !defined(TARGET_WINDOWS)
using namespace CONTROLLER;

VOID INTERACTIVE_LISTENER::Active(){
    _main_pid = PIN_GetPid();
    PIN_AddForkFunction(FPOINT_AFTER_IN_CHILD, AfterForkInChild, this);
   
    //create new thread that will listen to the socket
    PIN_SpawnInternalThread(WaitForUserSiganl, this, 0 ,NULL);
    PIN_AddFiniFunction(Fini,this);
    PIN_AddSyscallEntryFunction(MonitorFD,this);
}

// create the socket and listen to it
UINT32 INTERACTIVE_LISTENER::OpenSocket(){

    UINT32 pid = PIN_GetPid();
    _full_file = _file_name + "." + decstr(pid);
    UINT32 res;
    if (pid == _main_pid){
        printf("Main process pid: %d\n", pid);
    }
    else{
        printf("New process detected: %d\n", pid);
    }
    printf("  ** using file: %s\n", _full_file.c_str());

    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    _server_sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(_server_sock >= 0, "Failed to create socket, errno " + decstr(errno));

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(0); // any port

    /* Now bind the host address using bind() call.*/
    res = bind(_server_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    ASSERT(res >= 0, "bind() failed, errno " + decstr(errno));

    res = getsockname(_server_sock, (struct sockaddr *)&serv_addr, &addr_len);
    ASSERT(0 == res, "getsockname() failed, errno " + decstr(errno));

    res = listen(_server_sock, 3);
    ASSERT(0 == res, "listen() failed, errno " + decstr(errno));

    printf("  ** listening to port: %d\n", ntohs(serv_addr.sin_port));
    
    // writing the used port number to the file in Jason format.
    // e.g. {port:111}
    std::fstream fs;
    fs.open (_full_file.c_str(), std::fstream::out | std::fstream::trunc);

    fs << "{\"port\" : \"" << ntohs(serv_addr.sin_port) << "\"}";

    fs.close();
    
    return _server_sock;
}

// After fork,
// need to create new thread and listen to new fifo file with child pid suffix
VOID INTERACTIVE_LISTENER::AfterForkInChild(THREADID tid, const CONTEXT* ctxt, void* v){
    INTERACTIVE_LISTENER* l = static_cast<INTERACTIVE_LISTENER*>(v);
    close(l->_server_sock); //close the parents file descriptor
    PIN_SpawnInternalThread(WaitForUserSiganl, l, 0 ,NULL);
}

VOID INTERACTIVE_LISTENER::MonitorFD(THREADID tid, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v){

     INTERACTIVE_LISTENER* listener = static_cast<INTERACTIVE_LISTENER*>(v);
     ADDRINT syscall = PIN_GetSyscallNumber(ctxt, std);
     if (syscall == SYS_close){
         ADDRINT fd = PIN_GetSyscallArgument(ctxt, std, 0);
         if (fd == listener->_server_sock){
             //skip closing controllers fd
             PIN_SetSyscallArgument(ctxt, std, 0, static_cast<ADDRINT>(-1));
         }
     }
}

VOID INTERACTIVE_LISTENER::Fini(INT32, VOID* v){
    INTERACTIVE_LISTENER* l = static_cast<INTERACTIVE_LISTENER*>(v);
    close(l->_server_sock);
    unlink(l->_full_file.c_str());
}
 

VOID INTERACTIVE_LISTENER::WaitForUserSiganl(VOID* v){
    /* this function creates the server socket and listens to it.
     * then it checks if there is info ready to be read from the socket.
     * once in 0.5 sec we are checking if PIN is exiting in order to 
     * close the file descriptor and exit the loop.
     */
    INTERACTIVE_LISTENER* listener = static_cast<INTERACTIVE_LISTENER*>(v);
    
    UINT32 fd = listener->OpenSocket();
   
    char buf[1];
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Wait up to 0.5 seconds. */
    tv.tv_sec = 0;
    tv.tv_usec = 500000; //time in microsec.

    while (1){
        if (PIN_IsProcessExiting()){
            close(fd);
            return;
        }
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        memset(buf,0,1);
        retval = select(fd+1, &rfds, NULL, NULL, &tv);
        if(retval == -1){
            ASSERT(FALSE,"error in select function, errno: " + decstr(errno));
        }
        else if (retval){
            struct sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);

            int sessionSock = accept(fd, (struct sockaddr *)&cli_addr, &clilen);
            if (sessionSock >= 0)
            {
                int res = read(sessionSock,buf,1);
                if (res > 0)
                {
                    if(buf[0] == '1')
                    {
                        listener->_signaled = 1;
                    }
                }
                close(sessionSock);
            }
        }
    }
}

#endif
