/* $OpenBSD: pidfile.c,v 1.14 2019/06/28 14:20:40 schwarze Exp $ */
/* $NetBSD: pidfile.c,v 1.4 2001/02/19 22:43:42 cgd Exp $ */

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "libcompat.h"

#if !defined(HAVE_PIDFILE)

#include <sys/types.h>
#include <errno.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
	//#include <util.h>

static char *pidfile_path;
static pid_t pidfile_pid;

static void pidfile_cleanup(void);

static const char *
getappname(void)
{
	const char *progname = getprogname();
	const char *p1 = strrchr(progname, '\\'),
		*p2 = strrchr(progname, '/');
	if (p1 || p2) { // last component.
		return (p1 > p2 ? p1 : p2) + 1;
	}
	return progname;
}


int
pidfile(const char *appname)
{
#if defined(_WIN32)
	char tmppath[256] = {0};
	int tmprtn;
#endif
	int save_errno;
	pid_t pid;
	FILE *f;

	if (appname == NULL)
		appname = getappname();

	free(pidfile_path);
	pidfile_path = NULL;

	/* _PATH_VARRUN includes trailing / */
#if defined(_WIN32)
        tmprtn = GetTempPathA(sizeof(tmppath), tmppath); //TODO/XXX
	if (tmprtn <= 0 || tmprtn >= sizeof(tmppath) ||
			asprintf(&pidfile_path, "%s%s.pid", tmppath, appname) == -1)
		return (-1);
#else
	if (asprintf(&pidfile_path, "%s%s.pid", _PATH_VARRUN, appname) == -1)
		return (-1);
#endif

	if ((f = fopen(pidfile_path, "w")) == NULL) {
		save_errno = errno;
		free(pidfile_path);
		pidfile_path = NULL;
		errno = save_errno;
		return (-1);
	}

	pid = getpid();
	if (fprintf(f, "%ld\n", (long)pid) <= 0 || fflush(f) != 0) {
		save_errno = errno;
		(void) fclose(f);
		(void) unlink(pidfile_path);
		free(pidfile_path);
		pidfile_path = NULL;
		errno = save_errno;
		return (-1);
	}
	(void) fclose(f);

	pidfile_pid = pid;
	if (atexit(pidfile_cleanup) != 0) {
		save_errno = errno;
		(void) unlink(pidfile_path);
		free(pidfile_path);
		pidfile_path = NULL;
		pidfile_pid = 0;
		errno = save_errno;
		return (-1);
	}

	return (0);
}

static void
pidfile_cleanup(void)
{
	if (pidfile_path != NULL && pidfile_pid == getpid())
		(void) unlink(pidfile_path);
}

#else
extern void __stdlibrary_has_pidfile(void);

void
__stdlibrary_has_pidfile(void)
{
}

#endif

/*end*/
