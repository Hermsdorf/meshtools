/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_BYTESEX_H
#define __KAI_BYTESEX_H

/* Hard-code i386 values for byteorder */
#undef __BYTE_ORDER
#include <linux/version.h>

#ifndef __i386__
#error This version of bytesex.h expects i386 target.
#endif

#define __BYTE_ORDER	1234

#endif /* __KAI_BYTESEX_H */
