/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/

#ifndef __PDT_KAI_LIMITS_H
#define __PDT_KAI_LIMITS_H


#ifdef __APPLE__
/*
 * ANSI limits
 */
#define CHAR_BIT        8               /* max # of bits in a "char" */
#define CHAR_MAX        UCHAR_MAX       /* max value of a "char" */
#define CHAR_MIN        0               /* min value of a "char" */
#define INT_MAX         2147483647      /* max value of an "int" */
#define INT_MIN         (-2147483647-1) /* min value of an "int" */
#define SCHAR_MIN       (-128)          /* min value of a "signed char" */
#define SCHAR_MAX       127             /* max value of a "signed char" */
#define UCHAR_MAX       255U            /* max value of an "unsigned char" */

#define MB_LEN_MAX      5


#define SHRT_MIN        (-32768)        /* min value of a "short int" */
#define SHRT_MAX        32767           /* max value of a "short int" */
#define USHRT_MAX       65535U          /* max value of an "unsigned short int" */
#define UINT_MAX        4294967295U     /* max value of an "unsigned int" */
#define LONG_MIN        (-2147483647-1)         /* min value of a "long int" */
#define LONG_MAX        2147483647      /* max value of a "long int" */
#define ULONG_MAX       4294967295U     /* max value of an "unsigned long int" */
#define LLONG_MIN       (-9223372036854775807LL-1LL)
#define LLONG_MAX       9223372036854775807LL
#define ULLONG_MAX      18446744073709551615LLU
#define LONGLONG_MIN        (-9223372036854775807LL-1LL) /* min "long long int" */
#define LONGLONG_MAX        9223372036854775807LL /* max "long long int" */
#define ULONGLONG_MAX       18446744073709551615LLU /* max "unsigned long long int" */

#define _LIMITS_H___ 1

#else
#ifdef __STRICT_ANSI__
#include "/usr/include/limits.h"
#else /* GNU mode */
#define CHAR_BIT        8               /* max # of bits in a "char" */
#define CHAR_MAX        UCHAR_MAX       /* max value of a "char" */
#define CHAR_MIN        0               /* min value of a "char" */
#define INT_MAX         2147483647      /* max value of an "int" */
#define INT_MIN         (-2147483647-1) /* min value of an "int" */
#define SCHAR_MIN       (-128)          /* min value of a "signed char" */
#define SCHAR_MAX       127             /* max value of a "signed char" */
#define UCHAR_MAX       255U            /* max value of an "unsigned char" */

#define MB_LEN_MAX      5


#define SHRT_MIN        (-32768)        /* min value of a "short int" */
#define SHRT_MAX        32767           /* max value of a "short int" */
#define USHRT_MAX       65535U          /* max value of an "unsigned short int" */
#define UINT_MAX        4294967295U     /* max value of an "unsigned int" */
#define LONG_MIN        (-2147483647-1)         /* min value of a "long int" */
#define LONG_MAX        2147483647      /* max value of a "long int" */
#define ULONG_MAX       4294967295U     /* max value of an "unsigned long int" */
#define LLONG_MIN       (-9223372036854775807LL-1LL)
#define LLONG_MAX       9223372036854775807LL
#define ULLONG_MAX      18446744073709551615LLU
#define LONGLONG_MIN        (-9223372036854775807LL-1LL) /* min "long long int" */
#define LONGLONG_MAX        9223372036854775807LL /* max "long long int" */
#define ULONGLONG_MAX       18446744073709551615LLU /* max "unsigned long long int" */
#define _GCC_LIMITS_H_

#include "/usr/include/limits.h"
#endif /* STRICT_ANSI/GNU mode */
#endif /* __APPLE__ */

#endif /* __PDT_KAI_LIMITS_H */
