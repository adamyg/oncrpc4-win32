/*
 * gettimeofday
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

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <sys/cdefs.h>
#include <sys/time.h>
#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/timeb.h>
#endif
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "rpc_win32.h"

/*
//  NAME
//
//      gettimeofday - get the date and time
//
//  SYNOPSIS
//
//      #include <sys/time.h>
//
//      int gettimeofday(struct timeval *restrict tp, void *restrict tzp);
//
//  DESCRIPTION
//
//      The gettimeofday() function shall obtain the current time, expressed aa seconds and 
//      microseconds since the Epoch, and store it in the timeval structure pointed to by tp.
//      The resolution of the system clock is unspecified.
//
//      If tzp is not a null pointer, the behavior is unspecified.
//
//  RETURN VALUE
//      The  gettimeofday()  function  shall  return 0 and no value shall
//      be reserved to indicate an error.
//
//  ERRORS
//      No errors are defined.
//
*/

int
gettimeofday(struct timeval *tv, /*struct timezone*/ void *tz)
{
    if (tv) {
#if defined(_MSC_VER) || defined(__WATCOMC__)
        struct _timeb lt;

        _ftime(&lt);
        tv->tv_sec = (long)(lt.time + lt.timezone);
        tv->tv_usec = lt.millitm * 1000;
        assert(4 == sizeof(tv->tv_sec));

#elif defined(__MINGW32__)
#undef gettimeofday
        return gettimeofday(tv, tz)

#else //DEFAULT
        FILETIME ft;
        long long hnsec;

        (void) GetSystemTimeAsFileTime(&ft);
        hnsec = filetime_to_hnsec(&ft);
        tv->tv_sec = hnsec / 10000000;
        tv->tv_usec = (hnsec % 10000000) / 10;
#endif
        return 0;
    }

    errno = EINVAL;
    return -1;
}

/*end*/
