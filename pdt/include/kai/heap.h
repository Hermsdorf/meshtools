/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_HEAP_H
#define __KAI_HEAP_H

#if KAI_NONSTD_HEAP
// heap is a subset of algorithm

#include <heap>

using std::push_heap;
using std::pop_heap;
using std::make_heap;
using std::sort_heap;
#endif /* KAI_NONSTD_HEAP */

#endif /* __KAI_HEAP_H */

