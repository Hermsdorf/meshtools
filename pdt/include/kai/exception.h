/** -*- C++ -*-
 **
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_EXCEPTION_H
#define __KAI_EXCEPTION_H
/* 
 * <exception.h> -  header file for backwards compatibility with old-style 
 * headers without namespaces.
 *
 * This file includes <exception> followed by using declarations for
 * the 9 symbols defined by <exception>. 
 * KAI, 1997 
 */
#include <exception>

using std::exception;
using std::bad_exception;
using std::unexpected_handler;
using std::set_unexpected;
using std::unexpected;
using std::terminate_handler;
using std::set_terminate;
using std::terminate;
using std::uncaught_exception;

#endif /*__KAI_EXCEPTION_H*/
