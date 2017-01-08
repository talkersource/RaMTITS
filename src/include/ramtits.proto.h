#ifndef _RAMTITS_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN int LoadUser(User,char *);
EXTERN int SaveUser(User,int);
EXTERN void refund_user(User);
EXTERN int is_jailed(User);
EXTERN int is_clone(User);
EXTERN int is_muzzled(User);
EXTERN void status_text(User,int,char *);
EXTERN void rev_away(User);
EXTERN int is_staff(User);
EXTERN int is_member(User);
EXTERN int disabled_com(char *);
EXTERN int is_friend(User,User);
EXTERN int is_friend_name(User, char *);
EXTERN int is_enemy(User,User);
EXTERN int is_enemy_name(User,char *);
EXTERN int is_allowed(User,char *);
EXTERN int is_denied(User,char *);
EXTERN int is_busy(User);
EXTERN int user_exists(char *);
EXTERN void white_space(char []);
EXTERN int is_banned(char *,int);
EXTERN int wordfind(char *);
EXTERN int wordshift(char *);
EXTERN void clear_words(void);
EXTERN int yn_check(char *);
EXTERN int onoff_check(char *);
EXTERN void record(Room,char *);
EXTERN void record_tell(User,char *);
EXTERN void record_away(User,char *);
EXTERN void record_wiz(char *);
EXTERN void record_shout(char *);
EXTERN void RecordDebug(char *);
EXTERN void RecordLogin(char *);
EXTERN void record_channel(char *);
EXTERN int has_unread_mail(User);
EXTERN char *long_date(int);
EXTERN int valid_addy(User,char *,int);
EXTERN int file_exists(char *);
EXTERN int intrand(int);
EXTERN int ran(unsigned long);
EXTERN int count_lines(char *);
EXTERN int is_bday_today(char *);
EXTERN void create_bday_file(User);
EXTERN int user_allowcom(char *,char *);
EXTERN int user_denycom(char *,char *);
EXTERN void friend_check(User,int);
EXTERN void wait_check(User);
EXTERN void RemoveSysfiles(User);
EXTERN int IsUserVar(char *);
EXTERN int xwrite(FILE *,char *,int);
EXTERN int xread(FILE *,char *,int);
EXTERN int intcmp(const void *,const void *);
EXTERN int comp(const void *,const void *);
EXTERN int InConnlist(char *);
EXTERN int FindFreeConnslot(void);
EXTERN void ClearConnlist(void);
EXTERN void check_bad_vars(User);
EXTERN void send_mail(User,char *);
EXTERN void send_oneline_mail(User,char *,char *);
EXTERN int save_bans(int);
EXTERN void save_all_bans(void);
#undef EXTERN
