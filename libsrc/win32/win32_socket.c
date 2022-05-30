/*
 * socket/file syscall adapters
 *
 * Replacement socket API calls allowing:
 *
 *     o predictable/unix style file-descriptor allocation (rpc library assumption).
 *     o AF_LOCAL emulation, named-pipe underlying.
 *     o winsock error mapping.
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

#include "namespace.h"

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <poll.h>

#if !defined(__MINGW32__)
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif

#if defined(_DEBUG)
#define __ISDTRACE
#define __DTRACE(x__) printf x__;
#else
#define __DTRACE(x__)
#endif


/////////////////////////////////////////////////////////////////////////////////////////
//  misc

#if defined(__WATCOMC__)
int
_getmaxstdio(void)
{
	//XXX: default is _NFILES/20.
	int nfiles = _grow_handles(512);
	assert(512 == nfiles);
	return nfiles;
}
#endif  //__WATCOMC__


/////////////////////////////////////////////////////////////////////////////////////////
//  errno adapters

int
wsaerrno_map(int nerrno)
{
	if (nerrno < WSABASEERR)
		return nerrno;
	switch (nerrno) {
	case WSAEINTR:			nerrno = EINTR; break;
	case WSAEBADF:			nerrno = EBADF; break;
	case WSAEACCES: 		nerrno = EACCES; break;
	case WSAEFAULT: 		nerrno = EFAULT; break;
	case WSAEINVAL: 		nerrno = EINVAL; break;
	case WSAEMFILE: 		nerrno = EMFILE; break;
	case WSAENAMETOOLONG:		nerrno = ENAMETOOLONG; break;
	case WSAENOTEMPTY:		nerrno = ENOTEMPTY; break;
	case WSAENOBUFS:		nerrno = ENOMEM; break;
	case WSAENETDOWN:		nerrno = ENOSYS; break;
	case WSANOTINITIALISED: 	nerrno = ENOSYS; break;
	case WSAEWOULDBLOCK:		nerrno = EWOULDBLOCK; break;
	case WSAEADDRNOTAVAIL:		break; //10049
	case WSAENOTCONN:		break; //10057
	case WSAECONNREFUSED:		break; //10061
	default:
		assert(0);
		break;
	}
	return nerrno;
}


int
wsaerrno(void)
{
	const int nerrno = wsaerrno_map((int)WSAGetLastError());
	errno = nerrno;
	return nerrno;
}


#if defined(__MINGW32__)
static __thread char x_strerror[1024];
#else
static __declspec(thread) char x_strerror[1024];
#endif


const char *
wsastrerror(int xerrno)
{
#undef strerror
	if (xerrno < 250) {
		return strerror(xerrno);
	} else {
		const DWORD ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
					FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, xerrno,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), x_strerror, sizeof(x_strerror)-1, NULL);
		if (0 == ret) {
			_snprintf(x_strerror, sizeof(x_strerror)-1, "Unknown error (%d)", xerrno);
		}
		return x_strerror;
	}
}


int
wsainitialise(void)
{
	static int state = -1;
	if (-1 == state) {
		WSADATA wsaData = {0};
		state = WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	return state;
}


/////////////////////////////////////////////////////////////////////////////////////////
//  socket adapters

struct Socket {
	SOCKET	handle; 			// socket
	WSAEVENT wsaevt;			// socket event.
	SHORT	revents;			// event
};

typedef struct Pipe {
#define PIPE_SENDBUF	(32 * 1024)
#define PIPE_RECVBUF	(16 * 1044)
#define PIPE_TIMEOUT	500 /*connect timeout, milliseconds*/

	struct sockaddr_un addr;		// bind address.
	HANDLE	handle; 			// pipe handle.
	OVERLAPPED ov;				// accept/read overlapped state; inc event.
	int	type;				// type (TODO).
	BOOL	listen; 			// listen mode, otherwise read-mode.
	BOOL	blocking;			// blocking I/O; default.
	BOOL	rdpending;			// async accept/read pending.
	DWORD	rdcount;			// last read count.
	DWORD	rdpos;				// current read position <= count.
	SHORT	rdevents;			// outstanding read events.
	char	rdbuffer[PIPE_RECVBUF]; 	// read buffer.
} PIPE;

struct FileDescriptor {
	struct Socket sock;
	struct Pipe *pipe;
};

#define SOCKFD_BASE	32
#define SOCKFD_MAX	512

static CRITICAL_SECTION fds_lock;
static int fds_next;
static struct FileDescriptor fds_win32[SOCKFD_MAX];

static int pipe_create(int type);
static int pipe_bind(struct Pipe *pipe, const struct sockaddr *addr, socklen_t addrlen);
static int pipe_listen(struct Pipe *pipe);
static int pipe_connect(struct Pipe *pipe, const struct sockaddr *addr, socklen_t addrlen);
static int pipe_accept(int pipefd, struct Pipe *pipe, struct sockaddr *addr, int *addrlen);
static int pipe_ioctlsocket(struct Pipe *pipe, long cmd, u_long *argp);
static int pipe_io_read(struct Pipe *pipe);
static int pipe_io_complete(struct Pipe *pipe);
static int pipe_read(struct Pipe *pipe, char *buffer, unsigned count);
static int pipe_write(struct Pipe *pipe, const void *buffer, unsigned count);
static int pipe_getsockname(struct Pipe *pipe, struct sockaddr *addr, socklen_t *addrlen);
static int pipe_getpeername(struct Pipe *pipe, struct sockaddr *addr, socklen_t *addrlen);
static int pipe_getsockopt(struct Pipe *pipe, int level, int optname, void *optval, socklen_t *optlen);
static int pipe_setsockopt(struct Pipe *pipe, int level, int optname, const void *optval, socklen_t optlen);
static int pipe_close(int pipefd, struct Pipe *pipe);


static int
IsSocket(HANDLE h)
{
	return (GetFileType(h) == FILE_TYPE_PIPE && 0 == GetNamedPipeInfo(h, NULL, NULL, NULL, NULL));
}


static struct Socket *
issockfd(int sockfd)
{
	if (sockfd < SOCKFD_BASE || sockfd >= rpc_getmaxfd())
		return NULL;
	sockfd -= SOCKFD_BASE;
	if (0 == fds_win32[sockfd].sock.handle)
		return NULL;
	if (INVALID_SOCKET == fds_win32[sockfd].sock.handle) {
		assert(fds_win32[sockfd].pipe);
		return NULL;
	}
	assert(NULL == fds_win32[sockfd].pipe);
	assert(fds_win32[sockfd].sock.wsaevt);
	return &fds_win32[sockfd].sock;
}


static struct Pipe *
ispipefd(int pipefd)
{
	if (pipefd < SOCKFD_BASE || pipefd >= rpc_getmaxfd())
		return NULL;
	pipefd -= SOCKFD_BASE;
	if (INVALID_SOCKET != fds_win32[pipefd].sock.handle)
		return NULL;
	assert(0 == fds_win32[pipefd].sock.wsaevt);
	return fds_win32[pipefd].pipe;
}


static int
updatepipefd(int pipefd, struct Pipe *newpipe, struct Pipe *oldpipe)
{
	assert(pipefd >= SOCKFD_BASE && pipefd < rpc_getmaxfd());
	pipefd -= SOCKFD_BASE;
	assert(INVALID_SOCKET == fds_win32[pipefd].sock.handle);
	assert(oldpipe == fds_win32[pipefd].pipe);
	if (oldpipe == fds_win32[pipefd].pipe) {
		fds_win32[pipefd].pipe = newpipe;
		return 1;
	}
	return 0;
}


int
rpc_getmaxfd()
{
	return SOCKFD_BASE + _countof(fds_win32);
}


static int
fd_assign(SOCKET sock, struct Pipe *pipe)
{
	int loops, fd, rtn = -1;

	assert((INVALID_SOCKET != sock && IsSocket((HANDLE)sock) && NULL == pipe) || (INVALID_SOCKET == sock && pipe));

	if (0 == fds_lock.SpinCount) {
		InitializeCriticalSectionAndSpinCount(&fds_lock, 20000);
		assert(20000 == fds_lock.SpinCount);
	}

	EnterCriticalSection(&fds_lock);
	for (loops = (fds_next ? 2 : 1); loops--;) {
		for (fd = fds_next; fd < (int)_countof(fds_win32); ++fd) {
			if (0 == fds_win32[fd].sock.handle) {
				break;
			}
		}
		if (fd < (int)_countof(fds_win32)) {
			assert(0 == fds_win32[fd].sock.handle);
			if (INVALID_SOCKET != sock) {
				HANDLE wsaevt = WSACreateEvent();
				if (WSA_INVALID_EVENT == wsaevt) {
					assert(0);
					break;
				}
				WSAEventSelect(sock, wsaevt, FD_ACCEPT|FD_READ|FD_OOB|FD_WRITE|FD_CLOSE);
				fds_win32[fd].sock.wsaevt = wsaevt;
			}
			fds_win32[fd].sock.handle = sock;
			fds_win32[fd].pipe = pipe;
			fds_next = fd + 1;
			rtn = fd + SOCKFD_BASE;
			break;
		}
		fds_next = 0;
	}
	LeaveCriticalSection(&fds_lock);

	assert(rtn >= SOCKFD_BASE);
	return rtn;
}


static int
fd_release(int sockfd, SOCKET sock, struct Pipe *pipe)
{
	assert((INVALID_SOCKET != sock && IsSocket((HANDLE)sock) && NULL == pipe) || (INVALID_SOCKET == sock && pipe));
	assert(20000 == fds_lock.SpinCount);

	assert(sockfd >= SOCKFD_BASE && sockfd < rpc_getmaxfd());
	if (sockfd < SOCKFD_BASE || sockfd >= rpc_getmaxfd())
		return 0;
	sockfd -= SOCKFD_BASE;
	EnterCriticalSection(&fds_lock);
	assert(fds_win32[sockfd].sock.handle == sock);
	assert(fds_win32[sockfd].pipe == pipe);
	if (sock == fds_win32[sockfd].sock.handle) {
		assert((INVALID_SOCKET != sock && NULL == fds_win32[sockfd].pipe) || (INVALID_SOCKET == sock && NULL != fds_win32[sockfd].pipe));
		fds_win32[sockfd].sock.handle = 0;
		fds_win32[sockfd].sock.wsaevt = 0;
		fds_win32[sockfd].pipe = NULL;
		sockfd = -1;
	}
	LeaveCriticalSection(&fds_lock);
	return (sockfd < 0);
}


int
rpc_register_socket(SOCKET sock)
{
	return fd_assign(sock, NULL);
}


int
rpc_socket(int domain, int type, int protocol)
{
#undef socket
	unsigned attempts = 0;
	int ret = -1;

#ifdef AF_LOCAL /*WIN32*/
	if (AF_LOCAL == domain)
		ret = pipe_create(type);
	else
#endif
	{
retry:;		if ((ret = socket(domain, type, protocol)) == SOCKET_ERROR) {
			if (0 == attempts++) {
				if (WSAGetLastError() == WSANOTINITIALISED) {
					wsainitialise();
					goto retry;
				}
			}
			wsaerrno();
			ret = -1;
		} else {
			ret = rpc_register_socket(ret);
		}
	}

#if defined(__ISDTRACE)
	{
		const char *proto = "unknown";
		switch (domain) {
		case AF_INET:
			if (type == SOCK_STREAM) proto = "tcp";
			else if (type == SOCK_DGRAM) proto = "udp";
			break;
		case AF_INET6:
			if (type == SOCK_STREAM) proto = "tcp6";
			else if (type == SOCK_DGRAM) proto = "udp6";
			break;
#ifdef AF_LOCAL /*WIN32*/
		case AF_LOCAL:
			proto = "local";
			break;
#endif
		default:
			assert(0);
		}
		__DTRACE(("socket(%d,%d,%d)=%d [%s]\n", domain, type, protocol, ret, proto))
	}
#endif //__ISTRACE
	return ret;
}


int
rpc_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
#undef connect
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret = -1;

	if (sockfd < 0) {
		errno = EBADF;

	} else if ((sock = issockfd(sockfd)) != NULL) {
		SOCKET s;
		if ((s = connect(sock->handle, addr, addrlen)) == (SOCKET)SOCKET_ERROR) {
			wsaerrno();
		} else if ((ret = rpc_register_socket(s)) == -1) {
			closesocket(s);
			errno = EMFILE;
		} else {
			sock->revents = 0;
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_connect(pipe, addr, addrlen);
	} else {
		assert(0);
		errno = EBADF;
	}

#if defined(__ISDTRACE)
	{
		char name[NI_MAXHOST] = {0}, port[NI_MAXSERV] = {0};
		getnameinfo(addr, addrlen, name, sizeof(name), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
		__DTRACE(("connect(%d)=%d [%s,%s]\n", sockfd, ret, name, port))
	}
#endif //__ISTRACE
	return ret;
}


int
rpc_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
#undef bind
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret = -1;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = bind(sock->handle, addr, addrlen)) == SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		} else {
			sock->revents = 0;
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_bind(pipe, addr, addrlen);
	} else {
		assert(0);
		errno = EBADF;
	}

#if defined(__ISDTRACE)
	{
		char name[NI_MAXHOST] = {0}, port[NI_MAXSERV] = {0};
		getnameinfo(addr, addrlen, name, sizeof(name), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
		__DTRACE(("bind(%d)=%d [%s,%s]\n", sockfd, ret, name, port))
	}
#endif //__ISTRACE
	return ret;
}


int
rpc_listen(int sockfd, int backlog)
{
#undef listen
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret = -1;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = listen(sock->handle, backlog)) == SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		} else {
			sock->revents = 0;
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_listen(pipe);

	} else {
		assert(0);
		errno = EBADF;
	}

	__DTRACE(("listen(%d)=%d\n", sockfd, ret))
	return ret;
}


int
rpc_accept(int sockfd, struct sockaddr *addr, int *addrlen)
{
#undef accept
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret = -1;

	if ((sock = issockfd(sockfd)) != NULL) {
		SOCKET s;
		if ((s = accept(sock->handle, addr, addrlen)) == (SOCKET)SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		} else if ((ret = rpc_register_socket(s)) == -1) {
			closesocket(s);
			errno = EMFILE;
		} else {
			sock->revents = 0;
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_accept(sockfd, pipe, addr, addrlen);
	} else {
		assert(0);
		errno = EBADF;
	}

	__DTRACE(("accept(%d)=%d\n", sockfd, ret))
	return ret;
}


ssize_t
rpc_recvfrom(int sockfd, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len)
{
#undef recvfrom
	struct Socket *sock = NULL;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		sock->revents &= ~POLLIN;
		if ((ret = recvfrom(sock->handle, buffer, (int)length, flags, address, address_len)) == SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		}
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

	__DTRACE(("recvfrom(%d,%p,%u)=%d\n", sockfd, buffer, length, ret))
	return ret;
}


ssize_t
rpc_sendto(int sockfd, const void *buffer, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
#undef sendto
	struct Socket *sock = NULL;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		sock->revents &= ~POLLOUT;
		if ((ret = sendto(sock->handle, buffer, (int)length, flags, dest_addr, addrlen)) == SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		}
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

	__DTRACE(("sendto(%d,%p,%u)=%d\n", sockfd, buffer, length, ret))
	return ret;
}


int
rpc_ioctlsocket(int sockfd, long cmd, u_long *argp)
{
#undef ioctlsocket
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = ioctlsocket(sock->handle, cmd, argp)) == SOCKET_ERROR) {
			wsaerrno();
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_ioctlsocket(pipe, cmd, argp);
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

	__DTRACE(("ioctlsocket(%d,%ld)=%d\n", sockfd, cmd, ret))
	return ret;
}


int
rpc_read(int fd, void * const buffer, unsigned count)
{
#undef recvfrom
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(fd)) != NULL) {
		sock->revents &= ~POLLIN;
		if ((ret = recvfrom(sock->handle, buffer, (int)count, 0, NULL, 0)) == SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		}
	} else if ((pipe = ispipefd(fd)) != NULL) {
		ret = pipe_read(pipe, buffer, count);
	} else {
		assert(0);
		ret = _read(fd, buffer, (int)count);
	}

	__DTRACE(("read(%d,%p,%u)=%d\n", fd, buffer, count, ret))
	return ret;
}


int
rpc_write(int fd, const void *buffer, unsigned count)
{
#undef sendto
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(fd)) != NULL) {
		sock->revents &= ~POLLOUT;
		if ((ret = sendto(sock->handle, buffer, (int)count, 0, NULL, 0)) == SOCKET_ERROR) {
			wsaerrno();
			ret = -1;
		}
	} else if ((pipe = ispipefd(fd)) != NULL) {
		ret = pipe_write(pipe, buffer, count);
	} else {
		assert(0);
		ret = _write(fd, buffer, (int)count);
	}

	__DTRACE(("write(%d,%p,%u)=%d\n", fd, buffer, count, ret))
	return ret;
}


int
rpc_close(int fd)
{
#undef closesocket
#undef close
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(fd)) != NULL) {
		//
		// To ensure that all data is sent and received on a connected socket
		// before it is closed, an application should use shutdown to close the
		// connection before calling closesocket.
		//
		// Note: shutdown function does not block regardless of the SO_LINGER setting on the socket.
		//
#ifndef SD_BOTH
#define SD_RECEIVE	0x00
#define SD_SEND 	0x01
#define SD_BOTH 	0x02
#endif
		SOCKET s = sock->handle;
		WSAEVENT wsaevt = sock->wsaevt;

		if (fd_release(fd, s, NULL)) {
			(void) shutdown(s, SD_BOTH);
			if ((ret = closesocket(s)) == SOCKET_ERROR) {
				wsaerrno();
				ret = -1;
			}
			WSACloseEvent(wsaevt);
		} else {
			assert(0);
			errno = EBADF;
			ret = -1;
		}
	} else if ((pipe = ispipefd(fd)) != NULL) {
		ret = pipe_close(fd, pipe);
	} else if (fd < 0) {
		assert(0);
		errno = EBADF;
		ret = -1;
	} else {
		assert(0);
		ret = _close(fd);
	}

	__DTRACE(("close(%d)=%d\n", fd, ret))
	return ret;
}


int
rpc_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
#undef getsockopt
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = getsockopt(sock->handle, level, optname, optval, optlen)) == SOCKET_ERROR) {
			wsaerrno();
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_getsockopt(pipe, level, optname, optval, optlen);
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

	__DTRACE(("getsockopt(%d,%d,%d)=%d\n", sockfd, level, optname, ret))
	return ret;
}


int
rpc_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
#undef setsockopt
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = setsockopt(sock->handle, level, optname, optval, optlen)) == SOCKET_ERROR) {
			wsaerrno();
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_setsockopt(pipe, level, optname, optval, optlen);
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

	__DTRACE(("setsockopt(%d,%d,%d)=%d\n", sockfd, level, optname, ret))
	return ret;
}


int
rpc_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
#undef getsockname
	struct Socket *sock = NULL;
	struct Pipe *pipe;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = getsockname(sock->handle, addr, addrlen)) == SOCKET_ERROR) {
			wsaerrno();
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_getsockname(pipe, addr, addrlen);
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

#if defined(__ISDTRACE)
	{
		char name[NI_MAXHOST] = {0}, port[NI_MAXSERV] = {0};
		if (0 == ret) getnameinfo(addr, *addrlen, name, sizeof(name), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
		__DTRACE(("setsockname(%d)=%d [%s,%s]\n", sockfd, ret, name, port))
	}
#endif //__ISTRACE
	return ret;
}


int
rpc_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
#undef getpeername
	struct Socket *sock = NULL;
	struct Pipe *pipe = NULL;
	int ret;

	if ((sock = issockfd(sockfd)) != NULL) {
		if ((ret = getpeername(sock->handle, addr, addrlen)) == SOCKET_ERROR) {
			wsaerrno();
		}
	} else if ((pipe = ispipefd(sockfd)) != NULL) {
		ret = pipe_getpeername(pipe, addr, addrlen);
	} else {
		assert(0);
		errno = EBADF;
		ret = -1;
	}

#if defined(__ISDTRACE)
	{
		char name[NI_MAXHOST] = {0}, port[NI_MAXSERV] = {0};
		if (0 == ret) getnameinfo(addr, *addrlen, name, sizeof(name), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
		__DTRACE(("getpeername(%d)=%d [%s,%s]\n", sockfd, ret, name, port))
	}
#endif //__ISTRACE
	return ret;
}


int
rpc_getnameinfo(const struct sockaddr *addr, socklen_t addrlen,
	char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
#undef getnameinfo
	const struct sockaddr_un *sun =(const struct sockaddr_un *)(const void *)addr;

	if (NULL == addr || AF_LOCAL != addr->sa_family)
		return getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);

	if (addrlen < (socklen_t) offsetof(struct sockaddr_un, sun_path))
		return EAI_FAMILY;

	if (serv != NULL && servlen > 0)
		serv[0] = '\0';

	if (host && hostlen > 0)
		strlcpy(host, sun->sun_path, MIN((socklen_t) sizeof(sun->sun_path) + 1, hostlen));
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//  AF_LOCAL

static int
pipe_create(int type)
{
	struct Pipe *pipe = (PIPE *)calloc(1, sizeof(PIPE));
	int rtn = -1;

	__DTRACE(("pipe_create(%p,%d)\n", pipe, type))
	assert(sizeof(struct sockaddr_storage) >= sizeof(struct sockaddr_un));
	if (pipe) {
		pipe->type = type;
		pipe->blocking = TRUE;
		if ((rtn = fd_assign(INVALID_SOCKET, pipe)) < 0) {
			free(pipe);
		}
	}
	return rtn;
}


static int
pipe_bind(struct Pipe *pipe, const struct sockaddr *addr, socklen_t addrlen)
{
	const struct sockaddr_un *addr_un = (const struct sockaddr_un *)addr;

	__DTRACE(("pipe_bind(%p)\n", pipe))
	if (pipe->handle) {
		errno = WSAEISCONN;		// already connected
		return -1;
	} else if (pipe->addr.sun_family) {
		errno = EINVAL; 		// already bound
		return -1;
	} else if (NULL == addr || addrlen < (socklen_t) offsetof(struct sockaddr_un, sun_path)) {
		errno = EFAULT;
		return -1;
	} else if (AF_LOCAL != addr->sa_family ||
			0 == addr_un->sun_len || addr_un->sun_len > sizeof(pipe->addr.sun_path)) {
		errno = EINVAL;
		return -1;
	}

	memcpy(&pipe->addr, addr, sizeof(pipe->addr));
	return 1;
}


static int
pipe_listen(struct Pipe *pipe)
{
	char pipe_name[MAX_PATH] = {0};
	DWORD err = 0;

	__DTRACE(("pipe_listen(%p)\n", pipe))
	if (pipe->handle) {
		errno = WSAEISCONN;		// already connected
		return -1;
	} else if (AF_LOCAL != pipe->addr.sun_family) {
		errno = EINVAL; 		// unbound
		return -1;
	}

#if !defined(PIPE_REJECT_REMOTE_CLIENTS)
#define PIPE_REJECT_REMOTE_CLIENTS  0x00000008
#endif

	snprintf(pipe_name, sizeof(pipe_name)-1, "\\\\.\\pipe\\%s", pipe->addr.sun_path);
	pipe->handle = CreateNamedPipeA(pipe_name,
			    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
			    PIPE_UNLIMITED_INSTANCES,
			    PIPE_SENDBUF, PIPE_RECVBUF,
			    PIPE_TIMEOUT, NULL);
	if (pipe->handle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	memset(&pipe->ov, 0, sizeof(pipe->ov));
	if (NULL != (pipe->ov.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL))) {
		pipe->listen = TRUE;
		if (! ConnectNamedPipe(pipe->handle, &pipe->ov)) {
			if ((err = GetLastError()) == ERROR_IO_PENDING) {
				__DTRACE(("pipe_listen(%p): pending\n", pipe))
				pipe->rdpending = TRUE;
				return 0;
			} else if (ERROR_PIPE_CONNECTED == err) {
				__DTRACE(("pipe_listen(%p): complete\n", pipe))
				pipe->rdevents = POLLIN;
				pipe->rdpending = FALSE;
				return 0;
			}
		}
	}

	__DTRACE(("pipe_listen(%p): error [%u]\n", pipe, (unsigned)GetLastError()))
	CloseHandle(pipe->handle), pipe->handle = 0;
	assert(0 == pipe->rdpending);
	pipe->listen = FALSE;
	return -1;
}


static int
pipe_connect(struct Pipe *pipe, const struct sockaddr *addr, socklen_t addrlen)
{
	const struct sockaddr_un *addr_un = (const struct sockaddr_un *)addr;
	char pipe_name[MAX_PATH] = {0};
	DWORD mode = 0;

	__DTRACE(("pipe_connect(%p)\n", pipe))
	if (pipe->handle) {
		errno = WSAEISCONN;
		return -1;
	} else if (pipe->addr.sun_family) {
		errno = EINVAL; 		// already bound
		return -1;
	} else if (NULL == addr || addrlen < (socklen_t) offsetof(struct sockaddr_un, sun_path)) {
		errno = EFAULT;
		return -1;
	} else if (AF_LOCAL != addr->sa_family ||
		    0 == addr_un->sun_len || addr_un->sun_len > sizeof(pipe->addr.sun_path)) {
		errno = EINVAL;
		return -1;
	}

	snprintf(pipe_name, sizeof(pipe_name)-1, "\\\\.\\pipe\\%s", addr_un->sun_path);
	while (1) {
		DWORD err;

		__DTRACE(("pipe_connect(%p): open(%s)\n", pipe, pipe_name))
		pipe->handle = CreateFileA(pipe_name,
			GENERIC_READ|GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (pipe->handle != INVALID_HANDLE_VALUE)
			break;			// connected

		if ((err = GetLastError()) != ERROR_PIPE_BUSY) {
			__DTRACE(("pipe_connect(%p): open-error [%u]\n", pipe, err))
			switch (err) {
			case ERROR_ACCESS_DENIED:
			case ERROR_SHARING_VIOLATION:
				errno = EACCES;
				break;
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_INVALID_DRIVE:
				errno = ENOENT;
				break;
			default:
				errno = WSAECONNREFUSED;
				break;
			}
			return -1;		// general error
		}

		__DTRACE(("pipe_connect(%p): waiting ...\n", pipe))
		if (! WaitNamedPipeA(pipe_name, NMPWAIT_USE_DEFAULT_WAIT)) {
			__DTRACE(("pipe_connect(%p): wait-error [%u]\n", pipe, (unsigned)GetLastError()))
			errno = WSAECONNREFUSED;
			pipe->handle = 0;
			return -1;
		}
	}

	mode = PIPE_READMODE_MESSAGE | (pipe->blocking ? PIPE_WAIT : PIPE_NOWAIT);
	memset(&pipe->ov, 0, sizeof(pipe->ov));
	if (! SetNamedPipeHandleState(pipe->handle, &mode, NULL, NULL) ||
			(NULL == (pipe->ov.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL)))) {
		__DTRACE(("pipe_connect(%p): mode-error [%u]\n", pipe, (unsigned)GetLastError()))
		errno = WSAECONNREFUSED;
		pipe->handle = 0;
		return -1;
	}

	__DTRACE(("pipe_connect(%p): done\n", pipe))
	memcpy(&pipe->addr, addr, sizeof(pipe->addr));
	pipe_io_read(pipe);
	return 1;
}


static int
pipe_accept(int sockfd, struct Pipe *pipe, struct sockaddr *addr, int *addrlen)
{
	DWORD result = 0;
	int rtn = -1;

	__DTRACE(("pipe_accept(%p)\n", pipe))
	if (0 == pipe->handle) {
		errno = WSAENOTCONN;
	} else if (! pipe->listen) {
		errno = EINVAL;
	} else if (! pipe->rdpending ||
			GetOverlappedResult(pipe->handle, &pipe->ov, &result, pipe->blocking)) {
		struct Pipe *newpipe;

		if (NULL != (newpipe = (PIPE *)calloc(1, sizeof(PIPE))) &&
				(rtn = fd_assign(INVALID_SOCKET, pipe)) >= 0 && updatepipefd(sockfd, newpipe, pipe)) {
			// new client, rearm new listen instance.
			newpipe->addr = pipe->addr;
			pipe_listen(newpipe);
			pipe->listen = FALSE;
			pipe_io_read(pipe);
			if (addr && addrlen) {
				pipe_getpeername(pipe, addr, addrlen);
			}

		} else { // error, rearm original.
			CloseHandle(pipe->handle);
			pipe->handle = NULL;
			pipe_listen(pipe);
			free(newpipe);
		}
	} else {
		errno = EAGAIN;
	}
	return rtn;
}


static int
pipe_ioctlsocket(struct Pipe *pipe, long cmd, u_long *argp)
{
	__DTRACE(("pipe_ioctl(%p)\n", pipe))
	if (FIONBIO == (unsigned long)cmd) {
		if (NULL != argp) {
			pipe->blocking = (*argp ? FALSE : TRUE);
			return 0;
		}
	}
	assert(0);
	errno = EINVAL;
	return -1;
}


static int
pipe_io_read(struct Pipe *pipe)
{
	DWORD result = 0;

	__DTRACE(("pipe_io_read(%p)\n", pipe))
	assert(! pipe->listen);
	assert(! pipe->rdpending);

	if (ReadFile(pipe->handle, pipe->rdbuffer, sizeof(pipe->rdbuffer), &result, &pipe->ov)) {
		assert(result);
		SetEvent(pipe->ov.hEvent);
		pipe->rdevents = POLLIN;
		pipe->rdcount = result;
		pipe->rdpos = 0;

	} else {
		const DWORD err = GetLastError();
		if (ERROR_IO_PENDING == err) {
			pipe->rdpending = TRUE;
			pipe->rdevents = 0;
			__DTRACE(("pipe_io_read(%p): 0 [pending]\n", pipe))
			return 0;
		} else if (ERROR_MORE_DATA == err) {
			SetEvent(pipe->ov.hEvent);
			assert(result == sizeof(pipe->rdbuffer));
			pipe->rdevents = POLLIN;
			pipe->rdcount = sizeof(pipe->rdbuffer);
			pipe->rdpos = 0;
		} else if (ERROR_HANDLE_EOF == err || ERROR_BROKEN_PIPE == err) {
			pipe->rdevents = POLLHUP;
		} else {
			pipe->rdevents = POLLERR;
		}
	}

	assert(pipe->rdevents);
	pipe->rdpending = FALSE;
	__DTRACE(("pipe_io_read(%p): 1 [ready]\n", pipe))
	return 1;
}


static int
pipe_io_complete(struct Pipe *pipe)
{
	DWORD result = 0;

	__DTRACE(("pipe_io_complete(%p)\n", pipe))
	assert(pipe->rdpending);

	if (GetOverlappedResult(pipe->handle, &pipe->ov, &result, pipe->blocking)) {
		__DTRACE(("pipe_io_complete(%p): done [%u]\n", pipe, (unsigned)result))
		pipe->rdevents = POLLIN;
		pipe->rdcount = result;
		pipe->rdpos = 0;

	} else {
		const DWORD err = GetLastError();
		if (ERROR_IO_INCOMPLETE == err) {
			__DTRACE(("pipe_io_complete(%p): pending\n", pipe))
			pipe->rdevents = 0;
			return 0;
		}

		__DTRACE(("pipe_io_complete(%p): error [%u]\n", pipe, (unsigned)err))
		if (ERROR_MORE_DATA == err) {
			assert(result == sizeof(pipe->rdbuffer));
			pipe->rdevents = POLLIN;
			pipe->rdcount = result;
			pipe->rdpos = 0;
		} else if (ERROR_HANDLE_EOF == err || ERROR_BROKEN_PIPE == err) {
			pipe->rdevents = POLLHUP;
		} else {
			pipe->rdevents = POLLERR;
		}
	}

	assert(pipe->rdevents);
	pipe->rdpending = FALSE;
	return 1;
}


static int
pipe_read(struct Pipe *pipe, char *buffer, unsigned count)
{
	DWORD result = 0;
	int rtn = -1;

	__DTRACE(("pipe_read(%p)\n", pipe))
	if (0 == pipe->handle || pipe->listen) {
		errno = WSAENOTCONN;
	} else if (NULL == buffer) {
		errno = EFAULT;
	} else if (0 == count) {
		rtn = 0;
	} else if (! pipe->rdpending || pipe_io_complete(pipe)) {
		if (pipe->rdevents & POLLIN) {
			rtn = 0;
			do {
				DWORD cnt = pipe->rdcount - pipe->rdpos;

				__DTRACE(("pipe_read(%p): available[%u]\n", pipe, cnt))
				if (cnt > count)
					cnt = count;
				memcpy(buffer, pipe->rdbuffer + pipe->rdpos, cnt);

				if ((pipe->rdpos += cnt) == pipe->rdcount) {
					pipe_io_read(pipe); // rearm/read
				}

				buffer += cnt;
				count -= cnt;
				rtn += cnt;

			} while (count && (pipe->rdevents & POLLIN));
		} else if (pipe->rdevents & POLLHUP) {
			errno = EPIPE;
		} else {
			assert(pipe->rdevents & POLLERR);
			errno = EIO;
		}
	} else {
		errno = EAGAIN;
	}
	return rtn;
}


static int
pipe_write(struct Pipe *pipe, const void *buffer, unsigned count)
{
	int rtn = -1;

	__DTRACE(("pipe_write(%p,%p,%u)\n", pipe, buffer, count))
	if (0 == pipe->handle || pipe->listen) {
		errno = WSAENOTCONN;
	} else if (NULL == buffer) {
		errno = EFAULT;
	} else if (0 == count) {
		rtn = 0;
	} else {
		DWORD result = 0;
		if (WriteFile(pipe->handle, buffer, count, &result, NULL)) {
			__DTRACE(("pipe_write(%p): %u\n", pipe, (unsigned)result))
			rtn = (int)result;
		} else {
			const DWORD err = GetLastError();
			__DTRACE(("pipe_write(%p): err(%u)\n", pipe, err))
			if (ERROR_BROKEN_PIPE == err || ERROR_NO_DATA == err) {
				errno = EPIPE;
			} else if (ERROR_IO_PENDING == err) {
				errno = EAGAIN;
			} else {
				errno = EIO;
			}
		}
	}
	return rtn;
}


static int
pipe_getsockopt(struct Pipe *pipe, int level, int optname, void *optval, socklen_t *optlen)
{
	if (SOL_SOCKET == level) {
		if (SO_TYPE == optname) {
			if (*optlen == sizeof(int)) {
				*(int *)optval = pipe->type;
				return 0;
			}
		}
	}
	assert(0);
	errno = EINVAL;
	return -1;
}


static int
pipe_setsockopt(struct Pipe *pipe, int level, int optname, const void *optval, socklen_t optlen)
{
	assert(0);
	errno = WSAEAFNOSUPPORT;
	return -1;
}


static int
pipe_getsockname(struct Pipe *pipe, struct sockaddr *addr, socklen_t *addrlen)
{
	if (0 == pipe->handle) {
		errno = WSAENOTCONN;
		return -1;
	} else if (NULL == addr || NULL == addrlen) {
		errno = EFAULT;
		return -1;
	} else if (*addrlen < (socklen_t)sizeof(struct sockaddr_un)) {
		errno = EINVAL;
		return -1;
	}

	assert(AF_LOCAL == pipe->addr.sun_family);
	*(struct sockaddr_un *)addr = pipe->addr;
	*addrlen = sizeof(struct sockaddr_un);
	return 0;
}


static int
pipe_getpeername(struct Pipe *pipe, struct sockaddr *addr, socklen_t *addrlen)
{
	if (0 == pipe->handle || pipe->listen) {
		errno = WSAENOTCONN;
		return -1;
	} else if (NULL == addr || NULL == addrlen) {
		errno = EFAULT;
		return -1;
	} else if (*addrlen < (socklen_t)sizeof(struct sockaddr_un)) {
		errno = EINVAL;
		return -1;
	}

	assert(AF_LOCAL == pipe->addr.sun_family);
	*(struct sockaddr_un *)addr = pipe->addr;
	*addrlen = sizeof(struct sockaddr_un);
	return 0;
}


static int
pipe_close(int pipefd, struct Pipe *pipe)
{
	HANDLE p = pipe->handle;
	HANDLE e = pipe->ov.hEvent;
	int ret = -1;

	if (fd_release(pipefd, INVALID_SOCKET, pipe)) {
		CloseHandle(p);
		CloseHandle(e);
		ret = 0;
	} else {
		assert(0);
		errno = EBADF;
	}
	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////
//  poll - socket/pipe

int
rpc_pollts(struct pollfd * fds, nfds_t nfds, const struct timespec *ts, const sigset_t *sigmask)
{
	int timeout = -1 /*indefinity*/;
	(void) sigmask;

	if (ts) {
		timeout = (int)(ts->tv_sec * 1000) + (int)(ts->tv_nsec / 1000000);
	}
	return rpc_poll(fds, nfds, timeout);
}


int
rpc_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	struct pollfd t_fds[FD_SETSIZE] = { 0 };
	struct {
		struct Socket *sock;
		struct Pipe *pipe;
	} resources[FD_SETSIZE] = {0};
	HANDLE handles[FD_SETSIZE] = {0};
	struct Socket *sock;
	struct Pipe *pipe = NULL;
	SHORT revents = 0;
	size_t pipes = 0, n;
	int ret = 0;

	assert(FD_SETSIZE <= 64);
	assert(nfds < FD_SETSIZE);
	if (nfds <= 0) {
		errno = EINVAL;
		return -1;
	}

	__DTRACE(("poll(%p,%u,%d)\n", fds, nfds, timeout))
	for (n = 0; n < nfds; ++n) {
		const SHORT events = fds[n].events;
		const int fd = fds[n].fd;

		t_fds[n].events = events;
		fds[n].revents = 0;

		if ((sock = issockfd(fd)) != NULL) {
			t_fds[n].fd = sock->handle;
			resources[n].sock = sock;
			handles[n] = sock->wsaevt;
			if ((revents = (sock->revents & events)) != 0) {
				fds[n].revents = revents;
				++ret;
			}

		} else if ((pipe = ispipefd(fd)) != NULL) {
			resources[n].pipe = pipe;
			handles[n] = pipe->ov.hEvent;
			if ((revents = (pipe->rdevents & events)) != 0) {
				fds[n].revents = revents;
				++ret;
			}
			++pipes;

		} else {
			__DTRACE(("poll(%p) : %d (fd[%d] == EBADF)\n", fds, fd, n))
			assert(0);
			errno = EBADF;
			return -1;
		}
	}

	if (pipes && ret > 0 /*one or more pipes, plus local events pending*/) {
		__DTRACE(("poll(%p) : %d [pipes]\n", fds, ret))
		return ret;
	}

	if (pipes) {
		const DWORD waittm = (timeout < 0 ? INFINITE : (DWORD)timeout);
		DWORD evt;

		__DTRACE(("poll(%p) : wait-events (%d,%u)\n", fds, nfds, waittm))
		assert(0 == WSA_WAIT_EVENT_0);

		if ((evt = WSAWaitForMultipleEvents(nfds, handles, FALSE, waittm, TRUE)) < (STATUS_WAIT_0 + nfds)) {
			const SHORT events = fds[evt -= WSA_WAIT_EVENT_0].events | POLLHUP | POLLERR | POLLNVAL;

			revents = 0;
			if ((sock = resources[evt].sock) != NULL) {
				WSANETWORKEVENTS ne = {0};
				if (WSAEnumNetworkEvents(sock->handle, sock->wsaevt, &ne) == 0) {
					if (ne.lNetworkEvents & (FD_ACCEPT|FD_READ))
						revents |= POLLRDNORM;
					if (ne.lNetworkEvents & FD_OOB)
						revents |= POLLRDBAND;
					if (ne.lNetworkEvents & FD_WRITE)
						revents |= POLLWRNORM;
					if (ne.lNetworkEvents & FD_OOB)
						revents |= POLLPRI;
					if (ne.lNetworkEvents & FD_CLOSE)
						revents |= POLLHUP;
					__DTRACE(("poll(%p) : socket event [%ld=%u]\n", fds, ne.lNetworkEvents, revents))
				} else {
					revents = (WSAENETDOWN == WSAGetLastError() ? POLLHUP : POLLERR);
				}
				sock->revents |= revents;
				revents &= events;

			} else if ((pipe = resources[evt].pipe) != NULL) {
				__DTRACE(("poll(%p) : pipe event\n", fds))
				if (pipe_io_complete(pipe)) {
					revents = (pipe->rdevents & events);
				}

			} else {
				assert(0);
				revents = POLLNVAL|POLLERR;
			}

			if (revents) {
				fds[evt].revents = revents;
				ret = 1;
			} else {
				errno = EINTR;
				ret = -1;
			}

		} else {
			ret = -1;
			if (WSA_WAIT_TIMEOUT == evt) {
				__DTRACE(("poll(%p) : EAGAIN\n", fds))
				errno = EAGAIN;
				ret = 0;
			} else if (WSA_WAIT_IO_COMPLETION == evt) {
				__DTRACE(("poll(%p) : EINTR\n", fds))
				errno = EINTR;
			} else if (WAIT_FAILED == evt) {
				__DTRACE(("poll(%p) : EIO [%u]\n", fds, (unsigned)GetLastError()))
				errno = EIO;
			} else {
				__DTRACE(("poll(%p) : ??? [%u]\n", fds, (unsigned)GetLastError()))
				wsaerrno();
			}
		}

	} else {
		__DTRACE(("poll(%p) : wsapoll\n", fds))
		if ((ret = WSAPoll(t_fds, nfds, timeout)) == SOCKET_ERROR) {
			wsaerrno();
		}

		for (n = 0; n < nfds; ++n) {
		    fds[n].revents = t_fds[n].revents = 0;
		}
	}

	__DTRACE(("poll(%p)=%d\n", fds, ret))
	return ret;
}


int
rpc_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
#undef select
	struct pollfd fds[FD_SETSIZE] = { 0 };
	SOCKET *fd_array;
	nfds_t n;
	int polltm = -1;
	int ret;

	// XXX: current rpc usage (readfds only).
	// XXX: unless changed poll() is the default interface (see: svc_run)
	assert(0 == nfds);
	assert(NULL == writefds && NULL == exceptfds);
	if (writefds || exceptfds || nfds != 0) {
		errno = EINVAL;
		return -1;
	}

	assert(readfds);
	if (NULL == readfds) {
		errno = EINVAL;
		return -1;
	}

	nfds = readfds->fd_count;
	fd_array = readfds->fd_array;
	for (n = 0; n < (nfds_t)nfds; ++n) {
		fds[n].fd = fd_array[n];
		fds[n].events = POLLIN;
	}

	if (timeout) {
		polltm = (timeout->tv_sec * 1000) + (timeout->tv_usec / 1000);
	}

	FD_ZERO(readfds);
	if ((ret = rpc_poll(fds, nfds, polltm)) > 0) {
		int t_ret = 0;
		for (n = 0; n < (nfds_t)nfds; ++n) {
			if (fds[n].revents) {
				fd_array[t_ret++] = fds[n].fd;
			}
		}
		readfds->fd_count = t_ret;
		assert(t_ret == ret);
		ret = t_ret;
	}
	return ret;
}

/*end*/
