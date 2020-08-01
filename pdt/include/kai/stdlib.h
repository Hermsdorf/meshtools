/** -*- C++ -*-
 **
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#if !defined(_STDLIB_H_) /* OSF */ \
 && !defined(_H_STDLIB)  /* AIX */ \
 && !defined(__STDLIB_H__) /* Irix */ \
 && !defined(_STDLIB_H)  /* Solaris and Linux */ \
 && !defined(_STDLIB_INCLUDED) /* HPux */ \
 && !defined(_stdlib_h)  /* SUNos */ \
/*  Don't define these here, one of them arrives with the #include of stdlib.h */

#  ifdef __cplusplus
#    ifndef __BUILDING_RUNTIME
#      define atexit __kai_std_atexit
#    endif  /* __BUILDING_RUNTIME */
#    if defined(__sunos) || defined(_AIX)
       extern "C" {
#    endif /* __sunos || _AIX */
#  endif /* __cplusplus */
#        if defined(__ACC_INCLUDE_FILES)
#          include "inc-acc/stdlib.h"
#        elif defined(__GCC_INCLUDE_FILES)
#          include "inc-gcc/stdlib.h"
#        else
#          include "/usr/include/stdlib.h"
#        endif /* Which include directory */
#  ifdef __cplusplus
#    if defined(__sunos) || defined(_AIX)
       }
#    endif /* __sunos || _AIX */
#    ifndef __BUILDING_RUNTIME
#      undef atexit
#    endif  /* __BUILDING_RUNTIME */
#  endif /* __cplusplus */

#  pragma kai_intrinsic "malloc" malloc
#  pragma kai_intrinsic "calloc" calloc
#  pragma kai_intrinsic "realloc" realloc
#  pragma kai_intrinsic "free"   free
#  if defined(__sgi) && _XOPEN4UX && _NO_ANSIMODE
#    pragma kai_intrinsic "valloc" valloc
#  endif /* defined(__sgi) && _XOPEN4UX && _NO_ANSIMODE */

#  if defined(__cplusplus) && !defined(__BUILDING_RUNTIME)
     extern "C" {
	typedef void (*__pcvFv) (void);
	inline int atexit( __pcvFv finale ) 
	   { return __kai_std_atexit(                            finale   ); }
     }

     extern "C++" {
	typedef void (*__pCvFv) (void);
	inline int atexit( __pCvFv finale )
	   { return __kai_std_atexit( reinterpret_cast<__pcvFv>( finale ) ); }
     }
#  endif /* defined(__cplusplus) && !defined(__BUILDING_RUNTIME) */

#endif /* Nobody's stdlib.h guard macro had been defined */
