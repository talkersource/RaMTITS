#ifndef _MESSAGES_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void print_usage(User,int);
EXTERN void write_board(User,char *,int);
EXTERN void read_board(User,char *,int);
EXTERN void enter_profile(User,char *,int);
EXTERN void add_news(User,char *,int);
EXTERN void room_desc(User,char *,int);
EXTERN void suggestions(User,char *,int);
EXTERN void friend_smail(User,int);
EXTERN void staff_smail(User,int);
EXTERN void users_smail(User,int);
EXTERN void bugs(User,char *,int);
EXTERN void smail(User,char *,int);
EXTERN void enter_vote(User,char *,int);
EXTERN void enter_riddle(User,char *,int);
EXTERN void rmail(User,char *,int);
EXTERN void forward_smail(User,char *,int);
EXTERN void read_sent(User,char *,int);
EXTERN void edit_atmos(User,char *,int);
EXTERN void set_ccopy(User,char *,int);
EXTERN void wiznote(User,char *,int);
#undef EXTERN
