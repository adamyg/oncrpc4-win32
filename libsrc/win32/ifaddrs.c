/*
 * <ifaddrs.h> implementation
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

#include <sys/utypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "rpc_win32.h"
#include "ifaddrs.h"

#if defined(__MINGW32__)
#if defined(__MINGW64_VERSION_MAJOR) /*MingGW-w64/32*/
#include <Iphlpapi.h>
#else
#include <iptypes.h>  /*>=0x601*/
#include <ipifcons.h>
typedef struct {
    uint64_t Value;
    struct {
        uint64_t Reserved :24;
        uint64_t NetLuidIndex :24;
        uint64_t IfType :16;
    } Info;
} NET_LUID;
WINAPI ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags, PVOID Reserved, PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer);
#endif
#else   /*!__MINGW32__*/
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#endif  /*__MINGW32__*/

typedef DWORD (WINAPI *ConvertInterfaceGuidToLuid_t)(const GUID *, NET_LUID *);
typedef DWORD (WINAPI *ConvertInterfaceLuidToNameA_t)(const NET_LUID *, char *, size_t);

static HMODULE hIphlpapi = 0;
static ConvertInterfaceGuidToLuid_t fnConvertInterfaceGuidToLuid;
static ConvertInterfaceLuidToNameA_t fnConvertInterfaceLuidToNameA;

static void String2GUID(const char *str, GUID *guid);

LIBRPC_API int
getifaddrs(struct ifaddrs **ifap)
{
	IP_ADAPTER_ADDRESSES *root, *addr;
	struct ifaddrs *prev;
	ULONG size = 0;
	ULONG ret;

	ret = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &size);
	if (ret != ERROR_BUFFER_OVERFLOW) {
		errno = EINVAL;
		return -1;
	}

	if (NULL == (root = calloc(1, size)))
		return -1;
	ret = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, root, &size);
	if (ret != ERROR_SUCCESS) {
		errno = ENOMEM;
		free(root);
		return -1;
	}

	if (! hIphlpapi) {
		hIphlpapi = LoadLibraryA("Iphlpapi");
		if (hIphlpapi) {
			fnConvertInterfaceGuidToLuid =
			    (ConvertInterfaceGuidToLuid_t)GetProcAddress(hIphlpapi, "ConvertInterfaceGuidToLuid");
			fnConvertInterfaceLuidToNameA =
			    (ConvertInterfaceLuidToNameA_t)GetProcAddress(hIphlpapi, "ConvertInterfaceLuidToNameA");
		}
	}

	for (prev = NULL, addr = root; addr; addr = addr->Next) {
		struct ifaddrs *ifa;
		char name[IFNAMSIZ] = {0};
		GUID guid = {0};
		NET_LUID luid = {0};

		if (NULL == (ifa = calloc(1, sizeof(*ifa)))) {
			freeifaddrs(*ifap);
			return -1;
		}

		if (prev) prev->ifa_next = ifa;
		else *ifap = ifa;

		String2GUID(addr->AdapterName, &guid);
		if (fnConvertInterfaceGuidToLuid && fnConvertInterfaceLuidToNameA &&
				fnConvertInterfaceGuidToLuid(&guid, &luid) == NO_ERROR &&
				fnConvertInterfaceLuidToNameA(&luid, name, sizeof(name)) == NO_ERROR) {
			ifa->ifa_name = _strdup(name);
		} else {
			ifa->ifa_name = _strdup(addr->AdapterName);
		}
		if (NULL == ifa->ifa_name) {
			freeifaddrs(*ifap);
			return -1;
		}

		if (addr->IfType & IF_TYPE_SOFTWARE_LOOPBACK)
			ifa->ifa_flags |= IFF_LOOPBACK;

		if (addr->OperStatus == IfOperStatusUp) {
			ifa->ifa_flags |= IFF_UP;

			if (addr->FirstUnicastAddress) {
				IP_ADAPTER_UNICAST_ADDRESS *cur;
				int added = 0;

				for (cur = addr->FirstUnicastAddress; cur; cur = cur->Next) {
					if (cur->Flags & IP_ADAPTER_ADDRESS_TRANSIENT ||
						cur->DadState == IpDadStateDeprecated) {
						continue;
					}

					if (added) {
						prev = ifa;
						ifa = calloc(1, sizeof(*ifa));
						prev->ifa_next = ifa;
						ifa->ifa_name  = _strdup(prev->ifa_name);
						ifa->ifa_flags = prev->ifa_flags;
					}

					ifa->ifa_addr = malloc(cur->Address.iSockaddrLength);
					memcpy(ifa->ifa_addr, cur->Address.lpSockaddr, cur->Address.iSockaddrLength);
					added = 1;
				}
			}
		}

		prev = ifa;
	}

	free(root);
	return 0;
}


static char
hex2byte(const char *str)
{
	const char c0 = str[0], c1 = str[1];
	char value;

	value =  (isdigit(c0) ? c0 - '0' : toupper(c0) - 'A' + 10) << 4;
	value |= (isdigit(c1) ? c1 - '0' : toupper(c1) - 'A' + 10);
	return value;
}


static void
String2GUID(const char *str, GUID *guid)
{
	unsigned i;
	char *end;

	/* {00112233-4455-6677-8899-AABBCCDDEEFF} */
	if (*str == '{') str++;

	/* 00112233- */
	guid->Data1 = (long)strtoul(str, &end, 16);
	str += 9;

	/* 4455- */
	guid->Data2 = (unsigned short)strtoul(str, &end, 16);
	str += 5;

	/* 6677- */
	guid->Data3 = (unsigned short)strtoul(str, &end, 16);
	str += 5;

	/* 8899- */
	guid->Data4[0] = hex2byte(str);
	str += 2;
	guid->Data4[1] = hex2byte(str);
	str += 3;

	/* AABBCCDDEEFF} */
	for (i = 0; i < 6; ++i) {
		guid->Data4[i + 2] = hex2byte(str);
		str += 2;
	}
}


LIBRPC_API void
freeifaddrs(struct ifaddrs *ifp)
{
	while (ifp) {
		struct ifaddrs *next = ifp->ifa_next;
		if (ifp->ifa_addr) free(ifp->ifa_addr);
		if (ifp->ifa_name) free(ifp->ifa_name);
		free(ifp);
		ifp = next;
	}
}

//end
