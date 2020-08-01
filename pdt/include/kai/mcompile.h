/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
/**
 ** Lib++     : The Modena C++ Standard Library,
 **             Version 2.4, August 27th 1997
 **
 ** Copyright (c) 1994-1996 Modena Software Inc.
 **/
#ifndef MSIPL_COMPILE_H
#define MSIPL_COMPILE_H

#ifdef __cplusplus
#define MSIPL_STD_BEGIN	namespace std {
#define MSIPL_STD_END	}
#else
#define MSIPL_STD_BEGIN
#define MSIPL_STD_END
#endif

/* #include <stddef.h> */

#ifdef _BOOL
#  define MSIPL_BOOL_BUILTIN
#endif

/* Using open and fdopen make for tighter code and we actually can get the modes right. */
/* The problem is that it is not standard. */
#define __KAI_USE_FDOPEN	1

#define KAI_WCHAR_T	1
#define MSIPL_WCHART 	1

#define MSIPL_ANSI_HEADER	1
#define MSIPL_MEMBER_TEMPLATE	1
#define MSIPL_EXPLICIT_FUNC_TEMPLATE_ARG	1
#define MISPL_EXPLICIT_TEMPLATE_ARGUMENT	1
#define MSIPL_DEF_TEMPARG	1
#undef MSIPL_NON_TYPE_TEMPARG
#define MSIPL_USING_NAMESPACE 1
#define MSIPL_EXPINST_ALLOWED	1
#define MSIPL_TEMPL_NEWSPEC	1

/* Code dependent on partial specialization is enabled on a case-by-case
   basis, because ANSI Sept. '96 draft is self-contradictory about whether
   partial specializations can have default template arguments. */
#  define MSIPL_PARTIAL_TEMPL
#  define MSIPL_RELOPS /* Symbol invented by ADR - not from Modena */
#  define MSIPL_STATIC_CONST_INIT
#  undef MSIPL_FUNC_TEMPLATE_DEFARG

/* Is explicit keyword turned on? */
#ifdef _EXPLICIT  
#  define MSIPL_DEF_EXPLICIT
#else
#  define explicit
#endif /* _EXPLICIT */

/* Is typename keyword turned on? */
#ifdef _TYPENAME
#  define MSIPL_TYPENAME
#else
#  define typename
#endif /* _TYPENAME */

/*
 * Define MSIPL_MULTITHREAD to enable a basic level of MT safety.
 * At this level, the library will lock its own shared resources. The 
 * user is responsible for locking objects that the user has created.
 */
#if MSIPL_MULTITHREAD

#  define __KAI_DECL_SHARED_OBJ_LOCK(TYPE,OBJ)  __kai::TYPE OBJ;
#  define __KAI_LOCK(TYPE,MUT_BLOCK,MUT)  	__kai::mutex_block<__kai::TYPE> MUT_BLOCK(MUT)

#  define __KAI_CONCAT_HACK(A,B)        A##B
#  define __KAI_SREAD_LOCK(TYPE,MUT)	__KAI_LOCK(TYPE,__KAI_CONCAT_HACK(SREAD_,__LINE__), MUT)
#  define __KAI_SWRITE_LOCK(TYPE,MUT)	__KAI_LOCK(TYPE,__KAI_CONCAT_HACK(SWRITE_,__LINE__), MUT)
#  define __KAI_LOCK_STREAM(is)		__KAI_LOCK(recursive_mutex,__KAI_CONCAT_HACK(STREAM_,__LINE__), (is).__kai_get_mutex())

#  define __KAI_DEC_STATIC_MUTEX(OBJ,ID)	static __kai::static_mutex<__kai::ID> OBJ;
#  define __KAI_LOCK_STATIC(MUT_BLOCK,MUT,ID)	__kai::mutex_block<__kai::static_mutex<__kai::ID> > MUT_BLOCK(MUT)

#  define __KAI_DEC_MUTEX_ARITH(TYPE, MUTEX, OBJ) __kai::mutex_arith<TYPE, MUTEX> OBJ

#else	/* not MSIPL_MULTITHREAD */

/* Current version is not thread safe.  Making it thread safe at the
   library level will incur a very high performance penalty for all users,
   multithreaded or not! */
#  define __KAI_DECL_SHARED_OBJ_LOCK(TYPE,OBJ)
#  define __KAI_DEC_MUTEX_ARITH(TYPE, MUTEX, OBJ) TYPE OBJ
#  define __KAI_DEC_STATIC_MUTEX(OBJ,ID)
#  define __KAI_LOCK_STATIC(MUT_BLOCK, MUT, ID)
#  define __KAI_LOCK(TYPE,MUT_BLOCK,MUT)
#  define __KAI_SREAD_LOCK(TYPE,MUT)
#  define __KAI_SWRITE_LOCK(TYPE,MUT)
#  define __KAI_LOCK_STREAM(is)
#endif	/* not MSIPL_MULTITHREAD */


#ifndef MSIPL_BOOL_BUILTIN
#  define bool char	/* sizeof(bool)==sizeof(char) for KCC */
#  define true 1
#  define false 0
#endif

/* does your compiler instantiate const ARG as a
   template parameter properly */
#define M_CONST const

#if _EXCEPTIONS
#  define MSIPL_RETHROW	throw;
#  define MSIPL_TRY try
#  define MSIPL_CATCH catch(...)
#  define MSIPL_THROW throw()
#  define MSIPL_THROW_STR(strList) throw(strList)
#else
#  define MSIPL_RETHROW
#  define MSIPL_TRY
#  define MSIPL_CATCH if (0)
#  define MSIPL_THROW 
#  define MSIPL_THROW_STR(strList)
#endif


#define MSIPL_USING_STD using namespace std;
#define MSIPLSTD std

#ifndef __KAI_STRICT
#define __KAI_STRICT	0
#endif

namespace __kai {
// KAI changed streamsize from int to long for some platforms. 
// Type should match vendor's C++ version where possible.  Following gives
// ideal results for:
//	RS/6000
//	32-bit SGI
//	32-bit Sparc
#if (defined(__sgi)&&_MIPS_SZLONG==32) || defined(_AIX)
typedef int  streamoff_t;
typedef int  streamsize_t;
#else
typedef long streamoff_t;
typedef long streamsize_t;
#endif 
}	// namespace __kai

#if defined(__linux)||defined(__alpha)||defined(__sparc)
#define __KAI_MBSTATE_IS_INCOMPARABLE	1
#endif

#if MSIPL_MULTITHREAD
#include <mutex>
#endif

#endif /* MSIPL_COMPILE_H */

