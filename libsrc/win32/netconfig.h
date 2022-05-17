#pragma once

/*
 * <netconfig.h>
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

#define NETCONFIG	"/etc/netconfig"
#define NETPATH		"NETPATH"

struct netconfig {
	char *nc_netid;				/* Network ID */
	unsigned long nc_semantics;		/* Semantics (see below) */
	unsigned long nc_flag;			/* Flags (see below) */
	char *nc_protofmly;			/* Protocol family */
	char *nc_proto;				/* Protocol name */
	char *nc_device;			/* Network device pathname (unused) */
	unsigned long nc_nlookups;		/* Number of lookup libs (unused) */
	char **nc_lookups;			/* Names of the libraries (unused) */
	unsigned long nc_unused[9];		/* reserved */
};

typedef struct {
	struct netconfig **nc_head;
	struct netconfig **nc_curr;
} NCONF_HANDLE;

/*
 * nc_semantics values
 */
#define NC_TPI_CLTS	1
#define NC_TPI_COTS	2
#define NC_TPI_COTS_ORD	3
#define NC_TPI_RAW	4

/*
 * nc_flag values
 */
#define NC_NOFLAG	0x00
#define NC_VISIBLE	0x01
#define NC_BROADCAST	0x02

/*
 * nc_protofmly values
 */
#define NC_NOPROTOFMLY	"-"
#define NC_LOOPBACK	"loopback"
#define NC_INET		"inet"
#define NC_INET6	"inet6"
/*
#define NC_IMPLINK	"implink"
#define NC_PUP		"pup"
#define NC_CHAOS	"chaos"
#define NC_NS		"ns"
#define NC_NBS		"nbs"
#define NC_ECMA		"ecma"
#define NC_DATAKIT	"datakit"
#define NC_CCITT	"ccitt"
#define NC_SNA		"sna"
#define NC_DECNET	"decnet"
#define NC_DLI		"dli"
#define NC_LAT		"lat"
#define NC_HYLINK	"hylink"
#define NC_APPLETALK	"appletalk"
#define NC_NIT		"nit"
#define NC_IEEE802	"ieee802"
#define NC_OSI		"osi"
#define NC_X25		"x25"
#define NC_OSINET	"osinet"
#define NC_GOSIP	"gosip"
*/

/*
 * nc_proto values
 */
#define NC_NOPROTO	"-"
#define NC_TCP		"tcp"
#define NC_UDP		"udp"
#define NC_ICMP		"icmp"

__BEGIN_DECLS
LIBRPC_API struct netconfig *getnetpath(void *handlep);
LIBRPC_API void *setnetpath(void);
LIBRPC_API int endnetpath(void *handlep);

LIBRPC_API struct netconfig *getnetconfig(void *handlep);
LIBRPC_API void *setnetconfig(void);
LIBRPC_API int endnetconfig(void *handlep);
LIBRPC_API struct netconfig *getnetconfigent(const char *netid);
LIBRPC_API void freenetconfigent(struct netconfig *netconfigp);
LIBRPC_API char *nc_sperror(void);
LIBRPC_API void nc_perror(const char *msg);

__END_DECLS

/*end*/
