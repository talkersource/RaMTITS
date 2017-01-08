#ifndef MAIN_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif

/*
   PLEASE NOTE.. When adding a new level, I've decided to make this as easy
   as possible.. What you have to do.. is: first off... add the level below
   as well as an appropriate number for it, and name as well.. DO NOT change
   the original names.. if you have to change the numbers for them, then
   that's fine. Then, once you have the name, and appropriate number for it
   Add it correspondingly into the structure below... The first area is set
   for the normal name of the level, DO NOT CHANGE THE ORIGINAL NAMES!!!!!!
   Then, beside that, is the MALE name for the level, and beside that is the
   FEMALE name for the level.. Lastly is the Numeric value of the level...
   See how easy it is? ;)
*/
#define NEWBIE 0
#define USER   1
#define SUPER  2
#define POWER  3
#define WIZ    4
#define ADMIN  5
#define GOD    6
#define OWNER  7


EXTERN struct {
	char *name,*mname,*fname;
	short int lev;
	} level[]
#ifdef MAIN_SOURCE
	={
	/* norm. name,   m-name,	f-name       defined */
	{ "NEWBIE",	"Newbie",	"Newbie",	NEWBIE	},
	{ "USER"  ,	"User",		"User",		USER	},
	{ "SUPER",	"Super",	"Super",	SUPER	},
	{ "POWER",	"Power",	"Power",	POWER	},
	{ "WIZ",	"Wizard",	"Wizard",	WIZ	},
	{ "ADMIN",	"Admin",	"Admin",	ADMIN	},
	{ "GOD",	"God  ",	"Goddess",	GOD	},
	{ "OWNER",	"Owner",	"Owner",	OWNER	},
	{ "*",		"*",	  "*",		-1	} /* Stopping clause */
	}
#endif
;
#undef EXTERN
