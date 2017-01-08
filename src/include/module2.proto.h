#ifndef _MODULE2_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void delete_messages(User,char *,int);
EXTERN void reset_messages(User,int);
EXTERN void count_messages(User,int);
EXTERN void read_specific(User,char *,int,int);
EXTERN int count_mess(User,int);
EXTERN void view_from(User,char *,int);
EXTERN void review_buffer(User,char *,int);
EXTERN void clear_buffers(User,char *,int);
EXTERN void load_default_shortcuts(User);
EXTERN void shortcuts(User,char *,int);
EXTERN void sreboot_com(User,char *,int);
EXTERN int build_sysinfo(User);
EXTERN int build_loggedin_users_list(User);
EXTERN int build_loggedin_users_info(User);
EXTERN int build_room_info(User);
EXTERN void do_reboot(User);
EXTERN void trans_to_quiet(User,char[]);
EXTERN User create_user_template(User);
EXTERN Room create_room_template(Room);
EXTERN int retrieve_sysinfo(void);
EXTERN void retrieve_rooms(void);
EXTERN void retrieve_users(void);
EXTERN void redo_rooms(void);
EXTERN int possibly_reboot(void);
EXTERN void load_tvars(User,char *,int);
EXTERN void friends(User,char *,int);
EXTERN void enemies(User,char *,int);
EXTERN void disable_command(User,char *,int);
EXTERN void do_maintenance(void);
EXTERN int user_exists(char *);
EXTERN void screen_site(User,char *,int);
EXTERN char *process_trigger(char *trig,char *str);
EXTERN void set_splitscr(User,char *,int);
#undef EXTERN
