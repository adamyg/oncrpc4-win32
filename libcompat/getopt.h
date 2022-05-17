#pragma once

/* 
 *  <getopt.h> - public domain version
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

extern int opterr;          /* if error message should be printed */
extern int optind;          /* index into parent argv vector */
extern int optopt;          /* character checked for validity */
extern char *optarg;        /* optional argument */ 

extern int getopt(int argc, char * const *argv, const char *opts);

__END_DECLS

/*end*/
