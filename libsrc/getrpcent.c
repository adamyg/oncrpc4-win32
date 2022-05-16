/*	$NetBSD: getrpcent.c,v 1.23 2013/03/11 20:19:29 tron Exp $	*/

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
static char *sccsid = "@(#)getrpcent.c 1.14 91/03/11 Copyr 1984 Sun Micro";
#else
__RCSID("$NetBSD: getrpcent.c,v 1.23 2013/03/11 20:19:29 tron Exp $");
#endif
#endif

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include "namespace.h"

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc/rpc.h>

#ifdef __weak_alias
__weak_alias(endrpcent, _endrpcent)
__weak_alias(getrpcbyname, _getrpcbyname)
__weak_alias(getrpcbynumber, _getrpcbynumber)
__weak_alias(getrpcent, _getrpcent)
__weak_alias(setrpcent, _setrpcent)
#endif

/*
 * /etc/rpc.
 */

#if defined(_WIN32)
static const char *netconfig[] = {
        "# $NetBSD: rpc,v 1.7 2008 / 05 / 17 21 : 37 : 09 jmcneill Exp $\n",
        "#\n",
        "# rpc 88 / 08 / 01 4.0 RPCSRC; from 1.12   88 / 02 / 07 SMI\n",
        "#\n",
        "portmapper	100000	portmap sunrpc rpcbind\n",
        "rstatd		100001	rstat rstat_svc rup perfmeter\n",
        "rusersd	100002	rusers\n",
        "nfs		100003	nfsprog\n",
        "ypserv		100004	ypprog\n",
        "mountd		100005	mount showmount\n",
        "ypbind		100007\n",
        "walld		100008	rwall shutdown\n",
        "yppasswdd	100009	yppasswd\n",
        "etherstatd	100010	etherstat\n",
        "rquotad	100011	rquotaprog quota rquota\n",
        "sprayd		100012	spray\n",
        "3270_mapper	100013\n",
        "rje_mapper	100014\n",
        "selection_svc	100015	selnsvc\n",
        "database_svc	100016\n",
        "rexd		100017	rex\n",
        "alis		100018\n",
        "sched		100019\n",
        "llockmgr	100020\n",
        "nlockmgr	100021\n",
        "x25.inr	100022\n",
        "statmon	100023\n",
        "status		100024\n",
        "bootparam	100026\n",
        "ypupdated	100028	ypupdate\n",
        "keyserv	100029	keyserver\n",
        "tfsd		100037\n",
        "nsed		100038\n",
        "nsemntd	100039\n",
        "pcnfsd		150001	pcnfs\n",
        "amd		300019	amq\n"
        };

typedef struct {
	FILE *file;
	unsigned offset;
} NCFILE;

static NCFILE *
ncopen(const char *path, const char *mode) 
{
	NCFILE *ncfile;
	if (NULL != (ncfile = calloc(1, sizeof(NCFILE)))) {
		ncfile->file = fopen(path, mode);
	}
	return ncfile;
}

static char *
ncgets(char *str, int n, NCFILE *stream)
{
	if (stream) {
		if (stream->file) {
			return fgets(str, n, stream->file);
		}

		if (stream->offset < _countof(netconfig)) {
			strncpy(str, netconfig[stream->offset], n);
			++stream->offset;
			return str;
		}
	}
	return NULL;
}

static void
ncrewind(NCFILE *stream)
{
	if (stream) {
		if (stream->file) rewind(stream->file);
		stream->offset = 0;
	}
}

static void
ncclose(NCFILE *stream)
{
	if (stream) {
		if (stream->file) fclose(stream->file);
		free(stream);
	}
}
#endif  /*_WIN32*/

/*
 * Internet version.
 */

static struct rpcdata {
#if defined(_WIN32)
	NCFILE	*rpcf;
#else
	FILE	*rpcf;
#endif
	int	stayopen;
#define	MAXALIASES	35
	char	*rpc_aliases[MAXALIASES];
	struct	rpcent rpc;
	char	line[BUFSIZ+1];
} *rpcdata;

static	struct rpcent *interpret(char *val, size_t len);

#define	RPCDB	"/etc/rpc"

static struct rpcdata *_rpcdata(void);

static struct rpcdata *
_rpcdata(void)
{
	struct rpcdata *d = rpcdata;

	if (d == 0) {
		d = (struct rpcdata *)calloc(1, sizeof (struct rpcdata));
		rpcdata = d;
	}
	return (d);
}

LIBRPC_API struct rpcent *
getrpcbynumber(int number)
{
	struct rpcent *rpc;

	setrpcent(0);
	while ((rpc = getrpcent()) != NULL) {
		if (rpc->r_number == number)
			break;
	}
	endrpcent();
	return (rpc);
}

LIBRPC_API struct rpcent *
getrpcbyname(const char *name)
{
	struct rpcent *rpc;
	char **rp;

	_DIAGASSERT(name != NULL);

	setrpcent(0);
	while ((rpc = getrpcent()) != NULL) {
		if (strcmp(rpc->r_name, name) == 0)
			break;
		for (rp = rpc->r_aliases; *rp != NULL; rp++) {
			if (strcmp(*rp, name) == 0)
				goto found;
		}
	}
found:
	endrpcent();
	return (rpc);
}

LIBRPC_API void
setrpcent(int f)
{
	struct rpcdata *d = _rpcdata();

	if (d == 0)
		return;
	if (d->rpcf == NULL)
#if defined(_WIN32)
		d->rpcf = ncopen(RPCDB, "r");
#else
		d->rpcf = fopen(RPCDB, "re");
#endif
	else
#if defined(_WIN32)
		ncrewind(d->rpcf);
#else
		rewind(d->rpcf);
#endif
	d->stayopen |= f;
}

LIBRPC_API void
endrpcent(void)
{
	struct rpcdata *d = _rpcdata();

	if (d == 0)
		return;
	if (d->rpcf && !d->stayopen) {
#if defined(_WIN32)
   		ncclose(d->rpcf);
#else
		fclose(d->rpcf);
#endif
		d->rpcf = NULL;
	}
}

LIBRPC_API struct rpcent *
getrpcent(void)
{
	struct rpcdata *d = _rpcdata();

	if (d == 0)
		return(NULL);
#if defined(_WIN32)
	if (d->rpcf == NULL && (d->rpcf = ncopen(RPCDB, "r")) == NULL)
		return (NULL);
	if (ncgets(d->line, BUFSIZ, d->rpcf) == NULL)
		return (NULL);
#else
	if (d->rpcf == NULL && (d->rpcf = fopen(RPCDB, "re")) == NULL)
		return (NULL);
	if (fgets(d->line, BUFSIZ, d->rpcf) == NULL)
		return (NULL);
#endif
	return (interpret(d->line, strlen(d->line)));
}

static struct rpcent *
interpret(char *val, size_t len)
{
	struct rpcdata *d = _rpcdata();
	char *p;
	char *cp, **q;

	_DIAGASSERT(val != NULL);

	if (d == 0)
		return (0);
	(void) strncpy(d->line, val, len);
	p = d->line;
	d->line[len] = '\n';
	if (*p == '#')
		return (getrpcent());
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		return (getrpcent());
	*cp = '\0';
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		return (getrpcent());
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	d->rpc.r_name = d->line;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	d->rpc.r_number = atoi(cp);
	q = d->rpc.r_aliases = d->rpc_aliases;
	cp = strpbrk(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &(d->rpc_aliases[MAXALIASES - 1]))
			*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&d->rpc);
}
