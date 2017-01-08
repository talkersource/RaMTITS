#ifndef _MODULE1_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void WriteDebugFile(void);
EXTERN void icqpage(User,char *,int);
EXTERN int send_icq_page(User,int,char *,char *);
EXTERN void delete_old(User,char *,int);
EXTERN void delete_files(char *);
EXTERN void set_roomname(User,char *,int);
EXTERN void give_stuff(User,char *,int);
EXTERN void do_user_webpage(User);
EXTERN void process_profile(User,FILE *);
EXTERN char *htmlize(char *);
EXTERN char *html_var(char *);
EXTERN void who_webpage(void);
EXTERN void capture(User,char *,int);
EXTERN void rename_logfiles(void);
EXTERN void protect(User,char *,int);
EXTERN void view_logs(User,char *,int);
EXTERN char *get_social(void);
EXTERN void random_social(User,char *,int);
EXTERN void clone_exec(User,char *,int);
EXTERN void macros(User,char *,int);
EXTERN void user_alarm(User,char *,int);
EXTERN void sound_alarm(User);
EXTERN void word_time(User,long);
EXTERN void game_stats(User,char *,int);
EXTERN void guess_it(User,char *,int);
EXTERN void to_emote(User,char *,int);
EXTERN void what_time(User,char *,int);
EXTERN void redo_time(User,char *,int);
EXTERN void toggle_shoutban(User,char *,int);
#undef EXTERN
