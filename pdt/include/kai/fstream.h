/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef KAI_FSTREAM_H
#define KAI_FSTREAM_H

#include <fstream>
#include <iostream.h>	/* Add namespace pollution that old code expect! */

using std::basic_filebuf;
using std::filebuf;

using std::basic_ifstream;
using std::ifstream;

using std::basic_ofstream;
using std::ofstream;

using std::basic_fstream;
using std::fstream;

#ifdef MSIPL_WCHART
using std::wfilebuf;
using std::wifstream;
using std::wofstream;
using std::wfstream;
#endif

#endif /* KAI_FSTREAM_H */
