#pragma once
/*
 *  Copyright (c) 2020 - 2022, Adam Young.
 *  All rights reserved.
 *
 *  This file is part of oncrpc4-win32.
 *
 *  The applications are free software: you can redistribute it
 *  and/or modify it under the terms of the oncrpc4-win32 License.
 *
 *  Redistributions of source code must retain the above copyright
 *  notice, and must be distributed with the license document above.
 *
 *  Redistributions in binary form must reproduce the above copyright
 *  notice, and must include the license document above in
 *  the documentation and/or other materials provided with the
 *  distribution.
 *
 *  This project is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the Licence for details.
 *  ==end==
 */

#include "satomic.h"

typedef struct pthread_instance {
    unsigned magic;
    HANDLE handle;
    satomic_lock_t joining;
    unsigned id;
    void *(*routine)(void *);
    void *arg;
    void *ret;
} pthread_instance_t;

pthread_instance_t *_pthread_instance(int create);
void _pthread_tls_cleanup(pthread_instance_t *);

/*end*/
