#ifndef _GENERAL_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void set_desc(User,char *,int);
EXTERN void wake(User,char *,int);
EXTERN void visibility(User,char *,int);
EXTERN void stats(User,char *,int);
EXTERN void examine(User,char *,int);
EXTERN void toggle_ignall(User,char *,int);
EXTERN void afk(User,char *,int);
EXTERN void cls(User,char *,int);
EXTERN void suicide(User,char *,int);
EXTERN void change_pass(User,char *,int);
EXTERN void beg_wizzes(User,char *,int);
EXTERN void listen_up(User,char *,int);
EXTERN void ignore(User,char *,int);
EXTERN void helpme(User,char *,int);
EXTERN void nofollow(User,char *,int);
EXTERN void nofight(User,char *,int);
EXTERN void earn_money(User,char *,int);
EXTERN void follow(User,char *,int);
EXTERN void eight_ball(User,char *,int);
EXTERN void last(User,char *,int);
EXTERN void friend_who(User,char *,int);
EXTERN void show_money(User,char *,int);
EXTERN void text_read(User,char *,int);
EXTERN void with_user(User,char *,int);
EXTERN void picture_list(User,char *,int);
EXTERN void view_picture(User,char *,int);
EXTERN void room_picture(User,char *,int);
EXTERN void picture_tell(User,char *,int);
EXTERN void what_version(User,char *,int);
EXTERN void socials(User,char *,int);
EXTERN void wiz_who(User,char *,int);
EXTERN void waitfor_user(User,char *,int);
EXTERN void riddle(User,char *,int);
EXTERN void find_user(User,char *,int);
EXTERN void vote(User,char *,int);
EXTERN void talkers(User,char *,int);
EXTERN void hat_colour(User,char *,int);
EXTERN void check_session(User,char *,int);
EXTERN void comment(User,char *,int);
EXTERN void aliases(User,char *,int);
EXTERN void net_sex(User,char *,int);
EXTERN void staff_list(User,char *,int);
#undef EXTERN
