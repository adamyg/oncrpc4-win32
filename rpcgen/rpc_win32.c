/*
 * WIN32 support
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(HAVE_CONFIG_H)
#include "../liboncrpc/w32config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rpc_win32.h"

static const char RPCGEN[] = {"rpcgen"};
static const char *__progname = RPCGEN;

void
setprogname(const char *name)
{
	const char *p1 = strrchr(name, '\\'),
		*p2 = strrchr(name, '/');
	char *p;

	if (p1 || p2) {                         //last component.
		name = (p1 > p2 ? p1 : p2) + 1; //consume leading path.
	}

	if (__progname != RPCGEN) free((char *)__progname);
	__progname = strdup(name);              //clone buffer.

	if (NULL != (p = strrchr(__progname, '.')) && 0 == stricmp(p, ".exe")) {
		*p = 0;                         //consume trailing exe/com extension.
	}

	for (p = (char *)__progname; *p; ++p) { //hide case issues.
		*p = (char)tolower(*p);
	}
}


void
err(int rcode, const char *fmt, ...)
{
	const int xerrno = errno;
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "%s: ", __progname);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %s (%d)", strerror(xerrno), xerrno);
	va_end(ap);
	exit(rcode);
}


void 
errx(int rcode, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "%s: ", __progname);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(rcode);
}


int
vasprintf(char **str, const char *fmt, va_list ap)
{
	va_list tap;
	char *buf = NULL;
	int osize, size;

	if (NULL == str || NULL == fmt) {
		errno = EINVAL;
		return -1;
	}

	va_copy(tap, ap);
	osize = _vsnprintf(NULL, 0, fmt, tap);
	if (osize < 0 || (NULL == (buf = (char *)malloc(osize + 16)))) {
		size = -1;
	} else {
		size = _vsnprintf(buf, osize + 15, fmt, ap);
		assert(size == osize);
	}
	*str = buf;
	va_end(tap);

	return size;
}


int
asprintf(char **str, const char *fmt, ...)
{
	va_list ap;
	int size;

	va_start(ap, fmt);
	size = vasprintf(str, fmt, ap);
	va_end(ap);

	return size;
}


#if !defined(HAVE_STRLCAT)
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	while (n-- != 0 && *d != '\0') d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));
}
#endif /*HAVE_STRLCAT*/


#if !defined(HAVE_STRLCPY)
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	if (n == 0) {
		if (siz != 0) *d = '\0';
		while (*s++)
			;
	}

	return(s - src - 1);
}
#endif /*HAVE_STRLCPY*/

/*end*/
