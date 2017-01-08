/*
   Col[our] code values equal the following:
   CLS <ansi>
   RESET,BOLD,BLINK,REVERSE
   Foreground & background colours in order.
   BLACK,RED,GREEN,YELLOW/ORANGE,
   BLUE,MAGENTA,TURQUIOSE,WHITE
*/

/*
   The actual ansi colour codes. You can get more information on ansi
   or ascii's by reading all sorts of wonderful info. on the www.
*/
char *colcode[NumCols]=
{
	/* Clear the screen */
	"\033[H\033[J",

	/* Standard stuff */
	"\033[0m", "\033[1m", "\033[4m", "\033[5m", "\033[7m",

	/* Foreground/dullish colours */
	"\033[0;30m","\033[0;31m","\033[0;32m","\033[0;33m",
	"\033[0;34m","\033[0;35m","\033[0;36m","\033[0;37m",

	/* Background colour */
	"\033[40m","\033[41m","\033[42m","\033[43m",
	"\033[44m","\033[45m","\033[46m","\033[47m",

	/* Bolded colors */
	"\033[1;30m","\033[1;31m","\033[1;32m","\033[1;33m",
	"\033[1;34m","\033[1;35m","\033[1;36m","\033[1;37m",
};

/* Codes used in a string to produce the colours when prepended with a '~' */
char *colcom[NumCols]=
{
	"CL",
	"RS","OL","UL","LI","RV",
	"FK","FR","FG","FY", /* Dullish colours    */
	"FB","FM","FT","FW", /* Dullish colours    */
	"BK","BR","BG","BY", /* Background colours */
	"BB","BM","BT","BW", /* Background colours */
	"CK","CR","CG","CY", /* Hi-lighted colours */
	"CB","CM","CT","CW", /* Hi-lighted colours */
};

/*
   Added this so that users can have a fun colour command that produces
   a random colour ~RD on execution.
*/
char *rcolour[NumRand+1]=
{
	"",
	"\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;34m",
	"\033[1;35m", "\033[1;36m", "\033[1;37m", "\033[0;31m",
	"\033[0;32m", "\033[0;33m", "\033[0;34m", "\033[0;35m",
	"\033[0;36m", "\033[0;37m"
};

/* Who style names */
char *who_style[NumWhos+1]=
{
	"",
	"DragonS CovE",
	"Amnuts",
	"Moenuts Who",
	"Short who",
	"Emerald Isle",
	"m0ri who",
	"EI3 Who"
};

/* Help style names */
char *help_style[NumHelp+1]=
{
	"",
	"Normal",
	"Old Style",
	"Commands only"
};

/* Speech type names. */
char *speech_style[NumSpeech+1]=
{
	"Normal",
	"Scrambled",
	"Shifted",
	"Reversed"
};

/* Abbreviated the months just for the hell of it */
char *month[12]=
{
	"Jan.", "Feb.", "March", "April", "May",  "June",
	"July", "Aug.", "Sept.", "Oct.",  "Nov.", "Dec."
};

char *day[7]=
{
	"Sun.","Mon.","Tues.","Wed.","Thurs.","Fri.","Sat."
};

/* the color arrays for when the user buys a hat */
char *hatcolor[]=
{
	"",
	"~CG","~CM","~CT","~CR","~CY","~CB","~CW",
	"~FG","~FM","~FT","~FR","~FY","~FB","~RD"
};

char *noyes1[]=   { " NO","YES" };
char *noyes2[]=   { "NO ","YES" };
char *yesno1[]=   { "YES","NO " };
char *yesno2[]=   { "YES"," NO" };
char *offon[]=    { "OFF","ON " };
char *file_ext[]= { "ASC","ANS" };

/*
   These MUST be in lower case - the contains_swearing() function converts
   the string to be checked to lower case before it compares it against
   these. Also even if you dont want to ban any words you must keep the
   star as the first element in the array.
*/
char *swear_words[]=
{
	"fuck","shit","cunt","cock","slut","whore","dyke","nigger","fuk",
	"coon","faggot","queer","asshole","*"
};

/*
   These are the gender flags used in socials that get replaced by the users
   gender, such as he/she/it, him/her/its, etc
*/
char *gender_types[]=
{
	"$G1","$G2","$G3","$G4","$G5","$G6","*"
};

/*
   These are the flags that get replaced by the parse_string() function.
*/
char *user_vars[]=
{
	"$USER",    "$SITE",  "$HANGCH","$GENDER","$EMAIL","$URL",   "$LALIAS",
	"$BDAY",    "$RLEVEL","$RNAME", "$ICQ",   "$MONEY","$DMONEY","$CHIPS",
	"$DCHIPS",  "$AGE",   "$TIME",  "$PDESC", "$DESC", "*"
};

/* The pet array for when a user buys a pet. */
char *pet[]=
{
	"",
	"dog", "cat", "parakeet", "goldfish", "hamster", "dragon"
};

/*
   Users can now ignore swear words.. Below is a list of words that will
   be randomly placed instead of the swear word.
*/
char *censored[NumCensors+1]=
{
	"",
	"*bleep*",
	"*toot*",
	"*beeeeep*",
	"*fruck*",
	"*tweeet*",
	/* Had to add this one in here to humour Andy ;-) */
	"*smeg*"
};

/*
   Talker defined variables.. Eventually I'm going to be adding all this stuff
   into a talker defined structure so that it makes the talker easier to
   customize.. I just haven't had the time to do it for this release.. seeing
   as how everyone was bitching at me to get this released as it is.
*/
/* The different buffers */
Convbuff shoutbuff, wizbuff, channelbuff, debugbuff, loginbuff;

char	text[ARR_SIZE*2],		vtext[ARR_SIZE*2],
	word[MaxWords][WordLen+1],	wrd[8][81],
	progname[40],			confile[40],
	file[FSL],			sessionwho[UserNameLen+1],
	session[101],			staffmember[MaxStaff+1][UserNameLen+2],
	member[50][UserNameLen+2],	disabled[20][20],
	main_room[RoomNameLen+2],	idle_room[RoomNameLen+2],
	jail_room[RoomNameLen+2];

time_t	boot_time,		sessiontime,		sreboot_time;
jmp_buf jmpvar;  // not needed any more I don't think..

long	jackpot,		ulogins[4],		newlog[3];
int	resolve_sites,
	port[2],		listen_sock[2],		wizport_level,
	minlogin_level,		password_echo,		ignore_sigterm,
	max_users,		check_time,		max_clones,
	num_of_users,		num_of_logins,		heartbeat,
	config_line,		login_idle_time,	user_idle_time,
	word_count,		tyear,			tmonth,
	tday,			tmday,			twday,
	thour,			tmin,			tsec,
	mesg_life,		system_logging,		prompt_def,
	no_prompt,		force_listen,		gatecrash_level,
	min_private_users,	ignore_mp_level,	ban_swearing,
	mesg_check_hour,	mesg_check_min,		keepalive_interval,
	crash_action,		time_out_afks,		rs_countdown,
	autoauth,		destructed,		charecho_def,
	time_out_maxlevel,	user_count,		new_users,
	logging_in,		atmos_trigger,		backup_talker,
	shout_ban,		debug_talker,		webpage_on,
	arrest_swearers,	sclosetime,		seed,
	nuclear_countdown,	free_rooms,		auto_promote,
	auto_auth,		wiznote_num,		num_of_telnets,
	max_telnets,		ident_socket,		ident_state,
	ident_status,		ident_timer,		suggestion_num,
	bug_num,		disconnecting,		restart_loop,
	id_count,		last_uid;

short int 	boot_up;
unsigned	identpid;
time_t		rs_announce,	rs_which,		nuke_announce;
User		rs_user;

/* Big letter array map - for greet() */
int biglet[26][5][5]=
{
	{{0,1,1,1,0},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0}},
	{{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{0,1,1,1,1}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0}},
	{{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
	{{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
	{{0,1,1,1,0},{1,0,0,0,0},{1,0,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
	{{0,1,1,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
	{{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,0,0,0,1},{1,0,0,1,0},{1,0,1,0,0},{1,0,0,1,0},{1,0,0,0,1}},
	{{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}},
	{{1,0,0,0,1},{1,1,0,1,1},{1,0,1,0,1},{1,0,0,0,1},{1,0,0,0,1}},
	{{1,0,0,0,1},{1,1,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{1,0,0,0,1}},
	{{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
	{{0,1,1,1,0},{1,0,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{0,1,1,1,0}},
	{{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,1,0},{1,0,0,0,1}},
	{{0,1,1,1,1},{1,0,0,0,0},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
	{{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
	{{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}},
	{{1,0,0,0,1},{1,0,0,0,1},{0,1,0,1,0},{0,1,0,1,0},{0,0,1,0,0}},
	{{1,0,0,0,1},{1,0,0,0,1},{1,0,1,0,1},{1,1,0,1,1},{1,0,0,0,1}},
	{{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,0,0,1}},
	{{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
	{{1,1,1,1,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,1,1,1,1}}
};

/* Symbol array map - for greet() */
int bigsym[32][5][5]=
{
	{{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0}},
	{{0,1,0,1,0},{0,1,0,1,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,1,0,1,0},{1,1,1,1,1},{0,1,0,1,0},{1,1,1,1,1},{0,1,0,1,0}},
	{{0,1,1,1,1},{1,0,1,0,0},{0,1,1,1,0},{0,0,1,0,1},{1,1,1,1,0}},
	{{1,1,0,0,1},{1,1,0,1,0},{0,0,1,0,0},{0,1,0,1,1},{1,0,0,1,1}},
	{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,1,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,1,1,0},{0,1,0,0,0},{0,1,0,0,0},{0,1,0,0,0},{0,0,1,1,0}},
	{{0,1,1,0,0},{0,0,0,1,0},{0,0,0,1,0},{0,0,0,1,0},{0,1,1,0,0}},
	{{1,0,1,0,1},{0,1,1,1,0},{1,1,1,1,1},{0,1,1,1,0},{1,0,1,0,1}},
	{{0,0,1,0,0},{0,0,1,0,0},{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0}},
	{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,0,0},{1,1,0,0,0}},
	{{0,0,0,0,0},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{1,1,0,0,0},{1,1,0,0,0}},
	{{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,0,0,0,0}},
	{{0,1,1,1,0},{1,0,0,1,1},{1,0,1,0,1},{1,1,0,0,1},{0,1,1,1,0}},
	{{0,0,1,0,0},{0,1,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
	{{1,1,1,1,0},{0,0,0,0,1},{0,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
	{{1,1,1,1,0},{0,0,0,0,1},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
	{{0,0,1,1,0},{0,1,0,0,0},{1,0,0,1,0},{1,1,1,1,1},{0,0,0,1,0}},
	{{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
	{{0,1,1,1,0},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,1,1,1,1},{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0}},
	{{0,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
	{{1,1,1,1,1},{1,0,0,0,1},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1}},
	{{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0}},
	{{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,1,0,0,0}},
	{{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{0,0,1,0,0},{0,0,0,1,0}},
	{{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0}},
	{{0,1,0,0,0},{0,0,1,0,0},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0}},
	{{0,1,1,1,1},{0,0,0,0,1},{0,0,1,1,1},{0,0,0,0,0},{0,0,1,0,0}},
	{{0,1,0,0,0},{1,0,1,1,1},{1,0,1,0,1},{1,0,1,1,1},{0,1,1,1,0}}
};

/* END OF CUSTOM2.H */
