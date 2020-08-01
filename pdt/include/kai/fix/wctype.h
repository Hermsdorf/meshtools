#ifndef __APPLE__
#ifdef PDT_WINDOWS
#include "include\wctype.h"
#else /*PDT_WINDOWS*/
#include "/usr/include/wctype.h"
#endif /*PDT_WINDOWS*/
#else /* __APPLE__ */

typedef unsigned int    wctype_t;

#ifdef __cplusplus
extern "C" {
#endif 
int towlower(wint_t);
int towupper(wint_t);
int iswalnum(wint_t);
int iswalpha(wint_t);
int iswcntrl(wint_t);
int iswctype (wint_t, wctype_t);
int iswdigit(wint_t);
int iswupper(wint_t);
int iswxdigit(wint_t);
int iswgraph(wint_t);
int iswlower(wint_t);
int iswprint(wint_t);
int iswpunct(wint_t);
int iswspace(wint_t);

wctype_t wctype(const char*);


#ifndef WEOF
#define WEOF    (-1)
#endif  /* WEOF */



#ifdef __cplusplus
} 
#endif 
#endif /* __APPLE__ */



