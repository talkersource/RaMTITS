/*
  casino.c
    Source code for casino games such as blackjack, poker, slots, etc.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Robb Thomas, Andrew Collington, and Mike Irving.

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
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#define _CASINO_C
  #include "include/needed.h"
#undef _CASINO_C
#include "include/casinolib.h"

/* Creates the poker game and adds it to the linked list. */
Pgame create_pgame(void)
{
	Pgame game;

	if ((game = (Pgame)malloc(sizeof(struct poker_game))) == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in create_pgame()\n");
		return(NULL);
	}
	if (pgame_first == NULL)
	{
		pgame_first = game;
		game->prev = NULL;
	}
	else
	{
		pgame_last->next = game;
		game->prev = pgame_last;
	}
	game->next = NULL;
	pgame_last = game;
	/* Initialize everything. */
	game->name[0]	  = '\0';	game->room	  = NULL;
	game->players	  = NULL;	game->dealer	  = NULL;
	game->opened	  = NULL;	game->curr_player = NULL;
	game->num_players = 0;		game->num_raises  = 0;
	game->in_players  = 0;		game->newdealer	  = 0;
	game->top_card	  = 0;		game->state	  = 0;
	game->pot	  = 0;		game->bet	  = 0;
	return(game);
}

/* Now we destruct the poker game. */
void destruct_pgame(Pgame game)
{
	Room room;

	room = game->room;
	if (game == pgame_first)
	{
		pgame_first = game->next;
		if (game == pgame_last) pgame_last = NULL;
		else pgame_first->prev = NULL;
	}
	else
	{
		game->prev->next = game->next;
		if (game == pgame_last)
		{
			pgame_last = game->prev;
			pgame_last->next = NULL;
		}
		else game->next->prev = game->prev;
	}
	if (room)
	{
		WriteRoom(room,"~CB - ~FGGame ~CT%s ~FGhas ended.\n",game->name);
	}
	free(game);
	game = NULL;
}

/* This will create a poker player object, and add it to the linked list -
 - accordingly.                                                          */
Pplayer create_pplayer(Pgame pg)
{
	Pplayer pp;

	if ((pp = (Pplayer)malloc(sizeof(struct poker_player))) == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in create_pplayer()\n");
		return(NULL);
	}
	if (pplayer_first == NULL)
	{
		pplayer_first	= pp;
		pp->prev	= NULL;
		if (pg->first_player == NULL) pg->first_player = pp;
	}
	else
	{
		pplayer_last->next = pp;
		pp->prev = pplayer_last;
	}
	pp->next = NULL;
	pplayer_last = pp;
	pg->num_players++;
	/* Init the player structure. */
	pp->hand[0]	= -1;		pp->touched	= 0;
	pp->putin	= 0;		pp->rank	= 0;
	pp->user	= NULL;		pp->game	= pg;
	pp->dealt	= 0;
	return(pp);
}

/* And now we must be able to destruct it. */
void destruct_pplayer(Pplayer pp)
{
	Pgame pg = pp->game;
	Pplayer pp2;

	pg->num_players--;
	writeus(pp->user,"~CB - ~FGYou leave the game ~CY%s\n",pg->name);
	WriteRoomExcept(pp->user->room,pp->user,"~CB - ~CT%s~FT leaves the game ~CY%s\n",pp->user->recap,pg->name);
	if (pg->num_players >= 1)
	{
		if (pp->hand[0] != -1)
		{
			word_count = 1;
			fold_poker(pp->user,NULL,0);
		}
		if (pg->curr_player == pp) next_poker_player(pg);
		if (pg->dealer == pp) pass_the_deal(pg);
	}
	/* Now we remove the player from the linked list. */
	if (pp == pplayer_first)	/* Normal player linked list */
	{
		pplayer_first = pp->next;
		if (pp == pplayer_last) pplayer_last = NULL;
		else pplayer_first->prev = NULL;
	}
	else
	{
		pp->prev->next = pp->next;
		if (pp == pplayer_last)
		{
			pplayer_last = pp->prev;
			pplayer_last->next = NULL;
		}
		else pp->next->prev = pp->prev;
	}
	if (pp == pg->first_player)	/* We need to reassign the pp->first_player */
	{
		for_player(pp2)
		{
			if(pp2->game != pg || pp2 == pp) continue;
			pg->first_player = pp2;
			break;
		}
	}
	free(pp);
	pp = NULL;
	if (!pg->num_players || !pg->first_player) destruct_pgame(pg);
	if (!pg->state)
	{
		pg->curr_player = pg->dealer;
		write_user(pg->dealer->user,"~CB - ~FGIt's now your turn to deal.\n");
		WriteRoomExcept(pg->room,pg->dealer->user,"~CB - ~FTIt's now ~FT%s's ~FYturn to deal.\n",pg->dealer->user->recap);
	}
}

/* This will get a poker game by name */
Pgame get_pgame(char *name)
{
	Pgame pg;

	for_game(pg)
	{
		if (!strncasecmp(pg->name,name,strlen(name))) return(pg);
	}
	return(NULL);
}

/* This will get a poker game by room */
Pgame get_pgame_here(Room room)
{
	Pgame pg;

	for_game(pg)
	{
		if (pg->room == room) return(pg);
	}
	return(NULL);
}

/* Shuffle cards */
void shuffle_cards(int deck[])
{
	int i,j,k,tmp;

	/* init the deck */
	for (i = 0 ; i<52 ; i++) deck[i] = i;
	/* do this 7 times */
	for (k = 0 ; k<7 ; k++)
	{
		/* Swap a random card below the ith card with the ith card */
		for (i = 0 ; i<52 ; i++)
		{
			j = myRand(52-i);
			tmp = deck[j];
			deck[j] = deck[i];
			deck[i] = tmp;
		}
	}
}

/* look at hand */
void hand_poker(User user,char *inpstr,int rt)
{

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
	if (user->pop == NULL)
	{
        	write_user(user,"~FG Don't be silly, your not in a poker game.\n");
		return;
        }
	if (user->pop->hand[0] == -1)
	{
	        write_user(user,"~CB - ~FGIt appears you don't even have any cards.\n");
		return;
        }
	print_hand(user,user->pop->hand);
}

/* Print hand */
void print_hand(User user,int hand[])
{
	int i,j;

	sprintf(text," ");
	for (i = 0 ; i<CARD_LENGTH ; i++)
	{
		for (j = 0 ; j<5 ; j++)
		{
			strcat(text,ascii_cards[hand[j]][i]);
	                strcat(text,"   ");
                }
		strcat(text,"\n");
		write_user(user,text);
		sprintf(text," ");
	}
	write_user(user, " ~CB-=[~CR1~CB ]=-   -=[~CR2~CB ]=-   -=[~CR3~CB ]=-   -=[~CR4~CB ]=-   -=[~CR5~CB ]=-\n");
}

/* Print hand for the room */
void room_print_hand(User user,int hand[])
{
	int i,j;

	sprintf(text," ");
	for (i = 0 ; i<CARD_LENGTH ; i++)
	{
		for (j = 0 ; j<5 ; j++)
		{
			strcat(text,ascii_cards[hand[j]][i]);
			strcat(text,"   ");
		}
		strcat(text,"\n");
		write_room(user->room,text);
		sprintf(text," ");
        }
}

/* Start a poker Game */
void start_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;

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
	if (user->room->access == FIXED_PUBLIC)
	{
		write_user(user,"~FG Take the game to a less public area please.\n");
		return;
	}
	if (user->chips<50)
	{
		write_user(user,"~FR You would probably need at least 50 chips or more.\n");
		return;
	}
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRgame name~CB]");
		return;
        }
	if (strlen(word[1])>GameNameLen)
	{
        	write_user(user,"~FR That game name is a tad bit too long.\n");
	        return;
        }
	if ((pg = get_pgame(word[1])) != NULL)
	{
        	write_user(user,"~FR There is already a game with that name.\n");
		return;
        }
	if ((pg = get_pgame_here(user->room)) != NULL)
	{
        	write_user(user,"~FR There is already a game in this room.\n");
		return;
        }
	if (user->pop == NULL)
	{
        	if ((pg = create_pgame()) == NULL)
		{
	        	write_user(user,"~FR Sorry an error occurred, and you can't create a game right now.\n");
		        return;
		}
		else
		{
		        if ((pp = create_pplayer(pg)) == NULL)
			{
		        	write_user(user,"~FR Sorry an error occurred, and you can't create a game right now.\n");
				return;
	                }
			pp->user	= user;	user->pop	= pp;
			pg->curr_player	= pp;	pg->players	= pp;
			pg->dealer	= pp;	pg->room	= user->room;
			strncpy(pg->name,word[1],sizeof(pg->name)-1);
	 		writeus(user,"~CB - ~FGYou start a poker game called~CB: [~CY%s~CB]~FG.\n",pg->name);
		        WriteRoomExcept(user->room,user,"~CB - ~CT%s~FY starts a poker game called~CT %s\n",user->recap,pg->name);
		        write_user(user,"~CB -~FG It's now your turn to deal.\n");
	        }
		return;
	}
	else
	{
        	write_user(user,"~FR You're already playing in a game.\n");
		return;
        }
}

/* Join a poker game */
void join_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;

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
	if (user->chips<50)
	{
		write_user(user,"~FR You need at least 50 chips to join a game!\n");
		return;
	}
	if (user->pop != NULL)
	{
		write_user(user,"~FR You're already playing in a game.\n");
		return;
        }
	if ((pg = get_pgame_here(user->room)) == NULL)
	{
		write_user(user,"~FR You gotta be in the same room as the game.\n");
		return;
        }
	if (pg->num_players >= 6)
	{
		write_user(user,"~FR This poker game already has the max users.\n");
		return;
	}
	if ((pp = create_pplayer(pg)) == NULL)
	{
		write_user(user,"~FR Sorry, an error occurred and you can't join the game.\n");
		return;
	}
	else
	{
		pp->user = user;	user->pop = pp;
		writeus(user,"~CB - ~FGYou join the poker game~CB: [~CY%s~CB]~FG!\n",pg->name);
	        WriteRoomExcept(user->room,user,"~CB- ~CT%s~FY joins the poker game~CT %s\n",user->recap,pg->name);
	}
}

/* Leave a poker game */
void leave_poker(User user,char *inpstr,int rt)
{

	if (user->pop != NULL)
	{
        	destruct_pplayer(user->pop);
		user->pop = NULL;
        }
	else
	{
        	write_user(user,"~FR You're not even playing in a game.\n");
		return;
        }
}

/* List poker games */
void list_games(User user,char *inpstr,int rt)
{
	Pgame pg;
	int cnt = 0;

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
	for_game(pg)
	{
		if (++cnt == 1)
		{
			write_user(user,"~FG.----------------------------------------------------.\n");
			write_user(user,"~FG|          ~FYCurrent poker games being played          ~FG|\n");
			write_user(user,"~FG+-----------------+----------------------+-----------+\n");
			write_user(user,"~FG| ~FTGame Name       ~FG| ~FMRoom Name            ~FG| ~CY# ~CRplayers ~FG|\n");
			write_user(user,"~FG+-----------------+----------------------+-----------+\n");
		}
		writeus(user,"~FG|~FT %-15.15s ~FG|~FM %-20.20s~FG | ~CY%d~CR players ~FG|\n",pg->name,pg->room->name,pg->num_players);
		cnt++;
        }
	if (cnt == 0) sprintf(text,"~CB -~FG There are currently no games being played.\n");
	else sprintf(text,"~FG  Total of~FM %d~FG game%s being played.\n",cnt,cnt>1?"s":"");
	write_user(user,text);
}

/* Deal cards to all players */
void deal_poker(User user,char *inpstr,int rt)
{
	int i;
	Pgame pg;
	Pplayer pp;

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
	if (user->pop == NULL)
	{
	        write_user(user,"~FR Silly, your not even playing a poker game!\n");
		return;
        }
	pg = user->pop->game;
	if (user->pop != pg->dealer)
	{
		writeus(user,"~CB - ~CT%s~FG is the game's current dealer.\n",pg->dealer->user->recap);
		return;
        }
	if (pg->state != 0)
	{
        	write_user(user,"~CB - ~FGNow is not a time to deal.\n");
		return;
        } 
	if (pg->num_players<2)
	{
        	write_user(user,"~CB - ~FGYou need someone else in the game to deal.\n");
		return;
        }
	/* Reset game state */
	pg->top_card	= 0;		pg->bet		= 0;
	pg->pot		= 0;		pg->num_raises	= 0;
	pg->in_players	= 0;		pg->opened	= NULL;
	pg->newdealer	= 0;
	shuffle_cards(pg->deck);

	/* Reset players */
	for_player(pp)
	{
		if (pp->user->chips >= 5)
		{
			pp->hand[0]	= -1;	pp->touched	= 0;
			pp->putin	= 5;	pp->rank	= 0;
			pg->in_players++;
			write_user(pp->user,"~CB - ~FGYou ante 5 chips.\n");
        	}
		else write_user(pp->user,"~CB - ~FGYou need more chips.\n");
	}
	if (pg->in_players<2)
	{
		write_room(pg->room,"~CB - ~FY More people need chips to ante.\n");
		return;
	}
	write_room(user->room,"~CW - ~FY Everyone in the game has anted 5 chips....\n");
	write_user(user,"~CB - ~FGYou shuffle and deal up the cards.\n");
	WriteRoomExcept(user->room,user,"~CW - ~CT%s~FY shuffles and deals up the cards.\n",user->recap);
	/* Start with the player to the left of the dealer */
	pg->curr_player	= pg->dealer;
	next_poker_player(pg);
	pp = pg->curr_player;
	/* deal five to each player */
	for (i=0;i<5;i++)
	{
		do
		{
			if (pg->curr_player->putin == 5)
			{
		  		pg->curr_player->hand[i] = pg->deck[pg->top_card];
			  	pg->top_card++;
				if (i == 4)
				{
		     			pg->curr_player->user->chips -= 5;
					pg->pot += 5;
					pg->curr_player->putin = 0;
					print_hand(pg->curr_player->user,pg->curr_player->hand);
				}
			}
			/* Next Player */
			next_poker_player(pg);
		} while (pg->curr_player != pp);
	}
	/* Make this guy the default opened guy */
	pg->opened = pg->curr_player;
	write_user(pg->curr_player->user,"~CB - ~FGThe first round of betting starts with you.\n");
	WriteRoomExcept(pg->room,pg->curr_player->user,"~CB - ~FYThe first round of betting starts with ~CT%s~FY.\n",pg->curr_player->user->recap);
	pg->state = 1;
}

/* Next poker player */
void next_poker_player(Pgame game)
{

	if (game->curr_player->next == NULL)
	{
		if (game->first_player == NULL)
		{
			write_room(game->room,"~CB - ~FGAn error occurred... please restart the game.\n");
			destruct_pgame(game);
			return;
		}
		else game->curr_player = game->first_player;
	}
	else
	{
		/* First make sure the player is in our game. */
		if (game->curr_player->next->game != game)
		{
			if (game->first_player == NULL)
			{
				write_room(game->room,"~CB - ~FGAn error occurred... please restart the game.\n");
				destruct_pgame(game);
				return;
			}
			else game->curr_player = game->first_player;
		}
		else game->curr_player = game->curr_player->next;
	}
}

/* Next in player */
void next_in_player(Pgame game)
{

	do
	{
		if (game->curr_player->next == NULL)
		{
			if (game->first_player == NULL)
			{
				write_room(game->room,"~CB - ~FGAn error occurred... please restart the game.\n");
				destruct_pgame(game);
				return;
			}
			else game->curr_player = game->first_player;
		}
		else
		{
			/* First make sure the player is in our game. */
			if (game->curr_player->next->game != game)
			{
				if (game->first_player == NULL)
				{
					write_room(game->room,"~CB - ~FGAn error occurred... please restart the game.\n");
					destruct_pgame(game);
					return;
				}
				else game->curr_player = game->first_player;
			}
			else game->curr_player = game->curr_player->next;
		}
	} while (game->curr_player->hand[0] == -1);
}

/* Bet in poker */
void bet_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	int pbet=0;

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
	if (user->pop == NULL)
	{
		write_user(user,"~FR Silly, your not even playing a poker game.\n");
		return;
	}
	pg = user->pop->game;
	/* Check if it's possible to bet */
	if ((pg->state != 1) && (pg->state != 3))
	{
		write_user(user,"~CB - ~FGIt isn't time for you to bet.\n");
		return;
	} 
	if (pg->curr_player != user->pop)
	{
		write_user(user,"~CB - ~FGIt isn't your turn to bet.\n");
		return;
	}
	if (word_count<2)
	{
		write_user(user,"~CB - ~FGHow much do you want to bet?\n");
		return;
	}
	pbet = atoi(word[1]);
	if ((!pbet) && (word[1][0] != '0'))
	{
		write_user(user,"~CB - ~FGNumbers would probably work better.\n");
		return;
	}
	if (pbet<0)
	{
		write_user(user,"~CB - ~FGYou can't bet negative numbers.\n");
		return;
	}
	if (!pbet && pg->state == 1)
	{
		write_user(user,"CB - ~FGYou can't check now, you need to bet.\n");
		return;
	}
	if (!pbet && pg->state == 3)
	{
		check_poker(user,NULL,0);
		return;
	}
	if (pbet<pg->bet)
	{
		writeus(user,"~CB - ~FGYou must bet at least~FT %d~FG chips or fold.\n",pg->bet);
		return;
	}
	if (pbet-pg->bet>500)
	{
		if (!pg->bet)
		{
	        	write_user(user,"~CB - ~FGThe largest opening bet is 500 chips.\n");
		        return;
		}
		else
		{
	        	write_user(user,"~CB - ~FGThe largest raise is 500 chips.\n");
		        return;
		}
	}
	if ((pbet>pg->bet) && (pg->bet))
	{
        	if (pg->num_raises>5)
		{
	        	write_user(user,"~CB - ~FGThere is a limit of five raises. Either see the bet, or fold.\n");
		        return;
		}
		pg->num_raises++;
        }
	if (user->chips<pbet)
	{
		write_user(user,"~CB - ~FGYou don't even have that many chips.\n");
		return;
	} 
	bet_poker_aux(user,pbet);
}

/* Aux bet function */
void bet_poker_aux(User user,int pbet)
{
	Pgame pg;
	Pplayer pp;

	if (user == NULL) return;
	if (user->pop == NULL) return;
	pp = user->pop;
	pg = pp->game;
	user->pop->touched = 1;
	if (pbet == pg->bet)
	{
		writeus(user,"~CB - ~FGYou see the bet of~FT %d~FG chips.\n",pg->bet);
		WriteRoomExcept(pg->room,user,"~CB - ~CT%s ~FYsees the bet of~CT %d~FY chips.\n",user->recap,pg->bet);
	}
	else if (!pg->bet)
	{
		pg->opened = user->pop;
		writeus(user,"~CB - ~FGYou open up the betting with~CY %d~FG chips.\n",pbet);
	        WriteRoomExcept(pg->room,user,"~CB - ~CT%s ~FYopens up the betting with~CT %d~FY chips.\n",user->recap,pbet);
        }
	else
	{
		writeus(user,"~CB - ~FGYou raise the bet to~CY %d ~FGchips.\n",pbet);
		WriteRoomExcept(pg->room,user,"~CB - ~CT%s~FY raises the bet to~CT %d~FY chips!\n",user->recap,pbet);
	}
	pg->bet		= pbet;		pg->pot		+= (pbet-pp->putin);
	pp->putin	= pbet;		user->chips	-= (pbet-pp->putin);
	writeus(user,"~CB -~FG The pot is now sitting at ~FT%d ~FGchips.\n",pg->pot);
	next_in_player(pg);
	all_called_check(pg);
	bet_message(pg);
}

/* Raise the bet in poker */
void raise_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;
	int praise = 0;

	if (user == NULL) return;
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
	if (user->pop == NULL)
	{
	        write_user(user,"~FR Silly, your not even playing a poker game.\n");
		return;
        }
	pp = user->pop;
	pg = pp->game;

	/* Check if it's possible to raise */
	if ((pg->state != 1) && (pg->state != 3))
	{
		write_user(user,"~CB - ~FGDon't you think you should wait till it's possible to raise?\n");
		return;
	} 
	if (pg->curr_player != pp)
	{
		write_user(user,"~CB - ~FGYou can't raise when it isn't even your turn!\n");
		return;
	}
	if (word_count<2)
	{
		write_user(user,"~CB - ~FGYou should specify how much you would like to raise.\n");
		return;
	}
	praise = atoi(word[1]);
	if ((!praise) && (word[1][0]!='0'))
	{
		write_user(user,"~CB - ~FGBetting in numbers would probably work better.\n");
		return;
	}
	if (praise<0)
	{
		write_user(user,"~CB - ~FGI think a positive raise would be better.\n");
		return;
	}
	if (praise<10)
	{
		write_user(user,"~CB - ~FGThe smallest raise allowed is 10 chips.\n");
		return;
	}
	if (praise>500)
	{
		write_user(user,"~CB - ~FGThe largest raise allowed is 500 chips.\n");
		return;
	}
	if (pg->num_raises>5)
	{
		write_user(user,"~CB - ~FGThere is a limit of five raises. Either see the bet, or fold.\n");
		return;
	}
	praise += pg->bet;
	pg->num_raises++;
	if (user->chips<praise)
	{
		write_user(user,"~CB - ~FGWhat, you trying to bet with chips you don't have?!\n");
		return;
	}
	bet_poker_aux(user,praise);
}

/* see a bet */
void see_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;
	int pbet = 0;

	if (user == NULL) return;
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
	if (user->pop == NULL)
	{
		write_user(user,"~FR Silly, your not even playing a poker game.\n");
		return;
	}
	pp = user->pop;
	pg = pp->game;
	/* Check if it's possible to see a bet */
	if ((pg->state != 1) && (pg->state != 3))
	{
		write_user(user,"~CB - ~FGDon't you think you should wait till it's possible to see the bet?\n");
		return;
	} 
	if (pg->curr_player != pp)
	{
		write_user(user,"~CB - ~FGYou can't raise when it isn't even your turn.\n");
		return;
	}
	pbet = pg->bet;
	if (user->chips<pbet)
	{
		write_user(user,"~CB - ~FGYou don't even have that many chips.\n");
		return;
	}
	if (pbet == 0) check_poker(user,NULL,0);
	else bet_poker_aux(user,pbet);
}

/* All called check */
void all_called_check(Pgame game)
{
	Pplayer pp;

	if (game->curr_player->touched && game->curr_player->putin == game->bet)
	{
		/* reset the touched flags and putin ammts */
		for_player(pp)
		{
			if (pp->game != game) continue;
			pp->touched	= 0;
			pp->putin	= 0;
		}
		game->state++;
		switch (game->state)
		{
	 		case 2:
			  /* Start with the player to the left of the dealer */
			  game->curr_player = game->dealer;
			  next_in_player(game);
			  hand_poker(game->curr_player->user,NULL,0);
			  write_user(game->curr_player->user,"~CB - ~FGIt is now your turn to discard.\n");
			  write_user(game->curr_player->user,"~CB - ~FGTo discard type ~CW'~CR.discard ~CW<~CR# # ...~CW>\n");
			  WriteRoomExcept(game->room,game->curr_player->user,"~CB - ~FYIt is now~CT %s's~FY turn to discard.\n",game->curr_player->user->recap);
			  break;
			case 4:
			  /* Showdown ... who emerges as winner? */
			  write_room(game->room,center("~FB--~CB===~FMT ~CRSHOWDOWN~FT ]~CB===~FB--\n",80));
			  showdown_poker(game);
			  break;
		}
	}
}

/* pass the deal to the next player */
void pass_the_deal(Pgame game)
{

	if (game->curr_player->next == NULL)
	{
		if (game->first_player == NULL)
		{
			write_room(game->room,"~CW - ~FGAn error occurred... please restart the game.\n");
			destruct_pgame(game);
			return;
		}
		else game->curr_player=game->first_player;
	}
	else
	{
		/* First make sure the player is in our game. */
		if (game->curr_player->next->game != game)
		{
			if (game->first_player == NULL)
			{
				write_room(game->room,"~CW - ~FGAn error occurred... please restart the game.\n");
				destruct_pgame(game);
				return;
			}
			else game->curr_player = game->first_player;
		}
		else game->curr_player = game->curr_player->next;
	}
	if (game->state) game->newdealer = 1; /* we have a new dealer. */
	return;
}

/* Fold and pass turn to next player */
void fold_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;

	if (user == NULL) return;
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
	if (user->pop == NULL)
	{
	        write_user(user,"~FR Silly, your not even playing a poker game.\n");
		return;
        }
	if (user->pop->hand[0] == -1)
	{
        	write_user(user,"~CB - ~FGIt would probly work better if you had cards to fold with.\n");
		return;
        }
	pg = user->pop->game;
	/* Mark folding by making the first card in hand -1 */
	user->pop->hand[0] = -1;
	pg->in_players--;
	if (word_count<2)
	{
		write_user(user,"~CB - ~FGYou throw down the cards in disgust.\n");
		sprintf(text,"~CB - ~FT%s~FY throws down their cards in disgust.\n",user->recap);
	}
	else
	{
		writeus(user,"~CB - ~FGYou say, ~CW\"~CR%s~CW\" ~FGand throw down your cards in disgust!\n",inpstr);
		sprintf(text,"~CB - ~FT%s~FY says, ~CW\"~CR%s~CW\" ~FGand throws down their cards in disgust.\n",user->recap,inpstr);
	}
	write_room_except(user->room,user,text);
	/* Check if there is 1 player in */
	if (pg->in_players == 1)
	{
		next_in_player(pg);
		pg->curr_player->user->chips+=pg->pot;
		writeus(pg->curr_player->user,"~CB - ~FGWicked deal! You win ~CY %d~FG chips.\n",pg->pot);
		WriteRoomExcept(pg->room,pg->curr_player->user,"~CB - ~FT%s~FY wins~FG %d~FY chips.\n",pg->curr_player->user->recap,pg->pot);
		pg->pot = pg->bet=pg->state=0;
		for_player(pp)
		{
			if (pp->game != pg) continue;
			pp->hand[0] = -1;
		} 
		/* pass the deal if it hasn't already */
		if (!pg->newdealer) pass_the_deal(pg);
		pg->curr_player = pg->dealer;
		write_user(pg->dealer->user,"~CB - ~FGIt is now your turn to deal the cards.\n");
		WriteRoomExcept(pg->room,pg->dealer->user,"~CB - ~FYIt is now ~FT%s%s~FY turn to deal the cards.\n",pg->dealer->user->recap,pg->dealer->user->name[strlen(pg->dealer->user->name)-1]=='s'?"'":"'s");
	}
	else
	{
		/* check if it's my turn */
		if (pg->curr_player == user->pop)
		{
			next_in_player(pg);
			all_called_check(pg);
			bet_message(pg);
		}
	}
}

/* The chickens way out. */
void check_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;

	if (user == NULL) return;
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
	if (user->pop == NULL)
	{
		write_user(user,"~FR Silly, your not even playing a poker game.\n");
		return;
	}
	pp = user->pop;
	pg = pp->game;
	/* Check if it's possible to check */
	if (pg->state != 3)
	{
		write_user(user,"~CB - ~FGIt would be rather impossible to check at this point in time.\n");
		return;
        } 
	if (pg->curr_player != pp)
	{
		write_user(user,"~CB - ~FGHmmm..maybe this command would work if it were your turn.\n");
		return;
	}
	if (pg->bet>0)
	{
		writeus(user,"~CB - ~FGYou must bet at least~CY %d ~FGchips or fold.\n",pg->bet);
		return;
	}
	pp->touched = 1;
	write_user(user,"~CB - You taken the chickens way out and check.\n");
	WriteRoomExcept(pg->room,user,"~CB - ~CT %s~FY takes the chickens way out and checks.\n",user->recap);
	next_in_player(pg);
	all_called_check(pg);
	bet_message(pg);
}

/* Bet message */
void bet_message(Pgame game)
{

	if (game->state == 1 || game->state == 3)
	{
		writeus(game->curr_player->user,"~CB - ~FGYou have put in ~CY%d~FG this round. The bet is~CY %d ~FGchips to you.\n",game->curr_player->putin,game->bet);
		WriteRoomExcept(game->room,game->curr_player->user,"~CB - ~FYIt is now~CT %s's~FY turn to bet.\n",game->curr_player->user->recap);
	}
}

/* Discard cards in your hand */
void discard_poker(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;
	int i,have_ace,choice;
	int disc_these[5];

	if (user == NULL) return;
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
	if (user->pop == NULL)
	{
		write_user(user,"~FR Silly, your not even playing a poker game.\n");
		return;
	}
	pg = user->pop->game;
	/* Check if it's possible to discard */
	if (pg->state != 2)
	{
		write_user(user,"~CB - ~FGIt would be illegal for you to discard now.\n");
		return;
        } 
	if (pg->curr_player != user->pop)
	{
        	write_user(user,"~CB - ~FGHow do you expect to discard when it isn't your turn?!\n");
		return;
	}
	if (word_count>5)
	{
		write_user(user,"~CB - ~FGYou can't discard five cards.\n");
		return;
	}
	if (word_count>4)
	{
		/* Look for an ace */
		have_ace = i = 0;
		do
		{
			if (user->pop->hand[i]%13 == 12)
			have_ace = 1;
			i++;
		} while ((i<5) && (have_ace == 0));
		if (have_ace == 0)
		{
	        	write_user(user,"~CB - ~FGYou can only discard 4 cards if you have an ace in your hand.\n");
		        return;
	        }
	}
	user->pop->touched = 1;
	if ((word_count<2) || (word[1][0] == '0'))
	{
		write_user(user,"~CB - ~FGYou stand pat, obviously satisfied with your hand.\n");
		WriteRoomExcept(pg->room,user,"~CB - ~CT%s~FY stays with the cards they are holding.\n",user->recap);
	}
	else
	{
		/* discards - Init the array */
		for (i = 0 ; i<5 ; i++) disc_these[i] = 0;
		/* Get which cards to discard */
		for (i = 1 ; i<word_count ; i++)
		{
			choice = atoi(word[i]);
			if ((choice <= 0) || (choice>5))
			{
				write_user(user,"~CB - ~FGPlease choose numbers from 1 through 5.\n");
				return;
			}
			else disc_these[choice-1] = 1;
		}
		/* draw cards */
		for (i = 0 ; i<5 ; i++) {
			if (disc_these[i])
			{
				user->pop->hand[i] = pg->deck[pg->top_card];
				pg->top_card++;
			}
		}
		writeus(user,"~CB - ~FGYou have discarded~FT %d~FG card%s.\n",word_count-1,word_count-1>1?"s":"");
		WriteRoomExcept(pg->room,user,"~CB - ~FT%s~FY has discarded~CT %d~FY card%s.\n",user->name,word_count-1,word_count-1>1?"s":"");
		print_hand(user,user->pop->hand);
	}
	next_in_player(pg);
	if (pg->curr_player->touched)
	{
		/* We've dealt cards to everyone - reset the touched flags */
		for_player(pp)
		{
			if (pp->game != pg) continue;
			pp->touched=0;
		}
		/* reset the bet */
		pg->bet = 0;
		pg->num_raises = 0;
		/* Start with the player who opened if still in */
		pg->curr_player = pg->opened;
		if (pg->curr_player->hand[0] == -1) next_in_player(pg);
		WriteRoom(pg->room,"~CB - ~FYThe current pot is~CT %d ~FYchips.\n",pg->pot);
		write_user(user,"~CB - ~FGThe second round of betting starts with you.\n");
		WriteRoomExcept(pg->room,pg->curr_player->user,"~CB - ~FYThe second round of betting starts with~FT %s\n",pg->curr_player->user->recap);
		pg->state = 3;
	}
	else
	{
		hand_poker(pg->curr_player->user,NULL,0);
		write_user(pg->curr_player->user,"~CB - ~FGIt is now your turn to discard.\n");
		write_user(pg->curr_player->user,"~CB - ~FGTo discard type ~CW'~CR.discard ~CW<~CR# # ...~CW>\n");
		WriteRoomExcept(pg->room,pg->curr_player->user,"~CB - ~FYIt is now~FT %s's~FY turn to discard!\n",pg->curr_player->user->recap);
        }
}

/* Showdown */
void showdown_poker(Pgame game)
{
	Pplayer pp,winners[4];
	int num_winners,i,j,temp,loot;
	char text2[ARR_SIZE];

	winners[0] = game->first_player;
	num_winners = 1;
	for_player(pp)
	{
		if (pp->game != game) continue;
		/* assign ranks to all players hands */
		if (pp->hand[0] == -1) continue;
		for(i = 0 ; i<5 ; i++)	/* sort cards */
		{
			for(j = 0 ; j<4 ; j++)
			{
				if(pp->hand[j]%13<pp->hand[j+1]%13)
				{
					temp = pp->hand[j];
					pp->hand[j] = pp->hand[j+1];
					pp->hand[j+1] = temp;
				}
			}
		}
		/* check for straight or straight flush - Ace low 5432A */
		if ((pp->hand[0]%13 == (pp->hand[1]%13)+1) &&
		  (pp->hand[1]%13==(pp->hand[2]%13)+1) &&
		  (pp->hand[2]%13==(pp->hand[3]%13)+1) &&
		  (pp->hand[4]%13==12) && (pp->hand[0]%13 == 0))
		{
			pp->rank = 5;
			swap_cards(pp->hand,0,1);
			swap_cards(pp->hand,1,2);
			swap_cards(pp->hand,2,3);
			swap_cards(pp->hand,3,4);
		}
		else		/* Ace high AKQJ10 or other straight */
		{
			if ((pp->hand[0]%13==(pp->hand[1]%13)+1) &&
			  (pp->hand[1]%13==(pp->hand[2]%13)+1) &&
			  (pp->hand[2]%13==(pp->hand[3]%13)+1) &&
			  (pp->hand[3]%13==(pp->hand[4]%13)+1))
			{
				pp->rank = 5;
			}        
		}
		/* check for flush */
		if (((pp->hand[0]<13) && (pp->hand[1]<13) &&
		  (pp->hand[2]<13) && (pp->hand[3]<13) &&
		  (pp->hand[4]<13)) || (((pp->hand[0]<26) &&
		  (pp->hand[0]>12)) && ((pp->hand[1]<26) &&
		  (pp->hand[1]>12)) && ((pp->hand[2]<26) &&
		  (pp->hand[2]>12)) && ((pp->hand[3]<26) &&
		  (pp->hand[3]>12)) && ((pp->hand[4]<26) &&
		  (pp->hand[4]>12))) || (((pp->hand[0]<39) &&
		  (pp->hand[0]>25)) && ((pp->hand[1]<39) &&
		  (pp->hand[1]>25)) && ((pp->hand[2]<39) &&
		  (pp->hand[2]>25)) && ((pp->hand[3]<39) &&
		  (pp->hand[3]>25)) && ((pp->hand[4]<39) &&
		  (pp->hand[4]>25))) || (((pp->hand[0]<52) &&
		  (pp->hand[0]>38)) && ((pp->hand[1]<52) &&
		  (pp->hand[1]>38)) && ((pp->hand[2]<52) &&
		  (pp->hand[2]>38)) && ((pp->hand[3]<52) &&
		  (pp->hand[3]>38)) && ((pp->hand[4]<52) &&
	          (pp->hand[4]>38))))
		{
			/* We have a flush at least */
			if (pp->rank) pp->rank = 9;
			else pp->rank = 6;
		}
	  	/* check for four of a kind */
		if ((pp->rank == 0) && ((pp->hand[0]%13 == pp->hand[1]%13) &&
		  (pp->hand[1]%13 == pp->hand[2]%13) &&
		  (pp->hand[2]%13 == pp->hand[3]%13))) pp->rank = 8;
		if ((pp->rank == 0) && ((pp->hand[1]%13 == pp->hand[2]%13) &&
		  (pp->hand[2]%13 == pp->hand[3]%13) &&
		  (pp->hand[3]%13 == pp->hand[4]%13)))
		{
			pp->rank = 8;			/* 4 of a kind */
			swap_cards(pp->hand,0,4);
		}
		/* check for three of a kind or full house */
		if ((pp->rank == 0) && ((pp->hand[1]%13 == pp->hand[2]%13) &&
		  (pp->hand[2]%13==pp->hand[3]%13)))
		{
			pp->rank = 4;			/* 3 of a kind */
			swap_cards(pp->hand,0,3);
		}
		if ((pp->rank == 0) && ((pp->hand[0]%13 == pp->hand[1]%13) &&
		  (pp->hand[1]%13==pp->hand[2]%13)))
		{
			if (pp->hand[3]%13 == pp->hand[4]%13) pp->rank = 7; /* Full house */
			else pp->rank = 4;		/* 3 of a kind */
		}
		if ((pp->rank == 0) && ((pp->hand[2]%13 == pp->hand[3]%13) &&
		  (pp->hand[3]%13 == pp->hand[4]%13))) {
			if (pp->hand[0]%13 == pp->hand[1]%13)
			{
				pp->rank = 7;		/* Full house */
				swap_cards(pp->hand,0,3);
				swap_cards(pp->hand,1,4);
			}
			else
			{
				pp->rank = 4;		/* 3 of a kind */
				swap_cards(pp->hand,0,3);
				swap_cards(pp->hand,1,4);
			}
		}
		/* check for two pair */
		if ((pp->rank == 0) && ((pp->hand[0]%13 == pp->hand[1]%13) &&
		  (pp->hand[2]%13 == pp->hand[3]%13))) pp->rank = 3; /* 2 pair */
		if ((pp->rank == 0) && ((pp->hand[0]%13 == pp->hand[1]%13) &&
		  (pp->hand[3]%13 == pp->hand[4]%13)))
		{
			pp->rank = 3;			/* 2 pair */
			swap_cards(pp->hand,2,4);
		}		
		if ((pp->rank == 0) && ((pp->hand[1]%13 == pp->hand[2]%13) &&
		  (pp->hand[3]%13 == pp->hand[4]%13)))
		{
			pp->rank = 3;			 /* 2 pair */
			swap_cards(pp->hand,0,2);
			swap_cards(pp->hand,2,4);
		}
		/* Check for a pair */
		if ((pp->rank == 0) && (pp->hand[0]%13 == pp->hand[1]%13))
		  pp->rank = 2;
		if ((pp->rank == 0) && (pp->hand[1]%13 == pp->hand[2]%13))
		{
			pp->rank = 2;			/* 1 pair */
			swap_cards(pp->hand,0,2);
		}
		if ((pp->rank == 0) && (pp->hand[2]%13 == pp->hand[3]%13))
		{
			pp->rank = 2;			/* 1 pair */
			swap_cards(pp->hand,0,2);
			swap_cards(pp->hand,1,3);
		}
		if ((pp->rank == 0) && (pp->hand[3]%13 == pp->hand[4]%13))
		{
			pp->rank = 2;			/* 1 pair */
			swap_cards(pp->hand,0,2);
			swap_cards(pp->hand,1,3);
			swap_cards(pp->hand,0,4);
		}
		if (pp->rank == 0) pp->rank = 1;	/* a piddly high card */
        }
	/* show all hands that are in */
	for_player(pp)
	{
		if (pp->game != game || pp->rank == 0) continue;
		if (pp->rank>winners[0]->rank)
		{
			winners[0] = pp;
			num_winners=1;
		}
		if ((pp->rank == winners[0]->rank) && (pp != winners[0]))
		{
			for (i = 0 ; i<5 ; ++i)
			{
				if (pp->hand[i]%13>winners[0]->hand[i]%13)
				{
					winners[0] = pp;
					num_winners = 1;
					break;
				}
				else if (pp->hand[i]%13<winners[0]->hand[i]%13) break;
				else if (i == 4)	/* we have a tie? */
				{
					winners[num_winners]=pp;
					num_winners++;
				}
			}
			room_print_hand(pp->user,pp->hand);
			switch (pp->rank)
			{
				case 1:
				  sprintf(text2,"a whopping high card! ~CW:-(");
				  break;
				case 2:
				  sprintf(text2,"a whole pair! ~CW:)");
				  break;
				case 3:
				  sprintf(text2,"two whole pairs! ~CW:-)");
				  break;
				case 4:
				  sprintf(text2,"three of a kind! ~CW=)");
				  break;
				case 5:
				  sprintf(text2,"a straight..very nice! ~CW:):)");
				  break;
				case 6:
				  sprintf(text2,"nice... a flush! ~CW:-)");
				  break;
				case 7:
				  sprintf(text2,"kick ass, a full house!~CW =)");
				  break;
				case 8:
				  sprintf(text2,"four of the same kind! ~CW:^)");
				  break;
				case 9:
				  sprintf(text2,"a straight flush WOOHOO! ~CW=)");
				  break;
			}
			WriteRoom(game->room,"~CB - ~FT %s~FY has %s\n",pp->user->recap,text2);
		}
	}
	/* divide the loot - curr_player == the player who was called */
	for (i = 0 ; i<num_winners ; i++)
	{
		temp = (game->pot/(num_winners*5));
		loot = 5*temp;
		/* player called gets remainder */
		if (winners[i] == game->curr_player)
		  loot += game->pot%(num_winners*5);
		winners[i]->user->chips += loot;
		writeus(winners[i]->user,"~CB - ~FGYou have won ~CY%d ~FGchips.\n",loot);
		WriteRoomExcept(game->room,winners[i]->user,"~CB - ~CT%s~FY has won ~CT%d~FY chips.\n",winners[i]->user->recap,loot);
	}
	game->pot = 0;
	game->bet = 0;
	game->state = 0;
	for_player(pp)
	{
		if (pp->game != game) continue;
		pp->hand[0] = -1;
	}
	/* pass the deal if we haven't already */
	if (!game->newdealer) pass_the_deal(game);
	game->curr_player = game->dealer;
	write_user(game->dealer->user,"~CB - ~FGIt is now your turn to deal the cards.\n");
	WriteRoomExcept(game->room,game->dealer->user,"~CB - ~FYIt is now ~CT %s's~FY turn to deal the cards\n",game->dealer->user->recap);
}
  
/* swap_cards */
void swap_cards(int hand[],int c1,int c2)
{
	int tmp;

	tmp=hand[c1];
	hand[c1]=hand[c2];
	hand[c2]=tmp;
}

/* Show the poker players */
void show_players(User user,char *inpstr,int rt)
{
	Pgame pg;
	Pplayer pp;
	char turn_text[80],text2[80];

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
	if (word_count<2)
	{
		if (user->pop == NULL)
		{
			if ((pg = get_pgame_here(user->room)) == NULL)
			{
				write_user(user,"~CR Which poker game are you interested in?\n");
				return;
			}
		}
		else pg = user->pop->game;
	}
	else
	{
		if ((pg = get_pgame(word[1])) == NULL)
		{
			write_user(user,"~CR There are no poker games with that name being played.\n");
			return;
		}
	}
	write_user(user,"~FG.-----------------------------------------------------.\n");
	writeus(user,"~FG|        ~FB -~CB=~FT[ ~CYInfo for Game %-12.12s~FT ]~CB=~FB-          ~FG|\n",pg->name);
	write_user(user,"~FG|-----------------------------------------------------|\n");
	write_user(user,"~FG| ~FTPlayer Name    ~CB-=<>=- ~CRGame state ~CB-=<>=-  ~FM# of chips ~FG|\n");
	write_user(user,"~FG|-----------------------------------------------------|\n");
	switch (pg->state)
	{
		case 0:
		  sprintf(turn_text,"~CB<=- ~CB[~CYdealer~CB]~RS\n");
		  break;
		case 1:
		  sprintf(turn_text,"~CB<=- ~CB[~CYbetting~CB]~CW<~CRRound 1~CW >\n");
		  break;
		case 2:
		  sprintf(turn_text,"~CB<=- ~CB[~CYdiscarder~CB]\n");
		  break;
		case 3:
		  sprintf(turn_text,"~CB<=- ~CB[~CYbetting~CB]~CW<~CRRound 2~CW>\n");
		  break;
		case 4:
		  sprintf(turn_text,"~CB<=-\n");
		  break;
        } 
	for_player(pp)
	{
		if (pp->game != pg) continue;
		sprintf(text,"~FG| ~FT%-14.14s ~CB-=<>=-",pp->user->name);
		if (!pg->state) strcat(text,"~CR  waiting   ~CB-=<>=-");
		else
		{
			if (pp->hand[0]==-1) strcat(text,"   ~CRfolded   ~CB-=<>=-");
			else strcat(text,"  ~CRplaying   ~CB-=<>=-");
		}
		sprintf(text2," ~FG%5ld ~FMchips ~FG|",pp->user->chips);
		strcat(text,text2);
		if (pg->curr_player == pp) strcat(text,turn_text);
		else strcat(text,"\n");
		write_user(user,text);
	}
	write_user(user,"~FG|-----------------------------------------------------|\n");
	if ((pg->state == 1) || (pg->state == 3))
	{
		if (user->pop) writeus(user,"~CB -=> ~FGYou have put~CY %d ~FGchip%s into the pot during this betting round.\n",user->pop->putin,user->pop->putin>1?"s":"");
		writeus(user,"~CB -=> ~FGThe current bet is~CY %d~FG chip%s.\n",pg->bet,pg->bet>1?"s":"");
        }
	writeus(user,"~CB -=> ~FGThe current pot is~CY %d~FG chip%s.\n",pg->pot,pg->pot>1?"s":"");
	write_user(user,"~FG`-----------------------------------------------------'\n");
}

/* Robb's random function **/
int myRand(int max)
{
	int n,mask;

	/* Mask out as many bits as possible */
	for (mask = 1 ; mask<max ; mask *= 2);
	mask -= 1;
	/* Reroll until a number <= max is returned */
	do
	{
		n = random()&mask;
	} while (n >= max);
	return(n);
}

/* Slot machine */
void slots(User user,char *inpstr,int rt)
{
	int slot[3],j,i,win;

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
	if (user->chips<10)
	{
		write_user(user,"~FG You need at least 10 chips to play the slot machine.\n");
		return;
	}
	user->chips -= 10;
	write_user(user,"~CB - ~FGYou insert some coins and pull down the level...\n");
	for (i = 0; i < 13; ++i) slot[0] = ran((unsigned)12);
	for (i = 0; i < 13; ++i) slot[1] = ran((unsigned)12);
	for (i = 0; i < 13; ++i) slot[2] = ran((unsigned)12);
	i = 0;
	for (j = 0 ; j<9 ; j++)
	{
		text[0] = '\0';
		for (i = 0 ; i<3 ; ++i)
		{
			strcat(text,slots_pics[slot[i]][j]);
			strcat(text,"  ");
		}
		strcat(text,"\n");
		write_user(user,text);
		text[0]='\0';
	}
	if (slot[0] == slot[1] && slot[1] == slot[2]) /* true jackpot. */
	{
		switch (slot[0])
		{
			case 0 :
			  sprintf(text,"~FB--~CB===~FT[~CR JACKPOT - ~FRYou win 200 chips. ~FT]~CB===~FB--\n");
			  win = 200;  break;
			case 1 :
			  sprintf(text,"~FB--~CB===~FT[~CR JACKPOT - ~FRYou win 150 chips. ~FT]~CB===~FB--\n");
			  win = 150;  break;
			case 2 :
			  sprintf(text,"~FB--~CB===~FT[~CR JACKPOT - ~FRYou win 125 chips. ~FT]~CB===~FB--\n");
			  win = 125;  break;
			default:
			  sprintf(text,"~FB--~CB===~FT[~CR JACKPOT - ~FRYou win 100 chips. ~FT]~CB===~FB--\n");
			  win = 100;  break;
		}
		write_user(user,center(text,80));
		user->chips += win;
		return;
	}
	if (slot[0] == 1)		/* We got a cherry first..*/
	{
		switch (slot[1])
		{
			case 1 :	/* cherry - cherry - ? */
			  switch (slot[2]) {
				case 0 :	/* cherry - cherry - seven */
				  sprintf(text,"~FB--~CB===~FT[ ~FRYou win 150 chips. ~FT]~CB===~FB--\n");
				  win = 150;  break;
				case 2 :		/* cherry - cherry - coin */
				  sprintf(text,"~FB--~CB===~FT[ ~FRYou win 105 chips. ~FT]~CB===~FB--\n");
				  win = 105;  break;
				default:		/* cherry - cherry - anything */
				  sprintf(text,"~FB--~CB===~FT[ ~FRYou win 85 chips. ~FT]~CB===~FB--\n");
				  win = 85;   break;
			}
			case 0 :	/* cherry - seven - anything */
			  sprintf(text,"~FB--~CB===~FT[ ~FRYou win 75 chips. ~FT]~CB===~FB--\n");
			  win = 75;  break;
			case 2 :	/* cherry - coin - anything */
			  sprintf(text,"~FB--~CB===~FT[ ~FRYou win 55 chips. ~FT]~CB===~FB--\n");
			  win = 55;  break;
			default:	/* cherry - any - any */
			  sprintf(text,"~FB--~CB===~FT[ ~FRYou win 50 chips. ~FT]~CB===~FB--\n");
			  win = 50;  break;
		}
		write_user(user,center(text,80));
		user->chips+=win;
		return;
	}
	if (slot[0] == slot[1] && slot[2] == 0)	/* any - any - seven */
	{
		write_user(user,center("~FT--~CT===~CB> ~FRYou win 50 chips. ~CB<~CT===~FT--\n",80));
		user->chips += 50;
		return;
	}
	if (slot[0] == slot[1] && slot[2] == 1) /* any - any - cherry */
	{
		write_user(user,center("~FT--~CT===~CB> ~FRYou win 50 chips. ~CB<~CT===~FT--\n",80));
		user->chips+=50;
		return;
	}
	write_user(user,"~CB -~FG Damn.. you didn't win anything... better luck next time.\n");
	return;
}

/* Blackjack... the inner workings are based off of Andy's Blackjack to -
 - which I ported into the game_ops as to make it more fun :-) So big   -
 - thanks to Andy for this :-)						*/
void black_jack(User user,char *inpstr,int rt)
{

	/* Actually this function just calls sets up the users blackjack -
	 - stuff the real work is done in the game_ops...		 */
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
	if ((user->bjack = create_bjgame()) == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred trying to create your blackjack game...\n");
		return;
	}
	write_user(user,center("~FT-~CT=~CB>~FT Welcome to the Casino... to start off you can: ~CB<~CT=~FT-\n\n",80));
	write_user(user,"~CW- ~CB[~CRD~CB]~FTeal, ~CB[~CRQ~CB]~FTuit, or ~CB[~CRB~CB]~FTet....\n");
	write_user(user,"~CW- ~FGYour choice~CB:~RS ");
	user->ignall_store = user->ignall;
	user->ignall = 1;
	user->game_op= 6;
	return;
}

/* Create the blackjack game. */
Bjack create_bjgame(void)
{
	Bjack bj;
	int i;

	if ((bj = (Bjack)malloc(sizeof(struct bjack_struct))) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Memory allocation failure in create_bjgame().\n");
		return(NULL);
	}
	/* Init the game structure. */
	for (i = 0 ; i<52 ; i++) bj->deck[i] = i;
	for (i = 0 ; i<5 ; i++)
	{
		bj->hand[i] = -1;
		bj->dhand[i] = -1;
	}
	bj->bet = 10;
	bj->cardpos = 0;
	return(bj);
}

/* Now we need to shuffle the cards..*/
void shuffle_bjcards(Bjack bj)
{
	int i,tmp,j;

	srand(time(0));
	i = j = tmp = 0;
	for (i = 0 ; i<52 ; i++)
	{
		j = i+(rand()%(52-i));
		tmp = bj->deck[i];
		bj->deck[i] = bj->deck[j];
		bj->deck[j] = tmp;
	}
	/* Deal first set of cards. */
	bj->hand[0]  = bj->deck[bj->cardpos]; ++bj->cardpos;
	bj->dhand[0] = bj->deck[bj->cardpos]; ++bj->cardpos;
	bj->hand[1]  = bj->deck[bj->cardpos]; ++bj->cardpos;
	bj->dhand[1] = bj->deck[bj->cardpos]; ++bj->cardpos;
	return;
}

/* Destruct the bjack game. */
void destruct_bjgame(User user)
{

	if (user->bjack == NULL) return;
	free(user->bjack);
	user->bjack = NULL;
}

/* Show the hand. */
void show_bjack_cards(User user,int dealer,int start)
{
	int h,d,hand[5];
	char buff[ARR_SIZE];

	if (dealer && start)
	{
		for (d = 0 ; d<6 ; d++)
		{
			buff[0] = '\0';
			for (h = 0 ; h<5 ; ++h)
			{
				if (user->bjack->dhand[h] == -1) continue;
				if (h == 0) strcat(buff,cards[52][d]);
				else strcat(buff,cards[user->bjack->dhand[h]][d]);
			}
			writeus(user,"%s\n",buff);
		}
		return;
	}
	if (!dealer)
	{
		for (h = 0 ; h<5 ; h++) hand[h] = user->bjack->hand[h];
	}
	else for(h = 0 ; h<5 ; h++) hand[h] = user->bjack->dhand[h];
	for (d = 0 ; d<6 ; d++)
	{
		buff[0] = '\0';
		for (h = 0 ; h<5 ; ++h)
		{
			if (hand[h] == -1) continue;
			strcat(buff,cards[hand[h]][d]);
		}
		writeus(user,"%s\n",buff);
	}
}

/* Get the total of the users or dealers hand. */
int check_bjack_total(User user,int dealer)
{
	int h,total,i,has_ace,all_aces_one;

	has_ace = all_aces_one = 0;
	while (1)
	{
		total = 0;
		/* Get the users total */
		if (!dealer)
		{
			for (h = 0 ; h<5 ; h++)
			{
				if (user->bjack->hand[h] == -1) continue;
				i = user->bjack->hand[h]%13;
				switch (i)
				{
					case 0 :
					  if (!has_ace)
					  {
						has_ace = 1;
						total += 11;
					  }
					  else total++;
					  break;
					case 10: /* Jacks, Queens, and Kings */
					case 11:
					case 12:
					  total += 10;
					  break;
					default:
					  total += (i+1);
					  break;
				}
			}
		}
		/* Get the dealers total */
		else
		{
			for (h = 0 ; h<5 ; h++)
			{
				if (user->bjack->dhand[h] == -1) continue;
				i = user->bjack->dhand[h]%13;
				switch (i)
				{
					case 0 :
					  if (!has_ace)
					  {
						has_ace = 1;
						total += 11;
					  }
					  else total++;
					  break;
					case 10: /* Jacks, Queens, and Kings */
					case 11:
					case 12:
					  total += 10;
					  break;
					default:
					  total += (i+1);
					  break;
				}
			}
		}
		if (total>21 && has_ace && !all_aces_one) all_aces_one = 1;
		else return(total);
	}
}

/* Show the blackjack status. */
void show_bjack_status(User user)
{

	if (user->bjack == NULL) return;
	write_user(user,"~CW- ~FTYour current blackjack hand is~CB:~RS\n");
	show_bjack_cards(user,0,1);
	write_user(user,"~CW- ~FTThe dealers blackjack hand is~CB:~RS\n");
	show_bjack_cards(user,1,1);
	return;
}

void PlayHighDice(User user,char *inpstr,int rt)
{
	int dice1[2],dice2[2],bet = 0,j = 0,i;

	if (user == NULL) return;
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
	for (i = 0 ; i<2; i++)
	{
		dice1[i] = 0;
		dice2[i] = 0;
	} i = 0;
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRyour bet~CB]");
		return;
	}
	bet = atoi(word[1]);
	if (bet < 5 || bet > 250)
	{
		write_user(user,"~CW- ~FTPlease keep your bet between 5 and 250 chips.\n");
		return;
	}
	if (user->chips < bet)
	{
		write_user(user,"~CW- ~FTYou need that many chips first.\n");
		return;
	}
	user->chips -= bet;
	writeus(user,"~CW- ~FTYou place your bet of %d chips on the table...\n",bet);
	for (i = 0; i < 7; ++i) dice1[0] = ran(6);
	for (i = 0; i < 7; ++i) dice1[1] = ran(6);
	for (i = 0 ; i<5 ; ++i)
	{
		text[0] = '\0';
		for (j = 0 ; j<2 ; ++j)
		{
			if (user->ansi_on) strcat(text,ansi_dice[dice1[j]-1][i]);
			else strcat(text,ascii_dice[dice1[j]-1][i]);
			strcat(text,"~RS     ");
		}
		strcat(text,"\n");
		write_user(user,text);
		text[0] = '\0';
	}
	writeus(user,"~CW- ~FGThe dealer rolls the dice and gets~CB: ~CR%d ~FTand ~CR%d~FT which total %d.\n",dice1[0],dice1[1],(dice1[0]+dice1[1]));
	/* now the player */
	i = j = 0;
	for (i = 0; i < 7; ++i) dice2[0] = ran(6);
	for (i = 0; i < 7; ++i) dice2[1] = ran(6);
	for (i = 0 ; i<5 ; ++i)
	{
		text[0] = '\0';
		for (j = 0 ; j<2 ; ++j)
		{
			if (user->ansi_on) strcat(text,ansi_dice[dice2[j]-1][i]);
			else strcat(text,ascii_dice[dice2[j]-1][i]);
			strcat(text,"~RS     ");
		}
		strcat(text,"\n");
		write_user(user,text);
		text[0] = '\0';
	}
	writeus(user,"~CW- ~FGYou roll the dice and get~CB: ~CR%d ~FTand ~CR%d~FT which total %d.\n",dice2[0],dice2[1],(dice2[0]+dice2[1]));
	if ((dice1[0]+dice1[1]) >= (dice2[0]+dice2[1]))
	{
		writeus(user,"~CW-~FG Alas.. you lost your bet of %d chips.. you should try again..\n",bet);
		return;
	}
	else
	{
		writeus(user,"~CW-~FG Wicked stuff.. your roll was higher then the dealers.. you win ~FR%d ~FGchips.\n",(bet*2));
		user->chips += (bet*2);
		return;
	}
}

void play_triples(User user,char *inpstr,int rt)
{
	int dice[3],total = 0,bet = 0,i,j = 0;

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
	for (i = 0 ; i<3 ; ++i)
	{
		dice[i] = 0;
	} i = 0;
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRunder~CW|~CRover~CW|~CRnine~CB] [~CRyour bet~CB]");
		return;
	}
	bet = atoi(word[2]);
	if (bet<5 || bet>100)
	{
		write_user(user,"~CW-~FT Sorry, but you can only bet between 5 and 100 chips.\n");
		return;
	}
	if (user->chips < bet)
	{
		write_user(user,"~CW-~FT You can't bet with chips you don't have.\n");
		return;
	}
	if (strcasecmp(word[1],"under") && strcasecmp(word[1],"over")
	  && strcasecmp(word[1],"nine"))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRunder~CW|~CRover~CW|~CRnine~CB] [~CRyour bet~CB]");
		return;
	}
	user->chips -= bet;
	writeus(user,"~CW-~FT You place your bet of %d chips on the table.\n",bet);
	for (i = 0; i < 7; ++i) dice[0] = ran(6);
	for (i = 0; i < 7; ++i) dice[1] = ran(6);
	for (i = 0; i < 7; ++i) dice[2] = ran(6);
	for (i = 0 ; i < 5 ; ++i)
	{
		text[0] = '\0';
		for (j = 0 ; j<3 ; ++j)
		{
			if (user->ansi_on) strcat(text,ansi_dice[dice[j]-1][i]);
			else strcat(text,ascii_dice[dice[j]-1][i]);
			strcat(text,"~RS     ");
		}
		strcat(text,"\n");
		write_user(user,text);
		text[0] = '\0';
	}
	total = (dice[0]+dice[1]+dice[2]);
	writeus(user,"~CW-~FG You roll~CB: ~FT%d, %d, and %d, for a total of ~FR%d\n",dice[0],dice[1],dice[2],total);
	if ((total<9 && !strcasecmp(word[1],"under"))
	  || (total>9 && !strcasecmp(word[1],"over")))
	{
		/* Win 2x bet */
		writeus(user,"~CW-~FT Congrats, you've won %d chips.\n",(bet*2));
		user->chips += (bet*2);
		return;
	}
	else if (total == 9 && !strcasecmp(word[1],"nine"))
	{
		writeus(user,"~CW-~FT Congrats, you rolled the lucky nine and won %d chips.\n",(bet*4));
		user->chips += (bet*4);
		return;
	}
	else
	{
		write_user(user,"~CW-~FT Sorry... better luck next time..\n");
		return;
	}
}

void play_seven(User user,char *inpstr,int rt)
{
	int dice[2],total = 0,bet = 0,i,j = 0;

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
	for (i = 0 ; i<2 ; ++i)
	{
		dice[i] = 0;
	} i = 0;
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRunder~CW|~CRover~CW|~CRseven~CB] [~CRyour bet~CB]");
		return;
	}
	bet = atoi(word[2]);
	if (bet<5 || bet>100)
	{
		write_user(user,"~CW-~FT Sorry, but you can only bet between 5 and 100 chips.\n");
		return;
	}
	if (user->chips < bet)
	{
		write_user(user,"~CW-~FT You can't bet with chips you don't have.\n");
		return;
	}
	if (strcasecmp(word[1],"under") && strcasecmp(word[1],"over")
	  && strcasecmp(word[1],"seven"))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRunder~CW|~CRover~CW|~CRseven~CB] [~CRyour bet~CB]");
		return;
	}
	user->chips -= bet;
	writeus(user,"~CW-~FT You place your bet of %d chips on the table.\n",bet);
	for (i = 0; i < 7; ++i) dice[0] = ran(6);
	for (i = 0; i < 7; ++i) dice[1] = ran(6);
	for (i = 0 ; i < 5 ; ++i)
	{
		text[0] = '\0';
		for (j = 0 ; j<2 ; ++j)
		{
			if (user->ansi_on) strcat(text,ansi_dice[dice[j]-1][i]);
			else strcat(text,ascii_dice[dice[j]-1][i]);
			strcat(text,"~RS     ");
		}
		strcat(text,"\n");
		write_user(user,text);
		text[0] = '\0';
	}
	total = (dice[0]+dice[1]);
	writeus(user,"~CW-~FG You roll~CB: ~FT%d, and %d, for a total of ~FR%d\n",dice[0],dice[1],total);
	if ((total<7 && !strcasecmp(word[1],"under"))
	  || (total>7 && !strcasecmp(word[1],"over")))
	{
		/* Win 2x bet */
		writeus(user,"~CW-~FT Congrats, you've won %d chips.\n",(bet*2));
		user->chips += (bet*2);
		return;
	}
	else if (total == 7 && !strcasecmp(word[1],"seven"))
	{
		writeus(user,"~CW-~FT Congrats, you rolled the lucky nine and won %d chips.\n",(bet*4));
		user->chips += (bet*4);
		return;
	}
	else
	{
		write_user(user,"~CW-~FT Sorry... better luck next time..\n");
		return;
	}
}

void play_craps(User user,char *inpstr,int rt)
{
	int die[2],bet = 0,j,i,total = 0;

	if (user == NULL) return;
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
		writeus(user,usage,command[com_num].name,"~CB[~CRyour bet~CB]");
		return;
	}
	for (i = 0 ; i<2 ; ++i) die[i] = 0;
	bet = atoi(word[1]);
	if (bet<5 || bet>20)
	{
		write_user(user,"~CW-~FG Sorry, you can only bet between 5 and 20 chips.\n");
		return;
	}
	if (user->chips < bet)
	{
		write_user(user,"~CW- ~FGYou don't even have that many chips to bet.\n");
		return;
	}
	user->chips -= bet;
	for (i = 0; i < 7; ++i) die[0] = ran(6);
	for (i = 0; i < 7; ++i) die[1] = ran(6);
	write_user(user,"~CW-~FT You pick up the dice and roll them around..\n");
	for (j = 0 ; j<5 ; j++)
	{
		text[0] = '\0';
		for (i = 0 ; i < 2 ; i++)
		{
			if (user->ansi_on) strcat(text,ansi_dice[die[i]-1][j]);
			else strcat(text,ascii_dice[die[i]-1][j]);
			strcat(text,"~RS     ");
		}
		strcat(text,"\n");
		write_user(user,text);
		text[0] = '\0';
	}
	total = die[0]+die[1];
	writeus(user,"~CB --~FT Your total for this roll is~CB:~FG %d\n",total);
	if ((total == 7 || total == 11) && user->cft)
	{
		/* win on first roll 3x bet */
		writeus(user,"~CB --~FT Congratulations, you win %d chips.\n",(bet*3));
		user->chips	+= (bet*3);
		user->cft	= 1;
		user->last_roll	= 0;
		return;
	}
	else if (total == 3 || total == 12)
	{
		/* loses on first roll */
		write_user(user,"~CB --~FT Sorry, you lost your bet.\n");
		user->last_roll	= 0;
		user->cft	= 1;
		return;
	}
	else if ((user->last_roll == total) && user->cft == 0)
	{
		/* player makes 2x bet */
		writeus(user,"~CB --~FT Congratulations, you win %d chips.\n",(bet*2));
		user->chips	+= (bet*2);
		user->cft	= 1;
		user->last_roll	= 0;
		return;
	}
	else
	{
		write_user(user,"~CB --~FT You didn't win or lose.. roll the dice again..\n");
		user->chips	+= bet;
		user->last_roll	= total;
		user->cft	= 0;
		return;
	}
}

/* Lottery crap */
int add_ticket(char *name,int ball1,int ball2,int ball3,int ball4,int ball5,int ball6)
{
	Lotto l;

	if ((l = (Lotto)malloc(sizeof(struct lotto_struct))) == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in create_ticket()\n");
		return(0);
	}
	if (lotto_first == NULL)
	{
		lotto_first = l;
		l->prev = NULL;
	}
	else
	{
		lotto_last->next = l;
		l->prev = lotto_last;
	}
	l->next = NULL;
	lotto_last = l;
	strcpy(l->name,name);
	l->ball[0] = ball1;
	l->ball[1] = ball2;
	l->ball[2] = ball3;
	l->ball[3] = ball4;
	l->ball[4] = ball5;
	l->ball[5] = ball6;
	return(1);
}

int remove_ticket(char *name)
{
	Lotto l;

	l = lotto_first;
	while (l != NULL)
	{
		if (!strcasecmp(l->name,name)) break;
		l = l->next;
	}
	if (l == lotto_first)
	{
		lotto_first = l->next;
		if (l == lotto_last) lotto_last = NULL;
		else lotto_first->prev = NULL;
	}
	else
	{
		l->prev->next = l->next;
		if (l == lotto_last)
		{
			lotto_last = l->prev;
			lotto_last->next = NULL;
		}
		else l->next->prev = l->prev;
	}
	free(l);
	l = NULL;
	return(1);
}

void clear_tickets(void)
{
	Lotto lotto,lnext;

	lotto = lotto_first;
	while (lotto != NULL)
	{
		lnext = lotto->next;
		remove_ticket(lotto->name);
		lotto = lnext;
	}
	return;
}

int save_lotto(void)
{
	Lotto l;
	FILE *fp;
	char filename[FSL];
	int cnt = 0;

	sprintf(filename,"%s/lotto.dat",MiscDir);
	fp = fopen(filename,"w");
	if (fp == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Unable to open lotto.dat Reason: [%s]\n",strerror(errno));
		return(0);
	}
	for_lotto(l)
	{
		fprintf(fp,"%s %d %d %d %d %d %d\n",l->name,l->ball[0],l->ball[1],l->ball[2],l->ball[3],l->ball[4],l->ball[5]);
		++cnt;
	}
	FCLOSE(fp);
	if (cnt == 0) unlink(filename);
	return(1);
}

int load_lotto(void)
{
	FILE *fp;
	char filename[FSL],name[UserNameLen+1];
	int ball[6], i = 0;

	name[0] = '\0';
	for (i = 0; i < 6; ++i) ball[i] = 0;
	sprintf(filename,"%s/lotto.dat",MiscDir);
	if (!file_exists(filename)) return(0);
	fp = fopen(filename,"r");
	if (fp == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Unable to open lotto.dat. Reason: [%s]\n",strerror(errno));
		return(0);
	}
	while (!feof(fp))
	{
		fscanf(fp,"%s %d %d %d %d %d %d\n",name,&ball[0],&ball[1],&ball[2],&ball[3],&ball[4],&ball[5]);
		if (!(add_ticket(name,ball[0],ball[1],ball[2],ball[3],ball[4],ball[5])))
		{
			write_log(1,LOG1,"[ERROR] - Unable to parse the lottery file.\n");
			FCLOSE(fp);
			return(0);
		}
	}
	FCLOSE(fp);
	return(1);
}

int already_has_numbers(User user,int ball1,int ball2,int ball3,int ball4,int ball5,int ball6)
{
	Lotto l;

	for_lotto(l)
	{
		if (l->ball[0] == ball1 && l->ball[1] == ball2
		  && l->ball[2] == ball3 && l->ball[3] == ball4
		  && l->ball[4] == ball5 && l->ball[5] == ball6)
		{
			if (!strcasecmp(l->name,user->name)) return(1);
		}
	}
	return(0);
}

/* END OF CASINO.C */
