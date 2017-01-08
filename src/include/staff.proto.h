#ifndef _STAFF_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void shutdown_com(User,char *,int);
EXTERN void reboot_com(User,char *,int);
EXTERN void wiztell(User,char *,int);
EXTERN void wizemote(User,char *,int);
EXTERN void minlogin(User,char *,int);
EXTERN void clearline(User,char *,int);
EXTERN void swban(User,char *,int);
EXTERN void create_clone(User,char *,int);
EXTERN void destroy_clone(User,char *,int);
EXTERN void myclones(User,char *,int);
EXTERN void clone_switch(User,char *,int);
EXTERN void clone_hear(User,char *,int);
EXTERN void site(User,char *,int);
EXTERN void system_details(User,char *,int);
EXTERN void kill_user(User,char *,int);
EXTERN void promote(User,char *,int);
EXTERN void demote(User,char *,int);
EXTERN void delete_user(User,char *,int);
EXTERN void listbans(User,char *,int);
EXTERN void ban(User,char *,int);
EXTERN void ban_site(User,char *);
EXTERN void ban_new(User,char *);
EXTERN void ban_who(User);
EXTERN void ban_user(User,char *);
EXTERN void unban(User,char *,int);
EXTERN void unban_site(User);
EXTERN void unban_new(User);
EXTERN void unban_who(User);
EXTERN void unban_user(User);
EXTERN void arrest(User,char *,int);
EXTERN void unarrest(User,char *,int);
EXTERN void muzzle(User,char *,int);
EXTERN void unmuzzle(User,char *,int);
EXTERN void silence(User,char *,int);
EXTERN void unsilence(User,char *,int);
EXTERN void broad_cast(User,char *,int);
EXTERN void view_record(User,char *,int);
EXTERN void gag(User,char *,int);
EXTERN void fake_logoff(User,char *,int);
EXTERN void shackle(User,char *,int);
EXTERN void make_visible(User,char *,int);
EXTERN void make_invis(User,char *,int);
EXTERN void add_user_hist(User,char *,int);
EXTERN void samesite(User,char *,int);
EXTERN void change_name(User,char *,int);
EXTERN void save_users(User,char *,int);
EXTERN void resign_staff(User,char *,int);
EXTERN void deny_command(User,char *,int);
EXTERN void allow_command(User,char *,int);
EXTERN void force(User,char *,int);
EXTERN void warn_user(User,char *,int);
EXTERN char *getname(User);
EXTERN void get_real_name(User,char *,int);
EXTERN void make_newuser(User,char *,int);
EXTERN void memory_count(User,char *,int);
EXTERN void force_backup(User,char *,int);
#undef EXTERN
