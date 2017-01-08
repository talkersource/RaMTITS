#ifndef _CHECKERS_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void checkers_say(Checkers,char *);
EXTERN int win_checkers(Checkers);
EXTERN void checkers_com(User,char *,int);
EXTERN void move_checkers(User,char *,int);
EXTERN void init_checkerboard(User,Checkers);
EXTERN void print_checkerboard(User,char *);
EXTERN void print_old_checkerboard(User);
EXTERN void print_small_checkerboard(User,char *);
EXTERN void print_big_checkerboard(User,char *);
EXTERN int valid_white_move(Checkers,int,int,int,int);
EXTERN int valid_red_move(Checkers,int,int,int,int);
EXTERN void checkers_kill(User);
EXTERN void checkers_summary(User);

#undef EXTERN
