/*
 *  getprogname
 */

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "libcompat.h"

#if !defined(HAVE_GETPROGNAME)

#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>

static const char *xprogname = NULL;

const char *
getprogname(void)
{
	if (NULL == xprogname) {
		char t_buffer[1024];
		DWORD buflen;

		if ((buflen = GetModuleFileNameA(NULL, t_buffer, sizeof(t_buffer)-1)) > 0) {
			t_buffer[buflen] = 0;
			setprogname(t_buffer);
		}
	}
	return (xprogname ? xprogname : "program");
}


void
setprogname(const char *progname)
{
	free((void *)xprogname);
	xprogname = NULL;

	if (progname && *progname) {
		char *dot;

		xprogname = _strdup(progname);
		if (NULL != (dot = strrchr(xprogname, '.')) && 0 == _stricmp(dot, ".exe")) {
			*dot = 0;
		}
	}
}

#else
extern void __stdlibrary_has_getprogname(void);

void
__stdlibrary_has_getprogname(void)
{
}

#endif

/*end*/
