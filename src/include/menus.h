#ifndef MAIN_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif

/*
   Decided to go about doing the menu's in some sort of new fangled way, as
   to remove bloat from the original code, so instead of having a *_op for
   each type of menu.. just have the structure, and do all that we need to
   do within that structure.. this *should* be easier.. I dunno yet.
*/

/*
   First the .set structure.. in the first column, there is the item, such
   as '.set desc blah' where desc is the item in the first column.. the
   second column is the set_op, in which we pass on to the user->set_op
   flag.. then execute the .set command, and do what we need to do.. pretty
   basic
*/
EXTERN struct {
	char	*sword;
	short	set_op;
	} setting[]
#ifdef MAIN_SOURCE
	={
	{"desc",	1}, {"recap",		2}, {"pdesc",		3},
	{"lalias",	4}, {"email",		5}, {"www",		6},
	{"bday",	7}, {"age",		8}, {"icq",		9},
	{"inphr",	10},{"outphr",		11},{"loginmsg",	12},
	{"logoutmsg",	13},{"gender",		14},{"colour",		15},
	{"ansi",	16},{"who",		17},{"help",		18},
	{"hemail",	19},{"columns",		20},{"charecho",	21},
	{"prompt",	22},{"mode",		23},{"editor",		24},
	{"rmhide",	25},{"sstyle",		26},{"wspace",		27},
	{"tzone",	28},{"quit",		29},{"profile",		30},
	{"autoexec",	31},{"menus",		32},{"carriage",	33},
	{"bfafk",	34},{"icqhide",		35},{"rows",		36},
	/* Stopping clause */
	{"*",			-1}
	}
#endif
;

EXTERN struct {
	char	*bword;
	short 	bank_op;
	} banking[]
#ifdef MAIN_SOURCE
	={
	{"dmoney",	1},{"wmoney",	2},{"dchips",	3},
	{"wchips",	4},{"transfer", 5},
	/* Stopping clause */
	{"*",		-1}
	}
#endif
;

/*
   Ok.. the store structure will be a little different, because well, it
   will have two short's 1 is for the buying, and one is for selling..
   it's done this way again, for ease..
*/
EXTERN struct {
	char	*stword;
	short	buy_op,		sell_op;
	} storestuff[]
#ifdef MAIN_SOURCE
	={
	{"room",	1,	2 },{"atmos",	3,	-2},
	{"carriage",	4,	5 },{"hat",	6,	7 },
	{"condoms",	8,	-2},{"chips",	9,	10},
	{"bguard",	11,	12},{"bullets",	13,	-2},
	{"nukes",	14,	15},{"pet",	16,	17},
	{"key",		18,	-2},{"beer",	19,	-2},
	/* Stopping clause */
	{"*",		-1,	-1}
	}
#endif
;

#undef EXTERN
