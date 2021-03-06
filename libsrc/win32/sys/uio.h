#pragma once

/*
 * <sys/uio.h>
 *
 * Copyright (c) 2022, Adam Young.
 * All rights reserved.
 *
 * This file is part of oncrpc4-win32.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the oncrpc4-win32 License.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Licence for details.
 * ==end==
 */

#include <sys/cdefs.h>

#include <stddef.h>         /* size_t */
#include <limits.h>         /* INT_MAX */

#define IOV_MAX             64
#if !defined(__MINGW32__)
#if !defined(SSIZE_MAX)
#define SSIZE_MAX           INT_MAX
#endif
#endif

__BEGIN_DECLS

struct iovec {
    void *     iov_base;
    int        iov_len;
};

int readv(int, const struct iovec *, int);
int writev(int, const struct iovec *, int);

__END_DECLS

/*end*/
