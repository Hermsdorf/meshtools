/** -*- C++ -*-
 **
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001 Intel Corp. All rights reserved.
 **/
#if !defined(_PTHREAD_HEADER) /* OSF */ \
 && !defined(_PTHREAD_H)  /* Irix, Solaris, and Linux */ \
 && !defined(_PTHREAD_H_)  /* AIX */ \
 && !(defined(PTHREAD) && defined(__hpux)) /* HPux */
/*  Don't define these here, one of them arrives with the #include of pthread.h */
#if defined(_AIX) && defined(__cplusplus) 
#  if _AIX < 43
     extern "C" {
#  endif
#endif

#if defined(__hpux) && !defined(__hpux_11)
#      include "/opt/dce/include/pthread.h"
#else
#      include "/usr/include/pthread.h"
#endif

#if defined(_AIX) && defined(__cplusplus) 
#  if _AIX < 43
     }
#  endif
#endif

#ifdef __hpux_11
# ifdef __lint
#    pragma kai_intrinsic "pthread_create"             pthread_create
# else /* __lint */
#    pragma kai_intrinsic "pthread_create"             __pthread_create_system
# endif /* __lint */
#else /* not an HP-UX system */
# pragma kai_intrinsic "pthread_create"               pthread_create
#endif /* __hpux */

#pragma kai_intrinsic "pthread_join"                 pthread_join

#pragma kai_intrinsic "pthread_mutex_init"           pthread_mutex_init
#pragma kai_intrinsic "pthread_mutex_destroy"        pthread_mutex_destroy
#pragma kai_intrinsic "pthread_mutex_lock"           pthread_mutex_lock
#pragma kai_intrinsic "pthread_mutex_unlock"         pthread_mutex_unlock
#pragma kai_intrinsic "pthread_mutex_trylock"        pthread_mutex_trylock

#pragma kai_intrinsic "pthread_cond_init"            pthread_cond_init
#pragma kai_intrinsic "pthread_cond_destroy"         pthread_cond_destroy
#pragma kai_intrinsic "pthread_cond_broadcast"       pthread_cond_broadcast
#pragma kai_intrinsic "pthread_cond_signal"          pthread_cond_signal
#pragma kai_intrinsic "pthread_cond_wait"            pthread_cond_wait
#pragma kai_intrinsic "pthread_cond_timedwait"       pthread_cond_timedwait

#endif /* Nobody's pthread.h guard macro had been defined */
