/*
 * <randomid.h> implementation.
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

#include "namespace.h"

#include "rpc_win32.h"
#include "randomid.h"

#if defined(__MINGW32__)
#if !defined(__MINGW64__)
#include "mingw_bcrypt.h"
#endif
#else
#pragma comment(lib, "Bcrypt")
#endif


randomid_t
randomid_new(int bits, long timeo)
{
	BCRYPT_ALG_HANDLE prov = NULL;
	if (BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&prov, BCRYPT_RNG_ALGORITHM, NULL, 0))) {
		return prov;
	}
	return NULL;
}


unsigned
randomid(randomid_t ctx)
{
	unsigned rtn = 0;
	if (ctx) {
		if (BCRYPT_SUCCESS(BCryptGenRandom(ctx, (PUCHAR) (&rtn), sizeof(rtn), 0))) {
			return rtn;
		}
	}
	abort();
	return 0;
}


void
randomid_delete(randomid_t ctx)
{
	if (ctx) {
		BCryptCloseAlgorithmProvider(ctx, 0);
	}
}

/*end*/
