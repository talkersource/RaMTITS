#ifndef _CONFIG_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void change_system(User,char *,int);
EXTERN int LoadRoom(Room,char *);
EXTERN int SaveRoom(Room);
EXTERN void load_links(Room);
EXTERN int load_atmos(Room);
EXTERN void load_keys(Room);
EXTERN int room_exists(char *);
EXTERN int has_room_key(User,Room);
EXTERN int count_room_users(Room);
EXTERN void room_admin(User,char *,int);
EXTERN int add_link(User,Room,char *);
EXTERN void save_config(void);
#undef EXTERN
