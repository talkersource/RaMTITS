/*
  games.c
    This source file contains code for most of the games, other than the
    gambling games.


    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Andrew Collington, and James Garlick.

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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#define _GAMES_C
  #include "include/needed.h"
#undef _GAMES_C
#include "include/gamelib.h"

/* Added a user->game_ops flag so that the games could be menu driven */
int game_ops(User user,char *inpstr)
{
	User u;
	int move,count,blanks;
	int y,f;
	int user_total,dealer_total,blank,cnt,bet;
	char *name,*rname;
	char moves[]="lrud";
	size_t i = 0;

	if (user->invis == Invis) name=invisname;  else name = user->recap;
	switch (user->game_op)
	{
		case 1:
		  if (user->tic_opponent->invis == Invis) rname = invisname;
		  else rname = user->tic_opponent->recap;
		  if (!isdigit(word[0][0]))
		  {
			if (!strncasecmp(word[0],"board",1))
			{
				print_tic(user);
				write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
				write_user(user,"~CW- ~FTYour move~CB:~RS ");
				user->game_op = 1;
				return(1);
			}
			if (!strncasecmp(word[0],"quit",1))
			{
				sprintf(text,"~CW- ~FG%s~FT is calling the game quits.\n",name);
				write_user(user,text);
				write_user(user->tic_opponent,text);
				reset_game(user->tic_opponent,1);
				reset_game(user,1);
				return(1);
			}
			else
			{
				write_user(user,"\n");
				clear_words();
				word_count = wordfind(inpstr);
				com_num = -1;
				if (user->whitespace) white_space(inpstr);
				if (user->mode == EWTOO || inpstr[0] == '.'
				  || inpstr[0] == ',' || (user->mode == IRC
				  && inpstr[0] == '/') || is_scut(user,inpstr))
					exec_com(user,inpstr);
				else
				{
					com_num = get_comnum("say");
					say(user,inpstr,0);
				}
				write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
				write_user(user,"~CW- ~FTYour move~CB:~RS ");
				user->game_op = 1;
				return(1);
			}
		  }
		  move = atoi(word[0]);
		  if (legal_tic(user->array,move))
		  {
			user->array[move-1] = 1;
			user->tic_opponent->array[move-1] = 2;
			print_tic(user);
			print_tic(user->tic_opponent);
			if (!win_tic(user->array))
			{
				sprintf(text,"~CW- ~FGIt is now ~FM%s's~FG turn.\n",rname);
				write_user(user,text);
				write_user(user->tic_opponent,text);
				write_user(user->tic_opponent,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
				write_user(user->tic_opponent,"~CW- ~FTYour move~CB:~RS ");
				user->first = 0;
				user->game_op = 0;
				rev_away(user);
				user->tic_opponent->first=1;
				user->tic_opponent->game_op=1;
				return(1);
			}
			if (win_tic(user->array) == 1)
			{
				writeus(user,"~CW- ~FGYou have kicked ~FM%s%s~FG ass at tic tac toe..\n",rname,rname[strlen(rname)-1] == 's'?"'":"'s");
				writeus(user->tic_opponent,"~CW- ~FM%s~FT kicked your ass at tic tac toe...\n",name);
				write_user(user->tic_opponent,text);
				user->tic_win++;
				user->tic_opponent->tic_lose++;
				reset_game(user->tic_opponent,1);
				reset_game(user,1);
				return(1);
			}
			else if (win_tic(user->array) == 2)
			{
				writeus(user->tic_opponent,"~CW- ~FGYou have kicked ~FM%s%s~FG ass at tic tac toe..\n",name,name[strlen(name)-1] == 's'?"'":"'s");
				writeus(user,"~CW- ~FM%s~FT kicked your ass at tic tac toe..\n",rname);
				user->tic_lose++;
				user->tic_opponent->tic_win++;
				reset_game(user->tic_opponent,1);
				reset_game(user,1);
				return(1);
			}
			else
			{
				sprintf(text,"~CW- ~FRAwww, a tie between you 2, guess you'll have to try again..\n");
				write_user(user,text);
				write_user(user->tic_opponent,text);
				user->tic_draw++;  user->tic_opponent->tic_draw++;
				reset_game(user->tic_opponent,1);
				reset_game(user,1);
				return(1);
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~CRYou have entered an invalid move!\n");
			write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user,"~CW- ~FTYour move~CB:~RS ");
			user->game_op	= 1;
			user->first	= 1;
			return(1);
		  }
		  return(1);
		case 2:
		  if (user->c4_opponent->invis == Invis) rname = invisname;
		  else rname = user->c4_opponent->recap;
		  if (!isdigit(word[0][0]))
		  {
			if (!strncasecmp(word[0],"board",1))
			{
				print_board(user);
				write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
				write_user(user,"~CW- ~FTYour move~CB:~RS ");
				user->game_op = 2;
				return(1);
			}
			if (!strncasecmp(word[0],"quit",1))
			{
				sprintf(text,"~CW- ~FG%s~FR is calling the game quits..\n",name);
				write_user(user,text);
				write_user(user->c4_opponent,text);
				reset_game(user->c4_opponent,2);
				reset_game(user,2);
				return(1);
			}
			else
			{
				write_user(user,"\n");
				clear_words();
				word_count=wordfind(inpstr);
				com_num = -1;
				if (user->whitespace) white_space(inpstr);
				if (user->mode == EWTOO || inpstr[0] == '.'
				  || inpstr[0] == ',' || (user->mode == IRC
				  && inpstr[0] == '/') || is_scut(user,inpstr))
					exec_com(user,inpstr);
				else
				{
					com_num = get_comnum("say");
					say(user,inpstr,0);
				}
				write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
				write_user(user,"~CW- ~FTYour move~CB:~RS ");
				user->game_op	= 2;
				return(1);
			}
		  }
		  move = atoi(word[0]);
		  if ((move>0) && (move<8))
		  {
			for(y = 6; y > 0; y--)
			{
				f = y;
				if (user->board[move][f] == 0
				  || user->c4_opponent->board[move][f] == 0)
					break;
			}
			if (f == 1 && user->board[move][f] != 0)
			{
				write_user(user,"~CW- ~FGYou have entered an invalid move..\n");
				write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
				write_user(user,"~CW- ~FTYour move~CB:~RS ");
				user->game_op = 2;
				return(1);
			}
			user->board[move][f] = 1;
			user->c4_opponent->board[move][f] = 2;
			print_board(user);
			print_board(user->c4_opponent);
			user->c4_turns++;
			user->c4_opponent->c4_turns++;
		  }
		  if ((move < 0) || (move >= 8))
		  {
			write_user(user,"~CW- ~FGYou have entered an invalid move..\n");
			write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user,"~CW- ~FTYour move~CB:~RS ");
			user->game_op = 2;
			return(1);
		  }
		  if (!connect_win(user))
		  {
			sprintf(text,"~CW- ~FGIt is now ~FM%s's~FG turn...\n",rname);
			write_user(user,text);
			write_user(user->c4_opponent,text);
			write_user(user->c4_opponent,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user->c4_opponent,"~CW- ~FTYour move~CB:~RS ");
			user->turn	= 0;
			user->game_op	= 0;
			rev_away(user);
			user->c4_opponent->turn	= 1;
			user->c4_opponent->game_op = 2;
			return(1);
		  }
		  if (connect_win(user))
		  {
			writeus(user,"~CW- ~FTYou have kicked ~FM%s%s~FT ass at connect 4...\n",rname,rname[strlen(rname)-1] == 's'?"'":"'s");
			writeus(user->c4_opponent,"~CW- ~FM%s~FT kicked your ass at connect 4..\n",name);
			user->c4_win++;
			user->c4_opponent->c4_lose++;
			reset_game(user->c4_opponent,2);
			reset_game(user,2);
			return(1);
		  }
		  else if (connect_win(user->c4_opponent))
		  {
			writeus(user->c4_opponent,"~CW- ~FTYou have kicked ~FM%s%s~FT ass at connect 4..\n",name,name[strlen(name)-1] == 's'?"'":"'s");
			writeus(user,"~CW- ~FM%s~FT kicked your ass at connect 4..\n",rname);
			user->c4_lose++;
			user->c4_opponent->c4_win++;
			reset_game(user->c4_opponent,2);
			reset_game(user,2);
			return(1);
		  }
		  else if (user->c4_turns >= 42)
		  {
			user->myint3++;
			user->c4_opponent->myint3++;
			sprintf(text,"~CW- ~FRAwww, a tie between you 2, guess you'll have to try again..\n");
			write_user(user,text);
			write_user(user->c4_opponent,text);
			reset_game(user->c4_opponent,2);
			reset_game(user,2);
			return(1);
		  }
		  else
		  {
			write_user(user,"~CW- ~CRYou have entered an invalid move..\n");
			write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user,"~CW- ~FTYour move~CB:~RS ");
			user->game_op	= 2;
			return(1);
		  }
		  return(1);
		case 3:
		  if (!strlen(inpstr))
		  {
			write_user(user,"~CW- ~FREnter a word for your opponent~CB:~RS ");
			user->game_op	= 3;
			return(1);
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			if ((u = get_user(user->hangman_opp)) == NULL)
				reset_game(user,3);
			else
			{
				WriteRoom(user->room,"~CW- ~FT %s ~FGis calling this game quits...\n",user->recap);
				reset_game(u,3);
				reset_game(user,3);
			}
			return(1);
		  }
		  if (strlen(word[0]) < 5)
		  {
			write_user(user,"~CW- ~FRYou need a tad bit longer of a word..\n");
			write_user(user,"~CW- ~FRRe-enter your word~CB:~RS ");
			user->game_op	= 3;
			return(1);
		  }
		  if (strlen(word[0]) > 30)
		  {
			write_user(user,"~CW- ~FRThats a tad bit too long...\n");
			write_user(user,"~CW- ~FRRe-enter your word~CB:~RS ");
			user->game_op	= 3;
			return(1);
		  }
		  if ((u = get_user(user->hangman_opp)) == NULL)
		  {
			write_user(user,"~CW- ~FRWe cannot find your opponent...resetting your game..\n");
			reset_game(user,3);
			return(1);
		  }
		  i = 0;
		  strcpy(user->hangman_word,word[0]);
		  strcpy(u->hangman_display,user->hangman_word);
		  for (i = 0; i < strlen(u->hangman_display); ++i)
			u->hangman_display[i] = '*';
		  writeus(user,"~CW- ~FTYour word for ~FM%s~FT is ~CW\"~CR%s~CW\"~FT..\n",u->recap,user->hangman_word);
		  WriteRoomExcept(user->room,user,"~CW- ~FM%s ~FGhas entered their word for ~FT%s~FG..\n",user->recap,u->recap);
		  user->hangman_stage	= -3; /* can't start another game now */
		  u->hangman_stage	= 0;  /* starting the game            */
		  sprintf(text,tphanged[u->hangman_stage],u->hangman_display,"None yet!",user->recap,user->hangman_wins,u->recap,u->hangman_wins,u->hangman_rnds);
		  write_room(user->room,text);
		  record(user->room,text);
		  write_user(u,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
		  write_user(u,"~CW- ~FGYour choice~CB:~RS ");
		  u->game_op	= 4;
		  user->game_op	= 0;
		  return(1);
		case 4:
		  if (!strlen(inpstr))
		  {
			write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
			write_user(user,"~CW- ~FGYour choice~CB:~RS ");
			user->game_op	= 4;
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			if (user->hangman_opp[0] != '\0')
			{
				if ((u = get_user(user->hangman_opp)) == NULL)
					reset_game(user,3);
				WriteRoom(user->room,"~CW- ~FT%s ~FGis calling this game quits..\n",user->recap);
				reset_game(u,3);
				reset_game(user,3);
				return(1);
			}
			else
			{
				write_user(user,"~CW- ~FGOk, you quit your hangman game..\n");
				reset_game(user,3);
				return(1);
			}
		  }
		  if (!strcasecmp(word[0],"status"))
		  {
			if (user->hangman_opp[0])
			{
				if ((u = get_user(user->hangman_opp)) == NULL)
				{
					write_user(user,"~CW- ~FRYour opponent left...\n");
					reset_game(user,3);
					return(1);
				}
				writeus(user,tphanged[user->hangman_stage],user->hangman_display,user->hangman_guessed,u->recap,u->hangman_wins,user->recap,user->hangman_wins,user->hangman_rnds);
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op	= 4;
				return(1);
			}
			else
			{
				writeus(user,hanged[user->hangman_stage],user->hangman_display,user->hangman_guessed);
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op	= 4;
				return(1);
			}
		  }
		  if (user->hangman_mode == 2)
		  {
			if ((u = get_user(user->hangman_opp)) == NULL)
			{
				write_user(user,"~CW- ~FRYour opponent left, resetting your game..\n");
				reset_game(user,3);
				return(1);
			}
			if (strlen(word[0])>1)
			{
				if (!strcasecmp(word[0],u->hangman_word))
				{
					user->hangman_wins++;
					user->hang_wins++;
					user->hangman_rnds--;
					u->hangman_rnds--;
					sprintf(text,tphanged[user->hangman_stage],u->hangman_word,user->hangman_guessed,u->recap,u->hangman_wins,user->recap,user->hangman_wins,user->hangman_rnds);
					write_room(user->room,text);
					record(user->room,text);
					sprintf(text,"~CW- ~FG%s ~FYhas guessed the word~FT %s~FY and has gotten it right..\n",user->recap,word[0]);
					write_room(user->room,text);
					record(user->room,text);
					writeus(user,"~CW-~FG Enter a word now for ~CT%s~CG to guess~CB:~RS ",u->recap);
					user->game_op=3;
					WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FYis now entering a word for ~FT%s ~FYto guess..\n",user->recap,u->recap);
					return(1);
				}
				else
				{
					write_user(user,"~CW- ~FGSorry, that isn't the word..\n");
					user->hangman_stage++;
					sprintf(text,"~CW- ~FG%s ~FYguessed the word ~CW\"~CR%s~CW\"~FY and that isn't this word..\n",user->recap,word[0]);
					write_room_except(user->room,user,text);
					record(user->room,text);
					sprintf(text,tphanged[user->hangman_stage],user->hangman_display,user->hangman_guessed,u->recap,u->hangman_wins,user->recap,user->hangman_wins,user->hangman_rnds);
					write_room(user->room,text);
					record(user->room,text);
					if (user->hangman_stage >= 7)
					{
						WriteRoom(user->room,"~CW- ~FG%s ~FYwas unable to guess the word and got HANGED..\n",user->recap);
						user->hangman_rnds--;
						user->hang_loses++;
						u->hangman_rnds--;
						u->hangman_wins++;
						if (user->hangman_rnds == 0)
						{
							WriteRoom(user->room,"~CW- ~FYThe game is over with ~FG%s ~FYwinning ~FR%d ~FYgame%s, and ~FG%s~FY winning~FR %d~FY game%s..\n",user->recap,user->hangman_wins,user->hangman_wins>1?"s":"",u->recap,u->hangman_wins,u->hangman_wins>1?"s":"");
							reset_game(user,3);
							reset_game(u,3);
							return(1);
						}
						else
						{
							WriteRoom(user->room,"~CW- ~FYIt is now ~FG%s's ~FYturn to guess..\n",u->recap);
							user->hangman_display[0]='\0';
							user->hangman_guessed[0]='\0';
							u->hangman_word[0]='\0';
							writeus(user,"~CW- ~FGEnter a word now for ~CT%s~CG to guess~CB:~RS ",u->recap);
							user->game_op=3;
							WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FYis now entering a word for ~FT%s ~FYto guess..\n",user->recap,u->recap);
							return(1);
						}
					}
					write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
					write_user(user,"~CW- ~FGYour choice~CB:~RS ");
					return(1);
				}
			}
			if (strstr(user->hangman_guessed,word[0]))
			{
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGValid options~CB: [~CRq~CB]~CRuit, ~CB[~CRs~CB]~CRtatus, the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op=4;
				return(1);
			}
			count	= 0;
			blanks	= 0;
			for (i = 0; i < strlen(u->hangman_word); ++i)
			{
				if (u->hangman_word[i] == word[0][0])
				{
					user->hangman_display[i] = u->hangman_word[i];
					++count;
				}
				if (user->hangman_display[i] == '*') ++blanks;
			}
			strcat(user->hangman_guessed,word[0]);
			if (count == 0)
			{
				user->hangman_stage++;
				write_user(user,"~CW- ~FRSorry that letter is not in this word..\n");
				sprintf(text,"~CW- ~FG%s ~FYguessed the letter ~CW\"~CR%s~CW\"~FY and it's not in this word..\n",user->recap,word[0]);
				write_room_except(user->room,user,text);
				record(user->room,text);
				sprintf(text,tphanged[user->hangman_stage],user->hangman_display,user->hangman_guessed,u->recap,u->hangman_wins,user->recap,user->hangman_wins,user->hangman_rnds);
				write_room(user->room,text);
				record(user->room,text);
				if (user->hangman_stage >= 7)
				{
					WriteRoom(user->room,"~CW- ~FG%s ~FYwas unable to guess the word and got HANGED!\n",user->recap);
					user->hangman_rnds--;
					user->hang_loses++;
					u->hangman_rnds--;
					u->hangman_wins++;
					if (user->hangman_rnds == 0)
					{
						WriteRoom(user->room,"~CW- ~FYThe game is over with ~FG%s ~FYwinning ~FR%d ~FYgame%s, and ~FG%s~FY winning~FR %d~FY game%s...\n",user->recap,user->hangman_wins,user->hangman_wins>1?"s":"",u->recap,u->hangman_wins,u->hangman_wins>1?"s":"");
						reset_game(user,3);
						reset_game(u,3);
						return(1);
					}
					else
					{
						WriteRoom(user->room,"~CW- ~FYIt is now ~FG%s's ~FYturn to guess..\n",u->recap);
						user->hangman_display[0] = '\0';
						user->hangman_guessed[0] = '\0';
						u->hangman_word[0]	 = '\0';
						writeus(user,"~CW- ~FGEnter a word now for ~FT%s~FG to guess~CB:~RS ",u->recap);
						user->game_op	= 3;
						WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FYis now entering a word for ~FT%s ~FYto guess.\n",user->recap,u->recap);
						return(1);
					}
				}
				return(1);
			}
			writeus(user,"~CW- ~FRAlright, there %s %d occurence%s of the letter ~CW\"~CR%s~CW\"\n",count>1?"were":"was",count,count>1?"s":"",word[0]);
			if (count == 1) sprintf(text,"~CW- ~FM%s ~FYguessed the letter ~CW\"~CR%s~CW\"~FY and it was in the word once..\n",user->recap,word[0]);
			else sprintf(text,"~CW- ~FM%s~FY guessed the letter ~CW\"~CR%s~CW\"~FY and it was in the word %d times..\n",user->recap,word[0],count);
			write_room_except(user->room,user,text);
			record(user->room,text);
			sprintf(text,tphanged[user->hangman_stage],user->hangman_display,user->hangman_guessed,u->recap,u->hangman_wins,user->recap,user->hangman_wins,user->hangman_rnds);
			write_room(user->room,text);
			record(user->room,text);
			if (blanks == 0)
			{
				sprintf(text,"~CW- ~FM%s ~FYhas guessed the word ~FG%s ~FYwithout being hanged.\n",user->recap,u->hangman_word);
				write_room(user->room,text);
				record(user->room,text);
				user->hangman_rnds--;
				user->hangman_wins++;
				user->hang_wins++;
				u->hangman_rnds--;
				if (user->hangman_rnds == 0)
				{
					sprintf(text,"~CW- ~FYThe game is over with ~FG%s ~FYwinning ~FR%d ~FYgame%s, and ~FG%s~FY winning~FR %d~FY game%s.\n",user->recap,user->hangman_wins,user->hangman_wins>1?"s":"",u->recap,u->hangman_wins,u->hangman_wins>1?"s":"");
					write_room(user->room,text);
					record(user->room,text);
					reset_game(user,3);
					reset_game(u,3);
					return(1);
				}
				else
				{
					WriteRoom(user->room,"~CW- ~FYIt is now ~FG%s's ~FYturn to guess.\n",u->recap);
					user->hangman_display[0]='\0';
					user->hangman_guessed[0]='\0';
					u->hangman_word[0]='\0';
					writeus(user,"~CW- ~FGEnter a word now for ~FT%s~FG to guess~CB:~RS ",u->recap);
					user->game_op=3;
					WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FYis now entering a word for ~FT%s ~FYto guess..\n",user->recap,u->recap);
					return(1);
				}
			}
			else
			{
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op	= 4;
				return(1);
			}
			return(1);
		  }
		  if (user->hangman_mode == 3)
		  {
			if (strlen(word[0]) > 1)
			{
				if (!strcasecmp(word[0],user->hangman_word))
				{
					writeus(user,hanged[user->hangman_stage],user->hangman_word,user->hangman_guessed);
					user->hang_wins++;
					writeus(user,"~CW- ~FGYou have guessed the word ~FT%s ~FGwithout being hanged..\n",word[0]);
					reset_game(user,3);
					return(1);
				}
				else
				{
					user->hangman_stage++;
					write_user(user,"~CW- ~FRSorry that is not this word.\n");
					writeus(user,hanged[user->hangman_stage],user->hangman_display,user->hangman_guessed);
					if (user->hangman_stage >= 7)
					{
						writeus(user,"~CW-~FG You were unable to guess the word ~CW'~CR%s~CW' ~FGand you got HANGED.\n",user->hangman_word);
						reset_game(user,3);
						user->hang_loses++;
						return(1);
					}
					write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
					write_user(user,"~CW- ~FGYour choice~CB:~RS ");
					user->game_op	= 4;
					return(1);
				}
				return(1);
			}
			if (strstr(user->hangman_guessed,word[0]))
			{
				write_user(user,"~CW- ~FRYou have already guessed that letter...\n");
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op = 4;
				return(1);
			}
			count	= 0;
			blanks	= 0;
			for (i = 0; i < strlen(user->hangman_word); ++i)
			{
				if (user->hangman_word[i] == word[0][0])
				{
					user->hangman_display[i] = user->hangman_word[i];
					++count;
				}
				if (user->hangman_display[i] == '*') ++blanks;
			}
			strcat(user->hangman_guessed,word[0]);
			if (count == 0)
			{
				user->hangman_stage++;
				write_user(user,"~CW- ~FRSorry that letter is not in this word!\n");
				writeus(user,hanged[user->hangman_stage],user->hangman_display,user->hangman_guessed);
				if (user->hangman_stage >= 7)
				{
					writeus(user,"~CW- ~FGYou were unable to guess the word ~CW'~CR%s~CW' ~FGand you got HANGED.\n",user->hangman_word);
					reset_game(user,3);
					user->hang_loses++;
					return(1);
				}
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op = 4;
				return(1);
			}
			if (count == 1) sprintf(text,"~CW- ~FRAlright, there was 1 occurence of the letter ~CW\"~CR%s~CW\"\n",word[0]);
			else sprintf(text,"~CW- ~FRAlright, there were %d occurences of the letter ~CW\"~CR%s~CW\"\n",count,word[0]);
			write_user(user,text);
			writeus(user,hanged[user->hangman_stage],user->hangman_display,user->hangman_guessed);
			if (blanks == 0)
			{
				writeus(user,"~CW- ~FYYou have guessed the word ~CR%s ~FYwithout being hanged.\n",user->hangman_word);
				user->hang_wins++;
				reset_game(user,3);
			}
			else
			{
				write_user(user,"~CW- ~FGValid options~CB: ~CRquit~CW, ~CRstatus~CW,~FG the word to guess the word, or a letter.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				user->game_op = 4;
				return(1);
			}
			return(1);
		  }
		  return(1);
		case 5:
		  /*
		    GAEN puzzle source.. Originally from Sabin converted to
		    RaMTITS with his permission by me :-)  Thanks Sabin
		  */
		  if (!strlen(inpstr) || !strncasecmp(word[0],"status",2))
		  {
			write_user(user,"~CW- ~FTYour current puzzle status is~CB:~RS\n");
			puzzle_status(user);
			write_user(user,"~CW- ~FTYour moves~CB: [~CRL~CB]~FTeft, ~CB[~CRR~CB]~FTight, ~CB[~CRD~CB]~FTown, ~CB[~CRU~CB]~FTp, or ~CB[~CRQ~CB]~FTuit.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			return(1);
		  }
		  if (!strncasecmp(word[0],"quit",1))
		  {
			write_user(user,"~CW-~FT You decide to no longer play your puzzle game...\n");
			puzzle_quit(user);
			return(1);
		  }
		  for (i = 0; i < 4; i++)
		  {
			if (moves[i] == word[0][0])
			{
				puzzle_play(user,i);
				return(1);
			}
		  }
		  write_user(user,"~CW- ~FTYou entered an invalid moved..\n");
		  write_user(user,"~CW- ~FTYour moves~CB: [~CRL~CB]~FTeft, ~CB[~CRR~CB]~FTight, ~CB[~CRU~CB]~FTp, ~CB[~CRD~CB]~FTown, or ~CB[~CRQ~CB]~FTuit.\n");
		  write_user(user,"~CW- ~FTYour choice~CB:~RS ");
		  return(1);
		case 6:
		  /*
		    Blackjack.. Original source code provided by Andy (Amnuts)
		    and I converted it to my wants and needs.. Thanks Andy :-)
		  */
		  if (!strlen(inpstr))
		  {
		 	if (user->bjack == NULL)
			{
				write_user(user,"~CW- ~FGAn error occurred.. aborting..\n");
				user->game_op	= 0;
				rev_away(user);
				return(1);
			}
			write_user(user,"~CW- ~FTYour current status is:\n");
			show_bjack_status(user);
			write_user(user,"~CW- ~FTYour options~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			return(1);
		  }
		  if (!strncasecmp(word[0],"quit",1))
		  {
			write_user(user,"~CW-~FT Alrighty then... you leave the Dragons Den Casino...\n");
			WriteRoomExcept(user->room,user,"~CW- ~FT%s~FT leaves the Dragons Den Casino...\n",user->invis==Invis?invisname:user->recap);
			if (user->bjack != NULL) destruct_bjgame(user);
			prompt(user);
			user->game_op	= 0;
			rev_away(user);
			return(1);
		  }
		  if (!strncasecmp(word[0],"bet",1))
		  {
		 	if (user->bjack == NULL)
			{
				write_user(user,"~CW- ~FTAn error occurred.. aborting..\n");
				user->game_op=0;
				rev_away(user);
				return(1);
			}
			/* let them raise their bet. */
			writeus(user,"~CW- ~FTThe current bet is %d chips ~CW(~CRmax of 1000~CW)\n",user->bjack->bet);
			write_user(user,"~CW- ~FTHow much would you like to bet?~CB:~RS ");
			user->game_op	= 7;
			return(1);
		  }
		  if (!strncasecmp(word[0],"deal",1))
		  {
		 	if (user->bjack==NULL)
			{
				write_user(user,"~CW- ~FGAn error occurred.. aborting..\n");
				user->game_op = 0;
				rev_away(user);
				return(1);
			}
			if (user->chips<user->bjack->bet)
			{
				write_user(user,"~CW- ~FTAwww.. you don't have enough to play blackjack.\n");
				user->game_op	= 0;
				rev_away(user);
				return(1);
			}
			user->chips -= user->bjack->bet;
			writeus(user,"~CW-~FT You currently putin %d chips..\n",user->bjack->bet);
			write_user(user,"~CW- ~FGShuffling cards...\n");
			shuffle_bjcards(user->bjack);
			user_total = check_bjack_total(user,0);
			writeus(user,"~CW- ~FTYour current blackjack hand is~CB: [~FMTotal of ~CR%d~CB]\n",user_total);
			show_bjack_cards(user,0,1);
			write_user(user,"\n~CW- ~FTThe dealers hand is...\n");
			show_bjack_cards(user,1,1);
			if (user_total == 21)
			{
				writeus(user,"~CW- ~FGYou just got a blackjack.. The Dragons Den Casino pays you %d chips!\n",user->bjack->bet*3);
				user->chips += (user->bjack->bet*3);
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->bjack->bet = 10;
				user->game_op	 = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FGAt this stage you can ~CB[~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				return(1);
			}
			write_user(user,"~CW- ~FTYour choices~CB: [~CRH~CB]~FTit, ~CB[~CRSt~CB]~FTand, ~CB[~CRSu~CB]~FTrrender, or ~CB[~CRD~CB]~FTouble down.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			user->game_op = 8;
			return(1);
		  }
		  write_user(user,"~CW- ~FTYou entered an invalid option....\n");
		  write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
		  write_user(user,"~CW- ~FTYour choice~CB:~RS ");
		  return(1);
		case 7: /* User is entering a bet */
		  if (!strlen(inpstr))
		  {
			write_user(user,"~CW- ~FTOk, you decide not to enter a bet, returning to the start menu..\n");
			write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			user->game_op = 6;
			return(1);
		  }
		  bet = atoi(word[0]);
		  if (bet < 6 || bet > 1000)
		  {
			write_user(user,"~CW- ~FTThe minimum bet is 6 chips, the maximum is 1000.\n");
		 	write_user(user,"~CW- ~FTRe-enter your bet~CB:~RS ");
			return(1);
		  }
		  user->bjack->bet = bet;
		  writeus(user,"~CW- ~FTThe bet is now at %d chips... returning the main menu..\n",user->bjack->bet);
		  write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
		  write_user(user,"~CW- ~FTYour choice~CB:~RS ");
		  user->game_op = 6;
		  return(1);
		case 8: /* The actual game play */
		  if (user->bjack == NULL)
		  {
			write_user(user,"~CW- ~FGAn error occurred.. aborting..\n");
			user->game_op = 0;
			rev_away(user);
			return(1);
		  }
		  if (!strlen(inpstr))
		  {
			user_total = check_bjack_total(user,0);
			writeus(user,"~CW- ~FTYour current blackjack hand is~CB: [~FMTotal of ~CR%d~CB]\n",user_total);
			show_bjack_status(user);
			write_user(user,"~CW- ~FTYour choices~CB: [~CRH~CB]~FTit, ~CB[~CRSt~CB]~FTtand, ~CB[~CRQ~CB]~FTuit, ~CB[~CRSu~CB]~FTrrender, or ~CB[~CRD~CB]~FTouble down.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			return(1);
		  }
		  if (!strncasecmp(word[0],"double",1))
		  {
			if (user->chips < user->bjack->bet)
			{
				write_user(user,"~CW-~FT You can't afford to double your bet!\n");
				write_user(user,"~CW- ~FTYour choices~CB: [~CRH~CB]~FTit, ~CB[~CRSt~CB]~FTtand, ~CB[~CRQ~CB]~FTuit, ~CB[~CRSu~CB]~FTrrender, or ~CB[~CRD~CB]~FTouble down.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			user->chips -= user->bjack->bet;
			user->bjack->bet += user->bjack->bet;
			cnt = blank = 0;
			for (i = 0; i < 5; i++)
			{
				if (user->bjack->hand[i] == -1) blank++;
				else cnt++;
			}
			if (blank == 0)
			{
				writeus(user,"~CW- ~CRCongratz..~FT You got a five card hand, so you win %d chips..\n",((user->bjack->bet*2)+(user->bjack->bet/2)));
				user->chips += ((user->bjack->bet*2)+(user->bjack->bet/2));
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			user->bjack->hand[cnt]=user->bjack->deck[user->bjack->cardpos];
			++user->bjack->cardpos;
			user_total=check_bjack_total(user,0);
			writeus(user,"~CW- ~FTYour current blackjack hand is~CB: [~FMTotal of ~CR%d~CB]\n",user_total);
			show_bjack_cards(user,0,0);
			if (user_total > 21)
			{
				writeus(user,"~CW- ~FTCrappy deal.. you busted and lost your bet of %d chips..\n",user->bjack->bet);
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			if (++cnt >= 5)
			{
				writeus(user,"~CW- ~CRCongratz..~FT You got a five card hand, so you win %d chips..\n",((user->bjack->bet*2)+(user->bjack->bet/2)));
				user->money += ((user->bjack->bet*2)+(user->bjack->bet/2));
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			write_user(user,"~CW- ~FTThe dealer now takes his turn...\n");
			goto CARD_SKIP;
			return(1);
		  }
		  if (!strncasecmp(word[0],"hit",1))
		  {
			cnt = blank = 0;
			for (i = 0; i < 5; i++)
			{
				if (user->bjack->hand[i] == -1) blank++;
				else cnt++;
			}
			if (blank == 0)
			{
				writeus(user,"~CW- ~CRCongratz..~FT You got a five card hand, so you win %d chips..\n",((user->bjack->bet*2)+(user->bjack->bet/2)));
				user->money+=((user->bjack->bet*2)+(user->bjack->bet/2));
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FGAt this stage you can ~CB[~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FGYour choice~CB:~RS ");
				return(1);
			}
			user->bjack->hand[cnt] = user->bjack->deck[user->bjack->cardpos];
			++user->bjack->cardpos;
			user_total = check_bjack_total(user,0);
			writeus(user,"~CW- ~FTYour current blackjack hand is~CB: [~FMTotal of ~CR%d~CB]\n",user_total);
			show_bjack_cards(user,0,0);
			if (user_total>21)
			{
				writeus(user,"~CW- ~FTCrappy deal.. you busted and lost your bet of %d chips..\n",user->bjack->bet);
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52;i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			if (++cnt >= 5)
			{
				writeus(user,"~CW- ~CRCongratz..~FT You got a five card hand, so you win %d chips..\n",((user->bjack->bet*2)+(user->bjack->bet/2)));
				user->money += ((user->bjack->bet*2)+(user->bjack->bet/2));
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			write_user(user,"~CW- ~FTYour choices~CB: [~CRH~CB]~FTit, ~CB[~CRSt~CB]~FTtand, ~CB[~CRQ~CB]~FTuit, ~CB[~CRSu~CB]~FTrrender, or ~CB[~CRD~CB]~FTouble down.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			return(1);
		  }
		  if (!strncasecmp(word[0],"stand",2))
		  {
			write_user(user,"~CW- ~FTThe Dragons Den dealer steps up and takes his turn...\n");
			user_total = check_bjack_total(user,0);

			CARD_SKIP:
			  cnt = blank = 0;
			  for (i = 0; i < 5; i++)
			  {
				if (user->bjack->dhand[i] == -1) blank++;
				else cnt++;
			  }
			  if (blank == 0)
			  {
				writeus(user,"~CW- ~FTCrappy deal.. the dealer got a five card hand..\n~CW- ~FTYou lose your bet of %d chips..\n",user->bjack->bet);
				write_user(user,"~CW-~FT Re-starting your game...\n");
				user->game_op = 6;
				for (i = 0; i < 52; i++)
					user->bjack->deck[i] = i;
				for (i = 0; i < 5; i++)
				{
					user->bjack->hand[i] = -1;
					user->bjack->dhand[i]= -1;
				}
				user->bjack->cardpos = 0;
				write_user(user,"~CW- ~FTAt this stage you can ~CB[~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
				write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				return(1);
			}
			while ((dealer_total = check_bjack_total(user,1)) < 17)
			{
				user->bjack->dhand[cnt] = user->bjack->deck[user->bjack->cardpos];
				++user->bjack->cardpos;
				++cnt;
			}
			writeus(user,"\n~CW-~FT The dealer's hand is~CB: [~FMTotal of ~CR%d~CB]\n",dealer_total);
			show_bjack_cards(user,1,0);
			if (dealer_total > 21)
			{
				writeus(user,"~CW- ~FTCongratz... the dealer went and busted so you win %d chips..\n",user->bjack->bet*2);
				user->chips += (user->bjack->bet*2);
			}
			else if (dealer_total>user_total)
			{
				writeus(user,"~CW- ~FTCrappy deal.. the dealer beat your hand...\n~CW- ~FTYou lose your bet of %d chips..\n",user->bjack->bet);
			}
			else if (user_total>dealer_total)
			{
				writeus(user,"~CW- ~FTCongratz... You beat the dealers hand so you win %d chips..\n",user->bjack->bet*2);
				user->chips += (user->bjack->bet*2);
			}
			else
			{
				writeus(user,"~CW- ~FTPush... you tied the dealers score so you get your %d chips back...\n",user->bjack->bet);
				user->chips += user->bjack->bet;
			}
			write_user(user,"~CW-~FT Re-starting your game...\n");
			user->game_op = 6;
			for (i = 0; i < 52; i++) user->bjack->deck[i] = i;
			for (i = 0; i < 5; i++)
			{
				user->bjack->hand[i] = -1;
				user->bjack->dhand[i]= -1;
			}
			user->bjack->cardpos = 0;
			write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			return(1);
		  }
		  if (!strncasecmp(word[0],"surrender",2))
		  {
			user->chips += (user->bjack->bet/2);
			writeus(user,"~CW-~FT You surrender your game of blackjack and loose half of your bet.. all %d chips..\n",(user->bjack->bet/2));
			write_user(user,"~CW-~FT Re-starting your game...\n");
			user->bjack->bet= 10;
			user->game_op	= 6;
			for (i = 0; i < 52; i++) user->bjack->deck[i] = i;
			for (i = 0; i < 5; i++)
			{
				user->bjack->hand[i] = -1;
				user->bjack->dhand[i]= -1;
			}
			user->bjack->cardpos = 0;
			write_user(user,"~CW- ~FTYour choices~CB: [~CRD~CB]~FTeal, ~CB[~CRB~CB]~FTet, or ~CB[~CRQ~CB]~FTuit.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
			return(1);
		  }
		  if (!strncasecmp(word[0],"quit",1))
		  {
			write_user(user,"~CW-~FT Alrighty then... you leave the Dragons Den Casino...\n");
			WriteRoomExcept(user->room,user,"~CW- ~FT%s~FT leaves the Dragons Den Casino...\n",user->invis==Invis?invisname:user->recap);
			destruct_bjgame(user);
			prompt(user);
			user->game_op = 0;
			rev_away(user);
			return(1);
		  }
		  write_user(user,"~CW- ~FTYou entered an invalid option....\n");
		  write_user(user,"~CW- ~FTYour choices~CB: [~CRH~CB]~FTit, ~CB[~CRSt~CB]~FTtand, ~CB[~CRQ~CB]~FTuit, ~CB[~CRSu~CB]~FTrrender, or ~CB[~CRD~CB]~FTouble down.\n");
		  write_user(user,"~CW- ~FTYour choice~CB:~RS ");
		  return(1);
	}
	return(0);
}

/* Lets start with tic tac toe shall we? */
void tictac(User user,char *inpstr,int rt)
{
	User u;
	char *name,*rname;
	int turn = 0;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count<2 && user->tic_opponent == NULL)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if (word_count<2 && user->tic_opponent->tic_opponent)
	{
		writeus(user,usage,command[com_num].name,"~CR-accept");
		writeus(user,usage,command[com_num].name,"~CR-reject");
		return;
	}
	if (!strncasecmp(word[1],"-accept",2))
	{
		if (user->tchallenge == 1)
		{
			write_user(user,"~CW- ~FTYou can't accept the game, you challenged it.\n");
			return;
		}
		if (user->tic_opponent == NULL)
		{
			write_user(user,"~FGWho are you accepting a game with??\n");
			return;
		}
		if (user->tic_opponent->invis == Invis) rname = invisname;
		else rname = user->tic_opponent->recap;
		if (user->tic_opponent->first)
		{
			write_user(user,"~FGYou are already playing tic tac toe with someone.\n");
			return;
		}
		writeus(user->tic_opponent,"~FT%s~FR accepts your game of tic tac toe.\n",name);
		writeus(user,"~FRYou accept a game of tic tac toe with ~FT%s\n",rname);
		user->tic_opponent->tchallenge = 0;
		print_tic(user);
		print_tic(user->tic_opponent);
		turn = ran(2);	/* figure out who goes first */
		if (turn == 1 || turn == 0) /* turn = 0 shouldn't happen */
		{
			sprintf(text,"~CW-~FG It is now ~FM%s%s ~FGturn.\n",rname,rname[strlen(rname)-1] == 's'?"'":"'s");
			write_user(user,text);
			write_user(user->tic_opponent,text);
			user->tic_opponent->turn = 1;
			write_user(user->tic_opponent,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user->tic_opponent,"~CW- ~FTYour move~CB:~RS ");
			user->tic_opponent->game_op = 1;
		}
		else
		{
			sprintf(text,"~CW-~FG It is now ~FM%s%s ~FGturn.\n",name,name[strlen(name)-1] == 's'?"'":"'s");
			write_user(user,text);
			write_user(user->tic_opponent,text);
			user->turn = 1;
			write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user,"~CW- ~FTYour move~CB:~RS ");
			user->game_op = 1;
		}
		return;
	}
	if (!strncasecmp(word[1],"-reject",2))
	{
		if (user->tchallenge == 1)
		{
			write_user(user,"~CW- ~FTYou can't reject the game, you challenged it.\n");
			return;
		}
		if (user->tic_opponent == NULL)
		{
			write_user(user,"~FGWho are you rejecting a game from??\n");
			return;
		}
		if (user->tic_opponent->invis == Invis) rname = invisname;
		else rname = user->tic_opponent->recap;
		writeus(user->tic_opponent,"~FT%s~FR rejects your game of tic tac toe!\n",name);
		writeus(user,"~FRYou reject playing tic tac toe with ~FT%s\n",rname);
		user->tic_opponent->tchallenge = 0;
		user->tic_opponent->tic_opponent = NULL;
		return;
	}
	if (user->tic_opponent)
	{
		if (user->tic_opponent->tic_opponent == user)
		{
			write_user(user,"~FMTo accept a tic tac toe game, type ~CW'~FG.tictac -accept~CW'\n");
			write_user(user,"~FMTo reject a tic tac toe game, type ~CW'~FG.tictac -reject~CW'\n");
			return;
		}
		if (user->tic_opponent->tic_opponent != user)
		{
			write_user(user,"~FGSorry, that person is already playing a game of tic tac toe.\n");
			return;
		}
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FGHasn't anyone ever told you not to play with yourself?\n");
		return;
	}
	if (u->tic_opponent != NULL)
	{
		write_user(user,"~FGSorry, that person is already playing a game of tic tac toe.\n");
		return;
	}
	user->tic_opponent = u;
	if (user->tic_opponent->level<USER
	  && !is_allowed(user->tic_opponent,command[com_num].name))
	{
		write_user(user,"~FMThat user does not have that command..sorry!\n");
		user->tic_opponent = NULL;
		return;
	}
	else
	{
		if (user->tic_opponent->invis == Invis) rname = invisname;
		else rname = user->tic_opponent->recap;
		user->tic_opponent->tic_opponent = user;
		writeus(user->tic_opponent,"~FT%s~FY wants to play a game of tic tac toe with you!\n",name);
		write_user(user->tic_opponent,"~FGTo accept the game, type~CB:~CW '~FR.tictac -accept~CW'\n");
		write_user(user->tic_opponent,"~FGTo reject the game, type~CB:~CW '~FR.tictac -reject~CW'\n");
		writeus(user,"~FYYou ask ~FT%s~FY if they want to play tic tac toe!\n",rname);
		user->tchallenge = 1;
		return;
	}
	if (user->tic_opponent)
	{
		write_user(user,"~FRYou are already playing a game with somebody!\n");
		return;
	}
}

/* Checks to make sure that everything is legal */
int legal_tic(char *array,int move)
{
	int count1 = 0,count2 = 0;
	int i;

	if (array[move-1] == 1 || array[move-1] == 2) return(0);
	for (i = 0; i < 9; i++)
	{
		if (array[i] == 1)  count1++;
		else if (array[i] == 2)  count2++;
	}
	return(1);
}

/* checks for the winner */
int win_tic(char *array)
{
	int i,j;
	int person;

	for (person = 1; person < 3; person++)
	{
		for (i = 0; i < 3; i++)
			for (j = 0 ; j < 3; j++)
			{
				if (array[i*3+j] != person) break;
				if (j == 2) return(person);
			}
		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
			{
				if (array[j*3+i] != person) break;
				if (j == 2) return(person);
			}
		if (array[0] == person && array[4] == person
		  && array[8] == person)
			return(person);
		if (array[2] == person && array[4] == person
		  && array[6] == person)
			return(person);
	}
	for (i = 0, j = 0; i < 9; i++)
	{
		if (array[i] == 1 || array[i] == 2) j++;
	}
	if (j == 9) return(3);
	return(0);
}

/* Prints out our board */
void print_tic(User user)
{
	char array[10];
	int i;

	for (i = 0; i < 9; i++)
	{
		if (user->array[i] == 1) array[i] = 'X';
		else if (user->array[i] == 2) array[i] = 'O';
		else array[i]=' ';
	}
	write_user(user,"\n");
	if (user->ansi_on)
	{
		write_user(user,"~CBÉÍÑÍÍÍÍÑÍÑÍÍÍÍÑÍÑÍÍÍÍ»\n");
		write_user(user,"~CBº~CT1~CB³    ³~CT2~CB³    ³~CT3~CB³    º\n");
		writeus(user,"~CBÇÄÙ~FG %c  ~CBÃÄÙ~FG %c  ~CBÃÄÙ~FG %c  ~CBº\n",array[0],array[1],array[2]);
		write_user(user,"~CBº      ³      ³      º\n");
		write_user(user,"~CBÇÄÂÄÄÄÄÅÄÂÄÄÄÄÅÄÂÄÄÄÄ¶\n");
		write_user(user,"~CBº~CT4~CB³    ³~CT5~CB³    ³~CT6~CB³    º\n");
		writeus(user,"~CBÇÄÙ~FG %c  ~CBÃÄÙ~FG %c  ~CBÃÄÙ~FG %c  ~CBº\n",array[3],array[4],array[5]);
		write_user(user,"~CBº      ³      ³      º\n");
		write_user(user,"~CBÇÄÂÄÄÄÄÅÄÂÄÄÄÄÅÄÂÄÄÄÄ¶\n");
		write_user(user,"~CBº~CT7~CB³    ³~CT8~CB³    ³~CT9~CB³    º\n");
		writeus(user,"~CBÇÄÙ~FG %c  ~CBÃÄÙ~FG %c  ~CBÃÄÙ~FG %c  ~CBº\n",array[6],array[7],array[8]);
		write_user(user,"~CBº      ³      ³      º\n");
		write_user(user,"~CBÈÍÍÍÍÍÍÏÍÍÍÍÍÍÏÍÍÍÍÍÍ¼\n");
	}
	else
	{
		write_user(user,"~CB ____________________ \n");
		write_user(user,"~CB|~CT1~CB|    |~CT2~CB|    |~CT3~CB|    |\n");
		writeus(user,"~CB|~ ~FG %c  ~CB|~ ~FG %c  ~CB|~  ~FG%c  ~CB|\n",array[0],array[1],array[2]);
		write_user(user,"~CB|____________________|\n");
		write_user(user,"~CB|~CT4~CB|    |~CT5~CB|    |~CT6~CB|    |\n");
		writeus(user,"~CB|~  ~FG%c  ~CB|~  ~FG%c  ~CB|~  ~FG%c  ~CB|\n",array[3],array[4],array[5]);
		write_user(user,"~CB|____________________|\n");
		write_user(user,"~CB|~CT7~CB|    |~CT8~CB|    |~CT9~CB|    |\n");
		writeus(user,"~CB|~  ~FG%c  ~CB|~  ~FG%c  ~CB|~  ~FG%c  ~CB|\n",array[6],array[7],array[8]);
		write_user(user,"~CB`--------------------'\n");
	}
}

/*
   resets the game...added an type int..cuz that way it can reset either
   tic tac toe, or connect 4, or chess...
*/
void reset_game(User user,int type)
{
	int x,y;

	switch(type)
	{
		case 1:
		  write_user(user,"~FR Resetting your tic tac toe game....\n");
		  strcpy(user->array,"000000000");
		  user->tic_opponent	= NULL;
		  user->first		= 0;
		  user->game_op		= 0;
		  rev_away(user);
		  return;
		case 2:
		  write_user(user,"~FR Resetting your connect 4 game...\n");
		  for (y = 0; y < 7; y++)
			for (x = 0; x < 8; x++)
				user->board[x][y] = 0;
		  user->c4_opponent	= NULL;
		  user->turn		= 0;
		  user->game_op		= 0;
		  user->c4_turns	= 0;
		  rev_away(user);
		  return;	
		case 3:
		  write_user(user,"~FR Resetting your hangman game...\n");
		  user->hangman_opp[0]		= '\0';
		  user->hangman_guessed[0]	= '\0';
		  user->hangman_word[0]		= '\0';
		  user->hangman_display[0]	= '\0';
		  user->hangman_stage		= -2;
		  user->hangman_rnds		= 0;
		  user->hangman_wins		= 0;
		  user->hangman_mode		= 0;
		  user->game_op			= 0;
		  rev_away(user);
		  return;
		case 4:
		  write_user(user,"~FR Resetting your fighting status...\n");
		  fight.first_user	= NULL;
		  fight.second_user	= NULL;
		  fight.issued		= 0;
		  fight.time		= 0;
		  return;
	}
}

/* guess I'll do connect 4 now... */
void connect_four(User user,char *inpstr,int rt)
{
	User u;
	char *name,*rname;
	int turn = 0;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count<2 && user->c4_opponent == NULL)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if (word_count<2 && user->c4_opponent->c4_opponent)
	{
		writeus(user,usage,command[com_num].name,"~CR-accept");
		writeus(user,usage,command[com_num].name,"~CR-reject");
		return;
	}
	if (!strcasecmp(word[1],"-accept"))
	{
		if (user->cchallenge == 1)
		{
			write_user(user,"~CW- ~FTYou can't accept the game, you challenged it.\n");
			return;
		}
		if (user->c4_opponent == NULL)
		{
			write_user(user,"~FGWho are you accepting a game with??\n");
			return;
		}
		if (user->c4_opponent->turn)
		{
			write_user(user,"~FMYou are already playing with someone silly!\n");
			return;
		}
		if (user->c4_opponent->invis == Invis) rname = invisname;
		else rname = user->c4_opponent->recap;
		writeus(user->c4_opponent,"~FT%s~FR accepts your game of connect 4.\n",name);
		writeus(user,"~FRYou accept a game of connect 4 with ~FT%s.\n",rname);
		user->c4_opponent->cchallenge = 0;
		print_board(user);
		print_board(user->c4_opponent);
		turn = ran(2);	/* figure out who goes first */
		if (turn == 1 || turn == 0) /* turn = 0 shouldn't happen */
		{
			sprintf(text,"~CW-~FG It is now ~FM%s%s ~FGturn.\n",rname,rname[strlen(rname)-1] == 's'?"'":"'s");
			write_user(user,text);
			write_user(user->c4_opponent,text);
			user->c4_opponent->turn = 1;
			write_user(user->c4_opponent,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user->c4_opponent,"~CW- ~FTYour move~CB:~RS ");
			user->c4_opponent->game_op = 2;
		}
		else
		{
			sprintf(text,"~CW-~FG It is now ~FM%s%s ~FGturn.\n",name,name[strlen(name)-1] == 's'?"'":"'s");
			write_user(user,text);
			write_user(user->c4_opponent,text);
			user->turn = 1;
			write_user(user,"~CW- ~FGValid options~CB: [~CRb~CB]~CRoard, ~CB[~CRq~CB]~CRuit, or # of move.\n");
			write_user(user,"~CW- ~FTYour move~CB:~RS ");
			user->game_op = 2;
		}
		return;
	}
	if (!strcasecmp(word[1],"-reject"))
	{
		if (user->cchallenge == 1)
		{
			write_user(user,"~CW- ~FTYou can't reject the game, you challenged it.\n");
			return;
		}
		if (user->c4_opponent == NULL)
		{
			write_user(user,"~FGWho are you rejecting a game from??\n");
			return;
		}
		if (user->c4_opponent->invis == Invis) rname = invisname;
		else rname = user->c4_opponent->recap;
		writeus(user->c4_opponent,"~FT%s~FR rejects your game of connect 4.\n",name);
		writeus(user,"~FRYou reject playing connect 4 with ~FT%s\n",rname);
		user->c4_opponent->cchallenge = 0;
		user->c4_opponent->c4_opponent= NULL;
		return;
	}
	if (user->c4_opponent)
	{
		if (user->c4_opponent->c4_opponent == user)
		{
			write_user(user,"~FMTo accept a connect 4 game, type ~CW'~FG.connect4 -accept~CW'\n");
			return;
		}
		if (user->c4_opponent->c4_opponent != user)
		{
			write_user(user,"~FGSorry, that person is already playing a game of connect 4.\n");
			return;
		}
		write_user(user,"~FGYou are already playing a game of connect4!\n");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FGHasn't anyone ever told you not to play with yourself?\n");
		return;
	}
	if (u->c4_opponent != NULL)
	{
		write_user(user,"~FGSorry, that user is already playing a game of connect 4.\n");
		return;
	}
	user->c4_opponent = u;
	if (user->c4_opponent->level<USER
	  && !is_allowed(user->c4_opponent,command[com_num].name))
	{
		write_user(user,"~FMThat user does not have that command..sorry!\n");
		return;
	}
	else
	{
		if (user->c4_opponent->invis == Invis) rname = invisname;
		else rname = user->c4_opponent->recap;
		user->c4_opponent->c4_opponent = user;
		writeus(user->c4_opponent,"~FT%s~CT wants to play a game of connect 4 with you!\n",name);
		write_user(user->c4_opponent,"~CGTo accept the game, type~CB:~CW '~FR.connect4 -accept~CW'\n");
		write_user(user->c4_opponent,"~CGTo reject the game, type~CB:~CW '~FR.connect4 -reject~CW'\n");
		writeus(user,"~FYYou ask ~FT%s~FY if they want to play a game of connect 4!\n",rname);
		user->cchallenge = 1;
		return;
	}
}

/* Prints the board out */
void print_board(User user)
{
	int x,y;

	write_user(user,"\n");
	write_user(user,"~FR+-=-+-=-+-=-+-=-+-=-+-=-+-=-+\n");
	for (y = 1; y < 7;y++)
	{
		for (x = 1; x < 8; x++)
		{
			write_user(user,"~FR|");
			if (user->board[x][y] == 0) write_user(user,"   ");
			if (user->board[x][y] == 1) write_user(user,"~CG X ");
			if (user->board[x][y] == 2) write_user(user,"~CT O ");
			if (x == 7) write_user(user,"~FR|\n");
		}
		write_user(user,"~FR+-=-+-=-+-=-+-=-+-=-+-=-+-=-+\n");
	}
	write_user(user,"~CM  1   2   3   4   5   6   7  \n");
}

/* scans through the board looking for a winner */
int connect_win(User user)
{
	int x,y,p;

	for (y = 1; y < 7; y++)
	{
		for (x = 1; x < 8; x++)
		{
			for (p = 1; p < 3; p++)
			{
				if (x+3 < 8)
				{
					if ((user->board[x][y] == p)
					  && (user->board[x+1][y] == p)
					  && (user->board[x+2][y] == p)
					  && (user->board[x+3][y] == p))
						return(p);
				}
				if (y+3 < 7)
				{
					if ((user->board[x][y] == p)
					  && (user->board[x][y+1] == p)
					  && (user->board[x][y+2] == p)
					  && (user->board[x][y+3] == p))
						return(p);
				}
				if ((x+3 < 8) && (y+3 < 7))
				{
					if ((user->board[x][y] == p)
					  && (user->board[x+1][y+1] == p)
					  && (user->board[x+2][y+2] == p)
					  && (user->board[x+3][y+3] == p))
						return(p);
				}
				if ((x-3 > 0) && (y+3 < 7))
				{
					if ((user->board[x][y] == p)
					  && (user->board[x-1][y+1] == p)
					  && (user->board[x-2][y+2] == p)
					  && (user->board[x-3][y+3] == p))
						return(p);
				}
			}
		}
	}
	return(0);
}

/*
   Hangman...this started out as amnuts' hangman, but after extensive mods,
   I don't think it's even remotely similiar anymore :P but thanks Andy
*/
void hangman(User user,char *inpstr,int rt)
{
	User u;
	int rounds;
	size_t i = 0;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (user->hangman_stage == -2)
	{
		switch (user->hangman_mode)
		{
			case 0:
			  if (word_count<3)
			  {
				if (!strncasecmp(word[1],"-play",2))
				{
					strncpy(user->hangman_word,get_word(),sizeof(user->hangman_word)-1);
					strcpy(user->hangman_display,user->hangman_word);
					for (i = 0; i < strlen(user->hangman_display); ++i)
						user->hangman_display[i] = '*';
					user->hangman_stage=0;
					write_user(user,"~FT Your current hangman status is as follows~CB:~RS \n");
					writeus(user,hanged[user->hangman_stage],user->hangman_display,"None currently");
					write_user(user,"~FR Please enter a letter~CB:~RS ");
					user->game_op=4;
					user->hangman_mode=3;
					return;
				}
				writeus(user,usage,command[com_num].name,"~CB[~CR-play~CB]");
				writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRrounds to play~CB]");
				return;
			  }
			  if (user->room->access == FIXED_PUBLIC)
			  {
				write_user(user,"~FR Please take the room to a less public area...\n");
				return;
			  }
			  rounds = atoi(word[2]);
			  if (rounds < 1)
			  {
				write_user(user,"~FR Umm a few more rounds then that please :P\n");
				return;
			  }
			  if (rounds > 15)
			  {
				write_user(user,"~FR Why would you want to play so many rounds???\n");
				return;
			  }
			  if ((u = get_name(user,word[1])) == NULL)
			  {
				write_user(user,center(notloggedon,80));
				return;
			  }
			  if (u == user)
			  {
				write_user(user,"~FR Playing with yourself will make you go blind :P\n");
				return;
			  }
			  if (u->hangman_mode)
			  {
				write_user(user,"~FR That person is already playing a game of hangman..\n");
				return;
			  }
			  if (user->room != u->room)
			  {
				write_user(user,"~FR You both have to be in the same room...\n");
				return;
			  }
			  strcpy(u->hangman_opp,user->name);
			  writeus(u,"~CT %s~FG wants to play a ~CM%d~FG round game with you...\n",user->recap,user->hangman_rnds);
			  write_user(u,"~CT To accept the game type ~CW'~FR.hangman ~CG-accept~CW'\n");
			  write_user(u,"~CT To reject the game type ~CW'~FR.hangman ~CG-reject~CW'\n");
			  u->hangman_mode = 1; /* Challenge has been issued */
			  writeus(user,"~CT You ask ~CM%s~CT if they want to play hangman with you...\n",u->recap);
			  user->hangman_rnds = rounds;
			  return;
			case 1:
			  if (word_count < 2)
			  {
				writeus(user,usage,command[com_num].name,"~CR-accept");
				writeus(user,usage,command[com_num].name,"~CR-reject");
				return;
			  }
			  if (!strncasecmp(word[1],"-accept",2))
			  {
				if ((u = get_user(user->hangman_opp)) == NULL)
				{
					write_user(user,"~FR Aww, your opponent lefted...\n");
					reset_game(user,3);
					return;
				}
				if (user->room != u->room)
				{
					write_user(user,"~FR You both have to be in the same room...\n");
					return;
				}
				user->hangman_rnds = u->hangman_rnds;
				strcpy(u->hangman_opp,user->name);
				WriteRoom(user->room,"~CM %s~FG accepts a ~CT%d ~FGround game of hangman with ~CM%s~FG!\n",user->recap,user->hangman_rnds,u->recap);
				writeus(u,"~CG Enter a word now for ~CT%s~CG to guess~CB:~RS ",user->recap);
				u->game_op = 3;
				user->hangman_stage = -3;
				user->hangman_mode = 2;
				u->hangman_mode = 2;
				return;
			  }
			  if (!strncasecmp(word[1],"-reject",2))
			  {
				if ((u = get_user(user->hangman_opp)) == NULL)
				{
					reset_game(user,3);
					return;
				}
				writeus(u,"~CM %s~FG rejects your hangman game!\n",user->recap);
				writeus(user,"~FG You reject ~CM%s's~FG hangman game!\n",u->recap);
				write_user(user,text);
				reset_game(user,3);
				reset_game(u,3);
				return;
			  }
			  writeus(user,usage,command[com_num].name,"~CR-accept");
			  writeus(user,usage,command[com_num].name,"~CR-reject");
			  return;
			case 2:
			  write_user(user,"~FR You are already involved in a game of hangman!\n");
			  return;
		}
	}
	else
	{
		write_user(user,"~FR You are already playing in a game of hangman.\n");
		return;
	}
}

/* Fight another user!! */
void fight_another(User user,char *inpstr,int rt)
{
	User u = NULL;
	char other_user[ARR_SIZE];
	int mode;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		if (fight.issued == 0)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CR-reset~CB]");
			return;
		}
		else
		{
			write_user(user,"~FR Current fighting status...\n");
			writeus(user,"~CW - ~FTChallenger~CB: [~FG%s~CB]\n",fight.first_user->recap);
			writeus(user,"~CW - ~FTDefender~CB:   [~FG%s~CB]\n",fight.second_user->recap);
			return;
		}
	}
	mode = 0;
	if (!strcasecmp(word[1],"-reset"))
	{
		reset_game(user,4);
		return;
	}
	if (!strcasecmp(word[1],"yes")) mode = 2;
	if (!strcasecmp(word[1],"no"))  mode = 1;
	if (mode == 0)
	{
		if ((u = get_name(user,word[1])) == NULL)
		{
			write_user(user,center(notloggedon,80));
			return;
		}
		if (u->level < USER)
		{
			write_user(user,"~FG That user cannot accept fights.\n");
			return;
		}
		strcpy(other_user,word[1]);
		if (user->no_fight)
		{
			write_user(user,"~FR You aren't allowing fights yourself, therefore you can't fight.\n");
			return;
		}
		if (u->no_fight)
		{
			write_user(user,"~FR That user isn't allowing fights...\n");
			return;
		}
		if (u->afk)
		{
			if (u->afk_mesg[0]) sprintf(text,"~FT%s~FG is~CW [~CRA~CW]~FGway~CW [~CRF~CW]~FGrom~CW [~CRK~CW]~FGeyboard~CB:~FT %s\n",u->recap,u->afk_mesg);
			else sprintf(text,"~FT%s~FG is~CW [~CRA~CW]~FGway~CW [~CRF~CW]~FGrom~CW [~CRK~CW]~FGeyboard~FG at the moment!\n",u->recap);
			write_user(user,text);
			return;
		}
		if (user->room != u->room)
		{
			write_user(user,"~FR You have to be in the same room as the person first!\n");
			return;
		}
		mode = 3;
	}
	if (fight.issued && mode == 3)
	{
		write_user(user,"~FR You have to wait for the other fighters to finish.\n");
		return;
	}
	if (!fight.issued && (mode == 1 || mode == 2))
	{
		write_user(user,"~FR Noone has challenged you to fight.\n");
		return;
	}
	if ((mode == 1 || mode == 2) && fight.second_user != user)
	{
		write_user(user,"~FR You are not the challenged user, type ~CW'~FM.fight~CW' ~FRto see!\n");
		return;
	}
	if (mode == 3)
	{
		if (u == user)
		{
			write_user(user,"~FR Something is wrong with you... fighting yourself..sheesh!\n");
			return;
		}
		issue_challenge(user,u);
		return;
	}
	if (mode == 1)
	{
		WriteRoom(user->room,back_out_text[ran(num_back_out_text)],user->recap);
		reset_game(user,4);
		return;
	}
	if (mode == 2)
	{
		accept_challenge(user);
		fight.first_user = NULL;
		fight.second_user = NULL;
		fight.issued = 0;
		fight.time   = 0;
		return;
	}
}

void issue_challenge(User user,User u)
{

	fight.first_user	= user;
	fight.second_user	= u;
	fight.issued		= 1;
	fight.time		= time(0);
	WriteRoom(user->room,challenge_text[ran(num_challenge_text)],user->recap,u->recap);
	write_user(u,center(CHALLENGE_LINE,80));
	write_user(u,center(CHALLENGE_LINE2,80));
	write_user(user,center(CHALLENGE_ISSUED,80));
}

/* Gotta be able to accept the fight */
void accept_challenge(User user)
{
	int x,i;
	User u1,u2;

	u1 = fight.first_user;
	u2 = fight.second_user;

	if (u1 == NULL || u2 == NULL) return;
	for (i = 0; i < 4; ++i) x = ran(4);
	if (x == 1)
	{
		WriteRoom(u1->room,tie1_text[ran(num_tie1_text)],u1->recap,u2->recap);
		u1->draw++;
		u2->draw++;
		return;
	}
	else if (x == 2)
	{
		WriteRoom(u1->room,tie2_text[ran(num_tie2_text)],u1->recap,u2->recap);
		u1->lose++;
		u2->lose++;
		disconnect_user(u1,1);
		disconnect_user(u2,1);
		return;
	}
	else if (x == 3)	/* u1 is the winner */
	{
		WriteRoom(u1->room,wins1_text[ran(num_wins_text)],u1->recap,u2->recap);
		u1->win++;
		u2->lose++;
		disconnect_user(u2,1);
		return;
	}
	else			/* u2 is the winner */
	{
		WriteRoom(u1->room,wins2_text[ran(num_wins_text)],u2->recap,u1->recap);
		u1->lose++;
		u2->win++;
		disconnect_user(u1,1);
		return;
	}
}

/* Trivia game */
void trivia(User user,char *inpstr,int rt)
{
	FILE *fp;
	int i = 0;
	char filename[256],line[ARR_SIZE],*name;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->invis == Invis) name = invisname; else name = user->recap;

	if (user->triv_mode == 0)
	{
		writeus(user,"~FTHello %s~FT, thank you for playing trivia on ~FM%s~FT, with a little luck,\n",user->recap,TalkerRecap);
		write_user(user,"~FTyou could be walking off with a temp promotion. The steps are as follows,\n");
		write_user(user,"~FTYou will be asked a series of ten questions, and if you get 7 or more of them\n");
		write_user(user,"~FTright, you will earn yourself that temp promotion. To answer the question,\n");
		write_user(user,"~FTat the prompt type ~CW'~CRa~CW'~FT,~CW'~CRb~CW'~FT, or ~CW'~CRc~CW'~FT the letter that matches the answer you\n");
		write_user(user,"~FTwish to give. If at any time you wish not to play, just hit ~CW'~CRq~CW'~FT...\n");
		write_user(user,"~FTGood luck :)....\n");
		WriteRoomExcept(user->room,user,"~FG%s ~FThas started playing trivia...\n",user->recap);
		user->ignall_store = user->ignall;
		user->ignall	   = 1;
		/* Get all the users questions right off the bat */
		user->tnum[0] = ran(51); /* 1st ? */
		do			 /* 2nd ? */
		{
			user->tnum[1] = ran(51);
		} while (user->tnum[1] == user->tnum[0]);
		do			 /* 3rd ? */
		{
			user->tnum[2] = ran(51);
		} while ((user->tnum[2] == user->tnum[1])
		    || (user->tnum[2] == user->tnum[0]));
		do			 /* 4th ? */
		{
			user->tnum[3] = ran(51);
		} while ((user->tnum[3] == user->tnum[2])
		    || (user->tnum[3] == user->tnum[1])
		    || (user->tnum[3] == user->tnum[0]));
		do			 /* 5th	? */
		{
			user->tnum[4] = ran(51);
		} while ((user->tnum[4] == user->tnum[3])
		    || (user->tnum[4] == user->tnum[2])
		    || (user->tnum[4] == user->tnum[1])
		    || (user->tnum[4] == user->tnum[0]));
		do			 /* 6th ? */
		{
			user->tnum[5] = ran(51);
		} while ((user->tnum[5] == user->tnum[4])
		    || (user->tnum[5] == user->tnum[3])
		    || (user->tnum[5] == user->tnum[2])
		    || (user->tnum[5] == user->tnum[1])
		    || (user->tnum[5] == user->tnum[0]));
		do			 /* 7th ? */
		{
			user->tnum[6] = ran(51);
		} while ((user->tnum[6] == user->tnum[5])
		    || (user->tnum[6] == user->tnum[4])
		    || (user->tnum[6] == user->tnum[3])
		    || (user->tnum[6] == user->tnum[2])
		    || (user->tnum[6] == user->tnum[1])
		    || (user->tnum[6] == user->tnum[0]));
		do			 /* 8th ? */
		{
			user->tnum[7] = ran(51);
		} while ((user->tnum[7] == user->tnum[6])
		    || (user->tnum[7] == user->tnum[5])
		    || (user->tnum[7] == user->tnum[4])
		    || (user->tnum[7] == user->tnum[3])
		    || (user->tnum[7] == user->tnum[2])
		    || (user->tnum[7] == user->tnum[1])
		    || (user->tnum[7] == user->tnum[0]));
		do			 /* 9th ? */
		{
			user->tnum[8] = ran(51);
		} while ((user->tnum[8] == user->tnum[7])
		    || (user->tnum[8] == user->tnum[6])
		    || (user->tnum[8] == user->tnum[5])
		    || (user->tnum[8] == user->tnum[4])
		    || (user->tnum[8] == user->tnum[3])
		    || (user->tnum[8] == user->tnum[2])
		    || (user->tnum[8] == user->tnum[1])
		    || (user->tnum[8] == user->tnum[0]));
		do			 /* 9th ? */
		{
			user->tnum[9] = ran(51);
		} while ((user->tnum[9] == user->tnum[8])
		    || (user->tnum[9] == user->tnum[7])
		    || (user->tnum[9] == user->tnum[6])
		    || (user->tnum[9] == user->tnum[5])
		    || (user->tnum[9] == user->tnum[4])
		    || (user->tnum[9] == user->tnum[3])
		    || (user->tnum[9] == user->tnum[2])
		    || (user->tnum[9] == user->tnum[1])
		    || (user->tnum[9] == user->tnum[0]));

		user->trivia_ans[0]	= '\0';
		user->triv_right	= 0;
		user->triv_wrong	= 0;
		user->triv_mode		= 1;
		user->tvar1		= 1;
	}
	if (user->tvar1 == 1)
	{
		sprintf(filename,"%s/%d.q",TriviaDir,user->tnum[(user->triv_mode - 1)]);
		fp = fopen (filename,"r");
		if (fp == NULL)
		{
			write_user(user,"~CW-~FT An error occurred in your trivia game.. aborting..\n");
			user->trivia_ans[0]	= '\0';
			user->triv_right	= 0;
			user->triv_wrong	= 0;
			user->triv_mode		= 0;
			write_log(1,LOG1,"[ERROR] - Failed to open '%d.q' for reading in trivia()\n",user->tnum[(user->triv_mode)-1]);
			for (i = 0; i < 9; ++i) user->tnum[i] = 0;
			user->tvar1		= 0;
			rev_away(user);
			return;
		}
		writeus(user,"~CW- ~FGQuestion %d of 10\n",user->triv_mode);
		fgets(line,250,fp);
		write_user(user,line);
		line[0]	= '\0';
		fgets(line,81,fp);
		write_user(user,line);
		line[0] = '\0';
		fgets(line,81,fp);
		write_user(user,line);
		line[0] = '\0';
		fgets(line,81,fp);
		write_user(user,line);
		line[0] = '\0';
		fgets(user->trivia_ans,81,fp);
		user->trivia_ans[strlen(user->trivia_ans)-1] = '\0';
		FCLOSE(fp);
		write_user(user,"~CW-~FT Your Choice~CB: [~CRa~CW|~CRb~CW|~CRc~CB]:~RS ");
		no_prompt	= 1;
		user->tvar1	= 2;
		return;
	}
	if (user->tvar1 == 2)		/* Get the user's answer. */
	{
		switch (toupper(inpstr[0]))
		{
			case 'A':
			case 'B':
			case 'C':
			  if (user->trivia_ans[0] == toupper(inpstr[0]))
			  {
				write_user(user,"~CW - ~FRThat could be right....or not..\n");
				user->triv_right++;
			  }
			  else
			  {
				write_user(user,"~CW - ~FRThat could be right....or not..\n");
				user->triv_wrong++;
			  }
			  user->triv_mode++;
			  if (user->triv_mode != 11)
			  {
				user->tvar1 = 1;
				trivia(user,NULL,0);
				return;
			  }
			  break;
			case 'Q':
			  write_user(user,"~CW- ~FYYou decide to no longer play.\n");
			  WriteRoomExcept(user->room,user,"~FT%s ~FRquit playing trivia.\n",name);
			  user->triv_mode	= 0;
			  user->trivia_ans[0]	= '\0';
			  user->triv_right	= 0;
			  user->triv_wrong	= 0;
			  user->tvar1		= 0;
			  for (i = 0; i < 9; ++i) user->tnum[i] = 0;
			  no_prompt		= 0;
			  rev_away(user);
			  return;
			default :
			  write_user(user,"~FT That was an invalid choice.\n");
			  write_user(user,"~CW-~FT Your Choice~CB: [~CRa~CW|~CRb~CW|~CRc~CB]:~RS ");
			  user->tvar1	= 2;
			  no_prompt	= 1;
			  return;
		}
	}
	if (user->triv_right > 6)
	{
			if (user->level < WIZ && user->tpromoted == 0)
		{
			writeus(user,"~CW- ~FGAlright, you got ~FR%d~FG right out of 10!\n",user->triv_right);
			writeus(user,"~CW- ~FGYou have won yourself a temp promotion to level~CB: [~FT%s~CB]\n",user->gender==Female?level[user->level+1].fname:level[user->level+1].mname);
			WriteRoomExcept(user->room,user,"~FT%s ~FRhas kicked some ass at trivia...give them a round of applause!\n",name);
			user->orig_lev = user->level;
			user->level++;
			user->tpromoted=1;
		}
		else
		{
			writeus(user,"~CW- ~FGAlright, you got ~FR%d~FG right out of 10!\n",user->triv_right);
			WriteRoomExcept(user->room,user,"~FT%s ~FRhas kicked some ass at trivia...give them a round of applause!\n",name);
		}
	}
	else
	{
		writeus(user,"~FT Aww, you only got ~FR%d~FT right out of 10!\n",user->triv_right);
		WriteRoomExcept(user->room,user,"~FT%s ~FMhas bombed out of trivia...ahhh well.\n",name);
	}
	rev_away(user);
	user->triv_mode		= 0;
	user->trivia_ans[0]	= '\0';
	user->triv_right	= 0;
	user->triv_wrong	= 0;
	user->tvar1		= 0;
	for (i = 0; i < 9; ++i) user->tnum[i] = 0;
	return;
}

/*
  Flip game, Usage .flip <heads/tails> <bet>, upon betting the user flips
  a coin, and it will generate a random number either 1 or 2, if it's 
  1, and you choose heads you win.. if it's 2, and you choose tails you
  win, otherwise you lose.
*/
void flip_coin(User user,char *inpstr,int rt)
{
	int toss = 0,bet = 0,total = 0,i;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRheads~CW|~CRtails~CB] [~CRyour bet~CB]");
		return;
	}
	bet = atoi(word[2]);
	if (bet == 0)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRheads~CW|~CRtails~CB]");
		return;
	}
	if (bet>user->chips)
	{
		write_user(user,"~FG You don't even have that many chips to bet!\n");
		return;
	}
	if (bet < 5)
	{
		write_user(user,"~FG The minimum bet is 5 chips.\n");
		return;
	}
	if (bet > 25)
	{
		write_user(user,"~FG The maximum bet is 25 chips.\n");
		return;
	}
	user->chips -= bet;
	for (i = 0; i < 2; ++i) toss = ran(2);
	if (!strcasecmp(word[1],"heads"))
	{
		if (toss == 1)
		{
			total = bet*2;
			write_user(user,heads_coin);
			writeus(user,"~CB - ~FTWicked deal, you just won ~FM%d~FT chips.\n",total);
			user->chips += total;
			return;
		}
		else if (toss == 2 || toss == 0)
		{
			write_user(user,tails_coin);
			writeus(user,"~CB - ~FTCrappy deal, you just lost ~FM%d ~FTchips.\n",bet);
			return;
		}
		return;
	}
	if (!strcasecmp(word[1],"tails"))
	{
		if (toss == 1)
		{
			write_user(user,heads_coin);
			writeus(user,"~CB - ~FTCrappy deal, you just lost ~FM%d ~FTchips.\n",bet);
			return;
		}
		else if (toss == 2 || toss == 0)
		{
			write_user(user,tails_coin);
			total = bet*2;
			writeus(user,"~CB - ~FTWicked deal, you just won ~FM%d~FT chips.\n",total);
			user->chips += total;
			return;
		}
		return;
	}
	writeus(user,usage,command[com_num].name,"~CB[~CRheads~CW|~CRtails~CB]");
	return;
}

/* Start the puzzle game */
void play_puzzle(User user,char *inpstr,int rt)
{

	user->puzzle = (struct puzzle_struct *)malloc(sizeof(struct puzzle_struct));
	if (user->puzzle == NULL)
	{
		write_user(user,"~CW-~FT An error occurred.. failed to start the game..\n");
		write_log(0,LOG1,"[ERROR] - Failed to allocate memory in play_puzzle()\n");
		return;
	}
	init_table(user); /* Init the table */
	write_user(user,center("~FT-~CT=~CB>~FG Starting the puzzle game.. ~FT]~CB=~FT-\n",80));
	write_user(user,"~CW- ~FTYour current puzzle status looks like~CB:~RS\n");
	puzzle_status(user);
	write_user(user,"~CW- ~FTYour moves~CB: [~CRL~CB]~FTeft, ~CB[~CRR~CB]~FTight, ~CB[~CRB~CB]~FTackward, ~CB[~CRF~CB]~FTorward, or ~CB[~CRQ~CB]~FTuit.\n");
	write_user(user,"~CW- ~FTYour choice~CB:~RS ");
	user->game_op	= 5;
	user->ignall_store = user->ignall;
	user->ignall	= 1;
	return;
}

/* Initialize the table. */
void init_table(User user)
{
	int i,j,c;
	static unsigned long mseed;

	if (user->puzzle == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred.. aborting..\n");
		puzzle_quit(user);
		return;
	}
	if ((unsigned)mseed >= 9000000) mseed = 0;
	srand((++mseed)+time(0)*2);
	/* Init everything */
	user->puzzle->counter = 0;	/* Reset the puzzle counter */
	for (i = 0; i < DIMEN; i++)	/* fill puzzle table */
		for (j = 0; j < DIMEN; j++) user->puzzle->table[i][j] = -1;
			for (c = 1; c < DIMEN*DIMEN; c++)
			{
				do
				{
					i = random()%DIMEN;
					j = random()%DIMEN;
				} while(user->puzzle->table[i][j] != -1);
				user->puzzle->table[i][j] = c;
			}
	for (i = 0; i < DIMEN; i++)		/* Get free position */
		for (j = 0; j < DIMEN; j++)
			if (user->puzzle->table[i][j] == -1)
			{		/* It's free..    */
				user->puzzle->clin = i;
				user->puzzle->ccol = j;
				break;
			}
}

/* Print out the board. */
void puzzle_status(User user)
{
	int i,j,k;

	if (user->puzzle == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred.. aborting..\n");
		puzzle_quit(user);
		return;
	}
	for (i = 0; i < DIMEN; i++)
	{
		strcpy(text,"~FM -");
		for (k = 0; k < DIMEN*3; k++) strcat(text,"-");
		write_user(user,text);
		write_user(user,"\n~FM |");
		for (j = 0; j < DIMEN; j++)
		{
			if (user->puzzle->table[i][j] != -1)
				sprintf(text,"~CR%2d~FM|",user->puzzle->table[i][j]);
			else strcpy(text,"  ~FM|");
			write_user(user,text);
		}
		write_user(user,"\n");
	}
	strcpy(text,"~FM -");
	for (k = 0; k < DIMEN*3; k++) strcat(text,"-");
	write_user(user,text);
	write_user(user,"\n\n");
}

/* Swap the pieces */
void puzzle_swap(User user,int oldlin,int oldcol,int newlin,int newcol)
{
	int aux = 0;

	if (user->puzzle == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred.. aborting..\n");
		puzzle_quit(user);
		return;
	}
	aux = user->puzzle->table[oldlin][oldcol];
	user->puzzle->table[oldlin][oldcol] = user->puzzle->table[newlin][newcol];
	user->puzzle->table[newlin][newcol] = aux;
}

/* Quit the puzzle game */
void puzzle_quit(User user)
{

	if (user->puzzle==NULL)
	{
		write_user(user,"~CW- ~FTYour puzzle game has ended..\n");
		user->game_op = 0;
		rev_away(user);
		return;
	}
	free(user->puzzle);
	user->puzzle = NULL;
	write_user(user,"~CW- ~FTYour puzzle game has ended..\n");
	user->game_op = 0;
	rev_away(user);
	return;
}

/* Play the move.. */
void puzzle_play(User user,int move)
{
	int err = 0,ok = 0;

	if (user->puzzle==NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred.. aborting..\n");
		puzzle_quit(user);
		return;
	}
	switch (move)
	{
		case LFT:
		  if (!user->puzzle->ccol) err = 1;
		  else
		  {
			puzzle_swap(user,user->puzzle->clin,user->puzzle->ccol,user->puzzle->clin,user->puzzle->ccol-1);
			user->puzzle->ccol--;
			user->puzzle->counter++;
		  }
		  break;
		case RGT:
		  if (user->puzzle->ccol==DIMEN-1) err = 1;
		  else
		  {
			puzzle_swap(user,user->puzzle->clin,user->puzzle->ccol,user->puzzle->clin,user->puzzle->ccol+1);
			user->puzzle->ccol++;
			user->puzzle->counter++;
		  }
		  break;
		case FWD:
		  if (!user->puzzle->clin) err = 1;
		  else
		  {
			puzzle_swap(user,user->puzzle->clin,user->puzzle->ccol,user->puzzle->clin-1,user->puzzle->ccol);
			user->puzzle->clin--;
			user->puzzle->counter++;
		  }
		  break;
		case BWD:
		  if(user->puzzle->clin==DIMEN-1) err = 1;
		  else
		  {
			puzzle_swap(user,user->puzzle->clin,user->puzzle->ccol,user->puzzle->clin+1,user->puzzle->ccol);
			user->puzzle->clin++;
			user->puzzle->counter++;
		  }
		  break;
	}
	if (err)
	{
		write_user(user,"~CW- ~FTThat was an illegal move..\n");
		puzzle_status(user);
		write_user(user,"\n~CW- ~FTYour moves~CB: [~CRL~CB]~FTeft, ~CB[~CRR~CB]~FTight, ~CB[~CRD~CB]~FTown, ~CB[~CRU~CB]~FTp, or ~CB[~CRQ~CB]~FTuit.\n");
		write_user(user,"~CW- ~FTYour choice~CB:~RS ");
		return;
	}
	else
	{
		ok = game_solved(user);
		if (ok)
		{
			write_user(user,center("~FT-~CT=~CB>~FG Congratz.. you solved the puzzle.. ~CB<~CT=~FT-\n",80));
			puzzle_status(user);
			writeus(user,"~CW-~FT It took you ~CR%d~FT tries to solve the puzzle.\n",user->puzzle->counter);
			puzzle_quit(user);
			return;
		}
		else
		{
			write_user(user,"~CW- ~FTYour current puzzle status is...\n");
			puzzle_status(user);
			write_user(user,"\n~CW- ~FTYour moves~CB: [~CRL~CB]~FTeft, ~CB[~CRR~CB]~FTight, ~CB[~CRD~CB]~FTown, ~CB[~CRU~CB]~FTp, or ~CB[~CRQ~CB]~FTuit.\n");
			write_user(user,"~CW- ~FTYour choice~CB:~RS ");
		}
	}
}

/* Tells us if the game has been solved.. */
int game_solved(User user)
{
	struct puzzle_struct *p;

	if (user->puzzle == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred.. aborting..\n");
		puzzle_quit(user);
		return(0);
	}
	p = user->puzzle;
	if (p->table[0][0] == 1 && p->table[0][1] == 2
	  && p->table[0][2] == 3 && p->table[0][3] == 4
	  && p->table[1][0] == 5 && p->table[1][1] == 6
	  && p->table[1][2] == 7 && p->table[1][3] == 8
	  && p->table[2][0] == 9 && p->table[2][1] == 10
	  && p->table[2][2] == 11 && p->table[2][3] == 12
	  && p->table[3][0] == 13 && p->table[3][1] == 14
	  && p->table[3][2] == 15 && p->table[3][3] == -1)
		return(1);
	return(0);
}

/*
   Gets a word for one player hangman.
*/
char *get_word(void)
{
	FILE *fp;
	char filename[FSL];
	int lines,cnt,i;
	static char aword[250];

	lines = cnt = i = 0;
	sprintf(filename,"%s/%s",MiscDir,HangList);
	lines = count_lines(filename);
	cnt = ran(lines);
	sprintf(text,"loveable");
	if ((fp = fopen(filename,"r")) == NULL) return(text);
	fscanf(fp,"%s\n",aword);
	while (!feof(fp))
	{
		if (i == cnt)
		{
			FCLOSE(fp);
			return(aword);
		}
		++i;
		fscanf(fp,"%s\n",aword);
	}
	FCLOSE(fp);
	return(text);
}

/* END OF GAMES.C */
