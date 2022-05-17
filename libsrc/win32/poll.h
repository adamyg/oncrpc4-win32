#pragma once

/*
 * <poll.h>
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

__BEGIN_DECLS
LIBRPC_API int rpc_poll(struct pollfd *fds, nfds_t nfds, int timeout);
LIBRPC_API int rpc_pollts(struct pollfd * fds, nfds_t nfds, const struct timespec * ts, const sigset_t *sigmask);
__END_DECLS

#define poll(a__, b__, c__)		rpc_poll(a__, b__, c__)
#define pollts(a__, b__, c__, d__)	rpc_pollts(a__, b__, c__, d__)

/*end*/
