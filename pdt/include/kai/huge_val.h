/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef _HUGE_VAL_H
#define _HUGE_VAL_H 1

/* Section 4.5 of X3-159-1989 */
extern const int __kai_HUGE_VAL[2];
#define HUGE_VAL (*(const double*)(const void*)__kai_HUGE_VAL)

#endif /* _HUGE_VAL_H */
