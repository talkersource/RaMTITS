/*
  chess.c
    Source code for the chess game.					-
 									-
    The original author of this is Ben Kirchmeyer. Modifications, and
    updates to the original were made by phypor:
      (phypor@benland.muc.edu).
    Modifications on that have been made by Rob Melhuish:
      (squirt@ramtits.ca) and Dirk Lemstra (dirk@lemstra.com)

    If you find a bug, please submit it to 'squirt@ramtits.ca' along
    with any patches that you might have.  For more information on the
    licensing agreement, please see the 'LICENSE' file, found in this
    or the documents directory that came with this distribution.  If
    there is no such file, chances are you have an ILLEGAL copy of the
    RaMTITS code.  If such is the case, then whoever you obtained the
    code from may have inserted 'back doors' into the code to allow
    themselves access to whatever they wish.  It would be advised to
    delete this copy and download a fresh version from:
        http://www.ramtits.ca/

    July 21, 2001   Rob Melhuish
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "include/chess.h"
#define _CHESS_C
  #include "include/needed.h"
#undef _CHESS_C

void game_summary(User u)
{

	if (u->chess == NULL)
	{
		write_user(u,"~CW-~FT You're not in a chess game, so what are you viewing the summary of?\n");
		return;
	}
	ViewBoard(u,"");
	writeus(u,"~CW - ~FTWhite~CB:\t~RS%s\n",u->chess->white);
	writeus(u,"~CW - ~FGBlack~CB:\t~RS%s\n",u->chess->black);
	if (u->chess->last_move[0] != '\0')
	{
		if (u->chess->last_moved[0] != '\0') writeus(u,"~CW - ~FTLast Move~CB:\t~RS%s with %s",u->chess->last_move,u->chess->last_moved);
		else writeus(u,"~CW - ~FTLast Move~CB:\t~RS%s with %s",u->chess->last_move);
	}
	if (u->chess->last_taken[0] != '\0') writeus(u,", taking %s",u->chess->last_taken);
	write_user(u,"\n");
}

void tell_chess(Chess chess,char *str)
{
	User white,black;

	white = get_user(chess->white);
	if (white == NULL) return;
	black = get_user(chess->black);
	if (black == NULL) return;
	write_user(black,str);
	write_user(white,str);
}

void init_board(User u,Chess chess)
{
	int i,j;
	int y,z;

	for (i = 0 ; i<8 ; i++)
	  for (j = 0 ; j<8 ; j++)
	    chess->pos[i][j] = 0;
	/* init black pieces. */
	chess->pos[0][0] = chess->pos[0][7] = BlackRook;
	chess->pos[0][1] = chess->pos[0][6] = BlackKnight;
	chess->pos[0][2] = chess->pos[0][5] = BlackBishop;
	chess->pos[0][3] = BlackQueen;
	chess->pos[0][4] = BlackKing;
	for (i = 0 ; i < 8 ; i++)
	{
		chess->pos[1][i] = BlackPawn;
	}
	/* init white pieces */
	chess->pos[7][0] = chess->pos[7][7] = WhiteRook;
	chess->pos[7][1] = chess->pos[7][6] = WhiteKnight;
	chess->pos[7][2] = chess->pos[7][5] = WhiteBishop;
	chess->pos[7][3] = WhiteQueen;
	chess->pos[7][4] = WhiteKing;
	for (i = 0 ; i < 8 ; i++)
	{
		chess->pos[6][i] = WhitePawn;
	}
	chess->turn = u;
	for (y = 0 ; y < 8 ; y++)
	{
		for (z = 0 ; z < 8 ; z++)
		{
			chess->lastpos[y][z] = chess->pos[y][z];
		}
	}
}

void invalid(Chess chess,int fromchar,int fromint,int tochar,int toint)
{

	if (chess->new_turn == 0) return;
	fromint -= 7;
	fromint = abs(fromint);
	toint -= 7;
	toint = abs(toint);
	sprintf(text,"~CB[~FMChess~CB] - ~FGInvalid move attempted~CB: ~RS%c%d to %c%d\n",letters[fromchar],(fromint+1),letters[tochar],(toint+1));
	tell_chess(chess,text);
}

int move_white_pawn(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int value;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	value = fromint - toint;
	if (value <= 0)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (value > 2)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (value == 2)
	{
		if ((fromint != 6) || (chess->pos[toint][tochar] != 0) ||
		  (chess->pos[(toint+1)][tochar] != 0) || (fromchar != tochar))
		{
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		}
	}
	if (value == 1)
	{
		if (tochar > fromchar) value = tochar - fromchar;
		else value = fromchar - tochar;
		if (value == 0)			/* straight move */
		{
			if (chess->pos[toint][tochar] != 0) /* pos filled */
			{
				invalid(chess,fromchar,fromint,tochar,toint);
				return(1);
			}
		}
		else	/* diagonal.. check to see if we're taking a piece. */
		{
			if ((value > 1) || (chess->pos[toint][tochar] < 7))
			{
				invalid(chess,fromchar,fromint,tochar,toint);
				return(1);
			}
		}
	}
	if (toint == 0) chess->pos[fromint][fromchar] = WhiteQueen;
	return(0); /* 'twas a valid move. */
}

int move_black_pawn(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int value;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	value = toint - fromint;
	if (value <= 0)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (value > 2)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (value == 2)
	{
		if ((fromint != 1) || (chess->pos[toint][tochar] != 0) ||
		  (chess->pos[(toint-1)][tochar] != 0) || (fromchar != tochar))
		{
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		}
	}
	if (value == 1)
	{
		value = abs(tochar-fromchar); /* Difference between the two. */
		if (value == 0)	/* straight move */
		{
			if (chess->pos[toint][tochar] != 0) /* pos filled */
			{
				invalid(chess,fromchar,fromint,tochar,toint);
				return(1);
			}
		}
		else /* diagonal.. check to see if we're taking a piece. */
		{
			if ((value > 1) || (chess->pos[toint][tochar] == 0)
			  || (chess->pos[toint][tochar] > 6))
			{
				invalid(chess,fromchar,fromint,tochar,toint);
				return(1);
			}
		}
	}
	if (toint == 7) chess->pos[fromint][fromchar] = BlackQueen;
	return(0); /* 'twas a valid move. */
}

int move_white_knight(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int result1,result2;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((chess->pos[toint][tochar] < 7) && (chess->pos[toint][tochar] != 0))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	result1 = abs(toint-fromint);
	result2 = abs(tochar-fromchar);
	if (result1 == 2)
	  if (result2 == 1) return(0);
	if (result2 == 2)
	  if (result1 == 1) return(0);
	invalid(chess,fromchar,fromint,tochar,toint);
	return(1);
}

int move_black_knight(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int result1,result2;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (chess->pos[toint][tochar] > 6)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	result1 = abs(toint-fromint);
	result2 = abs(tochar-fromchar);
	if (result1 == 2)
	  if (result2 == 1) return(0);
	if (result2 == 2)
	  if (result1 == 1) return(0);
	invalid(chess,fromchar,fromint,tochar,toint);
	return(1);
}

int move_white_rook(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int value;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (fromchar == tochar)
	{
		if (fromint<toint)
		{
			for(value = (fromint+1) ; value <= toint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && (value != toint))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = toint ; value < fromint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && (value != toint))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else if (fromint == toint)
	{
		if (fromchar < tochar)
		{
			for (value = (fromchar+1) ; value <= tochar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && (value != tochar))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = tochar ; value < fromchar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && (value != tochar))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((chess->pos[toint][tochar] != 0)
	  && (chess->pos[toint][tochar] < 7))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	return(0);
}

int move_black_rook(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int value;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (fromchar == tochar)
	{
		if (fromint < toint)
		{
			for(value = (fromint+1) ; value <= toint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && (value != toint))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = toint ; value < fromint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && (value != toint))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else if (fromint == toint)
	{
		if (fromchar < tochar)
		{
			for(value = (fromchar+1) ; value <= tochar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && (value != tochar))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = tochar ; value < fromchar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && (value != tochar))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (chess->pos[toint][tochar] > 6)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	return(0);
}

int move_white_bishop(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int i = 0,j = 0,result1,result2;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (toint > fromint)
	{
		result1 = toint-fromint;
		i = 1;
	}
	else result1 = fromint-toint;
	if (tochar>fromchar)
	{
		result2 = tochar-fromchar;
		j = 1;
	}
	else result2 = fromchar-tochar;

	if (result1 != result2)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((i == 1) && (j == 1))
	{
		for (i = (fromint+1),j = (fromchar+1) ; i<toint ; i++,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 1) && (j == 0))
	{
		for (i = (fromint+1),j = (fromchar-1) ; i<toint ; i++,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((j == 1) && (i == 0))
	{
		for (i = (fromint-1),j = (fromchar+1) ; j<tochar ; i--,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 0) && (j == 0))
	{
		for (i=(fromint-1),j=(fromchar-1);i>toint;i--,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	if ((chess->pos[toint][tochar]!=0) && (chess->pos[toint][tochar]<7))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	return(0);
}

int move_black_bishop(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int i = 0,j = 0,result1,result2;

	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (toint>fromint)
	{
		result1 = toint-fromint;
		i = 1;
	}
	else result1 = fromint-toint;
	if (tochar>fromchar)
	{
		result2 = tochar-fromchar;
		j = 1;
	}
	else result2 = fromchar-tochar;

	if (result1 != result2)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((i == 1) && (j == 1))
	{
		for (i = (fromint+1),j = (fromchar+1) ; i<toint ; i++,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 1) && (j == 0))
	{
		for (i = (fromint+1),j = (fromchar-1) ; i<toint ; i++,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((j == 1) && (i == 0))
	{
		for (i = (fromint-1),j = (fromchar+1) ; j<tochar ; i--,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 0) && (j == 0))
	{
		for (i = (fromint-1),j = (fromchar-1) ; i>toint ; i--,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	if (chess->pos[toint][tochar]>6)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	return(0);
}

int move_white_queen(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int result1,result2,value,i,j;

	i = j = -1;  result1 = result2 = 0;
	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (fromchar == tochar)
	{
		if (fromint < toint)
		{
			for (value = (fromint+1) ; value <= toint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && ((chess->pos[value][fromchar] < 7)
				  && (value != toint)))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = toint ; value < fromint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && ((chess->pos[value][fromchar] < 7)
				  && (value != toint)))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else if (fromint == toint)
	{
		if (fromchar < tochar)
		{
			for (value = (fromchar+1) ; value <= tochar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && ((chess->pos[fromint][value] < 7)
				  && (value != tochar)))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = tochar ; value < fromchar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && ((chess->pos[fromint][value] < 7)
				  && (value != tochar)))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else
	{
		if (toint > fromint)
		{
			result1 = toint-fromint;
			i = 1;
		}
		else
		{
			result1 = fromint-toint;
			i = 0;
		}
		if (tochar > fromchar)
		{
			result2 = tochar-fromchar;
			j = 1;
		}
		else
		{
			result2 = fromchar-tochar;
			j = 0;
		}
	}
	if (result1 != result2)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((i == 1) && (j == 1))
	{
		for (i = (fromint+1),j = (fromchar+1) ; i<toint ; i++,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 1) && (j == 0))
	{
		for (i = (fromint+1),j = (fromchar-1) ; i<toint ; i++,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((j == 1) && (i == 0))
	{
		for (i = (fromint-1),j = (fromchar+1) ; j<tochar ; i--,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 0) && (j == 0))
	{
        	for (i = (fromint-1),j = (fromchar-1) ; i>toint ; i--,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	if ((chess->pos[toint][tochar] != 0)
	  && (chess->pos[toint][tochar] < 7))
	{
	        invalid(chess,fromchar,fromint,tochar,toint);
	        return(1);
        }
	return(0);
}

int move_black_queen(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int result1,result2,value,i,j;

	i = j = result1 = result2 = 0;
       	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (fromchar == tochar)
	{
		if (fromint<toint)
		{
			for (value = (fromint+1) ; value <= toint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && (value != toint))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = toint ; value<fromint ; value++)
			{
				if ((chess->pos[value][fromchar] != 0)
				  && (value != toint))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else if (fromint == toint)
	{
		if (fromchar<tochar)
		{
			for (value = (fromchar+1) ; value <= tochar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && (value!=tochar))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
		else
		{
			for (value = tochar ; value<fromchar ; value++)
			{
				if ((chess->pos[fromint][value] != 0)
				  && (value != tochar))
				{
					invalid(chess,fromchar,fromint,tochar,toint);
					return(1);
				}
			}
		}
	}
	else
	{
		if (toint>fromint)
		{
			result1 = toint-fromint;
			i = 1;
		}
		else result1 = fromint-toint;
		if (tochar>fromchar)
		{
			result2 = tochar-fromchar;
			j = 1;
		}
		else result2 = fromchar-tochar;
	}
	if (result1 != result2)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((i == 1) && (j == 1))
	{
		for (i = (fromint+1),j = (fromchar+1) ; i<toint ; i++,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 1) && (j == 0))
	{
		for (i = (fromint+1),j = (fromchar-1) ; i<toint ; i++,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((j == 1) && (i == 0))
	{
		for (i = (fromint-1),j = (fromchar+1) ; j<tochar ; i--,j++)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	else if ((i == 0) && (j == 0))
	{
		for (i = (fromint-1),j = (fromchar-1) ; i>toint ; i--,j--)
		  if (chess->pos[i][j] != 0)
		  {
			invalid(chess,fromchar,fromint,tochar,toint);
			return(1);
		  }
	}
	if ((chess->pos[toint][tochar] != 0) && (chess->pos[toint][tochar] > 6))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	return(0);
}

int move_white_king(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int result1,result2;
	int i,j,k,g;

	i = g = j = k = 0;
	if ((fromchar == tochar) && (fromint == toint))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if ((fromint == 7) && (fromchar == 4))
	{
		if (toint == 7)
		{
			if (tochar == 2)
			{
				if (chess->pos[7][0] == WhiteRook
				  && chess->pos[7][1] == 0
				  && chess->pos[7][2] == 0
				  && chess->pos[7][3] == 0
				  && (chess->wroc != 1 || chess->wroc !=2))
				{
					for (k = 1 ; k < 4 ; k++)
					{
						for (i = 0 ; i < 8 ; i++)
						{
							for (j = 0 ; j < 8 ; j++)
							{
								g = 1;
								chess->new_turn = 0;
								switch (chess->pos[i][j])
								{
									case BlackPawn	:
									  g = move_black_pawn(chess,j,i,k,7);
									  break;
									case BlackRook	:
									  g = move_black_rook(chess,j,i,k,7);
									  break;
									case BlackKnight:
									  g = move_black_knight(chess,j,i,k,7);
									  break;
									case BlackBishop:
									  g = move_black_bishop(chess,j,i,k,7);
									  break;
									case BlackQueen	:
									  g = move_black_queen(chess,j,i,k,7);
									  break;
									default		:
									  break;
								}
								chess->new_turn = 1;
								if (g == 0)
								{
									invalid(chess,fromchar,fromint,tochar,toint);
									return(1);
								}
							}
						}
					}
					chess->pos[7][0] = 0;
					chess->pos[7][3] = WhiteRook;
					return(0);
				}
			}
			else if (tochar == 6)
			{
				if (chess->pos[7][7] == WhiteRook
				  && chess->pos[7][6] == 0
				  && chess->pos[7][5] == 0
				  && (chess->wroc != 1 || chess->wroc != 3))
				{
					for (k = 5 ; k < 7 ; k++)
					{
						for (i = 0 ; i < 8 ; i++)
						{
							for (j = 0 ; j < 8 ; j++)
							{
								g = 1;
								chess->new_turn = 0;
								switch (chess->pos[i][j])
								{
									case BlackPawn	:
									  g = move_black_pawn(chess,j,i,k,7);
									  break;
									case BlackRook	:
									  g = move_black_rook(chess,j,i,k,7);
									  break;
									case BlackKnight:
									  g = move_black_knight(chess,j,i,k,7);
									  break;
									case BlackBishop:
									  g = move_black_bishop(chess,j,i,k,7);
									  break;
									case BlackQueen	:
									  g = move_black_queen(chess,j,i,k,7);
									  break;
									default		:
									  break;
								}
								chess->new_turn = 1;
								if (g == 0)
								{
									invalid(chess,fromchar,fromint,tochar,toint);
									return(1);
								}
							}
						}
					}
					chess->pos[7][7] = 0;
					chess->pos[7][5] = WhiteRook;
					return(0);
				}
			}
		}
	}
	result1 = abs(toint-fromint);
	result2 = abs(tochar-fromchar);
	if ((result1>1) || (result2>1) || ((result1 == 0) && (result2 == 0)))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (chess->pos[toint][tochar] != 0)
	  if (chess->pos[toint][tochar]<7)
	  {
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	  }
	return(0);
}

int move_black_king(Chess chess,int fromchar,int fromint,int tochar,int toint)
{
	int result1,result2;
	int i,j,k,g;

	i = g = j = k = 0;
	if ((fromint == 0) && (fromchar == 4))
	{
		if (toint == 0)
		{
			if (tochar == 2)
			{
				if (chess->pos[0][0] == BlackRook
				  && chess->pos[0][1] == 0
				  && chess->pos[0][2] == 0
				  && chess->pos[0][3] == 0
				  && (chess->broc != 1 || chess->broc != 2))
				{
					for (k = 1 ; k < 4 ; k++)
					{
						for (i = 0 ; i < 8 ; i++)
						{
							for (j = 0 ; j < 8 ; j++)
							{
								g = 1;
								chess->new_turn = 0;
								switch (chess->pos[i][j])
								{
									case WhitePawn	:
									  g = move_white_pawn(chess,j,i,k,7);
									  break;
									case WhiteRook	:
									  g = move_white_rook(chess,j,i,k,7);
									  break;
									case WhiteKnight:
									  g = move_white_knight(chess,j,i,k,7);
									  break;
									case WhiteBishop:
									  g = move_white_bishop(chess,j,i,k,7);
									  break;
									case WhiteQueen	:
									  g = move_white_queen(chess,j,i,k,7);
									  break;
									default		:
									  break;
								}
								chess->new_turn = 1;
								if (g == 0)
								{
									invalid(chess,fromchar,fromint,tochar,toint);
									return(1);
								}
							}
						}
					}
					chess->pos[0][0] = 0;
					chess->pos[0][3] = BlackRook;
					return(0);
				}
			}
			else if (tochar == 6)
			{
				if (chess->pos[0][7] == BlackRook
				  && chess->pos[0][6] == 0
				  && chess->pos[0][5] == 0
				  && (chess->broc != 1 || chess->broc != 3))
				{
					for (k = 5 ; k < 7 ; k++)
					{
						for (i = 0 ; i < 8 ; i++)
						{
							for (j = 0 ; j < 8 ; j++)
							{
								g = 1;
								chess->new_turn = 0;
								switch (chess->pos[i][j])
								{
									case WhitePawn	:
									  g = move_white_pawn(chess,j,i,k,7);
									  break;
									case WhiteRook	:
									  g = move_white_rook(chess,j,i,k,7);
									  break;
									case WhiteKnight:
									  g = move_white_knight(chess,j,i,k,7);
									  break;
									case WhiteBishop:
									  g = move_white_bishop(chess,j,i,k,7);
									  break;
									case WhiteQueen	:
									  g = move_white_queen(chess,j,i,k,7);
									  break;
									default		:
									  break;
								}
								chess->new_turn = 1;
								if (g == 0)
								{
									invalid(chess,fromchar,fromint,tochar,toint);
									return(1);
								}
							}
						}
					}
					chess->pos[0][7] = 0;
					chess->pos[0][5] = BlackRook;
					return(0);
				}
			}
		}
	}
	result1 = abs(toint-fromint);
	result2 = abs(tochar-fromchar);
	if ((result1>1) || (result2>1) || ((result1 == 0) && (result2 == 0)))
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	if (chess->pos[toint][tochar]>6)
	{
		invalid(chess,fromchar,fromint,tochar,toint);
		return(1);
	}
	return(0);
}

void ViewBoard(User user,char *str)
{
	Chess chess;
	User u;
	char temp[81];
	int i,j,k,row,rowm,pos = 0,alternating = 0;

	if (*str)
	{
		u = get_user(str);
		if (u == NULL) return;
	}
	else u = user;
	chess = u->chess;
	if (chess == NULL)
	{
		if (u == user) write_user(user,"~CW-~FT You haven't started a chess game yet.\n");
		else writeus(user,"~CW-~FM %s~FT doesn't appear to be playing a game of chess.\n",u->recap);
		return;
	}
	text[0] = '\0';
	if (u == user && !strcasecmp(u->name,u->chess->black))
	{
		rowm = 1;
		row = 1;
	}
	else
	{
		rowm = -1;
		row = 8;
	}
	for (i = 0 ; i<8 ; i++)
	{
		for (k = 0 ; k<2 ; k++) 
		{
			for (j = 0 ; j < 8 ; j++)
			{
				if (j == 0)
				{
					sprintf(temp,"%d",row);
					strcat(text,temp);
				}
				if (rowm == -1) pos = chess->pos[i][j];
				else pos = chess->pos[7-i][j];
				if (k == 1)
				{
					if (pos == 0) strcat(text,"|_____");
					else
					{
						if (pos < 7)
						{
							sprintf(temp,"|_~CW%s~RS_",smalls[k][pos]);
							strcat(text,temp);
						}
						else
						{
							sprintf(temp,"|_~FM%s~RS_",smalls[k][pos]);
							strcat(text,temp);
						}
					}
				}
				else
				{
					if ((pos == 0) && (alternating == 1)) strcat(text,"| ... ");
					else if (pos == 0) strcat(text,"|     ");
					else
					{
						if (pos < 7)
						{
							sprintf(temp,"| ~CW%s~RS ",smalls[k][pos]);
							strcat(text,temp);
						}
						else
						{
							sprintf(temp,"| ~FM%s~RS ",smalls[k][pos]);
							strcat(text,temp);
						}
					}
				}
				if (alternating == 0) alternating = 1;
				else alternating = 0;
			}
			strcat(text,"|\n");
		}
		if (alternating == 0) alternating = 1;
		else alternating = 0;
		row += rowm;
	}
	strcat(text,"    A     B     C     D     E     F     G     H\n");
	write_user(user,text);
	return;
}

/* the actual .chess command. */
void chess_com(User user,char *str,int rt)
{
	Chess chess;
	User u = NULL;


	if (user == NULL) return;
	if (user->chess != NULL)
	{
		if (!strncasecmp(word[1],"-quit",2))
		{
			sprintf(text,"~CW- ~FGThe chess game is over. ~FT%s gave up.\n",user->recap);
			tell_chess(user->chess,text);
			kill_chess(user);
			return;
		}
		else if (!strncasecmp(word[1],"-board",2))
		{
			ViewBoard(user,"");
			u = user->chess->turn;
			if (u == user) write_user(user,"~CW- ~FTIt is currently your turn.\n");
			else
			{
				if (u == NULL)
				{
					write_user(user,"~CW- ~FT It looks like your opponent left.. gotta reset the game.\n");
					kill_chess(user);
					return;
				}
				else
				{
					writeus(user,"~CW-~FT It is currently~FG %s%s ~FTturn.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
					return;
				}
			}
			return;
		}
		else if (!strncasecmp(word[1],"-gs",2))
		{
			game_summary(user);
			return;
		}
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CR-gs~CW|~CR-board~CW|~CR-quit~CB]");
			return;
		}
	}
	else if (user->chess_opp != NULL)
	{
		u = user->chess_opp;
		if (u == NULL)
		{
			write_user(user,"~CW- ~FTIt looks like the person who challenged you left.\n");
			user->chess_opp = NULL;
			return;
		}
		if (!strncasecmp(word[1],"-accept",2))
		{
			if (u->chess != NULL)
			{
				write_user(user,"~CW-~FT It looks like that user found someone else to play with.\n");
				user->chess_opp = NULL;
				return;
			}
			chess = (Chess)(malloc(sizeof(struct chess_struct)));
			if (chess == NULL)
			{
				write_user(user,"~CW-~FT An error occurred... try again later.\n");
				write_log(0,LOG1,"[ERROR] - Unable to create a chess board in chess_com().\n");
				user->chess_opp = NULL;
				u->chess_opp    = NULL;
				return;
			}
			writeus(user,"~CW-~FT You accept ~FG%s%s~FT game of chess.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
			writeus(u,"~CW-~FG %s~FT accepts your game of chess.\n",user->recap);
			user->chess_opp	= NULL;
			u->chess_opp	= NULL;
			memset(chess,0,sizeof(struct chess_struct));
			user->chess	= chess;
			u->chess	= chess;
			strcpy(chess->white,user->name);
			strcpy(chess->black,u->name);
			chess->turn	= user;
			sprintf(text,"~CW-~FT Chess game is starting, ~FM%s~FT is white~FM %s~FT is black.\n",user->recap,u->recap);
			write_user(u,text);
			write_user(user,text);
			init_board(user,chess);
			ViewBoard(user,"");
			ViewBoard(u,"");
			return;
		}
		else if (!strncasecmp(word[1],"-reject",2))
		{
			user->chess_opp = NULL;
			u->chess_opp = NULL;
			writeus(user,"~CW-~FT You reject ~FG%s%s ~FTgame of chess.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
			writeus(u,"~CW-~FG %s~FT rejects your game of chess.\n",user->recap);
			return;
		}
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CR-accept~CW|~CR-reject~CB]");
			return;
		}
	}
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~CW-~FT Do you like to play with yourself?\n");
		return;
	}
	if (is_enemy(u,user))
	{
		writeus(user,"~CW-~FG %s~FT doesn't want anything to do with you.\n",u->recap);
		return;
	}
	if (is_enemy(user,u))
	{
		write_user(user,"~CW-~FT You can't play games with your enemies.\n");
		return;
	}
	if (is_busy(u))
	{
		writeus(user,"~CW-~FT %s~FM is currently busy right now..\n",u->recap);
		return;
	}
	if (u->chess != NULL)
	{
		writeus(user,"~CW-~FT %s~FM is already playing a game of chess.\n",u->recap);
		return;
	}
	if (u->level<command[com_num].lev
	  && !is_allowed(u,command[com_num].name))
	{
		writeus(user,"~CW-~FT %s~FM doesn't have access to that command.\n",u->recap);
		return;
	}
	u->chess_opp = user;
	writeus(u,"~CW-~FM %s~FT would like to play a game of chess with you.\n"
		  "~CW-~FT To accept the game, type ~CW'~CR.chess -accept~CW'\n"
		  "~CW-~FT To reject the game, type ~CW'~CR.chess -reject~CW'\n",user->recap);
	writeus(user,"~CW-~FT You ask ~FM%s~FT to play a game of chess with you.\n",u->recap);
	return;
}

void move_pieces(User user,char *inpstr,int rt)
{
	User u = NULL;
	int legal = 0,y = 0,z = 0;
	int white = 0,black = 0;
	int frompiece,topiece,fromchar,fromint,tochar,toint;
	char from[3],to[3];

	from[0] = to[0] = '\0';
	if (user->chess == NULL)
	{
		write_user(user,"~CW-~FT You haven't even started a chess game.. try ~CW'~CR.h chess~CW'\n");
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRpiece from~CB] [~CRpiece to~CB]");
		return;
	}
	if (user->chess->turn != user)
	{
		write_user(user,"~CW-~FT It's not your turn.\n");
		return;
	}
	if ((strlen(word[1]) != 2) || (strlen(word[2]) != 2))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRpiece from~CB] [~CRpiece to~CB]");
		return;
	}
	strcpy(from,word[1]);		strcpy(to,word[2]);
	fromchar	= from[0];	fromchar 	-= 97;
	fromint		= from[1];	fromint 	-= 49;
	tochar		= to[0];	tochar 		-= 97;
	toint		= to[1];	toint 		-= 49;

	fromint		-= 7;		fromint		= abs(fromint);
	toint		-= 7;		toint		= abs(toint);
	if ((fromchar > 7) || (fromchar < 0) || (fromint > 7) || (fromint < 0)
	  || (tochar > 7) || (tochar < 0) || (toint > 7) || (toint < 0))
	{
		write_user(user,"~CW-~FT That move doesn't fit on an 8x8 board. See ~CW'~CR.h chess~CW'\n");
		return;
	}
	topiece		= user->chess->pos[toint][tochar];
	frompiece	= user->chess->pos[fromint][fromchar];
	if (frompiece == 0)
	{
		writeus(user,"~CW- ~FTThere is no piece at ~FY%s\n",from);
		return;
	}
	if (!strcasecmp(user->name,user->chess->white)) white = 1;
	else black = 1;
	if (white == 1)
	{
		if (frompiece > WhiteKing)
		{
			write_user(user,"~CW-~FT You can't move your opponent's pieces.\n");
			return;
		}
	}
	else
	{
		if (frompiece < BlackPawn)
		{
			write_user(user,"~CW-~FT You can't move your opponent's pieces.\n");
			return;
		}
	}
	user->chess->new_turn = 1;
	switch (frompiece)
	{
		case WhitePawn:
		  legal = move_white_pawn(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case BlackPawn:
		  legal = move_black_pawn(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case WhiteKnight:
		  legal = move_white_knight(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case BlackKnight:
		  legal = move_black_knight(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case WhiteBishop:
		  legal = move_white_bishop(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case BlackBishop:
		  legal = move_black_bishop(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case WhiteRook:
		  legal = move_white_rook(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case BlackRook:
		  legal = move_black_rook(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case WhiteQueen:
		  legal = move_white_queen(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case BlackQueen:
		  legal = move_black_queen(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case WhiteKing:
		  legal = move_white_king(user->chess,fromchar,fromint,tochar,toint);
		  break;
		case BlackKing:
		  legal = move_black_king(user->chess,fromchar,fromint,tochar,toint);
		  break;
	}
	if (legal == 1)
	{
		write_user(user,"~CW-~FT You attempted an illegal move, try again.\n");
		return;
	}
	user->chess->new_turn			= 0;
	user->chess->pos[fromint][fromchar]	= 0;
	user->chess->pos[toint][tochar]		= frompiece;
	if (white == 1)
	{
		if (white_in_check(user->chess) != 0)
		{
			write_user(user,"~CW-~FT You are in check, illegal move.\n");
			user->chess->pos[fromint][fromchar]	= frompiece;
			user->chess->pos[toint][tochar]		= topiece;
			return;
		}
	}
	else
	{
		if (black_in_check(user->chess) != 0)
		{
			write_user(user,"~CW-~FT You are in check, illegal move.\n");
			user->chess->pos[fromint][fromchar]	= frompiece;
			user->chess->pos[toint][tochar]		= topiece;
			return;
		}
	}
	strcpy(user->chess->last_moved,pieces[frompiece]);
	if (topiece>0) strcpy(user->chess->last_taken,pieces[topiece]);
	else user->chess->last_taken[0] = '\0';
	if (white == 1)
	{
		u = get_user(user->chess->black);
		if (u == NULL) return;
	}
	else
	{
		u = get_user(user->chess->white);
		if (u == NULL) return;
	}
	sprintf(text,"%s to %s",from,to);
	strcpy(user->chess->last_move,text);
	user->chess->turn = u;
	user->chess->last_turn = user;
	ViewBoard(user,"");
	ViewBoard(u,"");
	if (white == 1)
	{
		if (frompiece == WhiteKing)
		{
			user->chess->wroc = 1;
		}
		else if (frompiece == WhiteRook)
		{
			if (fromchar == 0)
			{
				if (user->chess->wroc != 1)
				{
					if (user->chess->wroc == 3)
					{
						user->chess->wroc = 1;
					}
					else
					{
						user->chess->wroc = 2;
					}
				}
			}
			if (fromchar == 7)
			{
				if (user->chess->wroc != 1)
				{
					if (user->chess->wroc == 2)
					{
						user->chess->wroc = 1;
					}
					else
					{
						user->chess->wroc = 3;
					}
				}
			}
		}
	}
	else
	{
		if (frompiece == BlackKing)
		{
			user->chess->broc = 1;
		}
		else if (frompiece == BlackRook)
		{
			if (fromchar == 0)
			{
				if (user->chess->broc != 1)
				{
					if (user->chess->broc == 3)
					{
						user->chess->broc = 1;
					}
					else
					{
						user->chess->broc = 2;
					}
				}
			}
			if (fromchar == 7)
			{
				if (user->chess->broc != 1)
				{
					if (user->chess->broc == 2)
					{
						user->chess->broc = 1;
					}
					else
					{
						user->chess->broc = 3;
					}
				}
			}
		}
	}
	if (white == 1)
	{
		if (black_in_check(user->chess) != 0)
		{
			if (black_is_checkmate(user->chess) == 1)
			{
				write_user(user,"~CW - ~FTCheck Mate.. You have won.\n");
				user->chess_wins++;
				if (u != NULL)
				{
					write_user(u,"~CW - ~FTCheck Mate.. You've lost the game of chess.\n");
					u->chess_lose++;
				}
				kill_chess(user);
				return;
			}
		}
	}
	else
	{
		if (white_in_check(user->chess) != 0)
		{
			if (white_is_checkmate(user->chess) == 1)
			{
				write_user(user,"~CW - ~FTCheck Mate.. You have won.\n");
				user->chess_wins++;
				if (u != NULL)
				{
					write_user(u,"~CW - ~FTCheck Mate.. You've lost the game of chess.\n");
					u->chess_lose++;
				}
				kill_chess(user);
				return;
			}
		}
	}
	if (white == 1)
	{
		if (black_stalemate(user->chess) == 0)
		{
			if (u != NULL) u->chess_draws++;
			user->chess_draws++;
			sprintf(text,"~CW-~FT Awww.. this game has ended in a stalemate.\n");
			tell_chess(user->chess,text);
			kill_chess(user);
			return;
		}
	}
	else
	{
		if (white_stalemate(user->chess) == 0)
		{
			if (u != NULL) u->chess_draws++;
			user->chess_draws++;
			sprintf(text,"~CW-~FT Awww.. this game has ended in a stalemate.\n");
			tell_chess(user->chess,text);
			kill_chess(user);
			return;
		}
	}
	if (white == 1)
	{
		if (black_in_check(user->chess) != 0)
		{
			if (u == NULL)
			{
				writeus(user,"~CW-~FT Couldn't find your opponent, aborting game.\n");
				kill_chess(user);
				return;
			}
			writeus(user,"~CW-~FT You've put ~FG%s~FT in check.\n",u->recap);
			writeus(u,"~CW-~FT You are now in check.\n");
		}
	}
	else
	{
		if (white_in_check(user->chess) != 0)
		{
			if (u == NULL)
			{
				writeus(user,"~CW-~FT Couldn't find your opponent, aborting game.\n");
				kill_chess(user);
				return;
			}
			writeus(user,"~CW-~FT You've put ~FG%s~FT in check.\n",u->recap);
			writeus(u,"~CW-~FT You are now in check.\n");
		}
	}
	/* save the last board */
	for (y = 0 ; y < 8 ; y++)
	{
		for (z = 0 ; z < 8 ; z++)
		{
			user->chess->lastpos[y][z] = user->chess->pos[y][z];
		}
	}
	writeus(user,"~CW-~FT It's now %s%s ~FTturn.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
	writeus(u,"~CW-~FT It's now your turn.\n");
	return;
}

/* Kill the chess game now. */
void kill_chess(User user)
{
	User u,u2;

	if (user->chess == NULL) return;
	u	= get_user(user->chess->black);
	u2	= get_user(user->chess->white);
	sprintf(text,"~CW-~FG The chess game has been reset.\n");
	tell_chess(user->chess,text);
	memset(user->chess,0,sizeof(struct chess_struct));
	free(user->chess);
	user->chess = NULL;
	if (u != NULL) u->chess = NULL;
	if (u2 != NULL) u2->chess = NULL;
	if ((u != NULL) && (u2 != NULL))
	{
		writeus(u,"~CW-~FT The chess game between you and ~FG%s ~FTis over.\n",u2->recap);
		writeus(u2,"~CW-~FT The chess game between you and ~FG%s ~FTis over.\n",u->recap);
	}
	return;
}

int white_in_check(Chess chess)
{
	int i,j,k,toint,tochar;

	i = j = k = toint = tochar = 0;
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (chess->pos[i][j] == WhiteKing)
			{
				toint = i;
				tochar = j;
				break;
			}
		}
	}
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			switch (chess->pos[i][j])
			{
				case BlackPawn	:
				  k = move_black_pawn(chess,j,i,tochar,toint);
				  if (k == 0) return(BlackPawn);
				  break;
				case BlackRook	:
				  k = move_black_rook(chess,j,i,tochar,toint);
				  if (k == 0) return(BlackRook);
				  break;
				case BlackKnight:
				  k = move_black_knight(chess,j,i,tochar,toint);
				  if (k == 0) return(BlackKnight);
				  break;
				case BlackBishop:
				  k = move_black_bishop(chess,j,i,tochar,toint);
				  if (k == 0) return(BlackBishop);
				  break;
				case BlackQueen	:
				  k = move_black_queen(chess,j,i,tochar,toint);
				  if (k == 0) return(BlackQueen);
				  break;
				case BlackKing	:
				  k = move_black_king(chess,j,i,tochar,toint);
				  if (k == 0) return(BlackKing);
				  break;
				default		:
				  break;
			}
		}
	}
	/* not in check */
	return(0);
}

int black_in_check(Chess chess)
{
	int i,j,k,toint,tochar;

	i = j = k = toint = tochar = 0;
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (chess->pos[i][j] == BlackKing)
			{
				toint = i;
				tochar = j;
				break;
			}
		}
	}
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			switch (chess->pos[i][j])
			{
				case WhitePawn	:
				  k = move_white_pawn(chess,j,i,tochar,toint);
				  if (k == 0) return(WhitePawn);
				  break;
				case WhiteRook	:
				  k = move_white_rook(chess,j,i,tochar,toint);
				  if (k == 0) return(WhiteRook);
				  break;
				case WhiteKnight:
				  k = move_white_knight(chess,j,i,tochar,toint);
				  if (k == 0) return(WhiteKnight);
				  break;
				case WhiteBishop:
				  k = move_white_bishop(chess,j,i,tochar,toint);
				  if (k == 0) return(WhiteBishop);
				  break;
				case WhiteQueen	:
				  k = move_white_queen(chess,j,i,tochar,toint);
				  if (k == 0) return(WhiteQueen);
				  break;
				case WhiteKing	:
				  k = move_white_king(chess,j,i,tochar,toint);
				  if (k == 0) return(WhiteKing);
				  break;
				default		:
				  break;
			}
		}
	}
	/* not in check */
	return(0);
}

int white_stalemate(Chess chess)
{
	int i,j,k,l,op,op2,g;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			for (k = 0; k < 8; k++)
			{
				for (l = 0; l < 8; l++)
				{
					g = 2;
					switch (chess->pos[i][j])
					{
						case WhitePawn	:
						  g = move_white_pawn(chess,j,i,l,k);
						  break;
						case WhiteRook	:
						  g = move_white_rook(chess,j,i,l,k);
						  break;
						case WhiteKnight:
						  g = move_white_knight(chess,j,i,l,k);
						  break;
						case WhiteBishop:
						  g = move_white_bishop(chess,j,i,l,k);
						  break;
						case WhiteQueen	:
						  g = move_white_queen(chess,j,i,l,k);
						  break;
						case WhiteKing	:
						  g = move_white_king(chess,j,i,l,k);
						  break;
						default		:
						  break;
					}
					if (g == 2)
					{
						continue;
					}
					if (g == 0)
					{
						op = chess->pos[i][j];
						op2 = chess->pos[k][l];
						chess->pos[i][j] = 0;
						chess->pos[k][l] = op;
						if (white_in_check(chess) == 0)
						{
							chess->pos[i][j] = op;
							chess->pos[k][l] = op2;
							return(1);
						}
						else
						{
							chess->pos[i][j] = op;
							chess->pos[k][l] = op2;
						}
					}
				}
			}
		}
	}
	return(0);
}

int black_stalemate(Chess chess)
{
	int i,j,k,l,op,op2,g;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			for (k = 0; k < 8; k++)
			{
				for (l = 0; l < 8; l++)
				{
					g = 2;
					switch (chess->pos[i][j])
					{
						case BlackPawn	:
						  g = move_black_pawn(chess,j,i,l,k);
						  break;
						case BlackRook	:
						  g = move_black_rook(chess,j,i,l,k);
						  break;
						case BlackKnight:
						  g = move_black_knight(chess,j,i,l,k);
						  break;
						case BlackBishop:
						  g = move_black_bishop(chess,j,i,l,k);
						  break;
						case BlackQueen	:
						  g = move_black_queen(chess,j,i,l,k);
						  break;
						case BlackKing	:
						  g = move_black_king(chess,j,i,l,k);
						  break;
						default		:
						  break;
					}
					if (g == 2)
					{
						continue;
					}
					if (g == 0)
					{
						op = chess->pos[i][j];
						op2 = chess->pos[k][l];
						chess->pos[i][j] = 0;
						chess->pos[k][l] = op;
						if (black_in_check(chess) == 0)
						{
							chess->pos[i][j] = op;
							chess->pos[k][l] = op2;
							return(1);
						}
						else
						{
							chess->pos[i][j] = op;
							chess->pos[k][l] = op2;
						}
					}
				}
			}
		}
	}
	return(0);
}

/* Check for white checkmate */
int white_is_checkmate(Chess chess)
{
	int i,j,x,y,g,h;
	int fromint,fromchar,frompiece,topiece,toint,tochar;

	fromint = fromchar = toint = tochar = g = h = i = j = x = y = 0;
	frompiece = topiece = 0;
	if (chess == NULL) return(-1);

	topiece = white_in_check(chess);
	/* Not even in check.. go away. */
	if (topiece == 0) return(0);
	/* find where the putting us in check is.. */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			if (chess->pos[x][y] == topiece)
			{
				toint = x;
				tochar = y;
				break;
			}
		}
	}

	/* Now see if we can kill it */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			frompiece = chess->pos[x][y];
			if ((frompiece == 0) || (frompiece > WhiteKing))
				continue;
			switch (frompiece)
			{
				case WhitePawn	:
				  g = move_white_pawn(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case WhiteRook	:
				  g = move_white_rook(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case WhiteKnight:
				  g = move_white_knight(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case WhiteBishop:
				  g = move_white_bishop(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case WhiteQueen	:
				  g = move_white_queen(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case WhiteKing	:
				  g = move_white_king(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);
			}
		}
	}
	/* Get king position */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			if (chess->pos[x][y] == WhiteKing)
			{
				fromint = x;
				fromchar = y;
				break;
			}
		}
	}

	/* Now see if we can move it anywhere.. */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			g = move_white_king(chess,fromchar,fromint,y,x);
			if (g == 0)
			{
				frompiece = chess->pos[fromint][fromchar];
				topiece = chess->pos[x][y];
				chess->pos[x][y] = frompiece;
				chess->pos[fromint][fromchar] = 0;
				h = white_in_check(chess);
				chess->pos[x][y] = topiece;
				chess->pos[fromint][fromchar] = frompiece;
				if (h == 0) return(0);
			}
		}
	}
	/*
	  Now see if we can just throw anything in the path and get out of
	  checkmate.
	*/
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			frompiece = chess->pos[x][y];
			for (i = 0; i < 8; ++i)
			{
				for (j = 0; j < 8; ++j)
				{
					topiece = chess->pos[i][j];
					if ((frompiece == 0)
					  || (frompiece > WhiteKing))
						continue;
					switch(frompiece)
					{
						case WhitePawn	:
						  g = move_white_pawn(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = white_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case WhiteRook	:
						  g = move_white_rook(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = white_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case WhiteKnight:
						  g = move_black_knight(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = white_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case WhiteBishop:
						  g = move_white_bishop(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = white_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case WhiteQueen	:
						  g = move_white_queen(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = white_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case WhiteKing	:
						  g = move_white_king(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = white_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);
					}
				}
			}
		}
	}
	return(1);
}

/* Check for black checkmate */
int black_is_checkmate(Chess chess)
{
	int i,j,x,y,g,h;
	int fromint,fromchar,frompiece,topiece,toint,tochar;

	fromint = fromchar = toint = tochar = g = h = i = j = x = y = 0;
	frompiece = topiece = 0;
	if (chess == NULL) return(-1);

	topiece = black_in_check(chess);
	/* Not even in check.. go away. */
	if (topiece == 0) return(0);
	/* find where the putting us in check is.. */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			if (chess->pos[x][y] == topiece)
			{
				toint = x;
				tochar = y;
				break;
			}
		}
	}

	/* Now see if we can kill it */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			frompiece = chess->pos[x][y];
			if ((frompiece == 0) || (frompiece < BlackPawn))
				continue;
			switch (frompiece)
			{
				case BlackPawn	:
				  g = move_black_pawn(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case BlackRook	:
				  g = move_black_rook(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case BlackKnight:
				  g = move_black_knight(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case BlackBishop:
				  g = move_black_bishop(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case BlackQueen	:
				  g = move_black_queen(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);

				case BlackKing	:
				  g = move_black_king(chess,y,x,tochar,toint);
				  if (g != 0) break;
				  chess->pos[toint][tochar] = frompiece;
				  chess->pos[x][y] = 0;
				  h = white_in_check(chess);
				  chess->pos[toint][tochar] = topiece;
				  chess->pos[x][y] = frompiece;
				  if (h != 0) break;
				  return(0);
			}
		}
	}
	/* Get king position */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			if (chess->pos[x][y] == BlackKing)
			{
				fromint = x;
				fromchar = y;
				break;
			}
		}
	}

	/* Now see if we can move it anywhere.. */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			g = move_black_king(chess,fromchar,fromint,y,x);
			if (g == 0)
			{
				frompiece = chess->pos[fromint][fromchar];
				topiece = chess->pos[x][y];
				chess->pos[x][y] = frompiece;
				chess->pos[fromint][fromchar] = 0;
				h = black_in_check(chess);
				chess->pos[x][y] = topiece;
				chess->pos[fromint][fromchar] = frompiece;
				if (h == 0) return(0);
			}
		}
	}
	/*
	  Now see if we can just throw anything in the path and get out of
	  checkmate.
	*/
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			frompiece = chess->pos[x][y];
			for (i = 0; i < 8; ++i)
			{
				for (j = 0; j < 8; ++j)
				{
					topiece = chess->pos[i][j];
					if ((frompiece == 0)
					  || (frompiece < BlackPawn))
						continue;
					switch(frompiece)
					{
						case BlackPawn	:
						  g = move_black_pawn(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = black_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case BlackRook	:
						  g = move_black_rook(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = black_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case BlackKnight:
						  g = move_black_knight(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = black_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case BlackBishop:
						  g = move_black_bishop(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = black_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case BlackQueen	:
						  g = move_black_queen(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = black_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);

						case BlackKing	:
						  g = move_black_king(chess,y,x,j,i);
						  if (g != 0) break;
						  chess->pos[i][j] = frompiece;
						  chess->pos[x][y] = 0;
						  h = black_in_check(chess);
						  chess->pos[i][j] = topiece;
						  chess->pos[x][y] = frompiece;
						  if (h != 0) break;
						  return(0);
					}
				}
			}
		}
	}
	return(1);
}

/* END OF CHESS.C */
