/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_VARARGS_H
#define __KAI_VARARGS_H

#ifdef __cplusplus
#error <varargs.h> is illegal in C++!
#endif

#define va_dcl int va_alist;
#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef void * __gnuc_va_list;
#endif /*__GNUC_VA_LIST*/

#ifndef _VA_LIST
#define _VA_LIST
typedef __gnuc_va_list va_list;
#endif /*_VA_LIST_*/

/* Undefine the macros in case <stdarg> was included too. */
#undef va_start
#undef va_end
#undef __va_rounded_size
#undef va_arg

#define va_start(AP)  AP=(char *) &va_alist;
#define va_end(AP)      ((void)0)
#define __va_rounded_size(TYPE)						\
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_arg(AP, TYPE)						\
(AP = (void *) ((char *) (AP) + __va_rounded_size (TYPE)),	\
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))

#endif /*__KAI_VARARGS_H*/
