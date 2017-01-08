/*
   Added more directories, not to make it more complex, but if you
   notice, it is easier to keep tabs on where everything is when
   you want to fix or work on a certain item. For more information, read
   the ramtits/docs/ABOUT.dirs file.
*/
#ifndef RTDEFINE_H
  #define RTDEFINE_H

#define __USE_XOPEN
#include <signal.h>
#include <sys/socket.h>

/*
   Stuff that defines the talker change to your own values, just keep
   the version as mine. *updated* also now this area defines stuff for
   the webpage.
*/
#define TalkerName	"RaMTITS Talker"	// You SHOULDN'T have colours in this.
#define TalkerRecap	"~FGRT2 Talker"		// This one yah..
#define TalkerMailName	"RT2_Talker"		// Try to keep under 12 chars.
#define TalkerOwner	"Test"
#define TalkerEmail	"dragons@ramtits.ca"
#define TalkerAddy	"ramtits.ca 6980"
//#define WWWDir		"/home/name/public_html" // define if you use www generator.
#define WWWDir		"/home/squirt/webpage"
#define UserIndex	"index.html"
#define WWWExec		"/usr/bin/lynx -dump -anonymous"
#define AtmosString	"~CB --~RS %s\n"
#define SendMailExec	"/usr/sbin/sendmail"


/* DON'T EDIT PAST HERE! */
#define VERSION    "RaMTITS v2.2"	// DON'T TOUCH!
#define DataDir    "../datafiles"	// datafiles directory.
#define UserDir    "../userfiles"	// userfiles directory.
#define UserMesg   "usermesg"		// mailfiles, profiles, etc.
#define UserMisc   "usermisc"		// user records, friends, enemies, etc.
#define UserSet    "settings"		// aliases, shortcuts, etc.
#define HelpDir    "../helpfiles"	// helpfiles directory.
#define TextDir    "../textfiles"	// textfiles directory.
#define RoomDir    "../roomfiles"	// roomfiles directory.
#define AtmosFiles "atmospherics"
#define Descript   "descriptions"
#define MesgBoard  "mesgboards"
#define SysLogDir  "../systemlogs"	// system logs directory.
#define SysLogs    "syslogs"		// dir that contains older syslogs.
#define LoginLogs  "loginlogs"		// dir that contains older loginlogs.
#define EmailLogs  "emaillogs"		// dir that contains older emaillogs.
#define WWWLogs    "wwwlogs"		// dir that contains older wwwlogs.
#define CapLogs    "caplogs"		// dir that contains older capture logs.
#define DebugLogs  "debuglogs"		// dir that contains older debug logs.
#define StaffLogs  "stafflogs"		// dir that contains logs of staff cmds
#define TelnetLogs "telnetlogs"		// dir that contains older telnetlogs.
#define PicDir     "../picfiles"	// dir that contains picfiles.
#define ScreenDir  "../screens"		// dir that contains the screens.
#define MiscDir    "../miscfiles"	// dir that contains misc files.
#define TriviaDir  "../trivia"
#define SourceDir  "../source"		// I hope you got the point now.
#define UserBack   "backups"
#define TempFiles  "tempfiles"
#define UserPics   "pictures"
#define SocialDir  "../socials"
#define BanNew	   "BANNED.new"
#define BanSites   "BANNED.sites"
#define BanUsers   "BANNED.users"
#define BanWho     "BANNED.who"
#define SiteScreen "BANNED.screen"

/*
   Various files
*/
#define ConfigFile	"config"	// Kind of obsolete now.
#define MAP1		"map1"		// your talkers main map
#define MAP2		"map2"		// the staff map
#define SysLog		"syslog"
#define LoginLog	"loginlog"
#define EmailLog	"emaillog"
#define TelnetLog	"telnetlog"
#define WWWLog		"wwwlog"
#define CapLog		"caplog"
#define DebugLog	"debug_file"
#define StaffLog        "stafflog"
#define OpenScreen	"openscreen"
#define CloseScreen	"closescreen"
#define Motd		"motd"
#define NewsFile	"newsfile"
#define TalkerFile	"talkerfile"
#define StaffFile	"staff_file"
#define SugFile		"suggestion_file"
#define BugFile		"bug_file"
#define RulesFile	"rulesfile"
#define StaffRules	"staffrules"
#define NewbieFile	"newuserfile"
#define PicList		"piclist"
#define TextList	"textlist"
#define HangList	"hangman_listing"
#define StaffList	"stafflist"
#define LevelList	"levellist"
#define MemberFile	"paid_file"
#define WhoWebpage	"whofile"
#define SocNames	"socials"

#define MaxOpen		5
#define MaxQuotes	50
#define OutBuffSize	1000
#define MaxWords	10
#define WordLen		40
#define ARR_SIZE	1024
#define MaxLines	50
#define DefaultLines	15
#define NumCols		30
#define NumRand		14
#define NumHats		15
#define NumWhos		8
#define NumHelp		4
#define NumSpeech	4
#define NumCensors	6
#define UserNameLen	12
#define UserDescLen	30
#define AfkMesgLen	60
#define PhraseLen	40
#define PassLen		20		  // only first 8 are encrypted.
#define PreDescLen 	8
#define RankLen		8		  // Length of user->lalias
#define ComNameLen	12		  // Command name length. (.debug)
#define RoomNameLen	20		  // For room names.
#define GameNameLen	25		  // For poker games.
#define TopicLen	75		  // Length of topics.
#define MaxLinks	10		  // Max amt of room links.
#define MaxAtmos	20		  // Max amt of atmos.
#define SiteNameLen	80		  // Length of sites, shouldn't be more.
#define MaxPlayers	5		  // Max amount of players for poker.
#define AliasLen	65		  // Length of alias'
#define AtmosLen	75		  // Length of atmospherics.
#define AtmosFreq	120		  // how often atmos should appear.
#define DeleteSecs	10368000	  // time before old accts get deleted.
#define MaxMbytes	25000  		  // Max amnt of bytes for s-mail
#define MaxSMbytes	15000		  // Sent s-mail bytes.
#define FindLen		30		  // For my macros.
#define ReplaceLen	80		  // For my macros.
#define TelnetWait	10

/* v2.x additions */
#define SiteBan		1
#define NewSBan		2
#define WhoSBan		3
#define UserBan		4
#define Screen		5
#define ScutLen		30		  // Length of the shortcut.
#define MaxStyleLen	5		  // for user->sstyle.
#define NumPets		5		  // Pets :-)
#define MaxKeys		20		  // User keys.
#define NumVars		19		  // Don't touch.
#define BirthDays	"birthdays"	  // birthday file.
#define PingInterval	10
#define FCLOSE(fp)     if (fp) fclose(fp)
#define DefaultJackpot	500		  // what the lotto jackpot resets to
#define MaxStaff	30		  // Max Number of staff members.
/*
   Various macros for ease of coding, got the idea for the object loops from
   Neil Robertson's MUD code.. I find this is make things much easier.
*/
#define for_users(u)	for (u = user_first; u != NULL; u = u->next)
#define for_rooms(r)	for (r = room_first; r != NULL; r = r->next)
#define for_comms(c)	for (c = first_command; c != NULL; c = c->next)
#define for_ausers(au)	for (au = user_afirst; au != NULL; au = au->anext)
#define for_telnet(t)	for (t = telnet_first; t != NULL; t = t->next)
#define for_edit(e)	for (e = edit_first; e != NULL; e = e->next)
#define for_player(pp)	for (pp = pplayer_first; pp != NULL; pp = pp->next)
#define for_game(pg)	for (pg = pgame_first; pg != NULL; pg = pg->next)
#define for_list(ul)	for (ul = ulist_first; ul != NULL; ul = ul->next)
#define for_rmlist(rl)	for (rl = rlist_first; rl != NULL; rl = rl->next)
#define for_socials(s)	for (s = first_social; s != NULL; s = s->next)
#define for_lotto(l)	for (l = lotto_first; l != NULL; l = l->next)
#define for_bans(b)	for (b = first_ban; b != NULL; b = b->next)
#define MaxPages	500			// Should be enough.
#define MyStrlen(x) ((int)strlen(x))		// Eliminate ANSI problem.
#define WriteDefault	0
#define WriteInputEcho	1
#define WritePrompt	2
#define WriteDirect	3
#define PagerPrompt1	"~CB[~CR%.0f%%~CB]:[~CR%d~CW/~CR%d~CB]~RS Your command: ~CW(~CREnter~RS|~CRh~RS|~CRq~RS|~CRr~RS|~CRp~RS|~CRt~RS|~CR#~CW)~RS: "
#define PagerPrompt2	"~CB[~CR100%%~CB]:[~CR%d~CW/~CR%d~CB]~RS Your command: ~CW(~CREnter~RS|~CRh~RS|~CRq~RS|~CRr~RS|~CRp~RS|~CRt~RS|~CR#~CW)~RS: "
#define EmailLen	80	  		// Length of e-mail or web urls.
#define FSL		256			// Length of filenames.
/*
   Things needed for the GAEN Puzzle game (From Sabin)
*/
#define DIMEN		4
#define LFT		0
#define RGT		1
#define FWD		2
#define BWD		3
#define usage		"~CW - ~FGCommand usage~CB: ~CR%s %s\n"
#define NUTS		0
#define EWTOO		1
#define IRC		2

/*
   Decided to combine log files all into one function. Less sloppy coding that
   Way in my opinion, easier to keep track of too. So now to write to a log
   it would be write_log(str,write_time,type) (I know it's like amnuts, but I
   see now why Andy did that.
*/
#define LOG1		1		  // syslog
#define LOG2		2		  // loginlog
#define LOG3		3		  // emaillog
#define LOG4		4		  // staff command log.
#define LOG5		5		  // www log
#define LOG6		6		  // telnet log

#define NUM_LINES	25

/* Room flags */
#define PUBLIC		0
#define PRIVATE		1
#define FIXED		2
#define FIXED_PUBLIC	2
#define FIXED_PRIVATE	3
#define PERSONAL	4
#define STAFF_ROOM	6
#define INV_MALE	13
#define INV_FEM		27
#define INV_ALL		53

/* user type & clone flags */
#define UserType	0
#define CloneType	1
#define Clone_Nothing	0
#define Clone_Swears	1
#define Clone_All	2

/* misc flags */
#define Female		2
#define Male		4
#define Invis		1
#define Hidden		2
#define Fakelog		3
#ifndef TRUE
  #define TRUE		1
#endif
#ifndef FALSE
  #define FALSE		0
#endif

/* Flags for user->ignore */
#define Tells		1 
#define Shouts		2
#define Logons		4
#define Beeps		Logons*2
#define Pictures	Beeps*2
#define Wizard		Pictures*2
#define Atmos		Wizard*2
#define Friends		Atmos*2
#define Channels	Friends*2
#define Swears		Channels*2
#define Blinks		Swears*2
#define Figlets		Blinks*2
#define Alert		Figlets*2
#define Execall		Alert*2
#define Quotd		Execall*2

/* status flagged jailed, silenced...etc,etc */
#define Jailed		1		// User is jailed.
#define Gagged		2		// User is gagged.
#define Silenced	4		// User is silenced.
#define Muzzled		8		// User is muzzled.
#define Shackled	16		// User is shackled.

/* Stuff for userfiles/roomfiles */
#define wbuff(buff)  if (!xwrite(fp,(char *)buff,sizeof(buff))) ++err; err+=(ferror(fp)!=0);
#define rbuff(buff)  if (!xread(fp, (char *)buff,sizeof(buff))) ++err; err+=(ferror(fp)!=0);
#define wval(val)    if (!xwrite(fp,(char *)&val,sizeof(val)))  ++err; err+=(ferror(fp)!=0);
#define rval(val)    if (!xread(fp, (char *)&val,sizeof(val)))  ++err; err+=(ferror(fp)!=0);

#if !defined(__GLIBC__) || (__GLIBC__ < 2)
  #define SIGNAL(x,y) signal(x,y)
#else
  extern __sighandler_t __sysv_signal __P ((int __sig, __sighandler_t __handler));
  #define SIGNAL(x,y) __sysv_signal(x,y)
#endif

#ifdef _AIX
  #include <sys/select.h>
#endif

#ifdef HPUX_SB
  #define FD_CAST (int *)
#else
  #define FD_CAST (fd_set *)
#endif

/*
  Some newer systems/compilers? may require the below #if statement uncommented,
*/
/*
#ifndef __socklen_t_defined
  typedef __socklen_t socklen_t;
#  define __socklen_t_defined
#endif
*/
#endif /* RTDEFINE_H */
