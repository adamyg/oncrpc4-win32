#pragma once

/*
 * <inttypes.h> / <sys/utypes.h>
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

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

typedef long long longlong_t;
typedef unsigned long long u_longlong_t;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

#if !defined(_SSIZE_T_DEFINED_)
#define _SSIZE_T_DEFINED_
typedef signed ssize_t;
#endif

typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

typedef struct {
	uint32_t __bits[4];
} sigset_t;

#if defined(__WATCOMC__)
typedef unsigned nfds_t;
#else
typedef size_t nfds_t;
#endif

typedef char *caddr_t;

#if defined(_MSC_VER)
typedef int gid_t;
typedef int uid_t;
typedef int pid_t;
#endif

/*end*/
