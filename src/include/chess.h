char letters[9]="ABCDEFGH";
char pieces[14][10]=
{
	"  ",
	"Pawn", "Knight", "Bishop", "Rook", "Queen", "King",
	"Pawn", "Knight", "Bishop", "Rook", "Queen", "King",
	""
};
char shorter[14][5]=
{
	"..",
	"WP", "WK", "WB", "WR", "WQ", " W",
	"BP", "BK", "BB", "BR", "BQ", " B",
	""
};
char smalls[2][14][20]=
{
	{
		"   ",  " o ",  "`%=",  " /)",  "nun",
		"(Q)",  "(K)",  " o ",  "`%=",  " /)",
		"nun",  "(Q)",  "(K)",  ""
	},
	{	"   ",  "(_)",  "]\\ ", "(_)",  "(_)",
		"(_)",  "(_)",  "(_)",  "]\\ ", "(_)",
		"(_)",  "(_)",  "(_)",  ""
	}
};

#define WhitePawn	1
#define WhiteKnight	2
#define WhiteBishop	3
#define WhiteRook	4
#define WhiteQueen	5
#define WhiteKing	6
#define BlackPawn	7
#define BlackKnight	8
#define BlackBishop	9
#define BlackRook	10
#define BlackQueen	11
#define BlackKing	12
