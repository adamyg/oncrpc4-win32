#pragma once

/*
 * <win32_socket.h>
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
#include <sys/socket.h>
#include <sys/un.h>

__BEGIN_DECLS

LIBRPC_API int wsainitialise(void);

LIBRPC_API int wsaerrno_map(int nerrno);
LIBRPC_API int wsaerrno(void);
LIBRPC_API const char *wsastrerror(int xerrno);

LIBRPC_API int rpc_getmaxfd(void);
LIBRPC_API int rpc_register_socket(SOCKET sockfd);

LIBRPC_API int rpc_socket(int domain, int type, int protocol);
LIBRPC_API int rpc_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
LIBRPC_API int rpc_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
LIBRPC_API int rpc_listen(int sockfd, int backlog);
LIBRPC_API int rpc_accept(int sockfd, struct sockaddr *addr, int *addrlen);
LIBRPC_API int rpc_recvfrom(int sockfd, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len);
LIBRPC_API int rpc_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

LIBRPC_API int rpc_open(const char *pathname, int flags, int mode);
LIBRPC_API int rpc_ioctlsocket(int sockfd, long cmd, u_long *argp);
LIBRPC_API int rpc_read(int fd, void * const buffer, unsigned count);
LIBRPC_API int rpc_write(int fd, const void *buffer, unsigned count);
LIBRPC_API int rpc_close(int fd);

LIBRPC_API int rpc_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
LIBRPC_API int rpc_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

LIBRPC_API int rpc_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
LIBRPC_API int rpc_getpeername(int sockfd, struct sockaddr *name, socklen_t *namelen);

LIBRPC_API int rpc_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

LIBRPC_API int rpc_getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);

__END_DECLS

#if (defined(LIBRPC_LIBRARY) || defined(LIBRPC_SOURCE)) && !defined(LIBRPC_NO_SOCKET_MAPPINGS)

#if (1)
#define AF_LOCAL 99				/* XXX: AF_LOCAL emulation, via named pipes. */
#endif

#undef strerror
#define strerror(a__)				wsastrerror(a__)

#undef socket
#define socket(a__, b__, c__)			rpc_socket(a__, b__, c__)
#undef connect
#define connect(a__, b__, c__)			rpc_connect(a__, b__, c__)
#undef bind
#define bind(a__, b__, c__)			rpc_bind(a__, b__, c__)
#undef listen
#define listen(a__, b__)			rpc_listen(a__, b__)
#undef accept
#define accept(a__, b__, c__)			rpc_accept(a__, b__, c__)

#undef recvfrom
#define recvfrom(a__, b__, c__, d__, e__, f__)	rpc_recvfrom(a__, b__, c__, d__, e__, f__)
#undef sendto
#define sendto(a__, b__, c__, d__, e__, f__)	rpc_sendto(a__, b__, c__, d__, e__, f__)

/*
 *  #undef  open
 *  #define open(a__, b__, c__)			rpc_open(a__, b__, c__)
 */
#undef ioctlsocket
#define ioctlsocket(a__, b__, c__)		rpc_ioctlsocket(a__, b__, c__)
#undef read
#define read(a__, b__, c__)			rpc_read(a__, b__, c__)
#undef write
#define write(a__, b__, c__)			rpc_write(a__, b__, c__)
#undef close
#define close(a__)				rpc_close(a__)

#undef getsockopt
#define getsockopt(a__, b__, c__, d__, e__)	rpc_getsockopt(a__, b__, c__, d__, e__)
#undef setsockopt
#define setsockopt(a__, b__, c__, d__, e__)	rpc_setsockopt(a__, b__, c__, d__, e__)

#undef getsockname
#define getsockname(a__, b__, c__)		rpc_getsockname(a__, b__, c__)
#undef getpeername
#define getpeername(a__, b__, c__)		rpc_getpeername(a__, b__, c__)

#undef select
#define select(a__, b__, c__, d__, e__) 	rpc_select(a__, b__, c__, d__, e__)

#undef getnameinfo
#define getnameinfo(a__, b__, c__, d__, e__, f__, g__) rpc_getnameinfo(a__, b__, c__, d__, e__, f__, g__)

#endif /*LIBRPC_LIBRARY||LIBRPC_WIN32_SOCKET*/

/*end*/
