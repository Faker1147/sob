//
// Copyright (C) 2013, David Anderson and AlliedModders LLC
// All rights reserved.
// 
// Original code from AMTL: https://github.com/alliedmodders/amtl/blob/2d3b1a3378a3728637f26660c9ffc2df3189cf62/amtl/am-platform.h
// Borrowed for SOB
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//  * Neither the name of AlliedModders LLC nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef __SOB_PLATFORM_H_
#define __SOB_PLATFORM_H_

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || \
    (defined(_MSC_VER) && defined(_M_X64))
#    define SOB_ARCH_X64
#    define SOB_ARCH_TAG "x86-64"
#elif defined(__i686) || defined(__i686__) || defined(__i386) || defined(__i386__) || \
    defined(i386) || (defined(_MSC_VER) && defined(_M_IX86))
#    define SOB_ARCH_X86
#    define SOB_ARCH_TAG "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#    define SOB_ARCH_ARM64
#    define SOB_ARCH_TAG "ARM64"
#elif defined(__arm__) || defined(_M_ARM)
#    define SOB_ARCH_ARM
#    define SOB_ARCH_TAG "ARM"
#else
#    define SOB_ARCH_UNKNOWN
#    define SOB_ARCH_TAG "UNKNOWN-ARCH"
#endif

#endif // !__SOB_PLATFORM_H_

