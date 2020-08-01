/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#__include_next <stdio.h>

/* Under some circumstances, <stdio.h> can actually be
 * included twice, so allow this wrapper to be included
 * twice under the same circumstances.
 */
#ifdef _STDIO_H
#define __KAI_STDIO_H
#endif
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KAI_STDIO_H */
