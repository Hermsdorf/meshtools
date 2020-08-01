/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_TYPEINFO_H
#define __KAI_TYPEINFO_H

/*
 * <typeinfo.h> - header file for backwards compatibility with old-style 
 * headers without namespaces.
 *
 * This file includes <typeinfo> followed by using declarations for
 * the 3 symbols defined in namespace std by <typeinfo>.
 * KAI, 1995
 */

#include <stdexcept.h>
#include <typeinfo>

using std::type_info;
using std::bad_cast;
using std::bad_typeid;

#endif /* __KAI_TYPEINFO_H */

