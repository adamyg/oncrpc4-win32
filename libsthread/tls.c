/*
 *  Simple win32 threads - thread local storage.
 *
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

#include "sthread.h"
#include "thread_instance.h"

#include <sys/socket.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

typedef void(*destructor_t)(void *);

typedef struct tlsatom {
    unsigned active;
    destructor_t destructor;
    DWORD key;  
} tlskeys_t[PTHREAD_MAX_KEYS];

static tlskeys_t tlskeys = {0};
static satomic_lock_t tlslock;

struct tlsvalue {
    unsigned magic;
    void *data;
};


int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    int tls_idx = -1;
    DWORD t_key;

    if (! key) return EINVAL;

    struct tlsvalue *value;
    if (NULL == (value = calloc(1, sizeof(struct tlsvalue)))) {
        return ENOMEM;
    }   

    value->magic = TLS_MAGIC;
    value->data = NULL;

    satomic_lock(&tlslock);
    for (int idx = 0; idx < PTHREAD_MAX_KEYS; ++idx) {
        if (0 == tlskeys[idx].active) {
            tlskeys[idx].destructor = destructor;
            tlskeys[idx].active = 1;
            tls_idx = idx;
            break;
        }
    }
    satomic_unlock(&tlslock);

    if ((t_key = TlsAlloc()) == TLS_OUT_OF_INDEXES ||
            ! TlsSetValue(t_key, (LPVOID)value)) {
        if (tls_idx >= 0) {
            tlskeys[tls_idx].active = 0;
        }
        free(value);
        return EAGAIN;
    }

    if (tls_idx >= 0) {
        tlskeys[tls_idx].key = t_key;
    }
    *key = t_key;

    return 0;
}


void
_pthread_tls_cleanup(pthread_instance_t *instance)
{
    tlskeys_t t_tlskeys;
    unsigned it, idx;

    /* snapshot */
    satomic_lock(&tlslock);
    memcpy(t_tlskeys, (const void *)tlskeys, sizeof(t_tlskeys));
    satomic_unlock(&tlslock);

    /* envoke destructors */
    for (it = 0; it < PTHREAD_DESTRUCTOR_ITERATIONS; ++it) {
        unsigned count = 0;

        for (idx = 0; idx < PTHREAD_MAX_KEYS; ++idx) {
            destructor_t destructor = NULL;

            if (t_tlskeys[idx].active) {
                if (NULL == (destructor = t_tlskeys[idx].destructor)) {
                    struct tlsvalue *value;
                    void *data;

                    if (NULL != (value = TlsGetValue(t_tlskeys[idx].key))) {
                        assert(TLS_MAGIC == value->magic);
                        if (NULL != (data = value->data)) {
                            value->data = NULL;
                            destructor(data);
                            ++count;
                        }
                    }
                }
            }
        }

        if (0 == count)
            break;
    }

    /* release local containers */
    for (idx = 0; idx < PTHREAD_MAX_KEYS; ++idx) {
        if (t_tlskeys[idx].active) {
            struct tlsvalue *value;

            if (NULL != (value = TlsGetValue(t_tlskeys[idx].key))) {
                assert(TLS_MAGIC == value->magic);
                free(value);
            }
        }
    }
}


int
pthread_key_delete(pthread_key_t key)
{
    struct tlsvalue *value;
    if (NULL != (value = TlsGetValue(key))) {
        assert(TLS_MAGIC == value->magic);

        satomic_lock(&tlslock);
        for (unsigned idx = 0; idx < PTHREAD_MAX_KEYS; ++idx) {
            if (tlskeys[idx].active) {
                if (key == tlskeys[idx].key) {
                    tlskeys[key].key = 0;
                    tlskeys[key].destructor = NULL;
                    tlskeys[key].active = 0;
                    break;
                }
            }
        }
        satomic_unlock(&tlslock);

        if (TlsFree(key)) {
            free(value);
            return 0;
        }
    }
    return EINVAL;
}


int
pthread_setspecific(pthread_key_t key, const void *pointer)
{
    struct tlsvalue *value;
    if (NULL != (value = TlsGetValue(key))) {
        assert(TLS_MAGIC == value->magic);
        value->data = (void *)pointer;
        return 0;
    }
    return EINVAL;
}


void *
pthread_getspecific(pthread_key_t key)
{
    struct tlsvalue *value;
    if (NULL != (value = TlsGetValue(key))) {
        assert(TLS_MAGIC == value->magic);
        return value->data;
    }
    return NULL;
}

/*end*/
