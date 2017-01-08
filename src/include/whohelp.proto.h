#ifndef _WHOHELP_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void who(User,char *,int);
EXTERN void dcove_who(User);
EXTERN void amnuts_who(User);
EXTERN void moenuts_who(User);
EXTERN void short_who(User);
EXTERN void emerald_who(User);
EXTERN void mori_who(User);
EXTERN void ei3_who(User);
EXTERN void help(User,char *,int);
EXTERN void normal_help(User,int);
EXTERN void command_help(User,int);
EXTERN void type_help(User,int);
EXTERN void help_level(User,int);
#undef EXTERN
