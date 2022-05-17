#pragma once

/*
 * <ifaddrs.h>
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

struct ifaddrs {
#define	IFNAMSIZ	 16

	struct ifaddrs  *ifa_next;
	char		*ifa_name;
	unsigned int	 ifa_flags;
	struct sockaddr	*ifa_addr;
	struct sockaddr	*ifa_netmask;
	struct sockaddr	*ifa_dstaddr;
#ifndef	ifa_broadaddr
#define	ifa_broadaddr	 ifa_dstaddr	/* broadcast address interface */
#endif
	void		*ifa_data;
	unsigned int	 ifa_addrflags;
};

__BEGIN_DECLS
LIBRPC_API int getifaddrs(struct ifaddrs **);
LIBRPC_API void freeifaddrs(struct ifaddrs *);
__END_DECLS

/*end*/
