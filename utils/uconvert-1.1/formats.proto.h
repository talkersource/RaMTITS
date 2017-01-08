#ifndef UCONVERT_FORMATS_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN User create_user(void);
EXTERN void destruct_user(User user);
EXTERN void reset_vars(User user);
EXTERN void strtolower(char *str);
EXTERN int LoadOldUser(User user,char *name);

#undef EXTERN
