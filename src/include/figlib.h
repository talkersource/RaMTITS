#ifndef FIGLIB_H
  #define FIGLIB_H

#ifndef _FIGLET_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

#define FONT_DIR "fonts"
typedef long inchr; /* "char" read from stdin */
EXTERN inchr *inchrline;   /* Alloc'd inchr inchrline[inchrlinelenlimit+1]; */
EXTERN int inchrlinelen,inchrlinelenlimit;

typedef struct fc {
	inchr ord;
	char **thechar;  /* Alloc'd char thechar[charheight][]; */
	struct fc *next;
	} fcharnode;

EXTERN fcharnode *fcharlist;
EXTERN char **currchar,**outline;
EXTERN int currcharwidth,outlinelen,justification,right2left,outputwidth;
EXTERN int outlinelenlimit,charheight,defaultmode;
EXTERN char hardblank;

#undef EXTERN
#endif /* FIGLIB_H */
