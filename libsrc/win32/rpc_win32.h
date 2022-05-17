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

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

#if !defined(_WINSOCK_DEPRECATED_NO_WARNINGS)
#define _WINSOCK_DEPRECATED_NO_WARNINGS         /* disable deprecate warnings */
#endif

#if !defined(HAVE_WINSOCK2_H_INCLUDED)
#define HAVE_WINSOCK2_H_INCLUDED
#if defined(gethostname)
#undef gethostname                              /* unistd.h name mangling */
#endif
#if defined(u_char)
#undef u_char                                   /* namespace issues (_BSD_SOURCE) */
#endif

#include <winsock2.h>
#include <ws2tcpip.h>                           /* getaddrinfo() */
#include <iphlpapi.h> //<netioapi.h>            /* if_nametoindex */
#include <mswsock.h>                            /* IOCP */
#endif /*HAVE_WINSOCK2_H_INCLUDED*/

#if !defined(HAVE_WINDOWS_H_INCLUDED)
#define HAVE_WINDOWS_H_INCLUDED
#ifndef WINDOWS_NOT_MEAN_AND_LEAN
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>
#endif /*HAVE_WINDOWS_H_INCLUDED*/

/*end*/
