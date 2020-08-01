/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef _KAI_MATH_H
#define _KAI_MATH_H

#__include_next <math.h>

#pragma kai_intrinsic "exp" exp expl
#pragma kai_intrinsic "sin" sin sinl
#pragma kai_intrinsic "cos" cos cosl
#pragma kai_intrinsic "fabs" fabs fabsl
#pragma kai_intrinsic "sqrt" sqrt sqrtl
#pragma kai_intrinsic "atan" atan atanl
#pragma kai_intrinsic "atan2" atan2 atan2l
#pragma kai_intrinsic "log" log logl

#endif /* _KAI_MATH_H */
