/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef _KAI_ALLOCA_H
#define _KAI_ALLOCA_H       1

#ifdef __cplusplus
extern "C"
#endif /*__cplusplus*/
    void * __kai_apply(const char *,...);

#undef alloca
#define alloca(size) __kai_apply("__builtin_alloca(%e)",(size))

#endif /*_KAI_ALLOCA_H*/

