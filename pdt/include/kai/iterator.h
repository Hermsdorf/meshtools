/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_ITERATOR_H
#define __KAI_ITERATOR_H

#include <utility.h>
#include <iosfwd.h>

#include <iterator>

using std::input_iterator_tag;
using std::output_iterator_tag;
using std::forward_iterator_tag;
using std::bidirectional_iterator_tag;
using std::random_access_iterator_tag;

#if KAI_NONSTD_ITERATOR
using std::output_iterator;
using std::input_iterator;
using std::forward_iterator;
using std::bidirectional_iterator;
using std::random_access_iterator;
#endif /* KAI_NONSTD_ITERATOR */

#if KAI_NONSTD_ITERATOR
using std::distance_type;
#endif /* KAI_NONSTD_ITERATOR */
using std::distance;
using std::advance;

#if KAI_NONSTD_ITERATOR
using std::reverse_bidirectional_iterator;
#endif /* KAI_NONSTD_ITERATOR */
using std::reverse_iterator;
using std::inserter;
using std::back_insert_iterator;
using std::back_inserter;
using std::front_insert_iterator;
using std::front_inserter;
using std::insert_iterator;
using std::istream_iterator;
using std::ostream_iterator;
using std::istreambuf_iterator;
using std::ostreambuf_iterator;

#endif /* __KAI_ITERATOR_H */
