/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

/* Merged SGI and Linux versions of wchar.h */

#ifndef __KAI_WCHAR_H
#define __KAI_WCHAR_H


#ifndef __APPLE__
#ifdef PDT_WINDOWS
#include "include\wchar.h"
#else
#include "/usr/include/wchar.h"
#endif /* PDT_WINDOWS */
#endif /* __APPLE__ */

#if (defined(__sgi))
#include <stdarg.h>
#include <string.h>


#ifndef _MBSTATE_T
typedef struct
{
  int count;            /* Number of bytes needed for the current character. */
  wint_t value;         /* Value so far.  */
} mbstate_t;
#define _MBSTATE_T
#endif

/* some definitions, as givin to me in examples, necessary, unfortunately */
#define BUFFERSIZE 512 /* ANSI defined is 509 */
#define FL_SIGN       0x0001      /* put plus or minus in front */
#define FL_SIGNSP     0x0002      /* put space or minus in front */
#define FL_LEFT       0x0004      /* left justify */
#define FL_LEADZERO   0x0008      /* pad with leading zeros */
#define FL_LONG       0x0010      /* long value given */
#define FL_SHORT      0x0020      /* short value given */
#define FL_SIGNED     0x0040      /* signed data given */
#define FL_ALTERNATE  0x0080      /* alternate form requested */
#define FL_NEGATIVE   0x0100      /* value is negative */
#define FL_FORCEOCTAL 0x0200      /* force leading '0' for octals */
#define FL_LONGDOUBLE 0x0400      /* long double value given */
#define FL_WIDECHAR   0x0800      /* wide characters */

/* and state definitions, so we can print properly */

enum STATE {
    ST_NORMAL,              /* normal state; outputting literal chars */
    ST_PERCENT,             /* just read '%' */
    ST_FLAG,                /* just read flag character */
    ST_WIDTH,               /* just read width specifier */
    ST_DOT,                 /* just read '.' */
    ST_PRECIS,              /* just read precision specifier */
    ST_SIZE,                /* just read size specifier */
    ST_TYPE                 /* just read type specifier */
};
#define NUMSTATES (ST_TYPE + 1)

/* and char types */
enum CHARTYPE {
    CH_OTHER,               /* character with no special meaning */
    CH_PERCENT,             /* '%' */
    CH_DOT,                 /* '.' */
    CH_STAR,                /* '*' */
    CH_ZERO,                /* '0' */
    CH_DIGIT,               /* '1'..'9' */
    CH_FLAG,                /* ' ', '+', '-', '#' */
    CH_SIZE,                /* 'h', 'l', 'L', 'N', 'F', 'w' */
    CH_TYPE                 /* type specifying character */
};
#define _T(x) L ## x
const char __lookuptable[] = {
         0x06, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00,
         0x10, 0x00, 0x03, 0x06, 0x00, 0x06, 0x02, 0x10,
         0x04, 0x45, 0x45, 0x45, 0x05, 0x05, 0x05, 0x05,
         0x05, 0x35, 0x30, 0x00, 0x50, 0x00, 0x00, 0x00,
         0x00, 0x20, 0x28, 0x38, 0x50, 0x58, 0x07, 0x08,
         0x00, 0x37, 0x30, 0x30, 0x57, 0x50, 0x07, 0x00,
         0x00, 0x20, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00,
         0x08, 0x60,
         0x68,
                           0x60, 0x60, 0x60, 0x60, 0x00,
         0x00, 0x70, 0x70, 0x78, 0x78, 0x78, 0x78, 0x08,
         0x07, 0x08, 0x00, 0x00, 0x07, 0x00, 0x08, 0x08,
         0x08, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00,
         0x07,
         0x08
};

typedef struct{
        long double x;
        } sgiLONGDOUBLE;
typedef struct {
        double x;
} sgiDOUBLE;
#define _LEADBYTE   0x8000
#define isleadbyte(_c)  (_c & _LEADBYTE)
#define ASCII       32
#define LEFT_BRACKET    ('[' | ('a' - 'A'))
#define ___decimal_point "."
typedef void (* PFV)(void);
#define _cfltcvt_tab[6] {void, void, void, void, void, void}
typedef void (* PF5)(sgiLONGDOUBLE*, char*, int, int, int);
typedef void (* PF2)(int, char*, char*);
typedef void (* PF0)(sgiDOUBLE*, char*, int, int, int);
typedef void (* PF3)(char*);
typedef void (* PF1)(char*);
#define _cropzeros(a)       (*((PF1)_cfltcvt_tab[1]))(a)
#define _forcdecpt(a)       (*((PF3)_cfltcvt_tab[3]))(a)
#define _cfltcvt(a,b,c,d,e) (*((PF0)_cfltcvt_tab[0]))(a,b,c,d,e)
#define _fassign(a,b,c)     (*((PF2)_cfltcvt_tab[2]))(a,b,c)
#define _cldcvt(a,b,c,d,e)  (*((PF5)_cfltcvt_tab[5]))(a,b,c,d,e)
#define _free_crt                       free
#define find_char_class(c)      ((c) < L' ' || (c) > L'x' ? CH_OTHER : __lookuptable[(c)-L' '] & 0xF)
#define find_next_state(class, state)   (__lookuptable[(class) * NUMSTATES + (state)] >> 4)
static wchar_t cbrackset[] = L"]";
static wchar_t sbrackset[] = L" \t-\r]";
static wchar_t removewhite(int *count, FILE* str);
int get_int_arg ( va_list *pformat_list );
#define get_ptr_arg(x) (void *)get_int_arg(x)
#define get_long_arg(x) (long)get_int_arg(x)
void write_wchar(wchar_t wc, FILE * str, int* pcount);
void write_multi_wchar(wchar_t wc, int num, FILE* str, int* pcount);
void write_wchar_string(wchar_t *wcs, int len, FILE* str, int* pcount);
static wchar_t hextodec(wchar_t wc);
int process_input(FILE* str, const wchar_t * format, va_list format_list);
int process_output(FILE* str, const wchar_t * format, va_list format_list, size_t n);
extern "C" {
int fwscanf( FILE * str, const wchar_t *format, ...);
int swscanf(const wchar_t *strg, const wchar_t *format, ...);
int wscanf( const wchar_t *format, ...);
int fwprintf(FILE *str, const wchar_t *format, ...);
int swprintf(wchar_t *strg, size_t n, const wchar_t *format, ...);
int wprintf(const wchar_t *format, ...);
int vfwprintf( FILE *str, const wchar_t *format, va_list format_list);
int vswprintf(wchar_t * strg, size_t n, const wchar_t *format, va_list format_list);
int vwprintf(const wchar_t *format, va_list format_list);
int vfwscanf(  FILE * str, const wchar_t *format, va_list format_list);
int vswscanf(const wchar_t *strg, const wchar_t *format, va_list format_list);
int vwscanf( const wchar_t *format, va_list format_list);
int wmemcmp(const wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemchr(const wchar_t *a, wchar_t b, size_t n);
wchar_t *wmemcpy(wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemset(wchar_t *a, wchar_t b, size_t n);
size_t mbrtowc(wchar_t *wc, const char *ch, size_t t, mbstate_t *pstate);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *pstate);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t n, mbstate_t *pstate);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t n, mbstate_t *pstate);
size_t mbrlen(const char* s, size_t n, mbstate_t *type);
}
#elif defined(__linux)

#include "/usr/include/wchar.h"
#include <stdio.h>

#include <stdarg.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __STRICT_ANSI__
int fwscanf( FILE * str, const wchar_t *format, ...);
int swscanf(const wchar_t *strg, const wchar_t *format, ...);
int wscanf( const wchar_t *format, ...);
int fwprintf(FILE *str, const wchar_t *format, ...);
int swprintf(wchar_t *strg, size_t n, const wchar_t *format, ...);
int wprintf(const wchar_t *format, ...);
int vfwprintf( FILE *str, const wchar_t *format, va_list format_list);
int vswprintf(wchar_t * strg, size_t n, const wchar_t *format, va_list format_list);
int vwprintf(const wchar_t *format, va_list format_list);
int vfwscanf(  FILE * str, const wchar_t *format, va_list format_list);
int vswscanf(const wchar_t *strg, const wchar_t *format, va_list format_list);
int vwscanf( const wchar_t *format, va_list format_list);
wint_t fputwc(wchar_t wc, FILE* str);
int fputws(const wchar_t *strg, FILE* str);
wint_t putwc(wchar_t wc, FILE* str);
wint_t putwchar(wchar_t wc);
wint_t fgetwc(FILE* str);
wchar_t *fgetws(wchar_t * strg, int count, FILE* str);
wint_t getwc(FILE *str);
wint_t getwchar(void);
wint_t ungetwc(wint_t wc, FILE* str);
size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format, const struct tm *timeptr);
#endif /* __STRICT_ANSI__ */

#define __KAI_NO_FWIDE 1 /* Not in glibc */

#ifdef  __cplusplus
}
#endif
#elif (defined(__cray) || defined (hpux) || defined(__hiuxmpp))

#ifndef _crayx1
typedef int mbstate_t;
#endif /* _crayx1 */



#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

int fwscanf( FILE * str, const wchar_t *format, ...);
int swscanf(const wchar_t *strg, const wchar_t *format, ...);
int wscanf( const wchar_t *format, ...);
int fwprintf(FILE *str, const wchar_t *format, ...);
int swprintf(wchar_t *strg, size_t n, const wchar_t *format, ...);
int wprintf(const wchar_t *format, ...);
int vfwprintf( FILE *str, const wchar_t *format, va_list format_list);
int vswprintf(wchar_t * strg, size_t n, const wchar_t *format, va_list format_list);
int vwprintf(const wchar_t *format, va_list format_list);
int vfwscanf(  FILE * str, const wchar_t *format, va_list format_list);
int vswscanf(const wchar_t *strg, const wchar_t *format, va_list format_list);int vwscanf( const wchar_t *format, va_list format_list);
int wmemcmp(const wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemchr(const wchar_t *a, wchar_t b, size_t n);
wchar_t *wmemcpy(wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemset(wchar_t *a, wchar_t b, size_t n);
size_t mbrtowc(wchar_t *wc, const char *ch, size_t t, mbstate_t *pstate);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *pstate);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t n, mbstate_t *pstate);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t n, mbstate_t *pstate);
size_t mbrlen(const char* s, size_t n, mbstate_t *type);

/* Other fixes for PDT */
#ifndef _crayx1
typedef const int * wctrans_t;
#else /* CRAY X1 */
#ifndef _WCTRANS_T
#   define _WCTRANS_T
    typedef unsigned long	wctrans_t;
#endif
#endif /* CRAY X1 */

wint_t btowc(int c); 
int wctob(wint_t c);
int mbsinit( const mbstate_t * ps);
wchar_t *wcsstr (const wchar_t *haystack, const wchar_t * needle); 
wint_t towctrans (wint_t wc, wctrans_t desc); 
wctrans_t wctrans(const char * property);




#ifdef __cplusplus
}
#endif /* __cplusplus */


#elif defined (__APPLE__)
typedef int mbstate_t;

#include <stdio.h>
#include <stdarg.h>



typedef int wint_t;

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

int fwscanf( FILE * str, const wchar_t *format, ...);
int swscanf(const wchar_t *strg, const wchar_t *format, ...);
int wscanf( const wchar_t *format, ...);
int fwprintf(FILE *str, const wchar_t *format, ...);
int swprintf(wchar_t *strg, size_t n, const wchar_t *format, ...);
int wprintf(const wchar_t *format, ...);
int vfwprintf( FILE *str, const wchar_t *format, va_list format_list);
int vswprintf(wchar_t * strg, size_t n, const wchar_t *format, va_list format_list);
int vwprintf(const wchar_t *format, va_list format_list);
int vfwscanf(  FILE * str, const wchar_t *format, va_list format_list);
int vswscanf(const wchar_t *strg, const wchar_t *format, va_list format_list);
int vwscanf( const wchar_t *format, va_list format_list);
int wmemcmp(const wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemchr(const wchar_t *a, wchar_t b, size_t n);
wchar_t *wmemcpy(wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemset(wchar_t *a, wchar_t b, size_t n);
size_t mbrtowc(wchar_t *wc, const char *ch, size_t t, mbstate_t *pstate);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *pstate);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t n, mbstate_t *pstate);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t n, mbstate_t *pstate);
size_t mbrlen(const char* s, size_t n, mbstate_t *type);

/* Other fixes for PDT */
typedef const int * wctrans_t;

wint_t btowc(int c);
int wctob(wint_t c);
int mbsinit( const mbstate_t * ps);
wchar_t *wcsstr (const wchar_t *haystack, const wchar_t * needle);
wint_t towctrans (wint_t wc, wctrans_t desc);
wctrans_t wctrans(const char * property);

/* APPLE */
wint_t fputwc(wchar_t wc, FILE* str);
wint_t fputws(const wchar_t *strg, FILE* str);
wint_t putwc(wchar_t wc, FILE* str);
wint_t putwchar(wchar_t wc);
wint_t fgetwc(FILE* str);
wchar_t *fgetws(wchar_t * strg, int count, FILE* str);
wchar_t getwc(FILE *str);
wchar_t getwchar(void);     
wint_t ungetwc(wint_t wc, FILE* str);
size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format, const struct tm *timeptr);

long wcstol(const wchar_t *, wchar_t **, int);
wchar_t *wcscat(wchar_t *, const wchar_t *);
wchar_t *wcschr(const wchar_t *,  wchar_t);
int wcscmp(const wchar_t *, const wchar_t *);
int wcscoll(const wchar_t *, const wchar_t *);
wchar_t *wcscpy(wchar_t *, const wchar_t *);
size_t wcscspn(const wchar_t *, const wchar_t *);
size_t wcslen(const wchar_t *);
wchar_t *wcsncat(wchar_t *, const wchar_t *, size_t);
int wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t *wcsncpy(wchar_t *, const wchar_t *, size_t);
wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t *wcsrchr(const wchar_t *, wchar_t);
size_t wcsspn(const wchar_t *, const wchar_t *);
double wcstod(const wchar_t *, wchar_t **);
unsigned long wcstoul(const wchar_t *, wchar_t **, int);
wchar_t *wcstok(wchar_t *, const wchar_t *);
size_t wcsxfrm(wchar_t *, const wchar_t *, size_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */




#endif /* sgi | linux | cray | hitachi | apple */

#endif /* __KAI_WCHAR_H */
