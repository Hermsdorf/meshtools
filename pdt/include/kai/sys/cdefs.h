/**  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

/* Fixes problem with cdefs.h defining __inline to null for
   non GNU compilers.  Which causes in particular a problem
   with __cmsg_nxthdr from socketbits.h.  Bug 83H04.  
*/
#__include_next <sys/cdefs.h>

#undef __inline
#define __inline inline

