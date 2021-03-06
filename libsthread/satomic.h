#if !defined(SATOMIC_H_INCLUDED)
#define SATOMIC_H_INCLUDED
/*
 *  Simple win32 threads - atomic operations
 *
 *  Copyright (c) 2020 - 2022, Adam Young.
 *  All rights reserved.
 *
 *  This file is part of oncrpc4-win32.
 *
 *  The applications are free software: you can redistribute it
 *  and/or modify it under the terms of the oncrpc4-win32 License.
 *
 *  Redistributions of source code must retain the above copyright
 *  notice, and must be distributed with the license document above.
 *
 *  Redistributions in binary form must reproduce the above copyright
 *  notice, and must include the license document above in
 *  the documentation and/or other materials provided with the
 *  distribution.
 *
 *  This project is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the Licence for details.
 *  ==end==
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
typedef long satomic_lock_t;
extern long __cdecl _InterlockedExchange(long volatile *, long);
#pragma intrinsic( _InterlockedExchange )
#define SATOMIC_INTERLOCK_EXCHANGE _InterlockedExchange

extern void _ReadWriteBarrier();
#pragma intrinsic( _ReadWriteBarrier )
#define SATOMIC_FENCE _ReadWriteBarrier();

#elif defined(__WATCOMC__)
typedef long satomic_lock_t;
#define SATOMIC_INTERLOCK_EXCHANGE InterlockedExchange

extern __inline void ReadWriteBarrier(void);
#pragma aux ReadWriteBarrier = "" parm [] modify exact [];
#define SATOMIC_FENCE ReadWriteBarrier();

#elif defined(__MINGW32__)
typedef long satomic_lock_t;
#if defined(__MINGW64_VERSION_MAJOR) /*MingGW-w64/32*/
#include <intrin.h>
#define SATOMIC_INTERLOCK_EXCHANGE _InterlockedExchange
#define SATOMIC_FENCE _ReadWriteBarrier();
#else
#define SATOMIC_INTERLOCK_EXCHANGE InterlockedExchange
#define SATOMIC_FENCE  asm volatile("": : :"memory");
#endif
#endif


static __inline int
satomic_try_lock(volatile satomic_lock_t *lock)
{
    long r = SATOMIC_INTERLOCK_EXCHANGE(lock, 1);
    SATOMIC_FENCE
    return (r == 0);
}


static __inline void
satomic_lock(volatile satomic_lock_t *lock)
{
    unsigned k = 0;
    while (! satomic_try_lock(lock)) {
        if (k < 4) {
        } else if (k < 32) {
            Sleep(0);
        } else {
            Sleep(1);
        }
        ++k;
    }
}


static __inline long
satomic_read(volatile satomic_lock_t *lock)
{
    SATOMIC_FENCE
    return *lock;
}


static __inline void
satomic_unlock(volatile satomic_lock_t *lock)
{
    SATOMIC_FENCE
    *lock = 0;
}

#ifdef __cplusplus
}
#endif

#endif /*SATOMIC_H_INCLUDED*/

/*end*/
