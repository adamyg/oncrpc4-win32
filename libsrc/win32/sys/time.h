#pragma once

/*
 * <sys/time.h>
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
#if defined(__MINGW32__)
#include_next <sys/time.h>              /* struct timeval */
#else
#include <sys/socket.h>                 /* struct timeval */
#endif
#include <sys/utypes.h>                 /* suseconds_t */
#include <time.h>

#if defined(__WATCOMC__)                        /* missing definitions */
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
#endif

#if !defined(TIMEVAL_TO_TIMESPEC)
#define TIMEVAL_TO_TIMESPEC(tv, ts) {       \
    (ts)->tv_sec = (tv)->tv_sec;            \
    (ts)->tv_nsec = (tv)->tv_usec * 1000;   \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {       \
    (tv)->tv_sec = (ts)->tv_sec;            \
    (tv)->tv_usec = (ts)->tv_nsec / 1000;   \
}
#endif

/* Operations on timevals. */
#if !defined(timerisset)
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#endif

#if !defined(timercmp)
#define timercmp(tvp, uvp, cmp)             \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?     \
        ((tvp)->tv_usec cmp (uvp)->tv_usec) : \
        ((tvp)->tv_sec cmp (uvp)->tv_sec))
#endif

#if !defined(timerclear)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#define timeradd(tvp, uvp, vvp)             \
    do {                                    \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
        (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec; \
        if ((vvp)->tv_usec >= 1000000) {    \
            (vvp)->tv_sec++;                \
            (vvp)->tv_usec -= 1000000;      \
        }                                   \
    } while (0)

#define timersub(tvp, uvp, vvp)             \
    do {                                    \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
        if ((vvp)->tv_usec < 0) {           \
            (vvp)->tv_sec--;                \
            (vvp)->tv_usec += 1000000;      \
        }                                   \
    } while (0)

#include <sys/cdefs.h>

__BEGIN_DECLS
int gettimeofday(struct timeval *tv, /*struct timezone*/ void *tz);
__END_DECLS
