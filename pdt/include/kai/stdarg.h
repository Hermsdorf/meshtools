/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_STDARG_H
#define __KAI_STDARG_H

/* GNU code that includes <varargs.h> also erroneously 
   includes <stdargs.h> on occasion. The following turns
   off <stdarg.h> if <varargs.h> has already been included. */
#ifndef __KAI_VARARGS_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

void * __kai_apply(const char *,...);

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef void * __gnuc_va_list;
#endif /*__GNUC_VA_LIST*/

#ifndef _VA_LIST
#define _VA_LIST
typedef __gnuc_va_list va_list;
#endif /*_VA_LIST_*/

#define va_start(AP, LASTARG) \
  (__kai_apply("(%a = ((__gnuc_va_list)__builtin_next_arg (%e)))", &AP, LASTARG ))

void va_end (__gnuc_va_list);
#define va_end(pvar)    ((void)0)

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
   *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __KAI_VARARGS_H */
#endif /* __KAI_STDARG_H */
