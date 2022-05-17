#pragma once

/*
 * <sys/cdefs.h>
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

#ifndef __RCSID
#define __RCSID(__x)            /*ident helper*/
#endif

#ifndef __RENAME
#define __RENAME(__x)           /*linkage hlper*/
#endif

#ifndef __UNCONST
#define __UNCONST(__x)          (void *)(__x)
#endif

#ifndef __unused
#define __unused
#endif

#ifndef __dead
#define __dead
#endif

#define __arraycount(__x)       (sizeof(__x) / sizeof(__x[0]))

#if !defined(_DIAGASSERT)
#define _DIAGASSERT(__x)        assert(__x)
#endif


/* 
 *  Library binding.
 */
#if !defined(LIBRPC_API)
#if defined(LIBRPC_DYNAMIC) && defined(LIBRPC_STATIC)
#error LIBRPC_DYNAMIC and LIBRPC_STATIC are mutually exclusive
#endif
#if defined(LIBRPC_DYNAMIC)
    #if defined(LIBRPC_LIBRARY)     /* library source */
        #define LIBRPC_API __declspec(dllexport)
    #else
        #define LIBRPC_API __declspec(dllimport)
    #endif

#else   /*static*/
    #if defined(LIBRPC_LIBRARY)     /* library source */
        #ifndef LIBRPC_STATIC                   /* verify STATIC/DYNAMIC configuration */
        #error  LIBRPC static library yet LIB32_STATIC not defined.
        #endif
        #ifdef _WINDLL                          /*verify target configuration */
        #error  LIBRPC static library yet _WINDLL defined.
        #endif
    #endif
#endif
#ifndef LIBRPC_API
#define LIBRPC_API extern
#endif
#endif //!LIBW32_API

/* 
 *  Calling convention.
 */
#ifndef __BEGIN_DECLS
#  ifdef __cplusplus
#     define __BEGIN_DECLS      extern "C" {
#     define __END_DECLS        }
#  else
#     define __BEGIN_DECLS
#     define __END_DECLS
#  endif
#endif
#ifndef __P
#  if (__STDC__) || defined(__cplusplus) || \
         defined(_MSC_VER) || defined(__GNUC__) || defined(__WATCOMC__)
#     define __P(x)             x
#  else
#     define __P(x)             ()
#  endif
#endif

/*
 *  Remove const cast-away warnings
 */
#ifndef __DECONST
#define __DECONST(__t,__a)      ((__t)(const void *)(__a))
#endif
#ifndef __UNCONST
#define __UNCONST(__a)          ((void *)(const void *)(__a))
#endif

/*end*/
