/*
 * WIN32 support
 */

#include <stdarg.h>

void setprogname(const char *argv0);
void errx(int rcode, const char *fmt, ...);
void err(int rcode, const char *fmt, ...);

int asprintf(char **str, const char *fmt, ...);
int vasprintf(char **str, const char *fmt, va_list ap);

size_t strlcat(char *dst, const char *src, size_t siz);
size_t strlcpy(char *dst, const char *src, size_t siz);

/*end*/
