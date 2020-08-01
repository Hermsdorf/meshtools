/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_NEW_H
#define __KAI_NEW_H

/*
 * <new.h> - header file for backwards compatibility with old-style headers
 * without namespaces.
 *
 * This file includes <new> followed by using declarations for
 * the 34 symbols defined in namespace std by <new>.
 */

#include <new>

// ANSI symbols
using std::bad_alloc;
using std::nothrow;
using std::nothrow_t;
using std::new_handler;		
using std::set_new_handler;

// Backwards compatibility for old codes.
using std::__new_handler;

#endif /* __KAI_NEW_H */
