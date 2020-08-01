/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef KAI_COMPLEX_H
#define KAI_COMPLEX_H

#ifdef PDT_C_LANG
#include "/usr/include/complex.h"
#else
/* for C++ */
#include <complex>

using std::complex;
using std::real;
using std::imag;
using std::norm;
using std::abs;
using std::arg;
using std::conj;
using std::cos;
using std::cosh;
using std::sin;
using std::sinh;
using std::tan;
using std::tanh;
using std::exp;
using std::log;
#if KAI_NONSTD_COMPLEX
using std::inv;
#endif /* KAI_NONSTD_COMPLEX */
using std::sqrt;
using std::polar;
using std::pow;

#endif /* PDT_C_LANG */
#endif /* KAI_COMPLEX_H */
