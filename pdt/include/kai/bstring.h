/**
 **  KAI C++ Compiler
 **
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_BSTRING_H
#define __KAI_BSTRING_H

#include <string>

// subsclause 21.1, character traits:
using std::char_traits;

// subclause 21.3, basic string:
using std::basic_string;
using std::operator+;
using std::operator==;
using std::operator!=;
using std::operator<;
using std::operator>;
using std::operator<=;
using std::operator>=;

// subclause 21.3.7.8
//using std::swap;
using std::operator>>;
using std::operator<<;
using std::getline;
using std::string;
#ifdef MSIPL_WCHART
using std::wstring;
#endif

#endif /* __KAI_BSTRING_H */
