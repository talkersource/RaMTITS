#ifndef RTNEEDS_H
  #define RTNEEDS_H

#include <time.h>
#include <sys/time.h>
#include <setjmp.h>
#include "structs.h"
#include "commands.h"
#include "levels.h"
#include "writelib.h"
#include "defines.h"
#include "protos.h"
/* external stuff from custom2.h */
extern char *colcode[NumCols];
extern char *mycolcode[NumCols];
extern char *colcom[NumCols];
extern char *rcolour[NumRand];
extern char *who_style[NumWhos+1];
extern char *help_style[NumHelp+1];
extern char *speech_style[NumSpeech+1];
extern char *month[12];
extern char *day[7];
extern char *hatcolor[];
extern char *noyes1[];
extern char *noyes2[];
extern char *yesno1[];
extern char *yesno2[];
extern char *offon[];
extern char *file_ext[];
extern char *swear_words[];
extern char *gender_types[];
extern char *user_vars[];
extern char *pet[];
extern char *censored[NumCensors+1];
extern char *identprog;
extern int biglet[26][5][5];
extern int bigsym[32][5][5];
extern Convbuff shoutbuff,wizbuff,channelbuff,loginbuff,debugbuff;
extern char text[ARR_SIZE*2],vtext[ARR_SIZE*2];
extern char word[MaxWords][WordLen+1],	wrd[8][81];
extern char progname[81],		confile[250];
extern char file[ARR_SIZE];
extern char sessionwho[UserNameLen+2],	session[101];
extern char staffmember[15][UserNameLen+2],member[50][UserNameLen+2];
extern char disabled[20][20];
extern char main_room[RoomNameLen+2],	idle_room[RoomNameLen+2];
extern char jail_room[RoomNameLen+2];
extern time_t boot_time, sessiontime,rs_announce,rs_which,check_time,sreboot_time;
extern long jackpot,ulogins[4],newlog[3];
extern int resolve_sites;
extern int port[2],	 listen_sock[2],   wizport_level,       minlogin_level;
extern int password_echo,ignore_sigterm,   max_users;
extern int max_clones,   num_of_users,     num_of_logins,       heartbeat;
extern int config_line,  login_idle_time,  user_idle_time,      word_count;
extern int tyear,	 tmonth,	   tday,		tmday;
extern int twday,	 thour,		   tmin,		tsec;
extern int mesg_life,	 system_logging,   prompt_def,          no_prompt;
extern int force_listen, gatecrash_level,  min_private_users,   ignore_mp_level;
extern int ban_swearing, mesg_check_hour,  mesg_check_min,      seed;
extern int crash_action, sclosetime,       time_out_afks,       rs_countdown;
extern int autoauth,	 id_count,	   last_uid;
extern int destructed,   charecho_def,     time_out_maxlevel,   user_count;
extern int new_users,    logging_in,       atmos_trigger,       backup_talker;
extern int shout_ban,    debug_talker,        webpage_on;
extern int arrest_swearers,  sock_error,          write_timeout;
extern int suggestion_num,           bug_num,  free_rooms,	auto_promote;
extern int auto_auth,    nuclear_countdown,nuke_announce,	wiznote_num;
extern int keepalive_interval, disconnecting,restart_loop;
extern int ident_socket, ident_state,	   ident_status,        ident_timer;
extern int num_of_telnets, max_telnets,	   clone_cnt;
extern short int boot_up;
extern unsigned identpid;
extern User rs_user;
/* These ones are in custom1.h */
extern char BfafkText[][];
extern char *qotd[];
extern char *atmos[MaxAtmos+1];
extern char *signature;
extern char *syserror;
extern char *nosuchroom;
extern char *nosuchuser;
extern char *notloggedon;
extern char *invisenter;
extern char *invisleave;
extern char *nosuchlev;
extern char *invisname;
extern char *invismesg;
extern char *noswearing;
extern char *login_vis;
extern char *login_invis;
extern char *login_hide;
extern char *login_fake;
extern char *logout_vis;
extern char *logout_inv;
extern char *logout_hide;
extern char *logout_fake;
extern char *wtf;
extern char *notallowed;
extern char *clnodo;
extern char *tempnodo;
extern char *muzztext;
extern char *qotdm;
extern char *nohelp;
extern char *salt;
extern char *login_name;
extern char *login_pwd;
extern char *login_vpwd;
extern char *login_prompt;
extern char *wrong_pwd;
extern char *wrong_pwd2;
extern char *new_tells1;
extern char *new_tells2;
extern char *continue2;
extern char *continue1;
extern char *gender1[];
extern char *gender2[];
extern char *gender3[];
extern char *gendert[];
extern char *edprompt;
extern char *menuprompt;
extern char *def_inphr;
extern char *def_outphr;
extern char *def_url;
extern char *def_bday;
extern char *def_lognmsg;
extern char *def_logtmsg;
extern char *def_desc;
extern char *def_topic;
extern char *ansi_line;
extern char *ansi_tline;
extern char *ansi_bline;
extern char *ascii_line;
extern char *ascii_tline;
extern char *ascii_bline;
extern char *mesg_line;
extern char *emp;
extern char *tab;
extern char *staff_room;
extern char *wedd_room;
extern char *html_header;
extern char *html_footer;

#endif /* RTNEEDS_H */
