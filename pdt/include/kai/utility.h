/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_UTILITY_H
#define __KAI_UTILITY_H

#include <utility>

// clause 20.1.1, operators:
using std::rel_ops::operator!=;
using std::rel_ops::operator>;
using std::rel_ops::operator<=;
using std::rel_ops::operator>=;

// clause 20.2.2, pairs:
using std::pair;
using std::operator==;
using std::operator<;
using std::operator!=;
using std::operator>;
using std::operator>=;
using std::operator<=;
using std::make_pair;

#endif /* __KAI_UTILITY_H */
