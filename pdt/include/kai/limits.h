/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/

#ifndef __KAI_LIMITS_H
#define __KAI_LIMITS_H

#__include_next <limits.h>

/* Ensure suffices "U" and "L" are used */

#undef  UCHAR_MAX
#define	UCHAR_MAX	255U
#ifdef  __CHAR_UNSIGNED
#  undef   CHAR_MAX
#  define  CHAR_MAX     UCHAR_MAX
#endif

#undef	USHRT_MAX
#define	USHRT_MAX	65535U

#undef	UINT_MAX
#define	UINT_MAX	4294967295U

#undef	LONG_MIN
#define	LONG_MIN	(-2147483647L-1L)

#undef	LONG_MAX
#define	LONG_MAX	2147483647L

#undef	ULONG_MAX
#define	ULONG_MAX	4294967295UL

#endif /* __KAI_LIMITS_H */
