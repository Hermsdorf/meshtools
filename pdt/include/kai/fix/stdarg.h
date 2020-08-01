/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __PDT_STDARG_H
#define __PDT_STDARG_H

#if (defined(__sgi))
#ifdef __cplusplus
  extern "C" {
#endif
    long __kai_apply(const char *,...);
#ifdef __cplusplus
  }
#endif

#define __NO_CFOLD_WARNING(x) (x)
#include "/usr/include/stdarg.h"
#undef va_arg

#    define va_arg(__list, __mode) ((__mode *)(__list = (char *) \
       ( \
         (__kai_apply("__builtin_alignof(%t)",(__mode*)0) == 8) ? \
          (((long)__list + sizeof(__mode) + 8 - 1) & (~0x7) ) : \
          (((long)__list + ((sizeof(__mode) > 4)?sizeof(__mode):4) + 4 - 1) & (~0x3)) \
       ) \
       )) [-1]


#else
#ifdef __APPLE__
#define va_start(v,l)
#define __builtin_va_start(v,l)
#define __builtin_va_end(a)
#define va_end(v)
#define va_copy(d,s)

#ifdef __i386__
#include <../stdarg.h>
#else
/* #include "/usr/include/stdarg.h"  */
#endif /* __i386__ Apple Intel Macs */
#define va_arg(v,l) (l)0
#else /* __APPLE__ */
#include <../stdarg.h>
#endif /* __APPLE__ */
#endif 

#endif /* __PDT_STDARG_H */
