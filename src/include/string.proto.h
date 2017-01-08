#ifndef _STRING_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void remove_ctrl_chars(User,char *);
EXTERN void terminate(char *);
EXTERN char *remove_first(char *);
EXTERN int contains_swearing(char *);
EXTERN int colour_com_count(char *);
EXTERN char *ColourStrip(char *);
EXTERN char *center(char *,int);
EXTERN int ainstr(char *,char *);
EXTERN char *istrstr(char *,char *);
EXTERN char *andys_replace_string(char *,char *,char *);
EXTERN void midcpy(char *,char *,int,int);
EXTERN int rtrim(char *);
EXTERN char *which_var(User,char *);
EXTERN char *parse_string(User,char *);
EXTERN char *replace_macros(User,char *);
EXTERN char *replace_swear_words(char *);
EXTERN void strtoupper(char *);
EXTERN void strtolower(char *);
EXTERN int isnumber(char *);
EXTERN char *smiley_type(char *,int);
#undef EXTERN
