/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_OSTREAM_H
#define __KAI_OSTREAM_H

#include <ostream>

using std::basic_ostream;
using std::ostream;
#ifdef MSIPL_WCHART
using std::wostream;
#endif

using std::endl;
using std::ends;
using std::flush;

using std::operator<<;

#endif /* __KAI_OSTREAM_H */

