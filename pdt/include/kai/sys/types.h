/**  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_SYS_TYPES_H
#define __KAI_SYS_TYPES_H

/*
 * For ANSI_C compatibility, remove gcc extensions in asm/types.h
 */

#define _I386_TYPES_H

#include <stddef.h>
#__include_next <sys/types.h>

#if __KAI_LONG_LONG && !defined( __GNUC__ )
  typedef __long_long int64_t;
  typedef unsigned __long_long u_int64_t;
#endif /* __KAI_LONG_LONG && !defined( __GNUC__) */

#endif /*  __KAI_SYS_TYPES_H */
