#ifndef _ROOMS_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void map(User,char *,int);
EXTERN void goto_myroom(User,char *,int);
EXTERN void home_room(User);
EXTERN void kick_out(User,char *,int);
EXTERN void rooms(User,char *,int);
EXTERN void go(User,char *,int);
EXTERN void set_room_access(User,char *,int);
EXTERN void knock(User,char *,int);
EXTERN void invite(User,char *,int);
EXTERN void uninvite(User,char *,int);
EXTERN void set_topic(User,char *,int);
EXTERN void staff_move(User,char *,int);
EXTERN void join(User,char *,int);
EXTERN void nobuff(User,char *,int);
EXTERN void look(User,char *,int);
EXTERN void room_look(User,char *,int);
EXTERN void atmos_assign(Room);
EXTERN int save_atmos(Room);
EXTERN void reset_access(Room);
EXTERN void move_user(User,Room,int);
EXTERN void room_key(User,char *,int);
EXTERN int has_room_access(User,Room);
#undef EXTERN
