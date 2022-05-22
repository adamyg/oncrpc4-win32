#pragma once

/*
 * <rpc_win32.h>
 *
 * Copyright (c) 2022, Adam Young.
 * All rights reserved.
 *
 * This file is part of oncrpc4-win32.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the oncrpc4-win32 License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Licence for details.
 * ==end==
 */
 
/*
 *  WinSock/Windows definitions
 */ 

#include "rpc_config.h"

#if !defined(HAVE_WINSOCK2_H_INCLUDED)
#define HAVE_WINSOCK2_H_INCLUDED
#if defined(gethostname)
#undef gethostname                              /* unistd.h name mangling */
#endif
#if defined(u_char)
#undef u_char                                   /* namespace issues (_BSD_SOURCE) */
#endif

#if defined(__WATCOMC__)
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06000000                /* iphlpapi.h requirement */
#endif
#endif

#include <winsock2.h>
#include <ws2tcpip.h>                           /* getaddrinfo() */
#include <iphlpapi.h>                           /* if_nametoindex */
#if defined(__WATCOMC__)
#include <ws2tcpip.h>                           /* inet_pton() .. */
#endif
#include <mswsock.h>                            /* IOCP */
#endif /*HAVE_WINSOCK2_H_INCLUDED*/
                                     
#if !defined(HAVE_WINDOWS_H_INCLUDED)
#define HAVE_WINDOWS_H_INCLUDED
#ifndef WINDOWS_NOT_MEAN_AND_LEAN
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>
#endif /*HAVE_WINDOWS_H_INCLUDED*/

#if defined(__WATCOMC__)                        /* missing definitions */
ULONG WINAPI if_nametoindex(PCSTR InterfaceName);
#endif

/*
 *  WATCOMC MSVC compat tweaks
 */ 
 
#if defined(__WATCOMC__)

#if !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#include <errno.h>                              /* standard definitions */

#if !defined(HAVE_TIMESPEC)
#define HAVE_TIMESPEC
#endif
#if !defined(_TIMESPEC_DEFINED) && (__WATCOMC__ < 1300)
#define _TIMESPEC_DEFINED                       /* OWC1.9=1290, OWC2.0=1300 */
struct timespec {
        time_t tv_sec;
        long tv_nsec;
};
#else
#include <signal.h>
#endif  /*TIMESPEC_STRUCT_T*/

extern int _getmaxstdio(void);

#ifndef _CRT_NO_POSIX_ERROR_CODES               /* POSIX Supplement */
#define EADDRINUSE      100
#define EADDRNOTAVAIL   101
#define EAFNOSUPPORT    102
#define EALREADY        103
#define EBADMSG         104
#define ECANCELED       105
#define ECONNABORTED    106
#define ECONNREFUSED    107
#define ECONNRESET      108
#define EDESTADDRREQ    109
#define EHOSTUNREACH    110
#define EIDRM           111
#define EINPROGRESS     112
#define EISCONN         113
#define ELOOP           114
#define EMSGSIZE        115
#define ENETDOWN        116
#define ENETRESET       117
#define ENETUNREACH     118
#define ENOBUFS         119
#define ENODATA         120
#define ENOLINK         121
#define ENOMSG          122
#define ENOPROTOOPT     123
#define ENOSR           124
#define ENOSTR          125
#define ENOTCONN        126
#define ENOTRECOVERABLE 127
#define ENOTSOCK        128
#define ENOTSUP         129
#define EOPNOTSUPP      130
#define EOTHER          131
#define EOVERFLOW       132
#define EOWNERDEAD      133
#define EPROTO          134
#define EPROTONOSUPPORT 135
#define EPROTOTYPE      136
#define ETIME           137
#define ETIMEDOUT       138
    //#define ETXTBSY   139
#define EWOULDBLOCK     140
#endif // _CRT_NO_POSIX_ERROR_CODES

#endif //__WATCOMC__

/*end*/
