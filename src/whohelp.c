/*
  whohelp.c
    The code in which contains the functions for the 'who' and 'help'
    commands, as well as the different types of listings for each.

    The source within this file contains Copyrights 1998 - 2001 from
 	Rob Melhuish, Mike Irving, and Andrew Collington.

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
#include <time.h>
#include <ctype.h>
#define _WHOHELP_C
  #include "include/needed.h"
#undef _WHOHELP_C

/* Show who is on */
void who(User user,char *inpstr,int rt)
{

	if (user->login)
	{
        	write_user(user,ascii_line);
	        sprintf(text,"~FT-~CT=~CB> ~FTCurrent users on ~FM%s~CB <~CT=~FT-\n",TalkerRecap);
	        write_user(user,center(text,80));
	        short_who(user);
		return;
	}
	/* Don't display this message if user->who is EMERALD_WHO */
	if (user->who < 5)
	{
		if (user->ansi_on)
		{
	        	write_user(user,ansi_line);
		        sprintf(text,"~FTðÄÄ¯¯> ~CY Current users on ~FM%s ~FT<®®ÄÄð\n",TalkerRecap);
		        write_user(user,center(text,80));
	        }
		else
		{
	        	write_user(user,ascii_line);
		        sprintf(text,"~FT-~CT=~CB> ~FTCurrent users on ~FM%s~CB <~CT=~FT-\n",TalkerRecap);
		        write_user(user,center(text,80));
	        }
	}
	switch(user->who)
	{
		case 1 :
		  dcove_who(user);
		  break;
		case 2 :
		  amnuts_who(user);
		  break;
		case 3 :
		  moenuts_who(user);
		  break;
		case 4 :
		  short_who(user);
		  break;
		case 5 :
		  emerald_who(user);
		  break;
		case 6 :
		  mori_who(user);
		  break;
		case 7 :
		  ei3_who(user);
		  break;
		default:
		  dcove_who(user);
		  break;
        }
}

/*
  Who list styles, modify all but the dcove who as you see fit, just leave my
  who style alone..cuz really, it is the best one ;)
*/
void dcove_who(User user)
{
	User	u;
	Room	rm;
	int	cnt, total, invis, mins, idle, in_room, hidden, fake,
		on_days, on_hours, on_mins, id_days, id_hours,
		id_mins, id_secs;
	char	line[500], timestr[16], status[6], levelname[10], temp[80],
		temp2[80], idlestr[16], name[251];

	total = invis = hidden = fake = on_days = on_hours = on_mins = 0;
	id_days = id_hours = id_mins = id_secs = 0;
	idlestr[0] = timestr[0] = '\0';
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		write_user(user,"~FT³~FGRanking~FM   time~FM idl~FY status~RS   Pre-description, name, & description            ~FT³\n");
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		write_user(user,"~FY|~FGRanking~FM   time~FM idl~FY status~RS   Pre-description, name, & description~FY            |\n");
		write_user(user,ascii_bline);
	}
	for_rooms(rm)
	{
		in_room = 0;
		for_ausers(u)
		{
			if (u->room != rm || u->login) continue;
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room) && user->level<GOD) continue;
			if (u->type == CloneType) strcpy(name,u->recap);
			else
			{
				if (u->invis>Invis) strcpy(name,u->temp_recap);
				else strcpy(name,u->recap);
			}
			mins	 = (int)(time(0)-u->last_login);
			on_days  = mins/86400;
			on_hours = (mins%86400)/3600;
			on_mins  = (mins%3600)/60;
			idle	 = (int)(time(0)-u->last_input);
			id_days	 = idle/86400;
			id_hours = (idle%86400)/3600;
			id_mins  = (idle%3600)/60;
			id_secs  = idle;
			if (on_days >= 1)
				sprintf(timestr,"%dday%s",on_days,on_days == 1?"":"s");
			else
				sprintf(timestr,"%2d:%2.2d",on_hours,on_mins);
			if (id_days > 0)
				sprintf(idlestr,"%2dd",id_days);
			else if (id_hours > 0)
				sprintf(idlestr,"%2dh",id_hours);
			else if (id_mins > 0)
				sprintf(idlestr,"%2dm",id_mins);
			else if (id_secs > 0)
				sprintf(idlestr,"%2ds",id_secs);
			else strcpy(idlestr,"N/A");
			if (u->lev_rank[0]) strcpy(levelname,u->lev_rank);
			else strcpy(levelname,user->gender == Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
			if (u->status & Jailed) strcpy(levelname,"JAILED");
			++total;
			if (u->predesc[0])
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s~RS %s%s~RS",u->predesc,name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s~RS %s~RS",u->predesc,name,u->desc);
			}
			else
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s%s~RS",name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s~RS",name,u->desc);
			}
			switch(u->invis)
			{
				case 1 :
				  invis++;   total--;
				  if (u->level>user->level) continue;
				  else line[0] = 'I';
				  break;
				case 2 :
				  hidden++;  total--;
				  if (u!=user && user->level<ADMIN) continue;
				  else line[0] = 'H';
				  break;
				case 3 :
				  fake++;    total--;
				  if (u!=user && user->level<GOD) continue;
				  else line[0] = 'F';
				  break;
				default:
				  line[0] = ' ';
			}
			if (u->room->hidden) line[0] = '+';
			if (u->afk) strcpy(status," AFK ");
			else if (u->set_op) strcpy(status,"SETUP");
			else if (u->bank_op) strcpy(status,"BANK");
			else if (u->store_op) strcpy(status,"STORE");
			else if (u->editor.editing) strcpy(status,"EDIT");
			else if (u->mail_op) strcpy(status,"MAIL");
			else if (idle) strcpy(status,"sleep");
			else if (!idle) strcpy(status,"awake");
	                /*
			   Count number of colour coms to be taken account of
			   when formatting
			*/
			cnt = colour_com_count(line);
			if (u->room == rm)
			  if (in_room == 0)
			  {
				writeus(user,"~CB:: ~RS%s\n",u->room->recap);
				in_room = 1;
			  }
			writeus(user," ~FG%-8.8s ~FM%-5.5s ~FM%3.3s ~FY%-5.5s  ~RS%-*.*s\n",levelname,timestr,idlestr,status,50+cnt*3,50+cnt*3,line);
		}
	}
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (user->level >= WIZ)
	{
		if (invis == 0) sprintf(text," ~FTNoone is invis.");
		else if (invis == 1) sprintf(text," ~FTThere is one invis. user");
		else sprintf(text," ~FTThere are ~FY%d ~FTinvis. users",invis);
	}
	if (user->level >= ADMIN)
	{
		if (hidden == 0) sprintf(temp,"~FT, noone is hidden");
		else if (hidden == 1) sprintf(temp,"~FT, one hidden user");
		else sprintf(temp,"~FT, ~FY%d ~FThidden users",hidden);
		strcat(text,temp);
	}
	if (user->level >= GOD)
	{
		if (fake == 0) sprintf(temp2,"~FT, and noone is fake logged off");
		else if (fake == 1) sprintf(temp2,"~FT, and one fake logged off user");
		else sprintf(temp2,"~FT, and ~FY%d ~FTfake logged off users",fake);
		strcat(text,temp2);
	}
	if (user->level >= WIZ)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	writeus(user,"~FT There %s a total of ~FY%d ~FTuser%s online\n",total == 1?"is":"are",total,total == 1?"":"s");
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
}

void amnuts_who(User user)
{
	User	u;
	Room	rm;
	int	cnt, total, invis, mins, idle, hidden, fake, on_days,
		on_hours, on_mins, id_days, id_hours, id_mins, id_secs;
	char	line[500], name[251], timestr[10], status[16],
		temp[81], temp2[81], idlestr[10];

	cnt = total = invis = mins = idle = hidden = fake = on_days = 0;
	on_hours = on_mins = id_days = id_hours = id_mins = id_secs = 0;
	if (user->ansi_on)
	{
		write_user(user,ansi_line);
		write_user(user,"~FT   Name                                          ~CB:~FG Room            ~CB:~FT Tm~CW/~FTId\n");
  		write_user(user,ansi_line);
	}
	else
	{
		write_user(user,ascii_line);
		write_user(user,"~FT   Name                                          ~CB:~FG Room            ~CB:~FT Tm~CW/~FTId\n");
		write_user(user,ascii_line);

	}
	for_rooms(rm)
	{
		for_ausers(u)
		{
			if (u->room != rm || u->login) continue;
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room) && user->level<GOD) continue;
			if (u->type == CloneType) strcpy(name,u->recap);
			else
			{
				if (u->invis>Invis) strcpy(name,u->temp_recap);
				else strcpy(name,u->recap);
			}
			mins	 = (int)(time(0) - u->last_login);
			on_days  = mins/86400;
			on_hours = (mins%86400)/3600;
			on_mins  = (mins%3600)/60;
			idle	 = (int)(time(0) - u->last_input);
			id_days	 = idle/86400;
			id_hours = (idle%86400)/3600;
			id_mins  = (idle%86400)/60;
			id_secs  = idle;
			++total;
			if (u->afk) strcpy(status," AFK ");
			else if (u->set_op) strcpy(status,"SETUP");
			else if (u->bank_op) strcpy(status,"BANK ");
	                else if (u->store_op) strcpy(status,"STORE");
			else if (u->mail_op) strcpy(status,"MAIL ");
			else if (u->editor.editing) strcpy(status,"EDIT ");
			else
			{
				if (on_days >= 1)
					sprintf(timestr,"%2dd",on_days);
				else if (on_hours >= 1)
					sprintf(timestr,"%2dh",on_hours);
				else
					sprintf(timestr,"%2dm",on_mins);
				if (id_mins >= 20)
					sprintf(idlestr,"zzz");
				else
					sprintf(idlestr,"%d",id_mins);
				sprintf(status,"%s/%s",timestr,idlestr);
			}
			if (u->predesc[0])
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s~RS %s%s",u->predesc,name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s~RS %s",u->predesc,name,u->desc);
			}
			else
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s%s",name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s",name,u->desc);
			}
			switch(u->invis)
			{
				case 1:
				  invis++;   total--;
				  if (u->level>user->level) continue;
				  line[0] = 'I';
				  break;
				case 2:
				  hidden++;  total--;
				  if (u!=user && user->level<ADMIN) continue;
				  line[0] = 'H';
				  break;
				case 3:
				  fake++;    total--;
				  if (u!=user && user->level<GOD) continue;
				  line[0] = 'F';
				  break;
				default:
				  line[0] = ' ';
				}
			if (u->room->hidden) line[0] = '+';
			/*
			   Count number of colour coms to be taken account of
			   when formatting.
			*/
			cnt = colour_com_count(line);
			writeus(user,"~RS %-*.*s ~CB: ~FG%-*s ~CB: ~FT%-7.7s\n",47+cnt*3,47+cnt*3,line,15+colour_com_count(rm->recap)*3,rm->recap,status);
		}
	}
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (user->level >= WIZ)
	{
		if (invis == 0) sprintf(text," ~FTNoone is invis.");
		else if (invis == 1) sprintf(text," ~FTThere is one invis. user");
		else sprintf(text," ~FTThere are ~FY%d ~FTinvis. users",invis);
	}
	if (user->level >= ADMIN)
	{
		if (hidden == 0) sprintf(temp,"~FT, noone is hidden");
		else if (hidden == 1) sprintf(temp,"~FT, one hidden user");
		else sprintf(temp,"~FT, ~FY%d ~FThidden users",hidden);
		strcat(text,temp);
	}
	if (user->level >= GOD)
	{
		if (fake == 0) sprintf(temp2,"~FT, and noone is fake logged off");
		else if (fake == 1) sprintf(temp2,"~FT, and one fake logged off user");
		else sprintf(temp2,"~FT, and ~FY%d ~FTfake logged off users",fake);
		strcat(text,temp2);
	}
	if (user->level >= WIZ)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	writeus(user,"~FT There %s a total of ~FY%d ~FTuser%s online\n",total == 1?"is":"are",total,total == 1?"":"s");
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
}

void moenuts_who(User user)
{
	User	u;
	Room	rm;
	int	cnt, total, invis, mins, idle, hidden, fake, on_days,
		on_hours, on_mins, id_days, id_hours, id_mins, id_secs;
	char	line[500], timestr[16], status[30], levelname[10], temp[80],
		temp2[80], idlestr[16], name[251], gender[10];

	total = invis = hidden = fake = on_days = on_hours = on_mins = 0;
	id_days = id_hours = id_mins = id_secs = 0;
	idlestr[0] = timestr[0] = '\0';
	if (user->ansi_on)
	{
		write_user(user,ansi_line);
		write_user(user,"~CT  Room Name:      ~CB|~CYG~CB|~CM Rank    ~CB|~CGTime~CW:~CRIdle~CB|~FT Name and Description\n");
		write_user(user,ansi_line);
	}
	else
	{
		write_user(user,ascii_line);
		write_user(user,"~CT  Room Name:      ~CB|~CYG~CB|~CM Rank    ~CB|~CGTime~CW:~CRIdle~CB|~FT Name and Description\n");
		write_user(user,ascii_line);
	}
	for_rooms(rm)
	{
		for_ausers(u)
		{
			if (u->room != rm || u->login) continue;
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room) && user->level<GOD) continue;
			if (u->type == CloneType) strcpy(name,u->recap);
			else
			{
				if (u->invis>Invis) strcpy(name,u->temp_recap);
				else strcpy(name,u->recap);
			}
			mins     = (int)(time(0) - u->last_login);
			on_days  = mins/86400;
			on_hours = (mins%86400)/3600;
			on_mins  = (mins%3600)/60;
			idle     = (int)(time(0) - u->last_input);
			id_days  = idle/86400;
			id_hours = (idle%86400)/3600;
			id_mins  = (idle%86400)/60;
			id_secs  = idle;
			if (u->lev_rank[0]) strcpy(levelname,u->lev_rank);
			else strcpy(levelname,user->gender == Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
			if (u->status & Jailed) strcpy(levelname,"JAILED");
			++total;
			if (u->predesc[0])
			{
				if (u->desc[0] == '\'') sprintf(line," ~RS%s~RS %s%s",u->predesc,name,u->desc);
				else sprintf(line," ~RS%s~RS %s~RS %s",u->predesc,name,u->desc);
			}
			else
			{
				if (u->desc[0] == '\'') sprintf(line," ~RS%s%s",name,u->desc);
				else sprintf(line," ~RS%s~RS %s",name,u->desc);
			}
			switch(u->invis)
			{
				case 1:
				  invis++;   total--;
				  if (u->level>user->level) continue;
				  line[0] = '!';
				  break;
				case 2:
				  hidden++;  total--;
				  if (u != user && user->level<ADMIN) continue;
				  line[0] = '#';
				  break;
				case 3:
				  fake++;    total--;
				  if (u != user && user->level<GOD) continue;
				  line[0] = '@';
				  break;
				default:
				  line[0] = ' ';
			}
			if (u->room->hidden) line[0] = '+';
			strcpy(gender,"~CGN");
			if (u->gender == Female) strcpy(gender,"~CMF");
			else if (u->gender == Male) strcpy(gender,"~CGM");
			if (u->afk) strcpy(status," AFK ");
			else if (u->set_op) strcpy(status,"Setup");
			else if (u->bank_op) strcpy(status,"Bank ");
			else if (u->store_op) strcpy(status,"Store");
			else if (u->mail_op) strcpy(status,"Mail ");
			else if (u->editor.editing) strcpy(status,"Edit ");
			else
			{
				if (on_days >= 1)
					sprintf(timestr,"%3dd",on_days);
				else if (on_hours >= 1)
					sprintf(timestr,"%3dh",on_hours);
				else
					sprintf(timestr,"%3dm",on_mins);
				if (id_mins >= 20)
					sprintf(idlestr,"zzzz");
				else
					sprintf(idlestr,"%-4d",id_mins);
				sprintf(status,"~CG%s~CW:~CR%s",timestr,idlestr);
			}
			cnt = colour_com_count(line);
			writeus(user,"  ~CT%-*s ~CB|%-*s~CB|~CM %-7.7s ~CB|%-*s~CB|~RS%-*.*s\n",15+colour_com_count(rm->recap)*3,rm->recap,1+colour_com_count(gender)*3,gender,levelname,9+colour_com_count(status),status,33+cnt*3,33+cnt*3,line);
		}
	}
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (user->level >= WIZ)
	{
		if (invis == 0) sprintf(text," ~FTNoone is invis.");
		else if (invis == 1) sprintf(text," ~FTThere is one invis. user");
		else sprintf(text," ~FTThere are ~FY%d ~FTinvis. users",invis);
	}
	if (user->level >= ADMIN)
	{
		if (hidden == 0) sprintf(temp,"~FT, noone is hidden");
		else if (hidden == 1) sprintf(temp,"~FT, one hidden user");
		else sprintf(temp,"~FT, ~FY%d ~FThidden users",hidden);
		strcat(text,temp);
	}
	if (user->level >= GOD)
	{
		if (fake == 0) sprintf(temp2,"~FT, and noone is fake logged off");
		else if (fake == 1) sprintf(temp2,"~FT, and one fake logged off user");
		else sprintf(temp2,"~FT, and ~FY%d ~FTfake logged off users",fake);
		strcat(text,temp2);
	}
	if (user->level>=WIZ)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	writeus(user,"~FT There %s a total of ~FY%d ~FTuser%s online\n",total == 1?"is":"are",total,total == 1?"":"s");
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
}

void short_who(User user)
{
	User u;
	int cnt,total,invis,hidden,fake,cnt2,idle;
	char line[ARR_SIZE],idlestr[30];
	char temp[80],temp2[80];

	total=invis=hidden=fake=cnt=idle=cnt2=0;
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	text[0] = '\0';
	for_ausers(u)
	{
		if (u->type == CloneType) continue;
		if (u->login) continue;
		if (u->room->hidden && user->login) continue;
		if ((u->room->hidden && !has_room_key(user,u->room)
		  && u->room != user->room) && user->level<GOD) continue;
		total++;
		idle=(int)(time(0) - u->last_input)/60;
		if (u->afk) strcpy(idlestr,"<AFK>");
		else strcpy(idlestr,"");
		if (idle)
		{
			if (u->afk) strcpy(idlestr,"[AFK]");
			else sprintf(idlestr,"[%3.3d]",idle);
		}
		if (u->invis>Invis) sprintf(line,"  %s %s",ColourStrip(u->temp_recap),idlestr);
		else sprintf(line,"  %s %s",ColourStrip(u->recap),idlestr);
		switch(u->invis)
		{
			case 1:
			  invis++;   total--;
			  if (u->level>user->level) continue;
			  line[0] = 'I';
			  break;
			case 2:
			  hidden++;  total--;
			  if (u != user && user->level<ADMIN) continue;
			  line[0] = 'H';
			  break;
			case 3:
			  fake++;    total--;
			  if (u != user && user->level<GOD) continue;
			  line[0] = 'F';
			  break;
			default:
			  line[0] = ' ';
		}
		if (u->room->hidden) line[0] = '+';
		sprintf(temp,"~FT%-20.20s",line);
		strcat(text,temp);
		cnt2++;
		if (cnt2 >= 4)
		{
			strcat(text,"\n");
			write_user(user,text);
			cnt2=0;  text[0] = '\0';
		}
	}
	if (cnt2>0 && cnt2<4)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (user->level >= WIZ)
	{
		if (invis == 0) sprintf(text," ~FT Noone is invisible");
		else if (invis == 1) sprintf(text," ~FT There is one invisible user");
		else sprintf(text," ~FT There are ~FY%d ~FTinvisible users",invis);
	}
	if (user->level >= ADMIN)
	{
		if (hidden == 0) sprintf(temp,"~FT, noone is hidden");
		else if (hidden == 1) sprintf(temp,"~FT, one hidden user");
		else sprintf(temp,"~FT, ~FY%d ~FThidden users",hidden);
		strcat(text,temp);
	}
	if (user->level >= GOD)
	{
		if (fake == 0) sprintf(temp2,"~FT, and noone is fake logged off");
		else if (fake == 1) sprintf(temp2,"~FT, and one fake logged off user");
		else sprintf(temp2,"~FT, and ~FY%d ~FTfake logged off users",fake);
		strcat(text,temp2);
	}
	if (user->level >= WIZ)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	writeus(user,"~FT  There %s a total of ~FY%d ~FTuser%s online\n",total == 1?"is":"are",total,total == 1?"":"s");
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
}

void emerald_who(User user)
{
	User u;
	Room rm;
	int cnt,total,invis,mins,idle,in_room,hidden,fake;
	char line[500];
	char status[10],levelname[20], name[251];

	total=invis=hidden=fake=0;
	write_user(user,"\n~FG.-==--==--==--==~CG<~CY*~CG>~FG==--==--==--==--==~CG<~CY*~CG>~FG==--==--==--==--==~CG<~CY*~CG>~FG==--==--==--==-.\n");
	write_user(user,"~FG`-==--==--==--==~CG<~CY*~CG>~FG==~CG< ~FT[Islanders currently logged on] ~CG>~FG==~CG<~CY*~CG>~FG==--==--==--==-'\n\n");
	for_rooms(rm)
	{
		in_room = 0;
		for_ausers(u)
		{
			if (u->room != rm || u->login) continue;
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room) && user->level<GOD) continue;
			if (u->type == CloneType) strcpy(name,u->recap);
			else
			{
				if (u->invis>Invis) strcpy(name,u->temp_recap);
				else strcpy(name,u->recap);
			}
			mins=(int)(time(0) - u->last_login)/60;
			idle=(int)(time(0) - u->last_input)/60;
			if (u->lev_rank[0]) strcpy(levelname,u->lev_rank);
			else strcpy(levelname,user->gender == Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
			if (u->status & Jailed) strcpy(levelname,"JAILED");
			++total;
			if (u->afk) strcpy(status," AFK ");
			else if (u->set_op) strcpy(status,"SETUP");
			else if (u->bank_op) strcpy(status,"BANK");
			else if (u->store_op) strcpy(status,"STORE");
			else if (u->editor.editing) strcpy(status,"EDIT ");
			else if (u->mail_op) strcpy(status,"MAIL ");
			else if (idle) strcpy(status,"sleep");
			else if (idle == 0) strcpy(status,"awake");
			if (u->predesc[0])
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s~RS %s%s",u->predesc,name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s~RS %s",u->predesc,name,u->desc);
			}
			else
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s%s",name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s",name,u->desc);
			}
			switch(u->invis)
			{
				case 1 :
				  invis++;   total--;
				  if (u->level>user->level) continue;
				  line[0] = 'I';
				  break;
				case 2 :
				  hidden++;  total--;
				  if (u != user && user->level<ADMIN) continue;
				  line[0] = 'H';
				  break;
				case 3 :
				  fake++;    total--;
				  if (u != user && user->level<GOD) continue;
				  line[0] = 'F';
				  break;
				default:
				  line[0] = ' ';
			}
			if (u->room->hidden) line[0] = '+';
			cnt = colour_com_count(line);
			if (u->room == rm)
			  if (in_room == 0)
			  {
				sprintf(text,"~FG--~CG==~CY>~CW %s ~CY<~CG==~FG--\n",u->room->recap);
				write_user(user,center(text,80));
				in_room = 1;
			  }
			writeus(user,"%-*.*s ~RS:~CY %-8.8s ~RS:  ~FG%-7.7s~RS: ~CW%-4.4d~RS:~CW%-2.2d\n",44+cnt*3,44+cnt*3,line,levelname,status,mins,idle);
		}
	}
	write_user(user,"\n~FG.-==--==--==--==~CG<~CY*~CG>~FG==--==--==--==--==~CG<~CY*~CG>~FG==--==--==--==--==~CG<~CY*~CG>~FG==--==--==--==-.\n");
	writeus(user,"~FG`-==--==--==--==~CG<~CY*~CG>~FG=~CG< ~FT[Total of ~CT%-2d ~FTIslanders signed on] ~CG>~FG=~CG<~CY*~CG>~FG==--==--==--==-'\n\n",total);
}

void mori_who(User user)
{
	User	u;
	Room	rm;
	int	cnt, cnt2, total, mins, idle, on_days, alternate,
		on_hours, on_mins, id_days, id_hours, id_mins, id_secs;
	char	line[500], timestr[16], status[30], levelname[10],
		idlestr[16], name[251];
	char	*mori_tline = "~FG/~FT-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-~FG\\\n";
	char	*mori_line  = "~FG-~FT=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-~FG|~FT-=-=--=-=-~FG|~FT-=-=-=-=-=-=-~FG|~FT-=-=-=~FG-\n";

	total = on_days = on_hours = on_mins = 0;
	id_days = id_hours = id_mins = id_secs = 0;
	idlestr[0] = timestr[0] = '\0';
	alternate = 0;
	write_user(user,mori_tline);
	write_user(user,"~FT       predesc, name and description        ~FT|~FG   rank   ~FT|~FG    where    ~FT| ~FGtime\n");
	write_user(user,mori_line);
	for_rooms(rm)
	{
		for_ausers(u)
		{
			if (u->room != rm || u->login) continue;
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room) && user->level<GOD) continue;
			if (u->type == CloneType) strcpy(name,u->recap);
			else
			{
				if (u->invis>Invis) strcpy(name,u->temp_recap);
				else strcpy(name,u->recap);
			}
			mins     = (int)(time(0) - u->last_login);
			on_days  = mins/86400;
			on_hours = (mins%86400)/3600;
			on_mins  = (mins%3600)/60;
			idle     = (int)(time(0) - u->last_input);
			id_days  = idle/86400;
			id_hours = (idle%86400)/3600;
			id_mins  = (idle%86400)/60;
			id_secs  = idle;
			if (u->lev_rank[0]) strcpy(levelname,u->lev_rank);
			else strcpy(levelname,user->gender == Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
			if (u->status & Jailed) strcpy(levelname,"JAILED");
			++total;
			if (u->predesc[0])
			{
				if (u->desc[0] == '\'') sprintf(line," ~RS%s~RS %s%s",u->predesc,name,u->desc);
				else sprintf(line," ~RS%s~RS %s~RS %s",u->predesc,name,u->desc);
			}
			else
			{
				if (u->desc[0] == '\'') sprintf(line," ~RS%s%s",name,u->desc);
				else sprintf(line," ~RS%s~RS %s",name,u->desc);
			}
			switch(u->invis)
			{
				case 1:
				  total--;
				  if (u->level>user->level) continue;
				  line[0] = '!';
				  break;
				case 2:
				  total--;
				  if (u != user && user->level<ADMIN) continue;
				  line[0] = '#';
				  break;
				case 3:
				  total--;
				  if (u != user && user->level<GOD) continue;
				  line[0] = '@';
				  break;
				default:
				  line[0] = ' ';
			}
			if (u->room->hidden) line[0] = '+';
			if (on_mins > 999)
			{
				if (on_days >= 1)
					sprintf(timestr,"%3dd",on_days);
				else if (on_hours >= 1)
					sprintf(timestr,"%3dh",on_hours);
			}
			else
				sprintf(timestr,"%-4.4d",on_mins);
			if (id_mins >= 99)
				strcpy(idlestr,"zz");
			else
				sprintf(idlestr,"%-2.2d",id_mins);
			if (alternate) sprintf(status,"%s~CT:~FT%s",timestr,idlestr);
			else sprintf(status,"%s~CG:~FG%s",timestr,idlestr);
			cnt = colour_com_count(line);
			cnt2 = colour_com_count(rm->recap);
			if (alternate)
			{
				writeus(user,"~RS%-*.*s ~FG|~FT %-8.8s~FG |~FT %-*.*s ~FG|~FT%-*s\n",43+cnt*3,43+cnt*3,line,levelname,11+cnt2*3,11+cnt2*3,rm->recap,7+colour_com_count(status)*3,status);
				alternate = 0;
			}
			else
			{
				writeus(user,"~RS%-*.*s ~FT|~FG %-8.8s~FT |~FG %-*.*s ~FT|~FG%-*s\n",43+cnt*3,43+cnt*3,line,levelname,11+cnt2*3,11+cnt2*3,rm->recap,7+colour_com_count(status)*3,status);
				alternate = 1;
			}
		}
	}
	writeus(user,"~FG\\~FT-=-=-=-=-=-=-=-=-~FG< [~FTThere %s currently ~FG%2d ~FTuser%s online~FG] >~FT-=-=-=-=-=-=-=-=-~FG/\n",total == 1?"is ":"are",total,total == 1?" ":"s");
}

void ei3_who(User user)
{
	User	u;
	Room	rm;
	int	cnt, total, mins, idle, in_room, cnt2;
	char	line[500], levelname[20], name[251], rmname[251];

	total = mins = idle = 0;
	write_user(user,"~CG.---------------------------- ~CYWho's on DragonS CovE ~CG--------------------------.\n");
	for_rooms(rm)
	{
		in_room = 0;
		for_ausers(u)
		{
			if (u->room != rm || u->login) continue;
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room) && user->level<GOD) continue;
			if (u->type == CloneType) strcpy(name,u->recap);
			else
			{
				if (u->invis>Invis) strcpy(name,u->temp_recap);
				else strcpy(name,u->recap);
			}
			mins=(int)(time(0) - u->last_login)/60;
			idle=(int)(time(0) - u->last_input)/60;
			++total;
			if (u->lev_rank[0]) strcpy(levelname,u->lev_rank);
			else strcpy(levelname,user->gender == Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
			if (u->status & Jailed) strcpy(levelname,"JAILED");
			if (u->predesc[0])
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s~RS %s%s~RS",u->predesc,name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s~RS %s~RS",u->predesc,name,u->desc);
			}
			else
			{
				if (u->desc[0] == '\'') sprintf(line,"  ~RS%s%s~RS",name,u->desc);
				else sprintf(line,"  ~RS%s~RS %s~RS",name,u->desc);
			}
			switch(u->invis)
			{
				case 1 :
				  total--;
				  if (u->level>user->level) continue;
				  line[0] = 'I';
				  break;
				case 2 :
				  total--;
				  if (u != user && user->level<ADMIN) continue;
				  line[0] = 'H';
				  break;
				case 3 :
				  total--;
				  if (u != user && user->level<GOD) continue;
				  line[0] = 'F';
				  break;
				default:
				  line[0] = ' ';
			}
			if (u->room->hidden) line[0] = '+';
			cnt = colour_com_count(line);
			if (u->room == rm)
			  if (in_room == 0)
			  {
				sprintf(rmname,"~CY%s~CY:",rm->recap);
				cnt2 = colour_com_count(rmname);
				write_user(user,"~CG|                                                                             |\n");
				writeus(user,"~CY + %-*s~CY                                                    ~CY--+\n",20+cnt2*3,rmname);
				in_room = 1;
			  }
			writeus(user,"~CG|~RS %-*.*s ~CW:~CY %-8.8s ~CW: ~RS%-5.5d~CW:~RS%-4.4d~CG  |\n",50+cnt*3,50+cnt*3,line,levelname,mins,idle);
		}

	}
	write_user(user,"~CG|                                                                             |\n");
	writeus(user,"~CG`---------------------- ~FTThere are ~CT%2d ~FTiSLaNDeRS signed on ~CG---------------------'\n",total);
}

/* Do the help */
void help(User user,char *inpstr,int rt)
{
	int lev;
	Commands com;
	char filename[80],*c;

	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count<2 || !strcasecmp(word[1],"commands"))
	{
		switch(user->help)
		{
			case 1 :
			  normal_help(user,0);
			  return;
			case 2 :
			  type_help(user,0);
			  return;
			case 3 :
			  command_help(user,0);
			  return;
			default:
			  normal_help(user,0);
			  return;
		}
	return;
	}
	/*
	   Check for any illegal crap in searched for filename so they cannot
	   list out the /etc/passwd file for instance.
	*/
	c = word[1];
	while(*c)
	{
		if (*c == '.' || *c++ == '/')
		{
			writeus(user,nohelp,word[1]);
			write_log(1,LOG1,"[Illegal Filename] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[1],command[com_num].name);
			return;
		}
	}
	if ((com=get_command(user,word[1]))==NULL)
	{
		/* Check to see if it's a level that we're searching for */
		if ((lev=get_level(word[1]))==-1)
		{
			writeus(user,wtf,word[1]);
			return;
		}
		if (lev>user->level)
		{
			write_user(user,"~CW - ~FGYou can't view the commands of a level higher then yours.\n");
			return;
		}
		help_level(user,lev);
		return;
	}
	if (com->min_lev>user->level)
	{
		writeus(user,"~FG You can't view the helpfile for that command.\n");
		return;
	}
	sprintf(filename,"%s/%s",HelpDir,com->name);
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  writeus(user,nohelp,com->name);
		  return;
		case 1:
		  user->misc_op=2;
	}
	return;
}

/* Show the commands available via normal NUTS 3.3.3 style */
void normal_help(User user,int wrap)
{
	Commands cmd;
	int com,cnt,lev,tot,lines;
	char temp[20];

	if (wrap == 0)
	{
		user->ignall_store = user->ignall;
		user->ignall = 1;
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~FRðÄÄ¯¯> ~CY Commands available for level~CB: [~FM%s~CB] ~FR<®®ÄÄð\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FR-=[ ~CYCommands available for level~CB: [~FM%s~CB] ~FR]=-\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		cmd		 = first_command;
	        user->helpw_lev  = NEWBIE;
	        user->helpw_com  = first_command->id;
	        user->helpw_same = 0;
		lines		 = 3;
	}
	if (wrap)		/* Find where we are */
	{
		lines = 0;
	        cmd   = first_command;
		while(cmd != NULL)
		{
			if (cmd->id == user->helpw_com) break;
			cmd = cmd->next;
		}
	}
	/* Scroll through the commands by level */
	tot = com = 0;
	for (lev = user->helpw_lev ; lev <= user->level ; ++lev)
	{
		cnt = 0;
		if (wrap == 0)
		{
			sprintf(text,"~CB:: ~FT%s\n",user->gender == Female?level[lev].fname:level[lev].mname);
		}
		else if (wrap && !user->helpw_same)
		{
			sprintf(text,"~CB:: ~FT%s\n",user->gender == Female?level[lev].fname:level[lev].mname);
		}
		else text[0] = '\0';
		user->helpw_same = 1;
		while (cmd != NULL)
		{
			if (cmd->min_lev != lev)
			{
				cmd = cmd->next;
				continue;
			}
			if (cmd->disabled)
			{
				cmd = cmd->next;
				continue;
			}
			sprintf(temp,"  %-11s",cmd->name);
			strcat(text,temp);
			++cnt;
			if (cnt >= 6)
			{
				strcat(text,"\n");
				write_user(user,text);
				text[0] = '\0';
				cnt = 0;
				++lines;
			}
			cmd = cmd->next;
			if (cmd == NULL) continue;
			if (user->rows != 0 && lines >= user->rows)
			{
				user->misc_op = 15;
				user->helpw_com = cmd->id;
				user->helpw_lev = lev;
				write_user(user,continue2);
				return;
			}
			if (cnt == 0) strcat(text,"");
		}
		if (cnt>0 && cnt<6)
		{
			strcat(text,"\n");
			write_user(user,text);
			++lines;
			memset(text,0,sizeof(text)-1);
		}
		user->helpw_same = 0;
		if (user->rows != 0 && lines >= user->rows)
		{
			user->misc_op = 15;
			user->helpw_com = first_command->id;
			user->helpw_lev = ++lev;
			write_user(user,continue2);
			return;
		}
		cmd = first_command;
	}
	cmd = first_command;
	while (cmd)
	{
		if (cmd->min_lev>user->level)
		{
			cmd = cmd->next;
			++com;
			continue;
		}
		cmd = cmd->next;
		++com;
		++tot;
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	writeus(user,"~FT  There are ~CR%d ~FTcommand%s total, of which you have ~CR%d~FT available to you!\n",com,com>1?"s":"",tot);
	write_user(user,"~FT  Please remember that all commands start with a ~CW'~CR.~CW'~FT or a ~CW'~CR,~CW'~FT.\n");
	write_user(user,"~FT  For help on a command type ~CW'~CM.help ~CW<~CRcommand~CW>'\n");
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
	user->helpw_com=user->helpw_lev=user->misc_op=user->helpw_same=0;
	rev_away(user);
}

/* Just shows the commands, nothing else.. just commands */
void command_help(User user,int wrap)
{
	Commands cmd;
	int com,cnt,tot,lines;
	char temp[51];

	if (wrap == 0)
	{
		user->ignall_store = user->ignall;
		user->ignall	   = 1;
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~FRðÄÄ¯¯> ~CY Commands available for level~CB: [~FM%s~CB] ~FR<®®ÄÄð\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FT-~CT=~CB[ ~FYCommands available for level~CB: [~FM%s~CB] ~CB]~CT=~FT-\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		lines		 = 3;
		text[0]		 = '\0';
		cmd		 = first_command;
		user->helpw_lev  = NEWBIE;
		user->helpw_com  = first_command->id;
		user->helpw_same = 0;
	}
	if (wrap)			/* Find where we are */
	{
		lines = 0;
		cmd   = first_command;
		while(cmd != NULL)
		{
			if (cmd->id == user->helpw_com) break;
			cmd = cmd->next;
		}
	}
	tot=cnt=com=0;
	while(cmd != NULL)
	{
		if (cmd->min_lev>user->level)
		{
			cmd = cmd->next;
			continue;
		}
		if (cmd->disabled)
		{
			cmd = cmd->next;
			continue;
		}
		sprintf(temp,"  ~FT%-11s",cmd->name);
		strcat(text,temp);
		cnt++;
		if (cnt == 6)
		{
			strcat(text,"\n");
			write_user(user,text);
			text[0] = '\0';
			cnt	= 0;
			++lines;
		}
		cmd = cmd->next;
		if (cmd == NULL) continue;
		if (user->rows && lines >= user->rows)
		{
                	user->misc_op	= 23;
			user->helpw_com = cmd->id;
			write_user(user,continue2);
			return;
		}
		if (cnt == 0) strcat(text,"");
	}
	if (cnt>0 && cnt<6)
	{
		strcat(text,"\n");
		write_user(user,text);
		++lines;
		text[0] = '\0';
	}
	cmd = first_command;
	while(cmd != NULL)
	{
		if (cmd->min_lev>user->level)
		{
			cmd = cmd->next;
			++com;
			continue;
		}
		cmd = cmd->next;
		++com;
		++tot;
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	writeus(user,"~FT  There are ~CR%d ~FTcommand%s total, of which you have ~CR%d~FT available to you!\n",com,com>1?"s":"",tot);
	write_user(user,"~FT  Please remember that all commands start with a ~CW'~CR.~CW'~FT or a ~CW'~CR,~CW'~FT.\n");
	write_user(user,"~FT  For help on a command type ~CW'~CM.help ~CW<~CRcommand~CW>'\n");
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
	user->helpw_com=user->helpw_lev=user->helpw_same=user->helpw_type=0;
	user->misc_op = 0;
	rev_away(user);
}

/*
  A sort by types, Andy gave me permission to use his amnuts201 code for this
  which is why the commands are done in their own structure now, I didn't
  know where to start with this, so I talked to Andy and he told me to just
  use the Amnuts201 code for it (With my own *SLIGHT* mods)
*/
void type_help(User user,int wrap)
{
	Commands cmd;
	int com,cnt,tot,lines,maxtype;
	char temp[50];

	if (wrap == 0)
	{
		user->ignall_store = user->ignall;
		user->ignall	   = 1;
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~FRðÄÄ¯¯> ~CY Commands available for level~CB: [~FM%s~CB] ~FR<®®ÄÄð\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FT-~CT=~CB[ ~CYCommands available for level~CB: [~FM%s~CB] ~CB]~CT=~FT-\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		lines		 = 3;
		cmd		 = first_command;
		user->helpw_type = 0;
	        user->helpw_lev	 = NEWBIE;
	        user->helpw_com	 = first_command->id;
	        user->helpw_same = 0;
	}
	if (wrap)			/* Find where we are */
	{
		lines = 0;
	        cmd   = first_command;
		while(cmd != NULL)
		{
			if (cmd->id == user->helpw_com) break;
			cmd = cmd->next;
		}
	}
	maxtype = 0;
	while (com_style[maxtype][0] != '*') ++maxtype;
	tot=com = 0;
	temp[0] = '\0';
	while (user->helpw_type <= maxtype)
	{
		cnt = 0;
		if (wrap == 0)
		{
			sprintf(text,"~CB:: ~FG%s\n",com_style[user->helpw_lev]);
			write_user(user,text);
			text[0] = '\0';
		}
		else if (wrap && !user->helpw_same
		  && com_style[user->helpw_lev][0] != '*')
		{
			sprintf(text,"~CB:: ~FG%s\n",com_style[user->helpw_lev]);
			write_user(user,text);
			text[0] = '\0';
		}
		user->helpw_same = 1;
		while (cmd != NULL)
		{
			if (cmd->type != user->helpw_type
			  || cmd->min_lev>user->level)
			{
				cmd = cmd->next;
				continue;
			}
			if (cmd->disabled)
			{
				cmd = cmd->next;
				continue;
			}
			cnt++;
			if (cnt >= 6) sprintf(temp," %s",cmd->name);
			else sprintf(temp," %-11s",cmd->name);
			strcat(text,temp);
			if (cnt >= 6)
			{
				strcat(text,"\n");
				write_user(user,text);
				text[0] = '\0';
				cnt	= 0;
				++lines;
				temp[0] = '\0';
			}
			cmd = cmd->next;
			if (cmd == NULL) continue;
			if (user->rows && lines >= user->rows)
			{
				user->misc_op	= 24;
				user->helpw_com = cmd->id;
				write_user(user,continue2);
				return;
			}
			if (cnt == 0)
			{
				text[0] = '\0';
				strcat(text,temp);
				temp[0] = '\0';
			}
		}
		user->helpw_type++;
		user->helpw_lev++;
		if (cnt)
		{
			strcat(text,"\n");
			write_user(user,text);
			++lines;  text[0] = '\0';
		}
		user->helpw_same = 0;
		if (user->rows && lines >= user->rows)
		{
			user->misc_op = 24;
			user->helpw_com = first_command->id;
			write_user(user,continue2);
			return;
		}
		cmd = first_command;
	}
	cmd = first_command;
	while (cmd != NULL)
	{
		if (cmd->min_lev>user->level)
		{
			++com;
			cmd = cmd->next;
			continue;
		}
		cmd = cmd->next;
		++com;
		++tot;
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	writeus(user,"~FT  There are ~CR%d ~FTcommand%s total, of which you have ~CR%d~FT available to you!\n",com,com>1?"s":"",tot);
	write_user(user,"~FT  Please remember that all commands start with a ~CW'~CR.~CW'~FT or a ~CW'~CR,~CW'~FT.\n");
	write_user(user,"~FT  For help on a command type ~CW'~CM.help ~CW<~CRcommand~CW>'\n");
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
	user->helpw_com=user->helpw_lev=user->misc_op=user->helpw_same=0;
	user->helpw_type = 0;
	rev_away(user);
}

/*
  this will allow a user to view the commands that they have at a certain
  level... so they could use .h god and it'll show GOD commands.
*/
void help_level(User user,int lev)
{
	Commands cmd;
	int cnt,tot,com;
	char temp[50];

	cnt=tot=com=0;
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~FRðÄÄ¯¯> ~CY Commands available for level~CB: [~FM%s~CB] ~FR<®®ÄÄð\n",user->gender==Female?level[lev].fname:level[lev].mname);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		sprintf(text,"~FT-~CT=~CB> ~CYCommands available for level~CB: [~FM%s~CB] ~CB<~CT=~FT-\n",user->gender==Female?level[lev].fname:level[lev].mname);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	text[0] = '\0';
	for_comms(cmd)
	{
		if (cmd->min_lev != lev)
		{
			++tot;
			continue;
		}
		sprintf(temp," %-11s",cmd->name);
		strcat(text,temp);
		cnt++;
		tot++;
		com++;
		if (cnt >= 6)
		{
			strcat(text,"\n");
			write_user(user,text);
			text[0] = '\0';
			cnt	= 0;
		}
	}
	if (cnt>0 && cnt<6)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	writeus(user,"~FT  There are ~CR%d ~FTcommands out of ~CR%d ~FTat level ~FT%s.\n",com,tot,user->gender==Female?level[lev].fname:level[lev].mname);
	write_user(user,"~FT  For help on a command type ~CW'~CM.help ~CW<~CRcommand~CW>'\n");
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
}

/* END OF WHOHELP.C */
