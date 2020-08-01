/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_STDEXCEPT_H
#define __KAI_STDEXCEPT_H

/*
 * <stdexcept.h> -  header file for backwards compatibility with old-style 
 * headers without namespaces.
 *
 * This file includes <stdexcept> followed by using declarations for
 * the 9 symbols defined in namespace std by <stdexcept>.
 * KAI, 1997
 *
 * This file also exports std::exception for reasons of backwards compatibility
 * to versions of KCC based on the April 1995 draft.
 */
#include <stdexcept>

using std::exception;
using std::logic_error;
using std::domain_error;
using std::invalid_argument;
using std::length_error;
using std::out_of_range;
using std::runtime_error;
using std::range_error;
using std::overflow_error;
using std::underflow_error;

#endif /*__KAI_STDEXCEPT_H*/

