#ifndef _SPEECH_C
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void say(User,char *,int);
EXTERN void emote(User,char *,int);
EXTERN void send_echo(User,char *,int);
EXTERN void think(User,char *,int);
EXTERN void sing(User,char *,int);
EXTERN void to_user(User,char *,int);
EXTERN void show(User,char *,int);
EXTERN void mutter(User,char *,int);
EXTERN void multi_message(User,char *,int);
EXTERN void private_echo(User,char *,int);
EXTERN void revtell(User,char *inpstr,int);
EXTERN void send_beep(User,char *,int);
EXTERN void shout(User,char *,int);
EXTERN void shout_emote(User,char *,int);
EXTERN void shout_echo(User,char *,int);
EXTERN void shout_think(User,char *,int);
EXTERN void shout_sing(User,char *,int);
EXTERN void welcome_user(User,char *,int);
EXTERN void yell(User,char *,int);
EXTERN void friend_tell(User,char *,int);
EXTERN void friend_emote(User,char *,int);
EXTERN void greet(User,char *,int);
EXTERN void send_message(User,char *,char *);
EXTERN void phone(User,char *,int);
EXTERN void answer_call(User,char *,int);
EXTERN void hangup_phone(User,char *,int);
EXTERN void channel_say(User,char *,int);
EXTERN void channel_emote(User,char *,int);
EXTERN void alter_speech(char *,int);
EXTERN void set_speech(User,char *,int);
EXTERN void net_marry(User,char *,int);
EXTERN void net_divorce(User,char *,int rt);
#undef EXTERN
