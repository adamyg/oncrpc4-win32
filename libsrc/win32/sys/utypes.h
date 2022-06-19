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

#if defined(_MSC_VER)
#pragma warning(disable:4115)

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error utypes.h: old WATCOM Version, upgrade to OpenWatcom ...
#endif

#elif defined(__MINGW32__)

#else
#error utypes.h: Unknown compiler
#endif

#include <sys/types.h>                          /* System types */
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
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef signed ssize_t;
#endif
#endif

typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

#if !defined(__MINGW32__) || \
    defined(__MINGW64_VERSION_MAJOR) /*MingGW-w64/32*/
typedef struct {
	uint32_t __bits[4];
} sigset_t;
#endif

#if defined(__WATCOMC__)
typedef unsigned nfds_t;
#else
typedef size_t nfds_t;
#endif

typedef char *caddr_t;

#if defined(_MSC_VER) || defined(__MINGW32__)
typedef int gid_t;
typedef int uid_t;
typedef int id_t;
#endif

#if defined(_MSC_VER) || \
        (defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR))
typedef int pid_t;
#endif

/*end*/
