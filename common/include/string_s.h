/*
   Copyright 2010-2016 Intel Corporation

   This software is licensed to you in accordance
   with the agreement between you and Intel Corporation.

   Alternatively, you can use this file in compliance
   with the Apache license, Version 2.


   Apache License, Version 2.0

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/**
********************************************************************************
**
**    @file string_s.h
**
**    @brief  Contains safe string implementation above posix string and mem functions
**
**
********************************************************************************
*/
#ifndef _STRING_S_H
#define _STRING_S_H

#ifndef WIN32

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

typedef int        errno_t;

#define sscanf_s sscanf

#ifdef __cplusplus
extern "C" {
#endif
/* Redifine Win32 secure string functions to Posix */
static inline errno_t memcpy_s(void *dest, size_t dest_cnt, const void *src, size_t n)
{
	if (dest_cnt < n)
		return ERANGE;

	if (dest == NULL || src == NULL)
		return EINVAL;

	memcpy(dest, src, n);
	return 0;
}
static inline errno_t strcpy_s(char *dest, size_t n, const char *src)
{
	if (dest == NULL || src == NULL)
		return EINVAL;
	if (n == 0)
		return ERANGE;
	/* FIXME: need also check for strlen(src) < n */
	strncpy(dest, src, n);
	return 0;
}

static inline int _waccess_s(const char *path, int mode)
{
	if (path == NULL)
		return EINVAL;
	return access(path, mode);
}

static inline int _wremove(const char *path)
{
	if (path == NULL)
		return EINVAL;
	return remove(path);
}

static inline int _wrename(const char *oldname, const char *newname)
{
	if (oldname == NULL || newname == NULL)
		return EINVAL;
	return rename(oldname, newname);
}

static inline int sprintf_s(char *str, size_t size, const char *format, ...)
{
	if (str == NULL || format == NULL)
		return EINVAL;
	if (size == 0)
		return ERANGE;

	va_list va;
	va_start(va, format);
	int ret = vsnprintf(str, size, format, va);
	va_end(va);
	return ret;
}

#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WIN32
#endif // _STRING_S_H
