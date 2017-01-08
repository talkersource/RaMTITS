#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "defines.h"
#include "structs.h"
#include "custom2.h"
#include "protos.h"
#include "commands.h"

/* Random afk messages */
#define NumBfafkMesgs 15
char BfafkText[][200]=
{
	"",		/* Leave this alone. */
	" -- ~FG%s ~FTreturns to their keyboard... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTwakes up after dozing off... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTdecides to come back to life... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTventures back to the land of the living... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTfigures their eyes got enough rest... ~FB*~FGRTK~FB*\n",

	" -- ~FG%s ~FTreturns to their keyboard... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTwakes up after dozing off... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTdecides to come back to life... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTventures back to the land of the living... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTfigures their eyes got enough rest... ~FB*~FGRTK~FB*\n",

	" -- ~FG%s ~FTreturns to their keyboard... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTwakes up after dozing off... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTdecides to come back to life... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTventures back to the land of the living... ~FB*~FGRTK~FB*\n",
	" -- ~FG%s ~FTfigures their eyes got enough rest... ~FB*~FGRTK~FB*\n",
};

/* Random Quote of the days..when the user logs onto the talker */
#define MAX_QUOTES 50
char *qotd[MAX_QUOTES+1]=
{
	"",	/* Leave this alone. */

	" ~CB-- ~FYNever eat more than you can lift.\n",
	" ~CB-- ~FYPassionate hatred can give meaning and purpose to an empty life.\n",
	" ~CB-- ~FYThink honk if you're a telepath.\n",
	" ~CB-- ~FYIf God had intended Man to Smoke, He would have set him on Fire.\n",
	" ~CB-- ~FYChicago law prohibits eating in a place that is on fire.\n",

	" ~CB-- ~FYDon't take life too seriously -- you'll never get out of it alive.\n",
	" ~CB-- ~FYBoy, n.:  	A noise with dirt on it.\n",
	" ~CB-- ~FYSome programming languages manage to absorb change but withstand progress.\n",
	" ~CB-- ~FYWhatever became of eternal truth?\n",
	" ~CB-- ~FYThe day after tomorrow is the third day of the rest of your life.\n",

	" ~CB-- ~FYWhen all other means of communication fail, try words.\n",
	" ~CB-- ~FYYour fault: core dumped\n",
	" ~CB-- ~FYI used to get high on life but lately I've built up a resistance.\n",
	" ~CB-- ~FYOptimization hinders evolution.\n",
	" ~CB-- ~FYHow long a minute is depends on which side of the bathroom door you're on.\n",

	" ~CB-- ~FYYou are the only person to ever get this message.\n",
	" ~CB-- ~FYIf I don't see you in the future, I'll see you in the pasture.\n",
	" ~CB-- ~FYNobody said computers were going to be polite.\n",
	" ~CB-- ~FYI must have slipped a disk -- my pack hurts\n",
	" ~CB-- ~FYOn-line, adj.:  The idea that a human being should always be accessible to a computer.\n",

	" ~CB-- ~FYJustice is incidental to law and order.\n",
	" ~CB-- ~FYIf God didn't mean for us to juggle, tennis balls wouldn't come three to a can.\n",
	" ~CB-- ~FYAlways borrow money from a pessimist; he doesn't expect to be paid back.\n",
	" ~CB-- ~FYTo invent, you need a good imagination and a pile of junk.\n",
	" ~CB-- ~FYI think sex is better than logic, but I can't prove it.\n",

	" ~CB-- ~FYAn idea is not responsible for the people who believe in it.\n",
	" ~CB-- ~FYTake everything in stride.  Trample anyone who gets in your way.\n",
	" ~CB-- ~FYNever try to outstubborn a cat.\n",
	" ~CB-- ~FYFine day to work off excess energy.  Steal something heavy.\n",
	" ~CB-- ~FYThere are many intelligent species in the universe.  They all own cats.\n",

	" ~CB-- ~FYReal Users know your home telephone number.\n",
	" ~CB-- ~FYCChheecckk yyoouurr dduupplleexx sswwiittcchh..\n",
	" ~CB-- ~FYThe revolution will not be televised.\n",
	" ~CB-- ~FYThe sum of the Universe is zero.\n",
	" ~CB-- ~FYPeople need good lies.  There are too many bad ones.\n",

	" ~CB-- ~FYHealth is merely the slowest possible rate at which one can die.\n",
	" ~CB-- ~FYHelp stamp out and abolish redundancy.\n",
	" ~CB-- ~FYAny time things appear to be going better, you have overlooked something.\n",
	" ~CB-- ~FYGo 'way!  You're bothering me!\n",
	" ~CB-- ~FYDo you have lysdexia?\n",

	" ~CB-- ~FY\"Virtual\" means never knowing where your next byte is coming from.\n",
	" ~CB-- ~FYThe wages of sin are high but you get your money's worth.\n",
	" ~CB-- ~FYGenetics explains why you look like your father, and if you don't, why you should.\n",
	" ~CB-- ~FYYou may have heard that a dean is to faculty as a hydrant is to a dog.\n",
	" ~CB-- ~FYJust because everything is different doesn't mean anything has changed.\n",

	" ~CB-- ~FYHelp a swallow land at Capistrano.\n",
	" ~CB-- ~FYI'd love to go out with you, but I never go out on days that end in 'Y'\n",
	" ~CB-- ~FYNew York is real.  The rest is done with mirrors.\n",
	" ~CB-- ~FYThere is no distinctly native American criminal class except Congress.\n",
	" ~CB-- ~FYElectrical Engineers do it with less resistance.\n",
};

/* Default atmospherics in case originals don't exist */
char *atmos[MaxAtmos+1]=
{
	"", /* Leave this alone. */

	"A faint scream can be heard in the distance...",
	"If you need help, ask a wiz or other staff member...",
	"If you're new to ramtits talkers, be sure to read ~CR.newbie",
	"For a list of your commands, type .help",
	"To learn about talkers colours, type .text colours",

	"If you need help with a command, type '.help command'",
	"A soft breeze blows through your hair...",
	"A warm wind breezes past you...",
	"You can set the majority of items in your profile with '.set'",
	"You can create command shortcuts with .scut",

	"Some commands have different options, such as .rmail",
	"Welcome to this talker... we're happy to have you here...",
	"Strange noises can be heard all around you...",
	"A brief scream echoes throughout...",
	"If you'd like to ignore these messages, use .ignore atmos",

	"If you need help with a command, type '.help command'",
	"A soft breeze blows through your hair...",
	"A warm wind breezes past you...",
	"You can set the majority of items in your profile with '.set'",
	"You can create command shortcuts with .scut"
};

char *signature=
"
---===---===---===---===---===---===---===---===---===---===---===---===---
   You've successfully received e-mail from the talker %s.
   Hopefully you enjoyed the talker (located at: %s).
   If you have any questions or concerns reguarding the talker, please
   feel free to e-mail the owners of this talker at: %s
   Thank you, and have a nice day.
   %s (%s)
---===---===---===---===---===---===---===---===---===---===---===---===---
";

/* Default stuff */
char *syserror=   "~CB[~CRWARNING~CB]~FT A system error has occured.\n";
char *nosuchroom= "~FT-~CT=~CB>~FT There is no such room, or it's not loaded in memory.~CB <~CT=~FT-\n";
char *nosuchuser= "~FT-~CT=~CB>~FT That user wasn't found, or multiple matches were found.~CB <~CT=~FT-\n";
char *notloggedon="~FT-~CT=~CB>~FT That user either isn't online or multiple matches were found.~CB <~CT=~FT-\n";
char *invisenter= "~FT A subtle tiptoe can be heard entering the room...\n";
char *invisleave= "~FY A subtle tiptoe can be heard exiting the room...\n";
char *nosuchlev=  "~FT-~CT=~CB> ~FTI've never heard of that level. ~CB<~CT=~FT-\n";
char *invisname=  "Shadow";
char *invismesg=  "Shadow";
char *noswearing= "There's no swearing allowed!\n";
char *login_vis=  "~CB[~CTC~CB]~FT Connecting is~CB:";
char *login_invis="~CB[~CTC-I~CB]~FT Connecting-invis is~CB:";
char *login_hide= "~CB[~CTC-H~CB]~FT Connecting-hidden is~CB:";
char *login_fake= "~CB[~CTC-F~CB]~FT Connecting-fake is~CB:";
//
char *logout_vis= "~FT[~CTQ~FT]~FG Leaving is~CB:-=>";
char *logout_inv= "~FT[~CTQ-I~FT]~FG Leaving-invis is~CB:";
char *logout_hide="~FT[~CTQ-H~FT]~FG Leaving-hide is~CB:";
char *logout_fake="~FG[~CTQ-F~FT]~FG Leaving-fake is~CB:";
char *wtf=        "Couldn't find command or social: %s\n";
char *notallowed= "The \"%s\" command is for level %s.\n";
char *clnodo=     "Clones can't execute the \"%s\" command.\n";
char *tempnodo=   "Temp. promoted users cannot use the \"%s\" command.\n";
char *muzztext=   "Muzzled users cannot play with the \"%s\" command.\n";
char *qotdm=      "~FG-- ~RSYour quote for today is...\n";
char *nohelp=     "I can't find a help file for the \"%s\" command.\n";
char *salt=       "NU"; /* This shouldn't be changed */
char *login_name= "Please enter your name: ";
char *login_pwd=  "And your password is?: ";
char *login_vpwd= "Enter your password again for verification: ";
char *login_prompt="Please enter your name or choice from the menu: ";
char *wrong_pwd=  " That password was either incorrect or the name you chose has been taken.\n If it's the latter, please type: ~CW\"~CRimnew~CW\"~RS now.\n";
char *wrong_pwd2= " That password was incorrect.\n";
char *new_tells1= "~CB:: ~RSYou have some new tells %s\n";
char *new_tells2= "~CB:: ~RSTo see them, type \"~CR.rev -tell~RS\"\n";
char *continue1=  "~CB::~RS Press ~CW[~CREnter~CW] ~RSto continue or ~CW[~CRQ~CW]~RS+~CW[~CREnter~CW]~RS to quit: ";
char *continue2=  "~CB:: [~CREnter~CW] ~RS= Continue, ~CW[~CRQ~CW]~RS+~CW[~CREnter~CW] ~RS= Abort: ";
char *gender1[]={ "its","her","her","his","his" };
char *gender2[]={ "it", "her","her","him","him" };
char *gender3[]={ "it", "she","she","he","he"   };
char *gendert[]={ "Unknown","Female","Female","Male","Male" };
char *edprompt=   "~CW(~CTS~CW)~RSave, ~CW(~CTA~CW)~RSbort, or ~CW(~CTR~CW)~RSedo?: ";
char *menuprompt= " Please enter your choice: ";

/*
   IMPORTANT: make sure these DO NOT exceed the arrays set for them, other-
   wise you'll get corrupted files.
*/
char *def_inphr=  "enters the room.";		/* MAX 40 chars. */
char *def_outphr= "leaves the room.";		/* MAX 40 chars. */
char *def_url=    "http://www.ramtits.ca";
char *def_bday=   "Currently Unset";
char *def_lognmsg="enters the talker...";	/* MAX 40 chars. */
char *def_logtmsg="leaves the talker...";	/* MAX 40 chars. */
char *def_desc=   "is new to the talker...";	/* MAX 30 chars. */
char *def_topic=  "Welcome to this talker...";	/* MAX 60 chars. */

char *ansi_line=  "~FT<®ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¯>\n";
char *ansi_tline= "~FTÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n";
char *ansi_bline= "~FTÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n";
char *ascii_line= "~FY------------------------------------------------------------------------------\n";
char *ascii_tline="~FY.----------------------------------------------------------------------------.\n";
char *ascii_bline="~FY`----------------------------------------------------------------------------'\n";
char *mesg_line=  "------------------------------------------------------------------------------\n";
char *emp=        "~FT-~CT=~FB>~FY This log file is empty.~FB <~CT=~FT-\n";
char *tab=        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

/* Pre defined rooms for warping and stuff */
char *staff_room= "sacrifice ring";
char *wedd_room=  "church";

/*
   Header and footer for the html generated codes. Figured it'd be easier to
   do it this way then have to do it for each page. I'm just lazy ;-)
*/
char *html_header=
"<html>
<head>
<title>RaMTITS generated webpage</title>
</head>
<body bgcolor=\"Black\" text=\"White\" link=\"White\" vlink=\"White\" alink=\"Green\">
";

char *html_footer=
"
</body>
</html>
";

/* END OF CUSTOM1.H */
