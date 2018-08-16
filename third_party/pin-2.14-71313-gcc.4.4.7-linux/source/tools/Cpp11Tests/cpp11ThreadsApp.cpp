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
#include <iostream>
#include <thread>
#include <vector>


using namespace std;

int num_threads = 4;
int vector_size = 1024;

void thread_func (int pid, vector<int> &v, int scalar, int L, int R) {
    cout << "Thread " << pid << endl;
    for (int i=L; i<R; i++) {
        v[i] = scalar;
    }
}

bool verify_vector(const vector<int> v, int scalar) {
    for (int i = 0; i < vector_size; i++){
        if (v[i] != scalar) {

            return false;
        }
    }
    return true;
}

int main() {
    std::thread threads[num_threads];
    std::vector<int> v(vector_size);
    int scalar = 50;
    int chunk = vector_size/ num_threads;

    for (int i = 0; i < num_threads; ++i) {
        threads[i] = thread(thread_func, i, std::ref(v), scalar, i*chunk, (i+1)*chunk);
    }

    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
    }

    if (!verify_vector(v, scalar)) {
        cout << "Found a cell not initialized! " << endl;
    } else {
        cout << "All vector is initialized to scalar" << endl;
    }

    //restarting threads to initialize vectore to new scalar
    scalar = 20;
    for (int i = 0; i < num_threads; ++i) {
        threads[i] = thread(thread_func, i, std::ref(v), scalar, i*chunk, (i+1)*chunk);
    }

    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
    }

    if (!verify_vector(v, scalar)) {
        cout << "Found a cell not initialized! " << endl;
    } else {
        cout << "All vector is initialized to scalar" << endl;
    }


    return 0;
}


