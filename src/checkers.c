/*
  checkers.c
    Source code for the checkers game.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish.

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

    July 21, 2001 Rob Melhuish
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "include/checkers.h"
#define _CHECKERS_C
  #include "include/needed.h"
#undef _CHECKERS_C

void checkers_say(Checkers ch,char *str)
{
	User red,white;

	red = get_user(ch->red);
	if (ch == NULL) return;
	white = get_user(ch->white);
	if (white == NULL) return;
	write_user(red,str);
	write_user(white,str);
}

/* See if anyone has won the game yet. */
int win_checkers(Checkers ch)
{
	int i,j,cnt1,cnt2;

	if (ch == NULL) return(0);
	i = j = cnt1 = cnt2 = 0;
	for (i = 0; i < 9; ++i)
	{
		for (j = 0; j < 9; ++j)
		{
			if (ch->pos[i][j] == 0) continue;
			if (ch->pos[i][j] < 3) ++cnt1;
			if (ch->pos[i][j] > 2) ++cnt2;
		}
	}
	if (cnt1 != 0 && cnt2 == 0) return(1); /* white wins */
	if (cnt1 == 0 && cnt2 != 0) return(2); /* red wins */
	return(0);
}

void checkers_com(User user,char *inpstr,int rt)
{
	User u = NULL;
	Checkers ch;

	if (user == NULL) return;
	if (user->checkers != NULL)
	{
		if (!strncasecmp(word[1],"-quit",2))
		{
			sprintf(text,"~CW-~FG The checkers game is over. ~FT%s~FG gave up.\n",user->recap);
			checkers_say(user->checkers,text);
			checkers_kill(user);
			return;
		}
		else if (!strncasecmp(word[1],"-pieces",2))
		{
			if (user->pieces == 0)
			{
				write_user(user,"~CW-~FT You change your checker pieces from big to small. \n");
				user->pieces = 1;
				return;
			}
			else
			{
				write_user(user,"~CW-~FT You change your checkers pieces from small to big.\n");
				user->pieces = 0;
				return;
			}
		}
		else if (!strncasecmp(word[1],"-board",2))
		{
			print_checkerboard(user,"");
			u = user->checkers->turn;
			if (u == user) write_user(user,"~CW-~FG It's currently your turn.\n");
			else
			{
				if (u == NULL)
				{
					write_user(user,"~CW-~FT It looks like your opponent left, gotta reset the game.\n");
					checkers_kill(user);
					return;
				}
				else
				{
					writeus(user,"~CW-~FT It is currently ~FG%s%s ~FTturn.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
					return;
				}
			}
			return;
		}
		else if (!strncasecmp(word[1],"-gs",2))
		{
			checkers_summary(user);
			return;
		}
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CR-gs~CW|~CR-board~CW|~CR-pieces~CW|~CR-quit~CB]");
			return;
		}
	}
	else if (user->checker_opp != NULL)
	{
		u = user->checker_opp;
		if (u == NULL)
		{
			write_user(user,"~CW-~FT It looks like the person who challenged you left.\n");
			user->checker_opp = NULL;
			return;
		}
		if (!strncasecmp(word[1],"-accept",2))
		{
			if (u->checkers != NULL)
			{
				write_user(user,"~CW-~FT It looks like that person already found someone else to play with.\n");
				user->checker_opp = NULL;
				return;
			}
			ch = (Checkers)(malloc(sizeof(struct checkers_struct)));
			if (ch == NULL)
			{
				write_user(user,"~CW-~FT An error occurred... try again later..\n");
				user->checker_opp = NULL;
				u->checker_opp = NULL;
				return;
			}
			writeus(user,"~CW-~FG You accept ~FT%s%s~FG game of checkers.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
			writeus(u,"~CW-~FG %s~FT accepts your game of checkers.\n",user->recap);
			user->checker_opp = NULL;
			u->checker_opp	   = NULL;
			memset(ch,0,sizeof(struct checkers_struct));
			user->checkers	   = ch;
			u->checkers	   = ch;
			strcpy(ch->white,user->name);
			strcpy(ch->red,u->name);
			ch->turn	   = user;
			init_checkerboard(user,ch);
			sprintf(text,"~CW-~FY Checkers game has started,~FM %s ~FYis going first.\n",ch->turn->recap);
			write_user(user,text);
			write_user(u,text);
			print_checkerboard(user,"");
			print_checkerboard(u,"");
			return;
		}
		else if (!strncasecmp(word[1],"-reject",2))
		{
			user->checker_opp = NULL;
			u->checker_opp = NULL;
			writeus(user,"~CW-~FT You reject ~FG%s%s ~FTgame of checkers.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
			writeus(u,"~CW-~FG %s~FT rejects your game of checkers.\n",user->recap);
			return;
		}
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CR-accept~CW|~CR-reject~CB]");
			return;
		}
	}
	if (word_count < 2)
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
		writeus(user,"~CW-~FT You can't play games with your enemies.\n");
		return;
	}
	if (is_busy(u))
	{
		writeus(user,"~CW-~FM %s ~FYis currently busy at the moment..\n",u->recap);
		return;
	}
	if (u->checkers != NULL)
	{
		writeus(user,"~CW-~FM %s~FY is already playing a game of checkers.\n",u->recap);
		return;
	}
	if (u->level < command[com_num].lev
	  && !is_allowed(u,command[com_num].name))
	{
		writeus(user,"~CW-~FM %s~FY doesn't have access to that command.\n",u->recap);
		return;
	}
	u->checker_opp = user;
	writeus(u,"~CW-~FM %s~FT would like to play a game of checkers with you.\n"                  "~CW-~FT To accept the game, type ~CW'~CR.%s -accept~CW'\n"
		  "~CW-~FT To reject the game, type ~CW'~CR.%s -reject~CW'\n",
		  user->recap,command[com_num].name,command[com_num].name);
	writeus(user,"~CW- ~FYYou ask ~FM%s~FY to play a game of checkers with you.\n",u->recap);
	return;
}

void move_checkers(User user,char *inpstr,int rt)
{
	User u = NULL,u2 = NULL;
	Checkers ch;
	int fromint[10],fromchar[10],toint[10],tochar[10],i,x,y;
	int topiece, frompiece,legal,jump_v,jump_h,win,jumppiece;
	int invalid_num,red,white,hops,jumped,kinged;
	char from[10][3], to[10][3],temp[ARR_SIZE];

	invalid_num = -1;
	red = white = x = y = i = win = frompiece = topiece = jumppiece = 0;
	jumped = kinged = 0;
	jump_v = jump_h = -1;
	ch = user->checkers;
	if (ch == NULL)
	{
		write_user(user,"~CW-~FT You haven't even started playing a game of checkers yet.\n");
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRpiece from~CB] [~CRpiece to~CB] ...");
		writeus(user,"~CW - ~FT For help on this command see ~CW'~CR.help %s~CW'\n",command[com_num].name);
		return;
	}
	if (user->checkers->turn != user)
	{
		write_user(user,"~CW-~FT It's not even your turn.\n");
		return;
	}
	for (i = 0; i < 10; ++i)
	{
		if (word[i][0] != '\0' && (strlen(word[i]) != 2) && word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRpiece from~CB] [~CRpiece to~CB] ...");
			writeus(user,"~CW- ~FT For help on this command see ~CW'~CR.help %s~CW'\n",command[com_num].name);
			return;
		}
	}
	u = get_user(ch->white);
	u2 = get_user(ch->red);
	if (u == user) u = u2;
	/* Save the original board pos cuz we're movin things around. */
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			ch->orig_pos[x][y] = ch->pos[x][y];
		}
	}
	for (i = 0; i < 10; ++i)
	{
		*from[i]    = '\0';
		*to[i]	    = '\0';
		toint[i]    = 0;
		fromint[i]  = 0;
		tochar[i]   = 0;
		fromchar[i] = 0;
	}
	strcpy(from[0],word[1]);
	strcpy(to[0],word[2]);	strcpy(to[1],word[3]);
	strcpy(to[2],word[4]);	strcpy(to[3],word[5]);
	strcpy(to[4],word[6]);	strcpy(to[5],word[7]);
	strcpy(to[6],word[8]);	strcpy(to[7],word[9]);
	strcpy(to[8],word[10]);	strcpy(to[9],word[11]);

	strcpy(from[1],to[0]);
	if (to[1][0] != '\0') strcpy(from[2],to[1]); else from[2][0] = '\0';
	if (to[2][0] != '\0') strcpy(from[3],to[2]); else from[3][0] = '\0';
	if (to[3][0] != '\0') strcpy(from[4],to[3]); else from[4][0] = '\0';
	if (to[4][0] != '\0') strcpy(from[5],to[4]); else from[5][0] = '\0';
	if (to[5][0] != '\0') strcpy(from[6],to[5]); else from[6][0] = '\0';
	if (to[6][0] != '\0') strcpy(from[7],to[6]); else from[7][0] = '\0';
	if (to[7][0] != '\0') strcpy(from[8],to[7]); else from[8][0] = '\0';
	if (to[8][0] != '\0') strcpy(from[9],to[8]); else from[9][0] = '\0';

	i = 0;
	text[0] = '\0';
	while (i < 10)
	{
		if (to[i][0] == '\0') break;
		fromchar[i]	= from[i][0];
		fromchar[i]	-= 97;
		tochar[i]	= to[i][0];
		tochar[i]	-= 97;
		fromint[i]	= from[i][1];
		fromint[i]	-= 49;
		toint[i]	= to[i][1];
		toint[i]	-= 49;
		fromint[i]	-= 7;
		fromint[i]	= abs(fromint[i]);
		toint[i]	-= 7;
		toint[i]	= abs(toint[i]);

		if ((fromchar[i] > 7) || (fromchar[i] < 0) || (fromint[i] > 7)
		  || (fromint[i] < 0) || (tochar[i] > 7) || (tochar[i] < 0)
		  || (toint[i] > 7) || (toint[i] < 0))
		{
			invalid_num = i;
			writeus(user,"~CW-~FT That move~CB: ~FY%s ~FTto~FY %s~FT doesn't fit on an 8x8 board.\n",from[i],to[i]);
			break;
		}
		topiece = ch->pos[toint[i]][tochar[i]];
		frompiece = ch->pos[fromint[i]][fromchar[i]];
		if (frompiece == 0)
		{
			invalid_num = i;
			writeus(user,"~CW-~FT There is no piece at~FY %s~FT.\n",from[i]);
			break;
		}
		if (!strcasecmp(user->name,ch->white)) white = 1;
		else red = 1;
		if (white)
		{
			if (frompiece > 2)
			{
				invalid_num = i;
				writeus(user,"~CW-~FT You can't move your opponent piece at ~FY%s~FT.\n",from[i]);
				break;
			}
		}
		else
		{
			if (frompiece < 3)
			{
				invalid_num = i;
				writeus(user,"~CW-~FT You can't move your opponent piece at ~FY%s~FT.\n",from[i]);
				break;
			}
		}
		if (white) legal = valid_white_move(ch,fromchar[i],fromint[i],tochar[i],toint[i]);
		else legal = valid_red_move(ch,fromchar[i],fromint[i],tochar[i],toint[i]);
		if (legal == 0)
		{
			invalid_num = i;
			writeus(user,"~CW-~FT You can't move piece ~FY%s ~FTto~FY %s~FT.\n",from[i],to[i]);
			break;
		}
		ch->pos[fromint[i]][fromchar[i]] = 0;
		ch->pos[toint[i]][tochar[i]] = frompiece;
		if ((ch->jump_v != -1) && (ch->jump_h != -1))
		{
			jumppiece = ch->pos[ch->jump_v][ch->jump_h];
			ch->pos[ch->jump_v][ch->jump_h] = 0;
			jump_v = ch->jump_v;
			jump_v -= 7;
			jump_v = abs(jump_v);
			jump_h = ch->jump_h;
			ch->jump_v = -1;
			ch->jump_h = -1;
			jumped = 1;
		}
		if (white == 1 && toint[i] == 0)
		{
			if (ch->pos[toint[i]][tochar[i]] == 1)
			{
				ch->pos[toint[i]][tochar[i]] = 2;
				kinged = 1;
			}
		}
		else if (red == 1 && toint[i] == 7)
		{
			if (ch->pos[toint[i]][tochar[i]] == 3)
			{
				ch->pos[toint[i]][tochar[i]] = 4;
				kinged = 1;
			}
		}
		if (jumped)
		{
			if (kinged) writeus(user,"~CW-~FT Move ~FY%s~FT to ~FY%s~FT jumping piece at ~FY%c%d~FT, and getting crowned was valid.\n",from[i],to[i],letters[jump_h],(jump_v+1));
			else writeus(user,"~CW-~FT Move ~FY%s~FT to ~FY%s~FT jumping piece at ~FY%c%d ~FTwas valid.\n",from[i],to[i],letters[jump_h],(jump_v+1));
			if (kinged) sprintf(temp,"~CW-~FT %s moves ~FY%s ~FTto ~FY%s~FT jumping piece at ~FY%c%d~FT, and has their checker crowned.\n",white == 1?ch->white:ch->red,from[i],to[i],letters[jump_h],(jump_v+1));
			else sprintf(temp,"~CW-~FT %s moves ~FY%s ~FTto ~FY%s~FT jumping piece at ~FY%c%d~FT.\n",white == 1?ch->white:ch->red,from[i],to[i],letters[jump_h],(jump_v+1));
		}
		else
		{
			if (kinged) writeus(user,"~CW-~FT Move ~FY%s ~FTto~FY %s~FT and getting crowned was valid.\n",from[i],to[i]);
			else writeus(user,"~CW-~FT Move ~FY%s ~FTto~FY %s~FT was valid.\n",from[i],to[i]);
			if (kinged) sprintf(temp,"~CW-~FT %s moves ~FY%s ~FTto ~FY%s~FT, and has their checker crowned.\n",white == 1?ch->white:ch->red,from[i],to[i]);
			else sprintf(temp,"~CW-~FT %s moves ~FY%s ~FTto ~FY%s~FT.\n",white == 1?ch->white:ch->red,from[i],to[i]);
		}
		strcat(text,temp);
		jumped = 0;
		kinged = 0;
		++hops;
		++i;
	}
	if (invalid_num >= 0)
	{
		for (x = 0; x < 8; x++)
		{
			for (y = 0; y < 8; y++)
			{
				ch->pos[x][y] = ch->orig_pos[x][y];
			}
		}
		writeus(user,"~CW-~FT Your move ~FG#%d~FT was invalid, try again.\n",invalid_num);
		return;
	}
	sprintf(ch->last_move,"%s",text);
	user->checkers->turn = u;
	user->checkers->last_turn = user;
	win = win_checkers(user->checkers);
	if (win)
	{
		sprintf(text,"~CW-~FT %s ~FGhas won. This is the board layout before the move.\n",user->recap);
		checkers_say(ch,text);
		print_old_checkerboard(user);
		print_old_checkerboard(u);
		sprintf(text,"~CW-~FT %s ~FGhas won. This is the board layout after the move.\n",user->recap);
		checkers_say(ch,text);
		print_checkerboard(user,"");
		print_checkerboard(u,"");
		sprintf(text,"~CW-~FT %s ~FGhas won this game of checkers.\n",user->recap);
		checkers_say(ch,text);
		checkers_say(ch,ch->last_move);
		++user->myint1;
		++u->myint2;
		checkers_kill(user);
		return;
	}
	print_checkerboard(user,"");
	print_checkerboard(u,"");
	checkers_say(ch,ch->last_move);
	writeus(user,"~CW-~FT It's now ~FG%s%s ~FTturn.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
	write_user(u,"~CW-~FT It's now your turn.\n");
	ch->jump_v = ch->jump_h = -1;
	ch->hops = 0;
	return;
}

void init_checkerboard(User u,Checkers ch)
{
	/* white pieces */
	ch->pos[7][0] = ch->pos[7][2] = 1;
	ch->pos[7][4] = ch->pos[7][6] = 1;
	ch->pos[6][1] = ch->pos[6][3] = 1;
	ch->pos[6][5] = ch->pos[6][7] = 1;
	ch->pos[5][0] = ch->pos[5][2] = 1;
	ch->pos[5][4] = ch->pos[5][6] = 1;

	/* red pieces */
	ch->pos[0][1] = ch->pos[0][3] = 3;
	ch->pos[0][5] = ch->pos[0][7] = 3;
	ch->pos[1][0] = ch->pos[1][2] = 3;
	ch->pos[1][4] = ch->pos[1][6] = 3;
	ch->pos[2][1] = ch->pos[2][3] = 3;
	ch->pos[2][5] = ch->pos[2][7] = 3;
	ch->jump_v = ch->jump_h = -1;
	
}

void print_old_checkerboard(User user)
{
	Checkers ch;
	int x,y;
	char temp_pos[10][10];

	ch = user->checkers;
	if (ch == NULL) return;
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			temp_pos[x][y] = ch->pos[x][y];
		}
	}
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			ch->pos[x][y] = ch->orig_pos[x][y];
		}
	}
	print_checkerboard(user,"");
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y < 8; ++y)
		{
			ch->pos[x][y] = temp_pos[x][y];
		}
	}
	return;
}

void print_checkerboard(User user,char *str)
{
	if (user->pieces == 1) print_small_checkerboard(user,str);
	else print_big_checkerboard(user,str);
	return;
}

void print_small_checkerboard(User user,char *str)
{
	Checkers ch;
	User u;
	char temp[81],board[2048*2];
	int i,j,k,row,rowm,pos = 0,alternating = 0;

	board[0] = temp[0] = '\0';
	if (*str)
	{
		u = get_user(str);
		if (u == NULL) return;
	}
	else u = user;
	ch = u->checkers;
	if (ch == NULL)
	{
		if (u == user) write_user(user,"~CW- ~FT You haven't even started a game of checkers yet.\n");
		else writeus(user,"~CW-~FM %s~FT doesn't appear to be playing a game of checkers.\n",u->recap);
		return;
	}
	if (u == user && !strcasecmp(u->name,u->checkers->red))
	{
		alternating = 0;
		rowm = 1;
		row  = 1;
	}
	else
	{
		alternating = 1;
		rowm = -1;
		row = 8;
	}
	
	write_user(user,"~FG  _______________________________\n");
	for (i = 0; i < 8; i++)
	{
		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 8; j++)
			{
				if (j == 0 && k == 1)
				{
					sprintf(temp,"~FY%d",row);
					strcat(board,temp);
				}
				else if (j == 0) strcat(board," ");
				if (rowm == -1) pos = ch->pos[i][j];
				else pos = ch->pos[7-i][j];
				if (k == 1)
				{
					if (pos == 0)
					{
						if (alternating == 1)
							sprintf(temp,"~FG|~BR___");
						else sprintf(temp,"~FG|___");
						strcat(board,temp);
					}
					else
					{

						if (pos < 3)
						{
							sprintf(temp,"~FG|~FT%s",small_cpieces[k][pos]);
							strcat(board,temp);
						}
						else
						{
							sprintf(temp,"~FG|~FM%s",small_cpieces[k][pos]);
							strcat(board,temp);
						}
					}
				}
				else
				{
					if (pos == 0)
					{
						if (alternating == 1)
							sprintf(temp,"~FG|~BR   ");
						else sprintf(temp,"~FG|   ");
						strcat(board,temp);
					}
					else
					{
						if (pos < 3)
						{
							sprintf(temp,"~FG|~FT%s",small_cpieces[k][pos]);
							strcat(board,temp);
						}
						else
						{
							sprintf(temp,"~FG|~FM%s",small_cpieces[k][pos]);
							strcat(board,temp);
						}
					}
				}
				if (alternating == 0) alternating = 1;
				else alternating = 0;
			}
			sprintf(temp,"~FG|~RS\n~RS");
			strcat(board,temp);
			write_user(user,board);
			*board = '\0';
		}
		if (alternating == 0) alternating = 1;
		else alternating = 0;
		row += rowm;
	}
	write_user(user,"~FY   A   B   C   D   E   F   G   H\n");
	return;
}

void print_big_checkerboard(User user,char *str)
{
	Checkers ch;
	User u;
	char temp[81],board[2048*2];
	int i,j,k,row,rowm,pos = 0,alternating = 0;

	board[0] = temp[0] = '\0';
	if (*str)
	{
		u = get_user(str);
		if (u == NULL) return;
	}
	else u = user;
	ch = u->checkers;
	if (ch == NULL)
	{
		if (u == user) write_user(user,"~CW- ~FT You haven't even started a game of checkers yet.\n");
		else writeus(user,"~CW-~FM %s~FT doesn't appear to be playing a game of checkers.\n",u->recap);
		return;
	}
	if (u == user && !strcasecmp(u->name,u->checkers->red))
	{
		alternating = 0;
		rowm = 1;
		row  = 1;
	}
	else
	{
		alternating = 1;
		rowm = -1;
		row = 8;
	}

	write_user(user,"~FG  _______________________________________________\n");
	for (i = 0; i < 8; i++)
	{
		for (k = 0; k < 3; k++)
		{
			for (j = 0; j < 8; j++)
			{
				if (j == 0 && k == 1)
				{
					sprintf(temp,"~FY%d",row);
					strcat(board,temp);
				}
				else if (j == 0) strcat(board," ");
				if (rowm == -1) pos = ch->pos[i][j];
				else pos = ch->pos[7-i][j];
				if (k == 2)
				{
					if (pos == 0)
					{
						if (alternating == 1)
							sprintf(temp,"~FG|~BR_____");
						else sprintf(temp,"~FG|_____");
						strcat(board,temp);
					}
					else
					{
						if (pos < 3)
						{
							sprintf(temp,"~FG|_%s_",big_cpieces[k][pos]);
							strcat(board,temp);
						}
						else
						{
							sprintf(temp,"~FG|_%s_",big_cpieces[k][pos]);
							strcat(board,temp);
						}
					}
				}
				else
				{
					if (pos == 0)
					{
						if (alternating == 1)
							sprintf(temp,"~FG|~BR     ");
						else sprintf(temp,"~FG|     ");
						strcat(board,temp);
					}
					else
					{
						if (pos < 3)
						{
							sprintf(temp,"~FG| ~FT%s ",big_cpieces[k][pos]);
							strcat(board,temp);
						}
						else
						{
							sprintf(temp,"~FG| ~FM%s ",big_cpieces[k][pos]);
							strcat(board,temp);
						}
					}
				}
				if (alternating == 0) alternating = 1;
				else alternating = 0;
			}
			sprintf(temp,"~FG|~RS\n~RS");
			strcat(board,temp);
			write_user(user,board);
			*board = '\0';
		}
		if (alternating == 0) alternating = 1;
		else alternating = 0;
		row += rowm;
	}
	write_user(user,"~FY    A     B     C     D     E     F     G     H\n");
	return;
}

int valid_white_move(Checkers ch,int fromchar, int fromint, int tochar, int toint)
{
	int value;
	int jumpint,jumpchar;

	jumpint = jumpchar = 0;
	if ((fromchar == tochar) && (fromint == toint))
	{
		return(0);
	}
	value = fromint - toint;
	if (value <= 0)
	{
		if (ch->pos[fromint][fromchar] == 1)
		{
			return(0);
		}
		value = toint - fromint;
		if (value == 0)
		{
			return(0);
		}
		if (value > 2)
		{
			return(0);
		}
		if (value == 2)
		{
			if (tochar > fromchar) value = tochar - fromchar;
			else value = fromchar - tochar;
			if (value == 0)
			{
				return(0);
			}
			if (ch->pos[toint][tochar] != 0)
			{
				return(0);
			}
			switch (fromint)
			{
				case 0:
				  switch (toint)
				  {
					case 2:
					  jumpint = 1;
					  break;
					default:
					  break;
				  }
				  break;
				case 1:
				  switch (toint)
				  {
					case 3:
					  jumpint = 2;
					  break;
					default:
					  break;
				  }
				  break;
				case 2:
				  switch (toint)
				  {
					case 0:
					  jumpint = 1;
					  break;
					case 4:
					  jumpint = 3;
					  break;
					default:
					  break;
				  }
				  break;
				case 3:
				  switch (toint)
				  {
					case 1:
					  jumpint = 2;
					  break;
					case 5:
					  jumpint = 4;
					  break;
					default:
					  break;
				  }
				  break;
				case 4:
				  switch (toint)
				  {
					case 2:
					  jumpint = 3;
					  break;
					case 6:
					  jumpint = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 5:
				  switch (toint)
				  {
					case 3:
					  jumpint = 4;
					  break;
					case 7:
					  jumpint = 6;
					  break;
					default:
					  break;
				  }
				  break;
				case 6:
				  switch (toint)
				  {
					case 4:
					  jumpint = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 7:
				  switch (toint)
				  {
					case 5:
					  jumpint = 6;
					  break;
					default:
					  break;
				  }
				  break;
			}
			switch (fromchar)
			{
				case 0:
				  switch (tochar)
				  {
					case 2:
					  jumpchar = 1;
					  break;
					default:
					  break;
				  }
				  break;
				case 1:
				  switch (tochar)
				  {
					case 3:
					  jumpchar = 2;
					  break;
					default:
					  break;
				  }
				  break;
				case 2:
				  switch (tochar)
				  {
					case 0:
					  jumpchar = 1;
					  break;
					case 4:
					  jumpchar = 3;
					  break;
					default:
					  break;
				  }
				  break;
				case 3:
				  switch (tochar)
				  {
					case 1:
					  jumpchar = 2;
					  break;
					case 5:
					  jumpchar = 4;
					  break;
					default:
					  break;
				  }
				  break;
				case 4:
				  switch (tochar)
				  {
					case 2:
					  jumpchar = 3;
					  break;
					case 6:
					  jumpchar = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 5:
				  switch (tochar)
				  {
					case 3:
					  jumpchar = 4;
					  break;
					case 7:
					  jumpchar = 6;
					  break;
					default:
					  break;
				  }
				  break;
				case 6:
				  switch (tochar)
				  {
					case 4:
					  jumpchar = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 7:
				  switch (tochar)
				  {
					case 5:
					  jumpchar = 6;
					  break;
					default:
					  break;
				  }
				  break;
			}
			if (ch->pos[jumpint][jumpchar] == 0)
			{
				return(0);
			}
			if (ch->pos[jumpint][jumpchar] < 3)
			{
				return(0);
			}
			/* we jumped the piece... */
			ch->jump_v = jumpint;
			ch->jump_h = jumpchar;
		}
		if (value == 1)
		{
			if (ch->hops > 1) return(0);
			if (tochar > fromchar) value = tochar - fromchar;
			else value = fromchar - tochar;
			if (value == 0)
			{
				return(0);
			}
			if ((value > 1) || (ch->pos[toint][tochar] != 0))
			{
				return(0);
			}
		}
        	return(1); /* 'twas a valid move. */

	}
        if (value > 2)
	{
		return(0);
	}
	if (value == 2)
	{
		if (tochar > fromchar) value = tochar - fromchar;
		else value = fromchar - tochar;
		if (value == 0)
		{
			return(0);
		}
		if (ch->pos[toint][tochar] != 0)
		{
			return(0);
		}
		switch (fromint)
		{
			case 0:
			  switch (toint)
			  {
				case 2:
				  jumpint = 1;
				  break;
				default:
				  break;
			  }
			  break;
			case 1:
			  switch (toint)
			  {
				case 3:
				  jumpint = 2;
				  break;
				default:
				  break;
			  }
			  break;
			case 2:
			  switch (toint)
			  {
				case 0:
				  jumpint = 1;
				  break;
				case 4:
				  jumpint = 3;
				  break;
				default:
				  break;
			  }
			  break;
			case 3:
			  switch (toint)
			  {
				case 1:
				  jumpint = 2;
				  break;
				case 5:
				  jumpint = 4;
				  break;
				default:
				  break;
			  }
			  break;
			case 4:
			  switch (toint)
			  {
				case 2:
				  jumpint = 3;
				  break;
				case 6:
				  jumpint = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 5:
			  switch (toint)
			  {
				case 3:
				  jumpint = 4;
				  break;
				case 7:
				  jumpint = 6;
				  break;
				default:
				  break;
			  }
			  break;
			case 6:
			  switch (toint)
			  {
				case 4:
				  jumpint = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 7:
			  switch (toint)
			  {
				case 5:
				  jumpint = 6;
				  break;
				default:
				  break;
			  }
			  break;
		}
		switch (fromchar)
		{
			case 0:
			  switch (tochar)
			  {
				case 2:
				  jumpchar = 1;
				  break;
				default:
				  break;
			  }
			  break;
			case 1:
			  switch (tochar)
			  {
				case 3:
				  jumpchar = 2;
				  break;
				default:
				  break;
			  }
			  break;
			case 2:
			  switch (tochar)
			  {
				case 0:
				  jumpchar = 1;
				  break;
				case 4:
				  jumpchar = 3;
				  break;
				default:
				  break;
			  }
			  break;
			case 3:
			  switch (tochar)
			  {
				case 1:
				  jumpchar = 2;
				  break;
				case 5:
				  jumpchar = 4;
				  break;
				default:
				  break;
			  }
			  break;
			case 4:
			  switch (tochar)
			  {
				case 2:
				  jumpchar = 3;
				  break;
				case 6:
				  jumpchar = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 5:
			  switch (tochar)
			  {
				case 3:
				  jumpchar = 4;
				  break;
				case 7:
				  jumpchar = 6;
				  break;
				default:
				  break;
			  }
			  break;
			case 6:
			  switch (tochar)
			  {
				case 4:
				  jumpchar = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 7:
			  switch (tochar)
			  {
				case 5:
				  jumpchar = 6;
				  break;
				default:
				  break;
			  }
			  break;
		}
		if (ch->pos[jumpint][jumpchar] == 0)
		{
			return(0);
		}
		if (ch->pos[jumpint][jumpchar] < 3)
		{
			return(0);
		}
		/* we jumped the piece.. remove it.. */
		ch->jump_v = jumpint;
		ch->jump_h = jumpchar;
	}
	if (value == 1)
	{
		if (ch->hops > 1) return(0);
		if (tochar > fromchar) value = tochar - fromchar;
		else value = fromchar - tochar;
		if (value == 0)
		{
			return(0);
		}
		if ((value > 1) || (ch->pos[toint][tochar] != 0))
		{
			return(0);
		}
	}
        if (toint == 0) ch->pos[fromint][fromchar] = 2; /* king me */
        return(1); /* 'twas a valid move. */
}

int valid_red_move(Checkers ch,int fromchar, int fromint, int tochar, int toint)
{
	int value;
	int jumpint, jumpchar;

	jumpint = jumpchar = 0;
	if ((fromchar == tochar) && (fromint == toint))
	{
		return(0);
	}
	value = toint - fromint;
	if (value == 0)
	{
		return(0);
	}
	if (value < 0)
	{
		if (ch->pos[fromint][fromchar] == 3)
		{
			return(0);
		}
		value = fromint - toint;
		if (value == 0)
		{
			return(0);
		}
		if (value > 2)
		{
			return(0);
		}
		if (value == 2)
		{
			value = abs(tochar - fromchar);
			if (value == 0)
			{
				return(0);
			}
			if (ch->pos[toint][tochar] != 0)
			{
				return(0);
			}
			switch (fromint)
			{
				case 0:
				  switch (toint)
				  {
					case 2:
					  jumpint = 1;
					  break;
					default:
					  break;
				  }
				  break;
				case 1:
				  switch (toint)
				  {
					case 3:
					  jumpint = 2;
					  break;
					default:
					  break;
				  }
				  break;
				case 2:
				  switch (toint)
				  {
					case 0:
					  jumpint = 1;
					  break;
					case 4:
					  jumpint = 3;
					  break;
					default:
					  break;
				  }
				  break;
				case 3:
				  switch (toint)
				  {
					case 1:
					  jumpint = 2;
					  break;
					case 5:
					  jumpint = 4;
					  break;
					default:
					  break;
				  }
				  break;
				case 4:
				  switch (toint)
				  {
					case 2:
					  jumpint = 3;
					  break;
					case 6:
					  jumpint = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 5:
				  switch (toint)
				  {
					case 3:
					  jumpint = 4;
					  break;
					case 7:
					  jumpint = 6;
					  break;
					default:
					  break;
				  }
				  break;
				case 6:
				  switch (toint)
				  {
					case 4:
					  jumpint = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 7:
				  switch (toint)
				  {
					case 5:
					  jumpint = 6;
					  break;
					default:
					  break;
				  }
				  break;
			}
			switch (fromchar)
			{
				case 0:
				  switch (tochar)
				  {
					case 2:
					  jumpchar = 1;
					  break;
					default:
					  break;
				  }
				  break;
				case 1:
				  switch (tochar)
				  {
					case 3:
					  jumpchar = 2;
					  break;
					default:
					  break;
				  }
				  break;
				case 2:
				  switch (tochar)
				  {
					case 0:
					  jumpchar = 1;
					  break;
					case 4:
					  jumpchar = 3;
					  break;
					default:
					  break;
				  }
				  break;
				case 3:
				  switch (tochar)
				  {
					case 1:
					  jumpchar = 2;
					  break;
					case 5:
					  jumpchar = 4;
					  break;
					default:
					  break;
				  }
				  break;
				case 4:
				  switch (tochar)
				  {
					case 2:
					  jumpchar = 3;
					  break;
					case 6:
					  jumpchar = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 5:
				  switch (tochar)
				  {
					case 3:
					  jumpchar = 4;
					  break;
					case 7:
					  jumpchar = 6;
					  break;
					default:
					  break;
				  }
				  break;
				case 6:
				  switch (tochar)
				  {
					case 4:
					  jumpchar = 5;
					  break;
					default:
					  break;
				  }
				  break;
				case 7:
				  switch (tochar)
				  {
					case 5:
					  jumpchar = 6;
					  break;
					default:
					  break;
				  }
				  break;
			}
			if (ch->pos[jumpint][jumpchar] == 0)
			{
				return(0);
			}
			if (ch->pos[jumpint][jumpchar] > 2)
			{
				return(0);
			}
			/* we jumped the piece.. remove it.. */
			ch->jump_v = jumpint;
			ch->jump_h = jumpchar;
		}
		if (value == 1)
		{
			if (ch->hops > 1) return(0);
			value = abs(tochar - fromchar);
			if (value == 0)
			{
				return(0);
			}
			if ((value > 1) || (ch->pos[toint][tochar] != 0))
			{
				return(0);
			}
		}
	        return(1); /* 'twas a valid move. */
	}
        if (value > 2)
        {
		return(0);
	}
	if (value == 2)
	{
		value = abs(tochar - fromchar);
		if (value == 0)
		{
			return(0);
		}
		if (ch->pos[toint][tochar] != 0)
		{
			return(0);
		}
		switch (fromint)
		{
			case 0:
			  switch (toint)
			  {
				case 2:
				  jumpint = 1;
				  break;
				default:
				  break;
			  }
			  break;
			case 1:
			  switch (toint)
			  {
				case 3:
				  jumpint = 2;
				  break;
				default:
				  break;
			  }
			  break;
			case 2:
			  switch (toint)
			  {
				case 0:
				  jumpint = 1;
				  break;
				case 4:
				  jumpint = 3;
				  break;
				default:
				  break;
			  }
			  break;
			case 3:
			  switch (toint)
			  {
				case 1:
				  jumpint = 2;
				  break;
				case 5:
				  jumpint = 4;
				  break;
				default:
				  break;
			  }
			  break;
			case 4:
			  switch (toint)
			  {
				case 2:
				  jumpint = 3;
				  break;
				case 6:
				  jumpint = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 5:
			  switch (toint)
			  {
				case 3:
				  jumpint = 4;
				  break;
				case 7:
				  jumpint = 6;
				  break;
				default:
				  break;
			  }
			  break;
			case 6:
			  switch (toint)
			  {
				case 4:
				  jumpint = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 7:
			  switch (toint)
			  {
				case 5:
				  jumpint = 6;
				  break;
				default:
				  break;
			  }
			  break;
		}
		switch (fromchar)
		{
			case 0:
			  switch (tochar)
			  {
				case 2:
				  jumpchar = 1;
				  break;
				default:
				  break;
			  }
			  break;
			case 1:
			  switch (tochar)
			  {
				case 3:
				  jumpchar = 2;
				  break;
				default:
				  break;
			  }
			  break;
			case 2:
			  switch (tochar)
			  {
				case 0:
				  jumpchar = 1;
				  break;
				case 4:
				  jumpchar = 3;
				  break;
				default:
				  break;
			  }
			  break;
			case 3:
			  switch (tochar)
			  {
				case 1:
				  jumpchar = 2;
				  break;
				case 5:
				  jumpchar = 4;
				  break;
				default:
				  break;
			  }
			  break;
			case 4:
			  switch (tochar)
			  {
				case 2:
				  jumpchar = 3;
				  break;
				case 6:
				  jumpchar = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 5:
			  switch (tochar)
			  {
				case 3:
				  jumpchar = 4;
				  break;
				case 7:
				  jumpchar = 6;
				  break;
				default:
				  break;
			  }
			  break;
			case 6:
			  switch (tochar)
			  {
				case 4:
				  jumpchar = 5;
				  break;
				default:
				  break;
			  }
			  break;
			case 7:
			  switch (tochar)
			  {
				case 5:
				  jumpchar = 6;
				  break;
				default:
				  break;
			  }
			  break;
		}
		if (ch->pos[jumpint][jumpchar] == 0)
		{
			return(0);
		}
		if (ch->pos[jumpint][jumpchar] > 2)
		{
			return(0);
		}
		/* we jumped the piece.. remove it.. */
		ch->jump_v = jumpint;
		ch->jump_h = jumpchar;
	}
	if (value == 1)
	{
		if (ch->hops > 1) return(0);
		value = abs(tochar - fromchar);
		if (value == 0)
		{
			return(0);
		}
		if ((value > 1) || (ch->pos[toint][tochar] != 0))
		{
			return(0);
		}
	}
        if (toint == 7) ch->pos[fromint][fromchar] = 4; /* king me */
        return(1); /* 'twas a valid move. */
}

/* Now we clear up the checkers game from memory. */
void checkers_kill(User user)
{
	User u, u2;

	if (user->checkers == NULL) return;
	u	= get_user(user->checkers->red);
	u2	= get_user(user->checkers->white);
	sprintf(text,"~CW-~FT This checkers game has been reset.\n");
	checkers_say(user->checkers,text);
	memset(user->checkers,0,sizeof(struct checkers_struct));
	free(user->checkers);
	if (u != NULL) u->checkers = NULL;
	if (u2 != NULL) u2->checkers = NULL;
	if ((u != NULL) && (u2 != NULL))
	{
		writeus(u,"~CW-~FT The checkers game between you and ~FY%s~FT is over.\n",u2->recap);
		writeus(u2,"~CW-~FT The checkers game between you and ~FY%s~FT is over.\n",u->recap);
	}
	return;
}

void checkers_summary(User user)
{
	Checkers ch;

	ch = user->checkers;
	if (ch == NULL)
	{
		write_user(user,"~CW-~FT I was unable to find your checkers game.. sorry.\n");
		return;
	}
	write_user(user,"~CW-~FT The current checker board layout looks like~CB:\n");
	print_checkerboard(user,"");
	if (ch->last_move[0] != '\0') writeus(user,"~CW-~FT The last move was~CB:\n%s",ch->last_move);
	if (ch->turn == user) write_user(user,"~CW-~FT It's currently your turn.\n");
	else writeus(user,"~CW-~FT It's currently ~FG%s%s ~FTturn.\n",ch->turn->recap,ch->turn->name[strlen(ch->turn->name)-1] == 's'?"'":"'s");
	return;
}
