#pragma once

/*
 * <sys/socket.h>
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

#include <sys/cdefs.h>
#include <sys/utypes.h>
#if !defined(_WINSOCK2_H)                       /* MINGW32 guard */
#include <rpc_win32.h>
#endif

/* missing definitions */

#if defined(_MSC_VER) || \
    defined(__MINGW64_VERSION_MAJOR) /* MingGW-w64/32 */
#include <Iphlpapi.h>                           /* if_nametoindex() */
#endif

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
INT WSAAPI inet_pton(INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
PCSTR WSAAPI inet_ntop(INT Family, const VOID *pAddr, PSTR pStringBuf, size_t StringBufSize);

ULONG WINAPI if_nametoindex(PCSTR InterfaceName);

typedef struct addrinfo ADDRINFOA, *PADDRINFOA;
INT WSAAPI getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA *pHints, PADDRINFOA *ppResult);
VOID WSAAPI freeaddrinfo(PADDRINFOA pAddrInfo);
#endif

#if defined(__WATCOMC__)
#if !defined(HAVE_TIMESPEC)                     /* missing definitions */
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

ULONG WINAPI if_nametoindex(PCSTR InterfaceName);
#endif

/*end*/
