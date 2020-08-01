/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_SSTREAM_H
#define __KAI_SSTREAM_H

#include <sstream>

using std::basic_stringbuf;
using std::stringbuf;

using std::basic_istringstream;
using std::istringstream;

using std::basic_ostringstream;
using std::ostringstream;

#ifdef MSIPL_WCHART
using std::stringbuf;
using std::wistringstream;
using std::wostringstream;
#endif

#endif /* __KAI_SSTREAM_H */
