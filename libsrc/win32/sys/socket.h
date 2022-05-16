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
#include <rpc_win32.h>

#if defined(_WIN32)
#define SOCKLEN_SOCKADDR(__sa) (AF_INET6 == __sa.sa_family ? sizeof(struct sockaddr_in6) :  sizeof(struct sockaddr_in))
#define SOCKLEN_SOCKADDR_PTR(__sa) (AF_INET6 == __sa->sa_family ? sizeof(struct sockaddr_in6) :  sizeof(struct sockaddr_in))
#define SOCKLEN_SOCKADDR_STORAGE(__ss) (AF_INET6 == __ss.ss_family ? sizeof(struct sockaddr_in6) :  sizeof(struct sockaddr_in))
#define SOCKLEN_SOCKADDR_STORAGE_PTR(__ss) (AF_INET6 == __ss->ss_family ? sizeof(struct sockaddr_in6) :  sizeof(struct sockaddr_in))
#else
#define SOCKLEN_SOCKADDR(__sa) __sa.sa_len
#define SOCKLEN_SOCKADDR_PTR(__sa) __sa->sa_len
#define SOCKLEN_SOCKADDR_STORAGE(__ss) __ss.ss_len
#define SOCKLEN_SOCKADDR_STORAGE_PTR(__ss) __ss->ss_len
#endif

/*end*/
