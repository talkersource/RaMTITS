#ifndef UCONVERT_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN int main(void);
EXTERN int xwrite(FILE *,char *,int);
EXTERN int xread(FILE *,char *,int);
EXTERN int SaveNewUser(User);
EXTERN int LoadNewUser(User,char *);
EXTERN void clear_words(void);
EXTERN int wordfind(char *);
EXTERN char *remove_first(char *);
EXTERN int LoadMacros(User);
EXTERN int load_user_stuff(User,int);
EXTERN void load_default_shortcuts(User);
#undef EXTERN
