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

#if defined(__WATCOMC__)
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06000000                /* iphlpapi.h requirement */
#endif
#endif

/* winsock and friends */

struct netconfig;                               /* gcc, quiet struct warnings */

#if !defined(HAVE_WINSOCK2_H_INCLUDED)
#define HAVE_WINSOCK2_H_INCLUDED
#if !defined(_WINSOCK2_H)                       /* MINGW32 guard */

#undef gethostname                              /* unistd.h name mangling */
#if defined(u_char)
#undef u_char                                   /* namespace issues (_BSD_SOURCE) */
#endif

#if defined(__MINGW32__) && defined(SLIST_ENTRY)
#pragma push_macro("SLIST_ENTRY")               /* <sys/queue.h> */
#undef SLIST_ENTRY
#include <winsock2.h>
#pragma pop_macro("SLIST_ENTRY")
#else
#include <winsock2.h>
#undef SLIST_ENTRY
#endif

#include <ws2tcpip.h>                           /* getaddrinfo() */
#include <mswsock.h>                            /* IOCP */

#endif /*_WINSOCK2_H*/
#endif /*HAVE_WINSOCK2_H_INCLUDED*/

/* windows.h*/

#if !defined(HAVE_WINDOWS_H_INCLUDED)
#define HAVE_WINDOWS_H_INCLUDED
#ifndef WINDOWS_NOT_MEAN_AND_LEAN
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>
#endif /*HAVE_WINDOWS_H_INCLUDED*/

/* _countof */

#if !defined(_MSC_VER) && !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif


/*
 *  MINGW tweaks
 */

#if defined(__MINGW32__)
#include <errno.h>                              /* standard definitions */

#ifndef _CRT_NO_POSIX_ERROR_CODES               /* POSIX Supplement */
#define EWOULDBLOCK     140
#endif /*_CRT_NO_POSIX_ERROR_CODES*/

#endif /*__MINGW32__*/

/*
 *  WATCOMC compat tweaks
 */

#if defined(__WATCOMC__)
#include <errno.h>                              /* standard definitions */
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
#endif /*_CRT_NO_POSIX_ERROR_CODES*/

#endif /*__WATCOMC__*/

/*end*/

