#ifndef _SERVER_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN int double_fork(void);
EXTERN int send_email(char *,char *);
EXTERN int backup_files(void);
EXTERN void uni_inger(User,char *,int);
EXTERN void uni_ois(User,char *,int);
EXTERN void ns_lookup(User,char *,int);
EXTERN int page_sysfile(User,char *,int);
EXTERN void goto_webpage(User,char *,int);
#undef EXTERN
