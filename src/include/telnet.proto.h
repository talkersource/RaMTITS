#ifndef _TELNET_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN Telnet create_telnet(void);
EXTERN void destruct_telnet(Telnet);
EXTERN Telnet get_telnet(User);
EXTERN void display_telnet(Telnet,char *,int);
EXTERN int connect_to_site(Telnet);
EXTERN void disconnect_telnet(Telnet);
EXTERN void connect_telnet(Telnet);
EXTERN void telnet_com(User,char *,int);
EXTERN void telnet_close(User,char *,int);
EXTERN void telnet_say(User,char *,int);
EXTERN void telnet_phone(User,char *,int);
EXTERN void telnet_list(User,char *,int);
EXTERN int check_telnet(Telnet,char *);
#undef EXTERN
