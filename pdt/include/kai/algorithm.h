/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#ifndef __KAI_ALGORITHM_H
#define __KAI_ALGORITHM_H

#include <algorithm>

// 25.1 non-modifying sequence operations:
using std::for_each;
using std::find;
using std::find_if;
using std::find_end;
using std::find_first_of;
using std::adjacent_find;
using std::count;
using std::count_if;
using std::mismatch;		// from algobase
using std::equal;		// from algobase
using std::search;
using std::search_n;

// 25.2 modifying sequence operations:
using std::copy;		// from algobase
using std::copy_backward;	// from algobase
using std::swap_ranges;
using std::iter_swap;		// from algobase
using std::transform;
using std::replace;
using std::replace_if;
using std::replace_copy;
using std::replace_copy_if;
using std::fill;		// from algobase
using std::fill_n;		// from algobase
using std::generate;
using std::generate_n;		// from algobase
using std::remove;
using std::remove_if;
using std::remove_copy;
using std::remove_copy_if;
using std::unique;
using std::unique_copy;
using std::reverse;
using std::reverse_copy;
using std::rotate;
using std::rotate_copy;
using std::random_shuffle;
using std::partition;
using std::stable_partition;

// 25.3.1 sorting:
using std::sort;
using std::stable_sort;
using std::partial_sort;
using std::partial_sort_copy;
using std::nth_element;

// 25.3.3 binary search:
using std::lower_bound;
using std::upper_bound;
using std::equal_range;
using std::binary_search;

// 25.3.4 merge:
using std::merge;
using std::inplace_merge;

// 25.3.5 set operatons:
using std::includes;
using std::set_union;
using std::set_intersection;
using std::set_difference;
using std::set_symmetric_difference;

// 25.3.6 heap operations:
using std::push_heap;		// from heap
using std::pop_heap;		// from heap
using std::make_heap;		// from heap
using std::sort_heap;		// from heap

// 25.3.7 minimum and maximum:
using std::max;			// from algobase
using std::min;			// from algobase
using std::max_element;
using std::min_element;
using std::lexicographical_compare;	// from algobase

// 25.3.9 permutations:
using std::next_permutation;
using std::prev_permutation;

#endif /* __KAI_ALGORITHM_H */

