#ifndef MEMH
  #define MEMH

#include <malloc.h>
/* $Id: mem.h, v1.17 1997/08/17 09:49:31 loth Exp $ */

#if defined(NEED_MALLOC) || defined(NEED_CALLOC) || defined(NEED_REALLOC)
  static void *malloc_return;
#endif /* NEED_MALLOC */
#if defined(NEED_STRDUP)
  static char *strdup_return;
#endif /* NEED_STRDUP */

#if defined(NEED_MALLOC)
  #define MALLOC(c) ((malloc_return=malloc(c))?malloc_return:(log("[DNS (mem)] - MALLOC failed at %s:%d\n",__FILE__,__LINE__,(void *)0))
#endif /* NEED_MALLOC */

#if defined(NEED_CALLOC)
  #define CALLOC(memb,size) ((malloc_return=calloc(memb,size))?malloc_return:(log("[DNS (mem)] - CALLOC failed at %s:%d\n",__FILE__,__LINE__),(void *)0))
#endif /* NEED_CALLOC */

#if defined(NEED_FREE)
  #define FREE(c)  { if (c) {\
			free(c);\
			c=0;\
			}
		     else {\
			log("[DNS (mem)] - Bad FREE at %s:%d\n",__FILE__,__LINE__);\
			}\
		    }
#endif /* NEED_FREE */

#if defined(NEED_REALLOC)
  #define REALLOC(p,c)  ((malloc_return=realloc(p,c))?malloc_return:(log("[DNS (mem)] - REALLOC failed at %s:%d\n",__FILE__,__LINE__),(void *)0))
#endif /* NEED_REALLOC */

#if defined(NEED_STRDUP)
  #define STRDUP(s) ((strdup_return=strdup((char *)s))?strdup_return:(log("[DNS (mem)] - STRDUP failed at %s:%d\n",__FILE__,__LINE__),(char *)0))
#endif /* NEED_STRDUP */

#if defined(HAVE_BZERO)
  #define ZERO(c,s) bzero(c,s)
#else
  #define ZERO(c,s) memset(c,0,s)
#endif /* HAVE_BZERO */

#endif /* MEMH */
