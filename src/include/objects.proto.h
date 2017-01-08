#ifndef _OBJECTS_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN User create_user(void);
EXTERN void alpha_sort(User);
EXTERN void destruct_user(User);
EXTERN void destruct_alpha(User);
EXTERN int add_userlist(char *,int);
EXTERN int clean_userlist(char *);
EXTERN void destroy_user_clones(User);
EXTERN Room create_room(void);
EXTERN void reset_room(Room);
EXTERN void destruct_room(Room);
EXTERN int add_roomlist(Room);
EXTERN int clean_roomlist(char *);
EXTERN int add_command(int);
EXTERN int destruct_command(int);
EXTERN int add_social(char *);
EXTERN int destruct_social(char *);
EXTERN User get_user(char *);
EXTERN User get_clone(char *);
EXTERN User get_clone_here(Room,char *);
EXTERN User get_userid(int);
EXTERN User get_name(User,char *);
EXTERN char *get_user_full(User,char *);
EXTERN Room get_room(char *);
EXTERN Room get_room_recap(char *);
EXTERN char *get_room_full(User,char *);
EXTERN int get_level(char *);
EXTERN Socials get_socname(char *);
EXTERN Commands get_command(User,char *);
EXTERN Commands get_command_full(char *);
EXTERN Bans new_ban(char *,int);
EXTERN void nuke_ban(Bans);
EXTERN Bans find_exact_ban(char *);
EXTERN Bans find_ban(char *,int);
EXTERN Convbuff create_convbuff(void);
EXTERN void delete_convbuff(Convbuff);
EXTERN void reset_vars(User);
#undef EXTERN
