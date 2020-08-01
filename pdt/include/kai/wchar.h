/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#__include_next <wchar.h>

#ifndef __KAI_WCHAR_H
#define __KAI_WCHAR_H

#include <stdarg.h>

#ifdef  __cplusplus
extern "C" {
#endif

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
wint_t fputws(const wchar_t *strg, FILE* str);
wint_t putwc(wchar_t wc, FILE* str);
wint_t putwchar(wchar_t wc);
wint_t fgetwc(FILE* str);
wchar_t *fgetws(wchar_t * strg, int count, FILE* str);
wchar_t getwc(FILE *str);
wchar_t getwchar(void);
wint_t ungetwc(wint_t wc, FILE* str);
size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format, const struct tm *timeptr);

#define __KAI_NO_FWIDE 1 /* Not in glibc */

#ifdef  __cplusplus
}
#endif

#endif /* __KAI_WCHAR_H */
