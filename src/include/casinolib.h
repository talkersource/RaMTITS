/*
   Some poker definitions.. NOTE there are two types of cards, poker cards
   and Blackjack cards... the two ARE different... although they look the
   same.
*/
#define CARD_LENGTH 6
#define CARD_WIDTH  8
static char *ascii_cards[52][CARD_LENGTH]={
	/* Clubs first: 2,3,4,5,6,7,8,9,10,J,Q,K,A */
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM K~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM A~CT|","~CT|______|"},
	/* Now spades */
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM K~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM A~CT|","~CT|______|"},
	/* Now hearts :-) */
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM K~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM A~CT|","~CT|______|"},
	/* And finally diamonds */
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM K~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM A~CT|","~CT|______|"},
	}; /* End of ascii_cards[][] */

/* The slot machine pictures. */
static char *slots_pics[13][9]={
	{"","","","","","","","",""},
	{"~CW+----------------+",
	 "~CW|~CG    777777777   ~CW|",
	 "~CW|~CG   777777777    ~CW|",
	 "~CW|~CG       7777     ~CW|",
	 "~CW|~CG      7777      ~CW|",
	 "~CW|~CG     7777       ~CW|",
	 "~CW|~CG    7777        ~CW|",
	 "~CW|~CG   7777         ~CW|",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|~CK   ~)           ~CW|",
	 "~CW|~CK     )~CR...       ~CW|",
	 "~CW|~CR   .oOOOOo.     ~CW|",
	 "~CW|~CR  .oOOOOOOo.    ~CW|",
	 "~CW|~CR  .oOOOOOOo.    ~CW|",
	 "~CW|~CR   .oOOOOo.     ~CW|",
	 "~CW|~CR      ~~        ~CW|",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|~CY     (~)=)      ~CW|",
	 "~CW|~CY    (/) )=)     ~CW|",
	 "~CW|~CY   (/ /  )=)    ~CW|",
	 "~CW|~CY   ( /|  )=)    ~CW|",
	 "~CW|~CY   ( / |/)=)    ~CW|",
	 "~CW|~CY    (  /)=)     ~CW|",
	 "~CW|~CY     (,)=)      ~CW|",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|                |",
	 "~CW|~FM  |----==----|  ~CW|",
	 "~CW|~FM  |~FT#~FRBAR~FT##~FRBAR~FT#~FM|  ~CW|",
	 "~CW|~FM  |~FT#~FRBAR~FT##~FRBAR~FT#~FM|  ~CW|",
	 "~CW|~FM  |~FT#~FRBAR~FT##~FRBAR~FT#~FM|  ~CW|",
	 "~CW|~FM  |----==----|  ~CW|",
	 "~CW|                |",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|~CY       _        ~CW|",
	 "~CW|~CY      (o'       ~CW|",
	 "~CW|~CY     (O'        ~CW|",
	 "~CW|~CY    (O;         ~CW|",
	 "~CW|~CY     (O`        ~CW|",
	 "~CW|~CY      (O`       ~CW|",
	 "~CW|~CY       (O`      ~CW|",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|                |",
	 "~CW|~FG      ...       ~CW|",
	 "~CW|~FG   .:::::::.    ~CW|",
	 "~CW|~FG <:::::::::::>  ~CW|",
	 "~CW|~FG   ':::::::'    ~CW|",
	 "~CW|~FG      '''       ~CW|",
	 "~CW|                |",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|~CT FFFFF          ~CW|",
	 "~CW|~CT FF             ~CW|",
	 "~CW|~CT FFF~CM .OO.       ~CW|",
	 "~CW|~CT F  ~CM(O  O)      ~CW|",
	 "~CW|~CM     `OO'~CR .OO.  ~CW|",
	 "~CW|~CR         (O  O) ~CW|",
	 "~CW|          ~CR`OO'  ~CW|",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|~FM      oooo      ~CW|",
	 "~CW|~FM    oooooooo    ~CW|",
	 "~CW|~FM   oooooooooo   ~CW|",
	 "~CW|~FM   oooooooooo   ~CW|",
	 "~CW|~FM    oooooooo    ~CW|",
	 "~CW|~FM      oooo      ~CW|",
	 "~CW|                |",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|                |",
	 "~CW|~FT  /~~~~~~~~~~\\  ~CW|",
	 "~CW|~FT |~FG-==========-~FT| ~CW|",
	 "~CW|~FT |~FG-==========-~FT| ~CW|",
	 "~CW|~FT |-==========-~FT|~CW |",
	 "~CW|~FT  \\__________/  ~CW|",
	 "~CW|                |",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|                |",
	 "~CW|~FM    .  ''' .    ~CW|",
	 "~CW|~FM  '     .    :  ~CW|",
	 "~CW|~FM :   .     . .  ~CW|",
	 "~CW|~FM  .    .    .   ~CW|",
	 "~CW|~FM    '  ....     ~CW|",
	 "~CW|                |",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|                |",
 	"~CW|~CG      ...       ~CW|",
	 "~CW|~CG    .ooo .      ~CW|",
	 "~CW|~CG   .oooo .      ~CW|",
	 "~CW|~CG    .ooo .      ~CW|",
	 "~CW|~CG      oo.       ~CW|",
 	"~CW|                |",
	 "~CW+----------------+"},
	{"~CW+----------------+",
	 "~CW|~CY     .oO.       ~CW|",
	 "~CW|~CY   .oooooo.     ~CW|",
	 "~CW|~CY  .oooooooo.    ~CW|",
	 "~CW|~CY  .oooooooo.    ~CW|",
	 "~CW|~CY  .oooooooo.    ~CW|",
	 "~CW|~CY   .oooooo.     ~CW|",
	 "~CW|~CY      ~~~       ~CW|",
	 "~CW+----------------+"},
	};
/* Now we have the blackjack cards... They look identical to the ones above. -
 - but as i'm using Andy's blackjack system, I've needed to make them diff.  -
 - in their own way..							     */
char *cards[53][6]={
	/* Clubs first: A,2,3,4,5,6,7,8,9,10,J,Q,K */
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM A~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CK  ()  ~CT|","~CT|~CK ()() ~CT|","~CT|~CK  ||~CM K~CT|","~CT|______|"},
	/* Now spades */
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM A~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CK  /\\  ~CT|","~CT|~CK (__) ~CT|","~CT|~CK  ||~CM K~CT|","~CT|______|"},
	/* Now hearts :-) */
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM A~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CR (\\/) ~CT|","~CT|~CR \\  / ~CT|","~CT|~CR  \\/~CM K~CT|","~CT|______|"},
	/* And finally diamonds */
	{"~CT ______ ","~CT|~CMA     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM A~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM2     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 2~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM3     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 3~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM4     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 4~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM5     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 5~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM6     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 6~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM7     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 7~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM8     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 8~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM9     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM 9~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CM10    ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM10~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMJ     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM J~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMQ     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM Q~CT|","~CT|______|"},
	{"~CT ______ ","~CT|~CMK     ~CT|","~CT|~CR  /\\  ~CT|","~CT|~CR <  > ~CT|","~CT|~CR  \\/~CM K~CT|","~CT|______|"},
	/* Lastly.. the hidden card. :) */
	{"~CT ______ ","~CT|~FGX~FRO~FGX~FRO~FGX~FRO~CT|","~CT|~FRO~FGX~FRO~FGX~FRO~FGX~CT|","~CT|~FGX~FRO~FGX~FRO~FGX~FRO~CT|","~CT|~FGO~FRX~FGO~FRX~FGO~FRX~CT|","~CT|______|"}
	};
static char *ascii_dice[6][5]={
{"~FK~BW.-------.",
 "~FK~BW|       |",
 "~FK~BW|   *   |",
 "~FK~BW|       |",
 "~FK~BW`-------'"},
{"~FK~BW.-------.",
 "~FK~BW| *     |",
 "~FK~BW|       |",
 "~FK~BW|     * |",
 "~FK~BW`-------'"},
{"~FK~BW.-------.",
 "~FK~BW| *     |",
 "~FK~BW|   *   |",
 "~FK~BW|     * |",
 "~FK~BW`-------'"},
{"~FK~BW.-------.",
 "~FK~BW| *   * |",
 "~FK~BW|       |",
 "~FK~BW| *   * |",
 "~FK~BW`-------'"},
{"~FK~BW.-------.",
 "~FK~BW| *   * |",
 "~FK~BW|   *   |",
 "~FK~BW| *   * |",
 "~FK~BW`-------'"},
{"~FK~BW.-------.",
 "~FK~BW| *   * |",
 "~FK~BW| *   * |",
 "~FK~BW| *   * |",
 "~FK~BW`-------'"}
};

static char *ansi_dice[6][5]={
{"~FK~BW�������Ŀ",
 "~FK~BW�       �",
 "~FK~BW�   *   �",
 "~FK~BW�       �",
 "~FK~BW���������"},
{"~FK~BW�������Ŀ",
 "~FK~BW� *     �",
 "~FK~BW�       �",
 "~FK~BW�     * �",
 "~FK~BW���������"},
{"~FK~BW�������Ŀ",
 "~FK~BW� *     �",
 "~FK~BW�   *   �",
 "~FK~BW�     * �",
 "~FK~BW���������"},
{"~FK~BW�������Ŀ",
 "~FK~BW� *   * �",
 "~FK~BW�       �",
 "~FK~BW� *   * �",
 "~FK~BW���������"},
{"~FK~BW�������Ŀ",
 "~FK~BW� *   * �",
 "~FK~BW�   *   �",
 "~FK~BW� *   * �",
 "~FK~BW���������"},
{"~FK~BW�������Ŀ",
 "~FK~BW� *   * �",
 "~FK~BW� *   * �",
 "~FK~BW� *   * �",
 "~FK~BW���������"}
};