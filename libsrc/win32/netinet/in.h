#pragma once

/*
 * <netinet/in.h> - Internet address family
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

/*
 *  in_port_t   - Equivalent to the type uint16_t as defined in <inttypes.h>.
 *  in_addr_t   - Equivalent to the type uint32_t as defined in <inttypes.h>.
 */

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

/*end*/
