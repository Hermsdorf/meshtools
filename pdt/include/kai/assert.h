/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

/* For multiple inclusions */
#undef assert
#undef __assert

#ifdef NDEBUG
#define assert(ex) ((void) 0)
#else

extern 
#ifdef __cplusplus
       "C"
#endif
        void __eprintf (const char *, const char *, unsigned, const char *);

#define assert(ex) ((void)((ex)?((void)0):__assert( #ex, __FILE__, __LINE__ )))
#define __assert(expr,file,line) (__eprintf ("%s:%u: failed assertion `%s'\n", file, line, expr))

#endif /* NDEBUG */
