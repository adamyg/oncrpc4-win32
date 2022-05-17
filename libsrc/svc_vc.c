/*	$NetBSD: svc_vc.c,v 1.34 2015/11/10 20:56:20 christos Exp $	*/

/*
 * Copyright (c) 2010, Oracle America, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the "Oracle America, Inc." nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char *sccsid = "@(#)svc_tcp.c 1.21 87/08/11 Copyr 1984 Sun Micro";
static char *sccsid = "@(#)svc_tcp.c	2.2 88/08/01 4.0 RPCSRC";
#else
__RCSID("$NetBSD: svc_vc.c,v 1.34 2015/11/10 20:56:20 christos Exp $");
#endif
#endif

/*
 * svc_vc.c, Server side for Connection Oriented based RPC. 
 *
 * Actually implements two flavors of transporter -
 * a tcp rendezvouser (a listner and connection establisher)
 * and a record/tcp stream.
 */

#include "namespace.h"
#include "reentrant.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>

#include <rpc/rpc.h>

#include "svc_fdset.h"
#include "rpc_internal.h"

#ifdef __weak_alias
__weak_alias(svc_fd_create,_svc_fd_create)
__weak_alias(svc_vc_create,_svc_vc_create)
#endif

#ifdef _REENTRANT
extern rwlock_t svc_fd_lock;
#endif

static SVCXPRT *makefd_xprt(int, u_int, u_int);
static bool_t rendezvous_request(SVCXPRT *, struct rpc_msg *);
static enum xprt_stat rendezvous_stat(SVCXPRT *);
static void svc_vc_destroy(SVCXPRT *);
static void __svc_vc_dodestroy(SVCXPRT *);
static int read_vc(caddr_t, caddr_t, int);
static int write_vc(caddr_t, caddr_t, int);
static enum xprt_stat svc_vc_stat(SVCXPRT *);
static bool_t svc_vc_recv(SVCXPRT *, struct rpc_msg *);
static bool_t svc_vc_getargs(SVCXPRT *, xdrproc_t, caddr_t);
static bool_t svc_vc_freeargs(SVCXPRT *, xdrproc_t, caddr_t);
static bool_t svc_vc_reply(SVCXPRT *, struct rpc_msg *);
static void svc_vc_rendezvous_ops(SVCXPRT *);
static void svc_vc_ops(SVCXPRT *);
static bool_t svc_vc_control(SVCXPRT *, const u_int, void *);
static bool_t svc_vc_rendezvous_control(SVCXPRT *, const u_int, void *);

struct cf_rendezvous { /* kept in xprt->xp_p1 for rendezvouser */
	u_int sendsize;
	u_int recvsize;
	int maxrec;
};

struct cf_conn {  /* kept in xprt->xp_p1 for actual connection */
	enum xprt_stat strm_stat;
	u_int32_t x_id;
	XDR xdrs;
	char verf_body[MAX_AUTH_BYTES];
	u_int sendsize;
	u_int recvsize;
	int maxrec;
	bool_t nonblock;
	struct timeval last_recv_time;
};

/*
 * Usage:
 *	xprt = svc_vc_create(sock, send_buf_size, recv_buf_size);
 *
 * Creates, registers, and returns a (rpc) tcp based transporter.
 * Once *xprt is initialized, it is registered as a transporter
 * see (svc.h, xprt_register).  This routine returns
 * a NULL if a problem occurred.
 *
 * The filedescriptor passed in is expected to refer to a bound, but
 * not yet connected socket.
 *
 * Since streams do buffered io similar to stdio, the caller can specify
 * how big the send and receive buffers are via the second and third parms;
 * 0 => use the system default.
 */
SVCXPRT *
svc_vc_create(const int fd, const u_int sendsize, const u_int recvsize)
{
	SVCXPRT *xprt;
	struct cf_rendezvous *r = NULL;
	struct __rpc_sockinfo si;
	struct sockaddr_storage sslocal;
	socklen_t slen, alen;
	int one = 1;

	if (!__rpc_fd2sockinfo(fd, &si))
		return NULL;

	r = mem_alloc(sizeof(*r));
	if (r == NULL) {
		warn("%s: out of memory", __func__);
		return NULL;
	}
	r->sendsize = __rpc_get_t_size(si.si_af, si.si_proto, (int)sendsize);
	r->recvsize = __rpc_get_t_size(si.si_af, si.si_proto, (int)recvsize);
	r->maxrec = __svc_maxrec;
	xprt = mem_alloc(sizeof(SVCXPRT));
	if (xprt == NULL) {
		warn("%s: out of memory", __func__);
		goto cleanup_svc_vc_create;
	}
	xprt->xp_tp = NULL;
	xprt->xp_p1 = (caddr_t)(void *)r;
	xprt->xp_p2 = NULL;
	xprt->xp_p3 = NULL;
	xprt->xp_verf = _null_auth;
	svc_vc_rendezvous_ops(xprt);
	xprt->xp_port = (u_short)-1;	/* It is the rendezvouser */
	xprt->xp_fd = fd;

	slen = sizeof (struct sockaddr_storage);
	if (getsockname(fd, (struct sockaddr *)(void *)&sslocal, &slen) < 0) {
		warn("%s: could not retrieve local addr", __func__);
		goto cleanup_svc_vc_create;
	}

	/*
	 * We want to be able to check credentials on local sockets.
	 */
#ifdef AF_LOCAL /*WIN32*/
#if defined(LOCAL_CREDS)
	if (sslocal.ss_family == AF_LOCAL)
		if (setsockopt(fd, 0, LOCAL_CREDS, &one, (socklen_t)sizeof one) == -1)
			goto cleanup_svc_vc_create;
#endif
#endif

	alen = SOCKLEN_SOCKADDR_STORAGE(sslocal);
	xprt->xp_ltaddr.maxlen = xprt->xp_ltaddr.len = alen /*sslocal.ss_len*/;
	xprt->xp_ltaddr.buf = mem_alloc((size_t)alen /*sslocal.ss_len*/);
	if (xprt->xp_ltaddr.buf == NULL) {
		warn("%s: out of memory", __func__);
		goto cleanup_svc_vc_create;
	}
	memcpy(xprt->xp_ltaddr.buf, &sslocal, (size_t)alen /*sslocal.ss_len*/);

	xprt->xp_rtaddr.maxlen = sizeof (struct sockaddr_storage);
	if (!xprt_register(xprt))
		goto cleanup_svc_vc_create;
	return xprt;
cleanup_svc_vc_create:
	if (xprt)
		mem_free(xprt, sizeof(*xprt));
	if (r != NULL)
		mem_free(r, sizeof(*r));
	return NULL;
}

/*
 * Like svtcp_create(), except the routine takes any *open* UNIX file
 * descriptor as its first input.
 */
SVCXPRT *
svc_fd_create(const int fd, const u_int sendsize, const u_int recvsize)
{
	struct sockaddr_storage ss;
	socklen_t slen, alen;
	SVCXPRT *ret;

	_DIAGASSERT(fd != -1);

	ret = makefd_xprt(fd, sendsize, recvsize);
	if (ret == NULL)
		return NULL;

	slen = sizeof (struct sockaddr_storage);
	if (getsockname(fd, (struct sockaddr *)(void *)&ss, &slen) < 0) {
		warn("%s: could not retrieve local addr", __func__);
		goto freedata;
	}
	alen = SOCKLEN_SOCKADDR_STORAGE(ss);
	ret->xp_ltaddr.maxlen = ret->xp_ltaddr.len = alen /*ss.ss_len*/;
	ret->xp_ltaddr.buf = mem_alloc((size_t)alen /*ss.ss_len*/);
	if (ret->xp_ltaddr.buf == NULL) {
		warn("%s: out of memory", __func__);
		goto freedata;
	}
	memcpy(ret->xp_ltaddr.buf, &ss, (size_t)alen /*ss.ss_len*/);

	slen = sizeof (struct sockaddr_storage);
	if (getpeername(fd, (struct sockaddr *)(void *)&ss, &slen) < 0) {
		warn("%s: could not retrieve remote addr", __func__);
		goto freedata;
	}
	alen = SOCKLEN_SOCKADDR_STORAGE(ss);
	ret->xp_rtaddr.maxlen = ret->xp_rtaddr.len = alen /*ss.ss_len*/;
	ret->xp_rtaddr.buf = mem_alloc((size_t)alen /*ss.ss_len*/);
	if (ret->xp_rtaddr.buf == NULL) {
		warn("%s: out of memory", __func__);
		goto freedata;
	}
	memcpy(ret->xp_rtaddr.buf, &ss, (size_t)alen /*ss.ss_len*/);
#ifdef PORTMAP
	if (ss.ss_family == AF_INET) {
		ret->xp_raddr = *(struct sockaddr_in *)ret->xp_rtaddr.buf;
		ret->xp_addrlen = sizeof (struct sockaddr_in);
	}
#endif

	return ret;

freedata:
	if (ret->xp_ltaddr.buf != NULL)
		mem_free(ret->xp_ltaddr.buf, rep->xp_ltaddr.maxlen);

	return NULL;
}

static SVCXPRT *
makefd_xprt(int fd, u_int sendsize, u_int recvsize)
{
	SVCXPRT *xprt;
	struct cf_conn *cd;
	const char *netid;
	struct __rpc_sockinfo si;
 
	_DIAGASSERT(fd != -1);

	xprt = mem_alloc(sizeof(SVCXPRT));
	if (xprt == NULL)
		goto outofmem;
	memset(xprt, 0, sizeof *xprt);
	cd = mem_alloc(sizeof(struct cf_conn));
	if (cd == NULL)
		goto outofmem;
	cd->strm_stat = XPRT_IDLE;
	xdrrec_create(&(cd->xdrs), sendsize, recvsize,
	    (caddr_t)(void *)xprt, read_vc, write_vc);
	xprt->xp_p1 = (caddr_t)(void *)cd;
	xprt->xp_verf.oa_base = cd->verf_body;
	svc_vc_ops(xprt);  /* truely deals with calls */
	xprt->xp_port = 0;  /* this is a connection, not a rendezvouser */
	xprt->xp_fd = fd;
	if (__rpc_fd2sockinfo(fd, &si) && __rpc_sockinfo2netid(&si, &netid))
		if ((xprt->xp_netid = strdup(netid)) == NULL)
			goto outofmem;

	if (!xprt_register(xprt))
		goto out;
	return xprt;

outofmem:
	warn("svc_tcp: makefd_xprt");
out:
	if (xprt)
		mem_free(xprt, sizeof(SVCXPRT));
	return NULL;
}

/*ARGSUSED*/
static bool_t
rendezvous_request(SVCXPRT *xprt, struct rpc_msg *msg)
{
	int sock;
	struct cf_rendezvous *r;
	struct cf_conn *cd;
	struct sockaddr_storage addr;
	socklen_t len;
	struct __rpc_sockinfo si;
	SVCXPRT *newxprt;

	_DIAGASSERT(xprt != NULL);
	_DIAGASSERT(msg != NULL);

	r = (struct cf_rendezvous *)xprt->xp_p1;
again:
	len = sizeof addr;
	if ((sock = accept(xprt->xp_fd, (struct sockaddr *)(void *)&addr,
	    &len)) < 0) {
		if (errno == EINTR)
			goto again;
		/*
		 * Clean out the most idle file descriptor when we're
		 * running out.
		 */
		if (errno == EMFILE || errno == ENFILE) {
			if (__svc_clean_idle(NULL, 0, FALSE))
				goto again;
		}
		return FALSE;
	}
	/*
	 * make a new transporter (re-uses xprt)
	 */
	newxprt = makefd_xprt(sock, r->sendsize, r->recvsize);
	if (newxprt == NULL)
		goto out;
	newxprt->xp_rtaddr.buf = mem_alloc(len);
	if (newxprt->xp_rtaddr.buf == NULL)
		goto out;
	memcpy(newxprt->xp_rtaddr.buf, &addr, len);
	newxprt->xp_rtaddr.len = len;
#ifdef PORTMAP
	if (addr.ss_family == AF_INET) {
		newxprt->xp_raddr = *(struct sockaddr_in *)newxprt->xp_rtaddr.buf;
		newxprt->xp_addrlen = sizeof (struct sockaddr_in);
	}
#endif
	if (__rpc_fd2sockinfo(sock, &si))
		__rpc_setnodelay(sock, &si);

	cd = (struct cf_conn *)newxprt->xp_p1;

	cd->recvsize = r->recvsize;
	cd->sendsize = r->sendsize;
	cd->maxrec = r->maxrec;

	if (cd->maxrec != 0) {
#if defined(_WIN32)
		u_long mode = (long)1;
		if (ioctlsocket(sock, FIONBIO, &mode) == -1 /*SOCKET_ERROR*/) {
			goto out;
		}
#else
		int flags = fcntl(sock, F_GETFL, 0);
		if (flags  == -1)
			goto out;
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
			goto out;
#endif
		if (cd->recvsize > (u_int)cd->maxrec)
			cd->recvsize = cd->maxrec;
		cd->nonblock = TRUE;
		__xdrrec_setnonblock(&cd->xdrs, cd->maxrec);
	} else
		cd->nonblock = FALSE;

	(void)gettimeofday(&cd->last_recv_time, NULL);

	return FALSE; /* there is never an rpc msg to be processed */
out:
	(void)close(sock);
	return FALSE; /* there was an error */
}

/*ARGSUSED*/
static enum xprt_stat
rendezvous_stat(SVCXPRT *xprt)
{

	return XPRT_IDLE;
}

static void
svc_vc_destroy(SVCXPRT *xprt)
{
	_DIAGASSERT(xprt != NULL);

	xprt_unregister(xprt);
	__svc_vc_dodestroy(xprt);
}

static void
__svc_vc_dodestroy(SVCXPRT *xprt)
{
	struct cf_conn *cd;
	struct cf_rendezvous *r;

	cd = (struct cf_conn *)xprt->xp_p1;

	if (xprt->xp_fd != RPC_ANYFD)
		(void)close(xprt->xp_fd);
	if (xprt->xp_port != 0) {
		/* a rendezvouser socket */
		r = (struct cf_rendezvous *)xprt->xp_p1;
		mem_free(r, sizeof (struct cf_rendezvous));
		xprt->xp_port = 0;
	} else {
		/* an actual connection socket */
		XDR_DESTROY(&(cd->xdrs));
		mem_free(cd, sizeof(struct cf_conn));
	}
	if (xprt->xp_rtaddr.buf)
		mem_free(xprt->xp_rtaddr.buf, xprt->xp_rtaddr.maxlen);
	if (xprt->xp_ltaddr.buf)
		mem_free(xprt->xp_ltaddr.buf, xprt->xp_ltaddr.maxlen);
	if (xprt->xp_tp)
		free(xprt->xp_tp);
	if (xprt->xp_netid)
		free(xprt->xp_netid);
	mem_free(xprt, sizeof(SVCXPRT));
}

/*ARGSUSED*/
static bool_t
svc_vc_control(SVCXPRT *xprt, const u_int rq, void *in)
{
	return FALSE;
}

/*ARGSUSED*/
static bool_t
svc_vc_rendezvous_control(SVCXPRT *xprt, const u_int rq, void *in)
{
	struct cf_rendezvous *cfp;

	cfp = (struct cf_rendezvous *)xprt->xp_p1;
	if (cfp == NULL)
		return FALSE;
	switch (rq) {
		case SVCGET_CONNMAXREC:
			*(int *)in = cfp->maxrec;
			break;
		case SVCSET_CONNMAXREC:
			cfp->maxrec = *(int *)in;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

/*
 * reads data from the tcp connection.
 * any error is fatal and the connection is closed.
 * (And a read of zero bytes is a half closed stream => error.)
 * All read operations timeout after 35 seconds.  A timeout is
 * fatal for the connection.
 */
static int
read_vc(caddr_t xprtp, caddr_t buf, int len)
{
	SVCXPRT *xprt;
	int sock;
	struct pollfd pollfd;
	struct sockaddr *sa;
	void *crmsg = NULL;
	struct cf_conn *cfp;
	static const struct timespec ts = { 35, 0 };

	xprt = (SVCXPRT *)(void *)xprtp;
	_DIAGASSERT(xprt != NULL);

	sock = xprt->xp_fd;

	sa = (struct sockaddr *)xprt->xp_rtaddr.buf;
#if !defined(_WIN32)
#ifdef AF_LOCAL /*WIN32*/
	if (sa->sa_family == AF_LOCAL && xprt->xp_p2 == NULL) {
		struct msghdr msg;
		struct sockcred *sc;
		struct cmsghdr *cmp;
		socklen_t crmsgsize;

		memset(&msg, 0, sizeof msg);
		crmsgsize = CMSG_SPACE(SOCKCREDSIZE(NGROUPS));
		crmsg = malloc(crmsgsize);
		if (crmsg == NULL)
			goto fatal_err;
		memset(crmsg, 0, crmsgsize);

		msg.msg_control = crmsg;
		msg.msg_controllen = crmsgsize;

		if (recvmsg(sock, &msg, 0) < 0)
			goto fatal_err;

		if (msg.msg_controllen == 0 ||
		    (msg.msg_flags & MSG_CTRUNC) != 0)
			goto fatal_err;

		cmp = CMSG_FIRSTHDR(&msg);
		if (cmp->cmsg_level != SOL_SOCKET ||
		    cmp->cmsg_type != SCM_CREDS)
			goto fatal_err;

		sc = (struct sockcred *)(void *)CMSG_DATA(cmp);

		xprt->xp_p2 = mem_alloc(SOCKCREDSIZE(sc->sc_ngroups));
		if (xprt->xp_p2 == NULL)
			goto fatal_err;

		memcpy(xprt->xp_p2, sc, SOCKCREDSIZE(sc->sc_ngroups));
		free(crmsg);
		crmsg = NULL;
	}
#endif //AF_LOCAL
#endif //_WIN32

	cfp = (struct cf_conn *)xprt->xp_p1;

	if (cfp->nonblock) {
		len = (int)read(sock, buf, (size_t)len);
		if (len < 0) {
			if (errno == EAGAIN)
				len = 0;
			else
				goto fatal_err;
		}
		if (len != 0)
			gettimeofday(&cfp->last_recv_time, NULL);
		return len;
	}

	do {
		pollfd.fd = sock;
		pollfd.events = POLLIN;
		switch (pollts(&pollfd, 1, &ts, NULL)) {
		case -1:
			if (errno == EINTR) {
				continue;
			}
			/*FALLTHROUGH*/
		case 0:
			goto fatal_err;

		default:
			break;
		}
	} while ((pollfd.revents & POLLIN) == 0);

	if ((len = (int)read(sock, buf, (size_t)len)) > 0) {
		gettimeofday(&cfp->last_recv_time, NULL);
		return len;
	}

fatal_err:
	if (crmsg != NULL)
		free(crmsg);
	((struct cf_conn *)(xprt->xp_p1))->strm_stat = XPRT_DIED;
	return -1;
}

/*
 * writes data to the tcp connection.
 * Any error is fatal and the connection is closed.
 */
static int
write_vc(caddr_t xprtp, caddr_t buf, int len)
{
	SVCXPRT *xprt;
	int i, cnt;
	struct cf_conn *cd;
	struct timeval tv0, tv1;

	xprt = (SVCXPRT *)(void *)xprtp;
	_DIAGASSERT(xprt != NULL);

	cd = (struct cf_conn *)xprt->xp_p1;

	if (cd->nonblock)
		gettimeofday(&tv0, NULL);

	for (cnt = len; cnt > 0; cnt -= i, buf += i) {
		if ((i = (int)write(xprt->xp_fd, buf, (size_t)cnt)) < 0) {
			if (errno != EAGAIN || !cd->nonblock) {
				cd->strm_stat = XPRT_DIED;
				return -1;
			}
			if (cd->nonblock) {
				/*
				 * For non-blocking connections, do not
				 * take more than 2 seconds writing the
				 * data out.
				 *
				 * XXX 2 is an arbitrary amount.
				 */
				gettimeofday(&tv1, NULL);
				if (tv1.tv_sec - tv0.tv_sec >= 2) {
					cd->strm_stat = XPRT_DIED;
					return -1;
				}
			}
			i = 0;
		}
	}
	return len;
}

static enum xprt_stat
svc_vc_stat(SVCXPRT *xprt)
{
	struct cf_conn *cd;

	_DIAGASSERT(xprt != NULL);

	cd = (struct cf_conn *)(xprt->xp_p1);

	if (cd->strm_stat == XPRT_DIED)
		return XPRT_DIED;
	if (! xdrrec_eof(&(cd->xdrs)))
		return XPRT_MOREREQS;
	return XPRT_IDLE;
}

static bool_t
svc_vc_recv(SVCXPRT *xprt, struct rpc_msg *msg)
{
	struct cf_conn *cd;
	XDR *xdrs;

	_DIAGASSERT(xprt != NULL);
	_DIAGASSERT(msg != NULL);

	cd = (struct cf_conn *)(xprt->xp_p1);
	xdrs = &(cd->xdrs);

	if (cd->nonblock) {
		if (!__xdrrec_getrec(xdrs, &cd->strm_stat, TRUE))
			return FALSE;
	}

	xdrs->x_op = XDR_DECODE;
	(void)xdrrec_skiprecord(xdrs);

	if (xdr_callmsg(xdrs, msg)) {
		cd->x_id = msg->rm_xid;
		return TRUE;
	}
	cd->strm_stat = XPRT_DIED;
	return FALSE;
}

static bool_t
svc_vc_getargs(SVCXPRT *xprt, xdrproc_t xdr_args, caddr_t args_ptr)
{

	_DIAGASSERT(xprt != NULL);
	/* args_ptr may be NULL */

	return (*xdr_args)(&(((struct cf_conn *)(xprt->xp_p1))->xdrs),
	    args_ptr);
}

static bool_t
svc_vc_freeargs(SVCXPRT *xprt, xdrproc_t xdr_args, caddr_t args_ptr)
{
	XDR *xdrs;

	_DIAGASSERT(xprt != NULL);
	/* args_ptr may be NULL */

	xdrs = &(((struct cf_conn *)(xprt->xp_p1))->xdrs);

	xdrs->x_op = XDR_FREE;
	return (*xdr_args)(xdrs, args_ptr);
}

static bool_t
svc_vc_reply(SVCXPRT *xprt, struct rpc_msg *msg)
{
	struct cf_conn *cd;
	XDR *xdrs;
	bool_t rstat;

	_DIAGASSERT(xprt != NULL);
	_DIAGASSERT(msg != NULL);

	cd = (struct cf_conn *)(xprt->xp_p1);
	xdrs = &(cd->xdrs);

	xdrs->x_op = XDR_ENCODE;
	msg->rm_xid = cd->x_id;
	rstat = xdr_replymsg(xdrs, msg);
	(void)xdrrec_endofrecord(xdrs, TRUE);
	return rstat;
}

static void
svc_vc_ops(SVCXPRT *xprt)
{
	static struct xp_ops ops;
	static struct xp_ops2 ops2;
#ifdef _REENTRANT
	extern mutex_t ops_lock;
#endif

/* VARIABLES PROTECTED BY ops_lock: ops, ops2 */

	mutex_lock(&ops_lock);
	if (ops.xp_recv == NULL) {
		ops.xp_recv = svc_vc_recv;
		ops.xp_stat = svc_vc_stat;
		ops.xp_getargs = svc_vc_getargs;
		ops.xp_reply = svc_vc_reply;
		ops.xp_freeargs = svc_vc_freeargs;
		ops.xp_destroy = svc_vc_destroy;
		ops2.xp_control = svc_vc_control;
	}
	xprt->xp_ops = &ops;
	xprt->xp_ops2 = &ops2;
	mutex_unlock(&ops_lock);
}

static void
svc_vc_rendezvous_ops(SVCXPRT *xprt)
{
	static struct xp_ops ops;
	static struct xp_ops2 ops2;
#ifdef _REENTRANT
	extern mutex_t ops_lock;
#endif
	mutex_lock(&ops_lock);
	if (ops.xp_recv == NULL) {
		ops.xp_recv = rendezvous_request;
		ops.xp_stat = rendezvous_stat;
		ops.xp_getargs =
		    (bool_t (*)(SVCXPRT *, xdrproc_t, caddr_t))abort;
		ops.xp_reply =
		    (bool_t (*)(SVCXPRT *, struct rpc_msg *))abort;
		ops.xp_freeargs =
		    (bool_t (*)(SVCXPRT *, xdrproc_t, caddr_t))abort;
		ops.xp_destroy = svc_vc_destroy;
		ops2.xp_control = svc_vc_rendezvous_control;
	}
	xprt->xp_ops = &ops;
	xprt->xp_ops2 = &ops2;
	mutex_unlock(&ops_lock);
}

/*
 * Destroy xprts that have not have had any activity in 'timeout' seconds.
 * If 'cleanblock' is true, blocking connections (the default) are also
 * cleaned. If timeout is 0, the least active connection is picked.
 */
LIBRPC_API bool_t
/*ARGSUSED1*/
__svc_clean_idle(fd_set *fds __unused, int timeout, bool_t cleanblock)
{
	int i, ncleaned, *fdmax;
	SVCXPRT *xprt, *least_active;
	struct timeval tv, tdiff, tmax;
	struct cf_conn *cd;

	gettimeofday(&tv, NULL);
	tmax.tv_sec = tmax.tv_usec = 0;
	least_active = NULL;
	rwlock_wrlock(&svc_fd_lock);
	fdmax = svc_fdset_getmax();
	if (fdmax == NULL)
		return FALSE;
	for (i = ncleaned = 0; i <= *fdmax; i++) {
		switch (svc_fdset_isset(i)) {
		case 0:
		case -1:
			continue;
		default:
			break;
		}

		xprt = __svc_xports[i];
		if (xprt == NULL || xprt->xp_ops == NULL ||
		    xprt->xp_ops->xp_recv != svc_vc_recv)
			continue;

		cd = (struct cf_conn *)xprt->xp_p1;
		if (!cleanblock && !cd->nonblock)
			continue;

		if (timeout == 0) {
			timersub(&tv, &cd->last_recv_time, &tdiff);
			if (timercmp(&tdiff, &tmax, >)) {
				tmax = tdiff;
				least_active = xprt;
			}
			continue;
		}

		if (tv.tv_sec - cd->last_recv_time.tv_sec > timeout) {
			__xprt_unregister_unlocked(xprt);
			__svc_vc_dodestroy(xprt);
			ncleaned++;
		}
	}
	if (timeout == 0 && least_active != NULL) {
		__xprt_unregister_unlocked(least_active);
		__svc_vc_dodestroy(least_active);
		ncleaned++;
	}
	rwlock_unlock(&svc_fd_lock);
	return ncleaned > 0 ? TRUE : FALSE;
}
