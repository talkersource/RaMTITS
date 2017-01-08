#ifndef _BOOTUP_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void load_and_parse_config(void);
EXTERN void parse_init_section(void);
EXTERN void parse_cmnds(void);
EXTERN void make_userlist(void);
EXTERN void make_roomlist(void);
EXTERN void init_globals(void);
EXTERN void init_signals(void);
EXTERN void boot_exit(int);
EXTERN void init_sockets(void);
EXTERN void make_sociallist(void);
EXTERN int GeneratePiclist(void);
EXTERN int GenerateTextList(void);
EXTERN int GenerateFigfontList(void);
EXTERN void check_messages(User,char *,int);
EXTERN void load_bans(int);
#undef EXTERN
