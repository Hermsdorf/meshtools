/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifdef __cplusplus
#ifndef __KAI_MEMORY_H
#define __KAI_MEMORY_H

#include <memory>

// Export symbols from Section 20.4 [lib.memory] of ISO C++ Standard.
using std::allocator;
using std::operator==;
using std::operator!=;
using std::raw_storage_iterator;
using std::get_temporary_buffer;
using std::return_temporary_buffer;
using std::uninitialized_copy;
using std::uninitialized_fill;
using std::uninitialized_fill_n;
using std::auto_ptr;

#endif /* __KAI_MEMORY_H */
#else

#__include_next "memory.h" 		/* Use system <memory.h> for C */

#endif /*__cplusplus*/
