/**
 **  PDT 
 **
 **/

#ifndef __PDT_SYS_TYPES_H
#define __PDT_SYS_TYPES_H

#if (defined(linux))
/* 64 bit types */

#ifdef PDT_MIPS64LINUX 
#define PDT_DEFINE_64BIT_TYPES
#endif /* PDT_MIPS64LINUX */

#ifdef PDT_DEFINE_64BIT_TYPES
#ifndef uint64_t
typedef unsigned long long int uint64_t;
#endif /* uint64_t */

#ifndef PDT_MIPS64LINUX
#ifndef int64_t
typedef long long int int64_t;
#endif /* int64_t */
#endif /* PDT_MIPS64LINUX */
#endif /* PDT_DEFINE_64BIT_TYPES */

#ifndef PDT_DEFINE_64INT
#define PDT_DEFINE_64INT
typedef long long int __int64;
#endif /* PDT_DEFINE_64INT */


#ifndef __dev_t_defined
#define dev_t unsigned long long int
#define __dev_t_defined
#endif /* __dev_t_defined */

#endif  /* linux */

#ifdef PDT_WINDOWS
#include "sys\types.h"
#else
#ifndef PDT_MULTI_ARCH
#include "/usr/include/sys/types.h"
#else /* PDT_MULTI_ARCH */

#ifdef PDT_MULTI_ARCH_x86_64
#include "/usr/include/x86_64-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_x86_64 */

#ifdef PDT_MULTI_ARCH_i386
#include "/usr/include/i386-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_i386 */

#ifdef PDT_MULTI_ARCH_i486
#include "/usr/include/i386-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_i486 */

#ifdef PDT_MULTI_ARCH_i586
#include "/usr/include/i386-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_i586 */

#ifdef PDT_MULTI_ARCH_i686
#include "/usr/include/i386-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_i686 */

#ifdef PDT_MULTI_ARCH_arm
#include "/usr/include/arm-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_arm */

#ifdef PDT_MULTI_ARCH_aarch64
#include "/usr/include/aarch64-linux-gnu/sys/types.h"
#endif /* PDT_MULTI_ARCH_aarch64 */

#endif /* PDT_MULTI_ARCH */
#endif /* PDT_WINDOWS */

#if (defined (linux))
#define __dev_t unsigned long long int
#endif /* linux */

#endif /* __PDT_SYS_TYPES_H */
