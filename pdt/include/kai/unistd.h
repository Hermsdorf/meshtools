/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef _KAI_UNISTD_H
#define _KAI_UNISTD_H

#ifdef __cplusplus
#define __inline__ inline
#else
#define __inline__
#endif

#ifndef PDT_WINDOWS
#__include_next <unistd.h>
#endif /*PDT_WINDOWS*/
#endif /* _KAI_UNISTD_H */

