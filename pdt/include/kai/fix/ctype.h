#ifndef __APPLE__
#ifdef PDT_WINDOWS
#include "include\ctype.h"
#else /*PDT_WINDOWS*/
#include "/usr/include/ctype.h"
#endif /* __PDT_WINDOWS__ */
#else /* __APPLE__ */

#ifdef __cplusplus
extern "C" {
#endif 
int      isalpha(int);
int      isalnum(int);
int      iscntrl(int);
int      isdigit(int);
int      isgraph(int);
int      islower(int);
int      isprint(int);
int      ispunct(int);
int      isspace(int);
int      isupper(int);
int      isxdigit(int);
int      toupper(int);
int      tolower(int);



#ifdef __cplusplus
} 
#endif 
#endif /* __APPLE__ */



