#ifndef _MODULE3_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void rroulette(User,char *,int);
EXTERN void drink(User,char *,int);
EXTERN void pet_stuff(User,char *,int);
EXTERN char *StringConvert(char *,char *);
EXTERN void nuclear_bomb(User,char *,int);
EXTERN void nuclear_count(void);
EXTERN void toggle_monitor(User,char *,int);
EXTERN void page_file(User,char *,int);
EXTERN void quit_user(User,char *,int);
EXTERN int save_talker_stuff(void);
EXTERN int load_talker_stuff(void);
EXTERN void ping(User);
EXTERN void ping_respond(User);
EXTERN void check_pings(void);
EXTERN void ping_timed(User);
EXTERN void ping_user(User,char *,int);
EXTERN char *ping_string(User);
EXTERN void gag_shouts(User);
EXTERN void gag_socials(User);
EXTERN void gag_private(User);
EXTERN void line_tell(User,char *,int);
EXTERN void auto_arrest(User,int);
EXTERN void cloak_level(User,char *,int);
EXTERN void email_user(User,char *,int);
EXTERN void mymake(User,char *,int);
EXTERN void make_helpfile(User,int);
EXTERN void reset_user(User,char *,int);
EXTERN void lottery(User,char *,int);
EXTERN void steal_stuff(User,char *,int);
EXTERN void ban_info(User,char *,int);
EXTERN void reset_junk(User);
#undef EXTERN
