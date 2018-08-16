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
/**
 * This application prints all the text and data segments of every image 
 * loaded using the dyld interfaces.
 * For more details on the test see the comment in split_image.cpp
 */
#include <cstdio>
#include <cstdlib>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

#if defined(TARGET_IA32)
# define SPLIT_IMAGE_MACH_HEADER const struct mach_header
#else
# define SPLIT_IMAGE_MACH_HEADER const struct mach_header_64
#endif

int main()
{
    for (int i=0; i < _dyld_image_count(); ++i)
    {
        unsigned long data_len, text_len;
        void *data_seg = getsegmentdata((SPLIT_IMAGE_MACH_HEADER *)_dyld_get_image_header(i), "__DATA", &data_len);
        void *text_seg = getsegmentdata((SPLIT_IMAGE_MACH_HEADER *)_dyld_get_image_header(i), "__TEXT", &text_len);
        if (data_seg) 
            fprintf(stderr, "%s, %p-%p\n", _dyld_get_image_name(i), data_seg, ((char*)data_seg) + data_len - 1);
        if (text_seg)
            fprintf(stderr, "%s, %p-%p\n", _dyld_get_image_name(i), text_seg, ((char*)text_seg) + text_len - 1);
    }
    return 0;
}
