#pragma once

/* -*- mode: c; indent-width: 4; -*- */
/*
 * <getopt_long.h>
 *
 * Copyright (c) 2022, Adam Young.
 * All rights reserved.
 *
 * This file is part of oncrpc4-win32.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#include <sys/cdefs.h>

#include <getopt.h>

__BEGIN_DECLS

extern int optreset;                            /* reset getopt */

/*
 *  GNU like getopt_long() and BSD4.4 getsubopt()/optreset extensions.
 */
#define no_argument         0
#define required_argument   1
#define optional_argument   2

struct option {
        const char *name;                       /* name of long option */
        /*
         *  one of no_argument, required_argument, and optional_argument:
         *  whether option takes an argument
         */
        int has_arg;
        int *flag;                              /* if not NULL, set *flag to val when option found */
        int val;                                /* if flag not NULL, value to set *flag to; else return value */
};

extern int getopt_long(int argvc, char * const *argv, const char *options, const struct option *long_options, int *idx);
extern int getopt_long2(int argvc, char * const *argv, const char *options, const struct option *long_options, int *idx, char *buf, int buflen);

__END_DECLS

/*end*/