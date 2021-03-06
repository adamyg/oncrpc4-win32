/*	$NetBSD: rpcbind.h,v 1.6 2017/08/16 08:44:40 christos Exp $	*/
/*	$FreeBSD: head/usr.sbin/rpcbind/rpcbind.h 293229 2016-01-06 00:00:11Z asomers $ */
/*-
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1986 - 1991 by Sun Microsystems, Inc.
 */

/* #ident	"@(#)rpcbind.h 1.4 90/04/12 SMI" */

/*
 * rpcbind.h
 * The common header declarations
 */

#ifndef rpcbind_h
#define	rpcbind_h

#ifdef PORTMAP
#include <rpc/pmap_prot.h>
#endif
#include <rpc/rpcb_prot.h>

#if defined(_WIN32)
#include <libcompat.h>
#if defined(__WATCOMC__)
#include <strings.h>            /* strcasecmp */
#endif
#endif
      

/*
 * Stuff for the rmtcall service
 */
struct encap_parms {
	u_int32_t arglen;
	char *args;
};

struct r_rmtcall_args {
	u_int32_t  rmt_prog;
	u_int32_t  rmt_vers;
	u_int32_t  rmt_proc;
	int     rmt_localvers;  /* whether to send port # or uaddr */
	char    *rmt_uaddr;
	struct encap_parms rmt_args;
};

extern int debugging;
extern int doabort;
#ifdef LIBWRAP
extern int libwrap;
#endif
extern int verboselog;
extern int insecure;
extern int oldstyle_local;
extern rpcblist_ptr list_rbl;	/* A list of version 3 & 4 rpcbind services */

#ifdef PORTMAP
extern struct pmaplist *list_pml; /* A list of version 2 rpcbind services */
extern const char *udptrans;	/* Name of UDP transport */
extern const char *tcptrans;	/* Name of TCP transport */
extern const char *udp_uaddr;	/* Universal UDP address */
extern const char *tcp_uaddr;	/* Universal TCP address */
#endif

extern const char rpcbind_superuser[];
extern const char rpcbind_unknown[];

int add_bndlist(struct netconfig *, struct netbuf *);
bool_t is_bound(const char *, const char *);
char *mergeaddr(SVCXPRT *, char *, char *, char *);
struct netconfig *rpcbind_get_conf(const char *);

void rpcbs_init(void); 
void rpcbs_procinfo(rpcvers_t, rpcproc_t);
void rpcbs_set(rpcvers_t, bool_t);
void rpcbs_unset(rpcvers_t, bool_t);
void rpcbs_getaddr(rpcvers_t, rpcprog_t, rpcvers_t, const char *, const char *);
void rpcbs_rmtcall(rpcvers_t, rpcproc_t, rpcprog_t, rpcvers_t, rpcproc_t,
			char *, rpcblist_ptr);
void *rpcbproc_getstat(void *, struct svc_req *, SVCXPRT *, rpcvers_t);

void rpcb_service_3(struct svc_req *, SVCXPRT *);
void rpcb_service_4(struct svc_req *, SVCXPRT *);

/* Common functions shared between versions */
void *rpcbproc_set_com(void *, struct svc_req *, SVCXPRT *, rpcvers_t);
void *rpcbproc_unset_com(void *, struct svc_req *, SVCXPRT *, rpcvers_t);
bool_t map_set(RPCB *, char *);
bool_t map_unset(RPCB *, const char *);
void delete_prog(rpcprog_t);
void *rpcbproc_getaddr_com(RPCB *, struct svc_req *, SVCXPRT *, rpcvers_t,
				 rpcvers_t);
void *rpcbproc_gettime_com(void *, struct svc_req *, SVCXPRT *,
				rpcvers_t);
void *rpcbproc_uaddr2taddr_com(void *, struct svc_req *,
					     SVCXPRT *, rpcvers_t);
void *rpcbproc_taddr2uaddr_com(void *, struct svc_req *, SVCXPRT *,
				    rpcvers_t);
int create_rmtcall_fd(struct netconfig *);
void rpcbproc_callit_com(struct svc_req *, SVCXPRT *, rpcvers_t,
			      rpcvers_t);
__dead void my_svc_run(void);

__dead void rpcbind_abort(void);
void reap(int);
void toggle_verboselog(int);

int check_access(SVCXPRT *, rpcproc_t, void *, unsigned int);
int check_callit(SVCXPRT *, struct r_rmtcall_args *, int);
void logit(int, struct sockaddr *, rpcproc_t, rpcprog_t, const char *);
int is_loopback(struct netbuf *);

#ifdef PORTMAP
extern void pmap_service(struct svc_req *, SVCXPRT *);
#endif

void write_warmstart(void);
void read_warmstart(void);

char *addrmerge(struct netbuf *caller, char *serv_uaddr, char *clnt_uaddr,
    const char *netid);
int listen_addr(const struct sockaddr *sa);
void network_init(void);
struct sockaddr *local_sa(int);

/* For different getaddr semantics */
#define	RPCB_ALLVERS 0
#define	RPCB_ONEVERS 1
/* To convert a struct sockaddr to IPv4 or IPv6 address */
#define	SA2SIN(sa)	((const struct sockaddr_in *)(sa))
#define	SA2SINADDR(sa)	(SA2SIN(sa)->sin_addr)
#ifdef INET6
#define	SA2SIN6(sa)	((const struct sockaddr_in6 *)(sa))
#define	SA2SIN6ADDR(sa)	(SA2SIN6(sa)->sin6_addr)
#endif

#endif /* rpcbind_h */
