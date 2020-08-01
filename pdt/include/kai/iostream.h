/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_IOSTREAM_H
#define __KAI_IOSTREAM_H

#include <mcompile.h>

#include <iostream>
#include <istream.h>
#include <ostream.h>
#include <fstream.h>
#include <streambuf.h>

using std::cin;
using std::cout;
using std::clog;
using std::cerr;

#ifdef MSIPL_WCHART
using std::wcin;
using std::wcout;
using std::wclog;
using std::wcerr;
#endif


#if KAI_NONSTD_IOSTREAM
using std::istream_withassign;
using std::ostream_withassign;
using std::iostream_withassign;
using std::iostream;
#endif /* KAI_NONSTD_IOSTREAM */

#endif /* __KAI_IOSTREAM_H */

