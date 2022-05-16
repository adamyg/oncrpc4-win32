#pragma once

/*
 * <reentrant.h>
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

#include "../../libsthread/sthread.h"

#ifdef _REENTRANT

/*
 * Abtract thread interface for thread-safe libraries.  These routines
 * will use stubs in libc if the application is not linked against the
 * pthread library, and the real function in the pthread library if it
 * is.
 */

#define thread_key_t		pthread_key_t
#define mutex_t			pthread_mutex_t
#define cond_t			pthread_cond_t
#define rwlock_t		pthread_rwlock_t
#define once_t			pthread_once_t

#define MUTEX_INITIALIZER	PTHREAD_MUTEX_INITIALIZER
#define COND_INITIALIZER	PTHREAD_COND_INITIALIZER
#define RWLOCK_INITIALIZER	PTHREAD_RWLOCK_INITIALIZER
#define ONCE_INITIALIZER	PTHREAD_ONCE_INIT

	//extern const unsigned __isthreaded;
#define __isthreaded		1

#define mutex_init(m, a)	pthread_mutex_init(m, a)
#define mutex_lock(m)		if (__isthreaded) pthread_mutex_lock(m)
#define mutex_unlock(m)		if (__isthreaded) pthread_mutex_unlock(m)
#define mutex_trylock(m)	(__isthreaded ? 0 : pthread_mutex_trylock(m))

#define cond_init(c, t, a)	pthread_cond_init((c), (a))
#define cond_signal(c)		pthread_cond_signal((c))
#define cond_broadcast(c)	pthread_cond_broadcast((c))
#define cond_wait(c, m)		pthread_cond_wait((c), (m))
#define cond_timedwait(c, m, t)	pthread_cond_timedwait((c), (m), (t))
#define cond_destroy(c)		pthread_cond_destroy((c))

#define rwlock_init(l, a)	pthread_rwlock_init(l, a)
#define rwlock_rdlock(l)	if (__isthreaded) pthread_rwlock_rdlock(l)
#define rwlock_wrlock(l)	if (__isthreaded) pthread_rwlock_wrlock(l)
#define rwlock_unlock(l)	if (__isthreaded) pthread_rwlock_unlock(l)

#define thr_keycreate(k, d)	pthread_key_create(k, d)
#define thr_setspecific(k, p)	pthread_setspecific(k, p)
#define thr_getspecific(k)	pthread_getspecific(k)
#define thr_keydelete(k)	pthread_key_delete(k)

#define thr_once(o, f)		pthread_once(o, f)

#else   /*_REENTRANT*/

#define thread_key_t int
#define mutex_t int
#define cond_t int
#define rwlock_t int
#define once_t int

#define __isthreaded 0

#define mutex_init(m, a)
#define mutex_lock(m)
#define mutex_unlock(m)
#define mutex_trylock(m)

#define cond_init(c, t, a)
#define cond_signal(c)
#define cond_broadcast(c)
#define cond_wait(c, m)
#define cond_timedwait(c, m, t)
#define cond_destroy(c)

#define rwlock_init(l, a)
#define rwlock_rdlock(l)
#define rwlock_wrlock(l)
#define rwlock_unlock(l)

#define thr_keycreate(k, d)
#define thr_setspecific(k, p)
#define thr_getspecific(k)
#define thr_keydelete(k)

#define thr_once(o, f)

#endif  /*_REENTRANT*/

#define thr_sigsetmask(a, b, c)

/*end*/
