#ifndef _COMMAND_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void exec_com(User,char *);
EXTERN int get_comnum(char *);
EXTERN Commands get_comname(char *);
EXTERN int is_scut(User,char *);
EXTERN int ExecSocial(User,char *);
EXTERN int is_social(User,char *);
EXTERN int is_alias(User,char *);
EXTERN void ExecMulti(User,char *);
EXTERN void exec_all(User,char *,int);
EXTERN int ExecAlias(User,char *,char *);
EXTERN char *which_gender(User,User,char *);
EXTERN char *check_gender(User,User,char *);
#undef EXTERN
