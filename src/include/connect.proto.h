#ifdef _CONNECT_SOURCE
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN void accept_connection(int,int);
EXTERN char *resolve(char *);
EXTERN void Login(User,char *);
EXTERN void attempts(User,int);
EXTERN void convert_vars(User);
EXTERN void connect_user(User);
EXTERN void disconnect_user(User,int);
#undef EXTERN
