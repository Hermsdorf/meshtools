/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_IOSFWD_H
#define __KAI_IOSFWD_H

#include <iosfwd>

using std::char_traits;
using std::allocator;

using std::basic_ios;
using std::basic_istream;
using std::basic_ostream;
using std::basic_iostream;

using std::basic_streambuf;
using std::basic_stringbuf;
using std::basic_filebuf;
using std::basic_ifstream;
using std::basic_ofstream;
using std::basic_fstream;
using std::basic_istringstream;
using std::basic_ostringstream;
using std::basic_stringstream;
using std::basic_istreambuf_iterator;
using std::basic_ostreambuf_iterator;

using std::ios;
using std::istream;
using std::ostream;
using std::iostream;

using std::streambuf;
using std::filebuf;
using std::stringbuf;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::istringstream;
using std::ostringstream;
using std::stringstream;

#ifdef MSIPL_WCHART
using std::wios;
using std::wistream;
using std::wostream;
using std::wiostream;

using std::wstreambuf;
using std::wfilebuf;
using std::wstringbuf;
using std::wifstream;
using std::wofstream;
using std::wfstream;
using std::wistringstream;
using std::wostringstream;
using std::wstringstream;
#endif

using std::fpos;
using std::streampos;
#ifdef MSIPL_WCHART
using std::wstreampos;
#endif

#endif /* __KAI_IOSFWD_H */
