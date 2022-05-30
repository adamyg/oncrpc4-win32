#pragma once

/*
 * <sys/un.h>
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

#include <sys/socket.h>

struct sockaddr_un {
#if defined(__MINGW32__)
	short sun_family;		/* AF_LOCAL */
#else
	ADDRESS_FAMILY sun_family;	/* AF_LOCAL */
#endif
	unsigned char sun_len;		/* length of sockaddr struct */
	char sun_path[108];		/* pathname */
};
#define SUN_LEN(su) (unsigned char)(sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))

#define NGROUPS 1

struct sockcred {
	uid_t sc_uid;			/* real user id */
	uid_t sc_euid;			/* effective user id */
	gid_t sc_gid;			/* real group id */
	gid_t sc_egid;			/* effective group id */
	int   sc_ngroups;		/* number of supplemental groups */
	gid_t sc_groups[1];		/* variable length */
};

/*end*/
