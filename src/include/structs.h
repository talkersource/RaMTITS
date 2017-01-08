#ifndef RTSTRUCTS_H
  #define RTSTRUCTS_H

#ifndef MAIN_SOURCE
  #ifdef EXTERN
    #undef EXTERN
  #endif
  #define EXTERN extern
#else
  #define EXTERN
#endif

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "defines.h"

/*
   The below are *USER* only structures to which I added basically for the
   fact that it made the code look cleaner, and as well as help reduce mem
   usage slightly..
*/
/* Structure for the user pager. */
struct pager_struct
{
	int 	  		last_page,		page_num,
				lines;
	short int 		start_page,		type;
	float     		pdone;
	unsigned long		pages[MaxPages],	filepos;
	char			page_file[256];
};

struct ban_struct
{
	short int		type;
	int			ban_length;
	char			who[81],	by[15],	reason[81];
	time_t			time_banned;
	struct ban_struct	*next,	*prev;
};
typedef struct ban_struct* Bans;
EXTERN Bans first_ban, last_ban;

/* Structure for the blackjack game.. Thanks Andy */
struct bjack_struct
{
	short int deck[52],	hand[5],	dhand[5],
		  cardpos;
	long	  bet;
};
typedef struct bjack_struct* Bjack;

struct lotto_struct
{	char			name[UserNameLen+1];
	int			ball[6];
	struct lotto_struct	*prev,	*next;
};
typedef struct lotto_struct* Lotto;
EXTERN Lotto lotto_first, lotto_last;

/* Structure for the editor */
struct edit_struct
{
	int       edit_line,	charcnt,	editing,
		  editor,	edit_op,	maxlines,
		  visual;
	char	  *malloc_start,		*malloc_end;
};

/* Structure for the user defined shortcuts */
struct scut_struct
{
	char	  scut,		scom[51];
};

/* Structure for the users pet. */
struct pet_struct
{
	int	  type;
	char	  name[UserNameLen+1];
	char	  alias[20][16],		trigger[20][76];
};

/*
   This structure is in fact one of the more important because this is the
   key in fixing up the default NUTS bug with the cut and pasting between
   sockets.. Thanks to Wookey for this.
*/
struct input_buff
{
	char	  *buff,	*start_buff,	*buff_max,
		  buffer[ARR_SIZE*2];
	int	  telnet_mode,	telnet_opt,	telnet_cmd;
	struct input_buff	*prev,		*next;
};
typedef struct input_buff* Input;

struct alias_struct
{
	char	find[25][12],	alias[25][151];
};

struct conv_buff
{
	int	  count;
	char	  *conv[NUM_LINES];
	struct conv_buff	*next,		*prev;
};
typedef struct conv_buff* Convbuff;
EXTERN Convbuff first_conv,last_conv;

/* The GAEN Puzzle structure.. Thanks Sabin */
struct puzzle_struct
{
	int	table[DIMEN][DIMEN];	/* Table w/ Numbers */
	int	clin,		ccol;	/* Position. */
	int	counter;		/* How many moves */
};

/*
   The elements vis, ignall, prompt, command_mode etc could all be bits in
   one flag variable as they're only ever 0 or 1, but I tried it and it
   made the code unreadable. Better to waste a few bytes
   Reduced some amounts here, as well as cleaned it up a bit -Sq
*/

struct user_struct
{
	char	name[UserNameLen+1],		recap[251],	
		desc[251],			predesc[81],
		lev_rank[RankLen+1],		pass[PassLen+6],
		email[EmailLen+1],		webpage[EmailLen+1],
		bday[81],			married_to[UserNameLen+1],
		in_phr[PhraseLen+1],		out_phr[PhraseLen+1],
		site[SiteNameLen+1],		last_site[SiteNameLen+1],
		ip_site[16],			fmail[EmailLen+1],
		mail_to[WordLen+1],		temp_desc[251],
		afk_mesg[AfkMesgLen+1],		sstyle[21],
		array[10],			talk_to[UserNameLen+1],
		hangman_guessed[28],		hangman_word[41],
		hangman_display[45],		check[ARR_SIZE],
		login_msg[PhraseLen+1],		logout_msg[PhraseLen+1],
		waiting[UserNameLen+1],		realname[41],
		temp_recap[251],		vows[ARR_SIZE],
		trivia_ans[5],			trivia_check[81],
		allowcom[25][13],		denycom[25][13],
		bfafk[AfkMesgLen+1],		triv_check[50],
		tzone[11],			emailing[EmailLen+1],
		rmacros[17][ReplaceLen+1],	mychar1[81],
		fmacros[17][FindLen+1],		social[5][251],
		friends[50][UserNameLen+1],	enemies[50][UserNameLen+1],
		ccopy[5][UserNameLen+2],	hangman_opp[UserNameLen+1],
		following[UserNameLen+1],	follow_by[UserNameLen+1],
		autoexec[21],			challenger[UserNameLen+1],
		mychar2[81],			mychar3[81],
		mychar4[81],			mychar5[81],
		mychar6[81],			mychar7[81],
		mychar8[81],			mychar9[81],
		mychar10[81];
	short 	login,		type,		chksmail,	notify,
		attempts,	invis,		ignall,		prompt,
		mode,		char_echo,	whitespace,	level,
		profile,	icqhide,	bank_op,	store_op,
		set_op,		warned,		no_fight,	clone_hear,
		afk,		ignall_store,	colour,		chkrev,
		ignore,		first,		hangman_stage,	hangman_mode,
		monitor,	age,		helpw_lev,	login_type,
		helpw_com,	hangman_rnds,	turn,		protection,
		talk,		rows,		cols,		quit,
		earning,	no_follow,	has_room,	tpromoted,
		orig_lev,	triv_mode,	triv_right,	tnum[9],
		triv_wrong,	game_op,	menus,		money_time,
		hat,		carriage,	gender,		hide_email,
		ansi_on,	help,		disttrig,	who,
		helpw_same,	helpw_type,	hide_room,	atmos,
		speech,		status,		cloak_lev,	delprotect,
		last_roll,	challenged,	has_macros,	playing,
		chances,	tries,		guess_num,	nukes,
		keys,		shot,		chamber,	drunk,
		mail_op,	input,		splitseffect,	splits,
		car_return,	gshout,		gpriv,		gsocs,
		alarm_set,	misc_op,	page_finished,  autofwd,
		c4_turns,	tchallenge,	cchallenge,	pieces,
		sys_com,	cft,		steal_tries,	site_screened,
		autodetected,	clone_cnt[20];
	int	port,		site_port,	socket,		hangman_wins,
		lose,		win,		draw,		board[8][7],
		c4_win,		c4_lose,	tvar3,		tvar2,
		logons,		tic_win,	tic_lose,	tic_draw,
		tvar1,		condoms,	examined,	distrig,
		hang_wins,	hang_loses,	chess_wins,	chess_lose,
		chess_draws,	beers,		bullets,	myint1,
		alarm_time,	bship_wins,	bship_lose,	next_ping,
		verifynum,	dns_id,		ident_id,	id,
		checker_win,	checker_lose,	checker_pieces,
		myint2,		myint3,		myint4,		myint5,
		myint6,		myint7,		myint8,		myint9,
		myint10;
	long	last_login_len,	money,		dep_money,	chips,
		icq,		dep_chips,	last_ping;
	time_t	last_input,	last_login,	total_login,	pause_time,
		idle_time;
	Input	buffer;
	Convbuff		away_buff,	tbuff;
	struct timeval		ping_timer;
	struct room_struct	*room,		*invite_room;
	struct user_struct	*prev,		*next,		*aprev,
				*anext,		*owner,		*chess_opp,
				*tic_opponent,	*c4_opponent,	*checker_opp;
	struct pager_struct	pager;
	struct chess_struct	*chess;
	struct checkers_struct	*checkers;
	struct poker_player	*pop;
	struct pet_struct	pet;
	struct scut_struct	scut[28];
	struct edit_struct	editor;
	Bjack			bjack;
	struct puzzle_struct	*puzzle;
	struct alias_struct	alias;
};
typedef struct user_struct* User;
EXTERN User user_first,user_last,user_afirst,user_alast,dead_user;

/*
   I decided to do this similiar to the amnuts style because from what I've
   seen.. to much file i/o causes huge problems.. and well, this will be
   more stable in the long run.. we have one structure which contains the
   list of all users and the level that each of the users are at...
*/
struct userlist_struct
{
	char	name[UserNameLen+1];
	short	lev;
	struct userlist_struct	*prev,	*next;
};
typedef struct userlist_struct* Ulobj;
EXTERN Ulobj ulist_first,ulist_last;

/* Room structure */
struct room_struct
{
	char	name[RoomNameLen+1],	topic[TopicLen+1];
	char	owner[UserNameLen+1],	map[6];
	char 	recap[81],		link[MaxLinks][RoomNameLen+2];
	char	key[MaxKeys][15],	atmos[MaxAtmos+1][AtmosLen+2];
	short	access,		mesg_cnt,	no_buff,	topic_lock,
		lock,		temp_access,	hidden,		level;
	Convbuff		revbuff;
	struct room_struct	*next,		*prev;
};
typedef struct room_struct* Room;
EXTERN Room room_first,room_last;

/* Room list structure */
struct rmlist_struct
{
	char 	name[RoomNameLen+1],	map[6];
	short	access;
	struct rmlist_struct	*prev,	*next;
};
typedef struct rmlist_struct* Rmlobj;
EXTERN Rmlobj rlist_first,rlist_last;

/* Telnet structure */
struct telnet_struct
{
	char	site[SiteNameLen+2];
	int	socket,			port;
	short	phone,			connected;
	time_t	opened,			last_received;
	Convbuff buff;
	struct user_struct	*owner;
	struct room_struct	*room;
	struct telnet_struct	*prev,	*next;
};
typedef struct telnet_struct* Telnet;
EXTERN Telnet telnet_first,telnet_last;

/* Poker player structure */
struct poker_player
{
	int	hand[5],	putin,		touched;
	int	rank,		dealt,		turn;
	struct user_struct	*user;
	struct poker_game	*game;
	struct poker_player	*prev,		*next;
};
typedef struct poker_player* Pplayer;
EXTERN Pplayer pplayer_first,pplayer_last;

/* Poker game structure */
struct poker_game
{
	char	name[GameNameLen+1];
	int	num_players,	num_raises,	in_players;
	int	newdealer,	deck[52],	top_card;
	int	state,		pot,		bet;
	struct room_struct	*room;
	struct poker_player	*players,	*dealer,
		*opened,	*curr_player,	*first_player;
	struct poker_game 	*prev,		*next;
};
typedef struct poker_game* Pgame;
EXTERN Pgame pgame_first,pgame_last;

/* Editor structure */
struct editor_struct
{
	char	line[MaxLines][80*3+1];
	int	current_line,	max_lines,	last_line;
	int	charcnt,	current_char,	edited;
	struct user_struct	*user;
	struct editor_struct	*prev,		*next;
};
typedef struct editor_struct* Editor;
EXTERN Editor edit_first,edit_last;
EXTERN int num_editors;

struct soc_struct
{
	char	name[21];
	struct soc_struct	*next,		*prev;
};
typedef struct soc_struct* Socials;
EXTERN Socials first_social,last_social;
	
typedef void (*user_function) (User,char *,int);
/* Command structure */
struct cmd_struct
{
	char	name[21];
	int	count,id;
	short	min_lev,	type,	disabled,	args;
	user_function funct;
	struct cmd_struct *next,*prev;
};
typedef struct cmd_struct* Commands;
EXTERN Commands first_command,last_command;

#ifdef HAVE_BSHIP
/* Battleship structure */
struct bship_struct
{
	char	name[UserNameLen+2];
	char	opponent[UserNameLen+2];
	int	flags,		score,		grid[8][8];
	struct bship_struct	*next;
};
typedef struct bship_struct* Bship;
EXTERN Bship bships_first;
#endif

struct fight_struct
{
	int	issued,		time;
	User	first_user,	second_user;
};
typedef struct fight_struct Fight;
EXTERN Fight fight;

/*
   Just a minor structure to limit the amount of connections which can be
   made within a minute ie.. to stop portfuckers who try to crash you. Big
   thanks to Cygnus (Ncohafmuta) for this.
*/
EXTERN struct
{
	char	site[16];
	int	connections;
} connlist[20];

struct chess_struct
{
	int	pos[10][10],		lastpos[10][10],	new_turn,
		wroc,			broc;
	char	white[UserNameLen+1],	black[UserNameLen+1],
		last_move[800],		last_taken[300],	last_moved[300];
	struct user_struct 		*turn,			*last_turn;
};
typedef struct chess_struct* Chess;

struct checkers_struct
{
	int	pos[10][10],	orig_pos[10][10], new_turn,	hops,
		jump_v,		jump_h;
	char	white[UserNameLen+1],	red[UserNameLen+1],
		last_move[ARR_SIZE*2];
	struct user_struct 		*turn,			*last_turn;
};
typedef struct checkers_struct* Checkers;

#undef EXTERN
#endif RTSTRUCTS_H
