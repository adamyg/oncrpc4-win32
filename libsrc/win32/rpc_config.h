#pragma once

/*
 * <rpc_config.h>
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

#if !defined(_CRT_NONSTDC_NO_DEPRECATE)
    /*
     *  disable deprecate warnings, strdup()
     */
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#if !defined(_CRT_SECURE_NO_DEPRECATE)
    /*
     *  disable deprecate warnings, strncpy(), strncat(), strcpy(), getenv()
     */
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

#if !defined(_WINSOCK_DEPRECATED_NO_WARNINGS)
#define _WINSOCK_DEPRECATED_NO_WARNINGS         /* disable deprecate warnings */
#endif

#if defined(_MSC_VER)
#pragma warning (disable : 4127)                // conditional expression is constant
#pragma warning (disable : 4702)                // unreachable code
#elif defined(__WATCOMC__)
#pragma disable_message(201)                    // unreachable code
#pragma disable_message(202)                    // symbol 'xxx' has been defined, but not referenced
#if !defined(__cplusplus)
#pragma disable_message(136)                    // comparison equivalent to 'unsigned == 0'
#pragma disable_message(303)                    // parameter 'xxx' has been defined, but not referenced.
#endif
#endif

/*end*/

