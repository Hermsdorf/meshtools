/** -*- C++ -*-
 **
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_FUNCTIONAL_H
#define __KAI_FUNCTIONAL_H

#include <functional>

// clause 20.3.1, base:
using std::unary_function;
using std::binary_function;

// clause 20.3.2, arithmetic operations
using std::plus;
using std::minus;
using std::multiplies;
using std::divides;
using std::modulus;
using std::negate;

// clause 20.3.3, comparisons:
using std::equal_to;
using std::not_equal_to;
using std::greater;
using std::less;
using std::greater_equal;
using std::less_equal;

// clause 20.3.4, logical operations:
using std::logical_and;
using std::logical_or;
using std::logical_not;

// clause 20.3.5, negators:
using std::unary_negate;
using std::not1;
using std::binary_negate;
using std::not2;

// clause 20.3.6, binders:
using std::binder1st;
using std::bind1st;
using std::binder2nd;
using std::bind2nd;

// clause 20.3.7, adaptors for pointers to functions:
using std::pointer_to_unary_function;
using std::pointer_to_binary_function;
using std::ptr_fun;

// clause 20.3.8, adaptors for pointers to members
using std::mem_fun_t;
using std::mem_fun1_t;
using std::mem_fun_ref_t;
using std::mem_fun1_ref_t;
using std::mem_fun;
using std::mem_fun_ref;

#if KAI_NONSTD_FUNCTIONAL
using std::mem_fun1;	// use mem_fun instead.
using std::mem_fun1_ref;	// use mem_fun_ref instead.
#endif /* KAI_NONSTD_FUNCTIONAL */

using std::const_mem_fun_t;
using std::const_mem_fun1_t;
using std::const_mem_fun_ref_t;
using std::const_mem_fun1_ref_t;

#if KAI_NONSTD_FUNCTIONAL
// Extensions inherited from HP's STL.
using std::unary_compose;
using std::compose1;
using std::binary_compose;
using std::compose2;
using std::select1st;
using std::select2nd;
using std::ident;
#endif /* KAI_NONSTD_FUNCTIONAL */

#endif /* __KAI_FUNCTIONAL_H */
