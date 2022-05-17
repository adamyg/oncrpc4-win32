#pragma once

/*
 *  <err.h> - Formatted error message.
 *
 *  The err() and warn() family of functions display a formatted error message on stderr. For a comparison of the members of this family, see err().
 *
 *  The vwarn() function produces a message that consists of:
 *
 *      o the last component of the program name, followed by a colon and a space
 *
 *      O the formatted message, followed by a colon and a space, if the fmt argument isn't NULL
 *
 *      O the string associated with the current value of errno
 *
 *      o a newline character
 *
 *  The vwarnx() function produces a similar message, except that it doesn't include the string associated with errno. The message consists of:
 *
 *      o the last component of the program name, followed by a colon and a space
 *
 *      o the formatted message, if the fmt argument isn't NULL
 *
 *      o a newline character
 */

#include <sys/cdefs.h>
#include <stdarg.h>

__BEGIN_DECLS

void warn(const char *fmt, ...);
void vwarn(const char *fmt, va_list ap);
void warnx(const char *fmt, ...);
void vwarnx(const char *fmt, va_list ap);

void err(int rcode, const char *fmt, ...);
void verr(int rcode, const char *fmt, va_list ap);
void errx(int rcode, const char *fmt, ...);
void verrx(int eval, const char *fmt, va_list ap);

__END_DECLS

/*end*/
