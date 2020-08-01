/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_IOS_H
#define __KAI_IOS_H

#include <ios>

using std::streamoff;
using std::streamsize;
//using std::fstate_t;
using std::streampos;
using std::ios_base;
using std::basic_ios;
using std::ios;

#ifdef MSIPL_WCHART
using std::wstreampos;
using std::wios;
#endif

using std::boolalpha;
using std::noboolalpha;
using std::showbase;
using std::noshowbase;
using std::showpoint;
using std::noshowpoint;
using std::showpos;
using std::noshowpos;
using std::skipws;
using std::noskipws;
using std::uppercase;
using std::nouppercase;

using std::internal;
using std::left;
using std::right;

using std::dec;
using std::hex;
using std::oct;

using std::fixed;
using std::scientific;

#endif /* __KAI_IOS_H */
