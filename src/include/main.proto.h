#ifndef MAIN_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN int main(int argc,char *argv[]);
EXTERN void setup_readmask(fd_set *mask);
EXTERN void sig_handler(int);
EXTERN int misc_ops(User,char *);
EXTERN void set_date_time(void);
EXTERN void talker_shutdown(User,char *,int);
EXTERN void do_events(int);
EXTERN void reset_alarm(void);
EXTERN void check_reboot_shutdown(void);
EXTERN void check_idle_and_timeout(void);
EXTERN void check_atmos(void);
EXTERN void check_logins(void);
EXTERN void check_sockets(void);
EXTERN void friend_check(User,int);
EXTERN void wait_check(User);
EXTERN void close_sockets(void);
EXTERN void CheckSysCommands(void);
EXTERN void LotteryDraw(void);
EXTERN void write_pid_file(void);
#undef EXTERN
