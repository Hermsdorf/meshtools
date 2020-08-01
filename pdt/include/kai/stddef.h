/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_STDDEF_H
#define __KAI_STDDEF_H

/* Hard-wired for linux on i386 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __i386
#error This include file expects i386 system.
#endif

#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ unsigned int
#endif /* __SIZE_TYPE__ */

#ifndef _SIZE_T
#define _SIZE_T
typedef __SIZE_TYPE__ size_t;
#endif /* _SIZE_T */

#ifndef __PTRDIFF_TYPE__
#define __PTRDIFF_TYPE__ int
#endif
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/* _WCHAR_T is defined if wchar_t is a keyword in C++ */
#ifndef _WCHAR_T
#ifndef __WCHAR_TYPE__
#define __WCHAR_TYPE__ long int
#endif
typedef __WCHAR_TYPE__ wchar_t;
#endif /* _WCHAR_T */

#ifndef NULL
#define NULL 0
#endif 

#ifndef offsetof
#define offsetof(s,m) ((int)&(((s *)0))->m)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KAI_STDDEF_H */
