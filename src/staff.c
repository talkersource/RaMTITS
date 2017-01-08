/*
  staff.c
    Source code for the majority of staff commands, such as .allow, .deny
    and basically every command for levels >= WIZ

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Mike Irving, Neil Robertson, and Arnaud Abelard.

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

#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "include/stafflib.h"
#define _STAFF_C
  #include "include/needed.h"
#undef _STAFF_C
#include <unistd.h>
extern char *crypt __P ((__const char *__key, __const char *__salt));

/* Shutdown the talker */
void shutdown_com(User user,char *inpstr,int rt)
{

	if (user == NULL) return;
	if (user->tpromoted && user->orig_lev<GOD)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
	if (rs_which == 1)
	{
		write_user(user,"~FG The reboot countdown is currently active. Cancel it first.\n");
		return;
	}
	if (rs_which == 2)
	{
		write_user(user,"~FG The seamless reboot countdown is currently active. Cancel it first.\n");
		return;
	}
	if (!strncasecmp(word[1],"-cancel",2))
	{
		if (!rs_countdown || rs_which != 0)
		{
			write_user(user,"~FG The shutdown countdown isn't currently active!\n");
			return;
		}
		if (rs_countdown && !rs_which && rs_user == NULL)
		{
			write_user(user,"~FG Someone else has set the shutdown countdown.\n");
			return;
		}
		write_room(NULL,"~CB[~CRRaMTITS~CB] ~CW- ~CRShutdown cancelled!\n");
		write_log(1,LOG1,"[SHUTDOWN] - User: [%s] [CANCELLED]\n",user->name);
		rs_countdown	= 0;
		rs_announce	= 0;
		rs_which	= -1;
		rs_user		= NULL;
		return;
	}
	if (word_count<1 && !isnumber(word[1]))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CW<~CRsecs~CW|~CR-cancel~CW>~CB]");
		return;
	}
	if (rs_countdown && !rs_which)
	{
		write_user(user,"~CTThe shutdown countdown is currently active, cancel it first!\n");
		return;
	}
	if (word_count<2)
	{
		rs_countdown	= 0;
		rs_announce	= 0;
		rs_which	= -1;
		rs_user		= NULL;
	}
	else {
		rs_countdown	= atoi(word[1]);
		rs_which	= 0;
	}
	write_user(user,"\07~CB[~CGWarning~CB]~CW - ~CRThis will shutdown the talker!\n");
	write_user(user,"~FG Are you sure you want to do this?~CB:~RS ");
	user->misc_op		= 1;
	user->ignall_store 	= user->ignall;
	user->ignall		= 1;
	no_prompt		= 1;
}

/* reboot the talker */
void reboot_com(User user,char *inpstr,int rt)
{

	if (user == NULL) return;
	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
	if (rs_which == 0)
	{
		write_user(user,"~FG The shutdown countdown is currently active. Cancel it first.\n");
		return;
	}
	if (rs_which == 2)
	{
		write_user(user,"~FG The seamless reboot countdown is currently active. Cancel it first.\n");
		return;
	}
	if (!strncasecmp(word[1],"-cancel",2))
	{
		if (rs_countdown == 0)
		{
			write_user(user,"~FG The reboot countdown isn't currently active!\n");
			return;
		}
		if (rs_countdown && rs_user == NULL)
		{
			write_user(user,"~FG Someone else has set the reboot countdown.\n");
			return;
		}
		write_room(NULL,"~CB[~CRRaMTITS~CB]~CW - ~FRReboot cancelled!\n");
		write_log(1,LOG1,"%s cancelled the reboot.\n",user->name);
		rs_countdown	= 0;	rs_announce	= 0;
		rs_which	= -1;	rs_user		= NULL;
		return;
	}
	if (word_count<1 && !isnumber(word[1]))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRsecs~CW|~CR-cancel~CB]");
		return;
	}
	if (rs_countdown)
	{
		write_user(user,"~CTThe reboot countdown is currently active, cancel it first!\n");
		return;
	}
	if (word_count < 2)
	{
		rs_countdown	= 0;	rs_announce	= 0;
		rs_which	= -1;	rs_user		= NULL;
	}
	else
	{
		rs_countdown	= atoi(word[1]);
		rs_which	= 1;
	}
	write_user(user,"\07~CB[~CRWarning~CB]~CW - ~CRThis will reboot the talker!\n");
	write_user(user,"~FG Are you sure you want to do this?~CB:~RS ");
	user->misc_op	= 14;		user->ignall_store = user->ignall;
	user->ignall	= 1;		no_prompt	   = 1;
}

/* tell something to all wizzes and above, unless a levelname is specified */
void wiztell(User user,char *inpstr,int rt)
{
	char *type,*tstr;
	int lev;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[<~CRlevel~CB>] [~CRmessage~CB]");
		return;
	}
	if (user->drunk >= 10) alter_speech(inpstr,5);
	tstr = ColourStrip(inpstr);
	type = smiley_type(tstr,1);
	inpstr = replace_macros(user,inpstr);
	if ((lev = get_level(word[1])) == -1) lev = WIZ;
	else
	{
		if (lev < WIZ) lev = WIZ;
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CB[<~CRlevel~CB>] [~CRmessage~CB]");
			return;
		}
		if (lev > user->level)
		{
			write_user(user,"~FMYou can't wiztell to a higher level then your own.\n");
			return;
		}
		inpstr = remove_first(inpstr);
		if (!user->hat) sprintf(text,"~CB[~FM%s~CB]~FG %s~FY %ss%s~RS %s\n",level[lev].name,user->recap,type,user->sstyle,inpstr);
		else sprintf(text,"~CB[~FM%s~CB]~FG %s~FY %ss%s %s%s\n",level[lev].name,user->recap,type,user->sstyle,hatcolor[user->hat],inpstr);
		write_user(user,text);
		write_level(lev,user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CB[~FMWIZ~CB] ~FG%s~FY %ss%s~RS %s\n",user->recap,type,user->sstyle,inpstr);
	else sprintf(text,"~CB[~FMWIZ~CB] ~FG%s~FY %ss%s %s%s\n",user->recap,type,user->sstyle,hatcolor[user->hat],inpstr);
	write_user(user,text);
	write_level(WIZ,user,text);
	record_wiz(text);
}

/* emote something to all wizzes and above, unless a levelname is given */
void wizemote(User user,char *inpstr,int rt)
{
	int lev;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[<~CRlevel~CB>] [~CRmessage~CB]");
		return;
	}
	if (user->drunk >= 10) alter_speech(inpstr,5);
	inpstr = replace_macros(user,inpstr);
	if ((lev = get_level(word[1])) == -1) lev = WIZ;
	else
	{
		if (lev < WIZ) lev = WIZ;
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CB[<~CRlevel~CB>] [~CRmessage~CB]");
			return;
		}
		if (lev > user->level)
		{
			write_user(user,"~FMYou can't wizemote to a higher level then your own!\n");
			return;
		}
		inpstr = remove_first(inpstr);
		if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
		  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
		{
			if (!user->hat) sprintf(text,"~CB[~FM%s~CB] ~FG%s~CM%s\n",level[lev].name,user->recap,inpstr);
			else sprintf(text,"~CB[~FM%s~CB] ~FG%s%s%s\n",level[lev].name,user->recap,hatcolor[user->hat],inpstr);
		}
		else
		{
			if (!user->hat) sprintf(text,"~CB[~FM%s~CB] ~FG%s ~CM%s\n",level[lev].name,user->recap,inpstr);
			else sprintf(text,"~CB[~FM%s~CB] ~FG%s %s%s\n",level[lev].name,user->recap,hatcolor[user->hat],inpstr);
		}
		write_user(user,text);
		write_level(lev,user,text);
		return;
	}
	if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
	  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
	{
		if (!user->hat) sprintf(text,"~CB[~FMWIZ~CB] ~FG%s~CM%s\n",user->recap,inpstr);
		else sprintf(text,"~CB[~FMWIZ~CB] ~FG%s%s%s\n",user->recap,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[~FMWIZ~CB] ~FG%s ~CM%s\n",user->recap,inpstr);
		else sprintf(text,"~CB[~FMWIZ~CB] ~FG%s %s%s\n",user->recap,hatcolor[user->hat],inpstr);
	}
	write_user(user,text);
	write_level(WIZ,user,text);
	record_wiz(text);
}

/* Sets the talkers minimum login level */
void minlogin(User user,char *inpstr,int rt)
{
	User u,next;
	char levstr[5],*name;
	int lev,cnt;

	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRlevel~CW|~CRnone~CB]");
		return;
	}
	if ((lev = get_level(word[1])) == -1)
	{
		if (strcasecmp(word[1],"none"))
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRlevel~CW|~CRnone~CB]");
			return;
		}
		lev = -1;
		strcpy(levstr,"NONE");
	}
	else strcpy(levstr,level[lev].name);
	if (lev > user->level)
	{
		write_user(user,"~FM You can't set the minlogin level to a level above your own!\n");
		return;
	}
	if (minlogin_level == lev)
	{
		write_user(user,"~FM The minlogin level is already set to that!\n");
		return;
	}
	minlogin_level = lev;
	writeus(user,"~FY Minlogin level has been set to~CB: [~CR%s~CB]\n",levstr);
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	WriteRoomExcept(NULL,user,"~CT%s~FY has set the minlogin level to~CB: [~CR%s~CB]\n",name,levstr);
	write_log(1,LOG1,"[MINLOGIN] User: [%s] Level: [%s]\n",user->name,levstr);
	/* Kick off anyone below the level */
	cnt = 0;  u = user_first;
	while(u != NULL)
	{
		next = u->next;
		if (!u->login && u->type != CloneType && u->level<lev)
		{
			write_user(u,"~FR Your level is below that of the minlogin level..disconnecting your socket...\n");
			disconnect_user(u,1);
			++cnt;
		}
		u = next;
	}
	writeus(user,"~FMTotal of ~CR%d~FM user%s %s disconnected.\n",cnt,cnt>1?"s":"",cnt>1?"were":"was");
}

/* Free up a hung socket */
void clearline(User user,char *inpstr,int rt)
{
	User u;
	int sock,found = 0;

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
	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 2 || !isnumber(word[1]))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRline~CB]");
		return;
	}
	sock = atoi(word[1]);
	/* Find the line */
	for_users(u)
	{
		if (u->type != CloneType && u->socket == sock)
		{
			found = 1;
			break;
		}
	}
	if (found == 0)
	{
		write_user(user,"~CW- ~FTSorry, that line isn't currently active.\n");
		return;
	}
	if (u->login == 0)
	{
		write_user(user,"~CW- ~FTSorry, you cannot clear the line of a logged on user.\n");
		return;
	}
	write_user(u,"\n~CW- ~CRThis line is being cleared....\n");
	write_log(1,LOG4,"[CLEARLINE] - User: [%s] Line: [%d]\n",user->name,sock);
	writeus(user,"~CW - ~FGLine ~FY%d ~FGcleared..\n",sock);
	disconnect_user(u,1);
}

/* Toggle either the swearing ban on/off, or the arrest_swearers on/off */
void swban(User user,char *inpstr,int rt)
{

	if (user->tpromoted && user->orig_lev<GOD)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
		writeus(user,usage,command[com_num].name,"~CB[~CR-ban~CW|~CR-arrest~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-ban",2))
	{
		if (ban_swearing == 0)
		{
			if (arrest_swearers)
			{
				write_user(user,"~FG The swearing ban is set to arrest users, you must toggle that off first.\n");
				return;
			}
			write_user(user,"~FT Swearing ban ~CRON!\n");
			write_log(1,LOG4,"[SWEARBAN] - User: [%s] Type: [BAN-ON]\n",user->name);
			ban_swearing = 1;
			return;
		}
		write_user(user,"~FT Swearing ban~CR OFF.\n");
		write_log(1,LOG4,"[SWEARBAN] - User: [%s] Type: [BAN-OFF]\n",user->name);
		ban_swearing = 0;
		return;
	}
	if (!strncasecmp(word[1],"-arrest",2))
	{
		if (arrest_swearers == 0)
		{
			if (ban_swearing)
			{
				write_user(user,"~FG The swearing ban is set to ban, you must toggle this off first!\n");
				return;
			}
			write_user(user,"~FT Swearing arrest ban ~CRON!\n");
			write_log(1,LOG4,"[SWEARBAN] - User: [%s] Type: [ARREST-ON]\n",user->name);
			arrest_swearers = 1;
			return;
		}
		write_user(user,"~FT Swearing arrest ban~CR OFF.\n");
		write_log(1,LOG4,"[SWEARBAN] - User: [%s] Type: [ARREST-OFF]\n",user->name);
		arrest_swearers = 0;
		return;
	}
	writeus(user,usage,command[com_num].name,"~CB[~CR-ban~CW|~CR-arrest~CB]");
	return;
}
	
/* creates a clone in another room, or the room the user is in */
void create_clone(User user,char *inpstr,int rt)
{
	User	u;
	Room	rm;
	int	cnt, i, c_cnt;
	char	*rmname,clone_name[25];

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
	if (user->room->hidden)
	{
		write_user(user,"~CW-~FT You can't create clones while in hidden rooms.\n");
		return;
	}
	if (user->invis)
	{
		write_user(user,"~FG What's the point of creating a clone if your hiding?\n");
		return;
	}
	if (word_count < 2) rm = user->room;
	else
	{
		if ((rmname = get_room_full(user,inpstr)) == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
		if ((rm = get_room(inpstr)) == NULL)
		{
			if ((rm = create_room()) == NULL)
			{
				write_user(user,"~CW-~FT Unable to create a clone in that room.\n");
				return;
			}
			strcpy(rm->name,rmname);
			if ((LoadRoom(rm,rmname)) < 1)
			{
				write_user(user,"~CW-~FT Unable to create a clone in that room.\n");
				destruct_room(rm);
				return;
			}
		}
	}
	if (!has_room_access(user,rm))
	{
		write_user(user,"~FG That room is currently locked, you cannot create a clone there.\n");
		return;
	}
	/* if personal room and don't have an invite no can do */
	if ((rm->access & PERSONAL && user->invite_room != rm)
	  && strcmp(rm->owner,user->name))
	{
		write_user(user,"~FT That is a personal room in which you don't own.\n");
		return;
	}
	/* count clones....no point in having 2 in one room */
	cnt = 0;
	for_users(u)
	{
		if (u->type == CloneType && u->owner == user)
		{
			if (u->room == rm)
			{
				writeus(user,"~FMWhats the point of having 2 clones in the ~FT%s?\n",rm->recap);
				return;
			}
			if (++cnt == max_clones)
			{
				write_user(user,"~FTYou already have the max number of clones permitted!\n");
				return;
			}
		}
	}
	/* Create the clone */
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FM An error occurred in creating your clone.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create user copy in clone().\n");
		return;
	}
	/*
	  We do all the writing here to prevent it to writing the stuff and
	  the user seeing it through the clone.
	*/
	if (rm == user->room) write_user(user,"~FT You chant some ancient black magic, creating a clone here...\n");
	else writeus(user,"~FT You chant some ancient black magic creating a clone in the ~FR%s~FT!\n",rm->name);
	WriteRoomExcept(user->room,user,"~FM%s ~FTchants some ancient black magic....\n",user->recap);
	if (user->predesc[0]) WriteRoomExcept(rm,user,"%s %s %s\n",user->predesc,user->recap,user->in_phr);
	else WriteRoomExcept(rm,user,"%s %s\n",user->recap,user->in_phr);
	/* Now we set up the clone stuff. */
	u->type		= CloneType;
	u->socket	= user->socket;
	u->room		= rm;
	u->owner	= user;
	u->gender	= user->gender;
	u->hat		= user->hat;
	u->level	= user->level;
	u->colour	= user->colour;
	u->ansi_on	= user->ansi_on;
	u->last_login	= user->last_login;
	u->level	= user->level;
	u->car_return	= user->car_return;
	u->char_echo	= user->char_echo;
	u->rows		= user->rows;
	u->cols		= user->cols;
	c_cnt = 0;
	if (user->clone_cnt[c_cnt] == 1)
	{
		do
		{
			++c_cnt;
			sprintf(clone_name,"%c%c%c%c%d",user->name[0],user->name[1],user->name[2],user->name[3],c_cnt);
		} while(user->clone_cnt[c_cnt] == 1 || c_cnt >= 20);
	}
	if (c_cnt >= 20)
	{
		write_user(user,"~CW-~FT An error occurred, the talker detects you already have 20 clones, you cannot have more then this.\n");
		return;
	}
	strcpy(u->name,user->name);
	strcpy(u->recap,user->recap);
	strcpy(u->desc,user->desc);
	strcpy(u->predesc,user->predesc);
	strcpy(u->lev_rank,user->lev_rank);
	strcpy(u->sstyle,user->sstyle);
	for (i = 0; i < 17; ++i)
		strcpy(u->fmacros[i],user->fmacros[i]);
	for (i = 0; i < 17; ++i)
		strcpy(u->rmacros[i],user->rmacros[i]);
	for (i = 0; i < 25; ++i)
		strcpy(u->alias.find[i],user->alias.find[i]);
	for (i = 0; i < 25; ++i)
		strcpy(u->alias.alias[i],user->alias.alias[i]);
	for (i = 0; i < 28; ++i)
		u->scut[i].scut = user->scut[i].scut;
	for (i = 0; i < 28; ++i)
		strcpy(u->scut[i].scom,user->scut[i].scom);
	alpha_sort(u);
	user->clone_cnt[c_cnt] = 1;
	return;
}

/* Now destroy that pesky clone */
void destroy_clone(User user,char *inpstr,int rt)
{
	User u,u2;
	Room rm = NULL,rm2 = NULL;
	char *name,*rmname,c_name[UserNameLen+1];
	int c_cnt = -1;

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
	/* check room and users */
	if (word_count < 2) rm = user->room;
	else
	{
		rmname = get_room_full(user,inpstr);
		rm = get_room(rmname);
		if (rm == NULL)
		{
			rm = get_room_recap(inpstr);
		}
	}
	if (rm == NULL && word_count > 2)
	{
		if ((u2 = get_name(user,word[2])) == NULL)
		{
			write_user(user,center(notloggedon,80));
			return;
		}
		if (u2->level >= user->level && user->level < OWNER)
		{
			write_user(user,"~FMSorry, you can't destroy a clone of a user of an equal or higher level then yours.\n");
			writeus(u2,"~FT %s~FG attempted to snuff out your clone..\n",user->recap);
			return;
		}
		rm = u2->room;
	}
	else u2 = user;
	if (rm == NULL) rm = user->room;
	for_users(u)
	{
		if (u->type == CloneType && u->room == rm && u->owner == u2)
		{
			sscanf(u->name,"%c%c%c%c%d",&c_name[0],&c_name[1],&c_name[2],&c_name[3],&c_cnt);
			destruct_alpha(u);
			rm2 = u->room;
			destruct_user(u);
			reset_access(rm);
			destruct_room(rm2);
			if (c_cnt == -1) write_user(user,"Error?\n");
			else writeus(user,"c_cnt = %d.\n",c_cnt);
			u->owner->clone_cnt[c_cnt] = 0;
			writeus(user,"~CW-~RS You summon up mysterious forces to swallow the clone of ~CR%s ~RSalive..\n",u2->invis>Invis?u2->temp_recap:u2->recap);
			WriteRoomExcept(rm,user,"~FT %s~RS watches helplessly as a mysterious force encloses over %s...\n",u2->recap,gender2[u2->gender]);
			if (user->invis == Invis)  name = invisname;
			else name = user->recap;
			if (u2 != user) writeus(u2,"~CB[~CWSYSTEM~CB] - ~FY %s~RS destroyed your clone in the ~FR%s~RS.\n",name,rm->recap);
			return;
		}
	}
	if (u2 == user) writeus(user,"~FR You don't have a clone in the ~FM%s~FR.\n",rm->recap);
	else writeus(user,"~FT%s~FR doesn't have a clone in the ~FM%s~FR.\n",u2->recap,rm->recap);
}

/* Shows the user a list of their clones */
void myclones(User user,char *inpstr,int rt)
{
	User u;
	int cnt;

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
	cnt = 0;
	for_users(u)
	{
		if (u->type != CloneType && u->owner != user) continue;
		if (++cnt == 1)
		{
			if (user->ansi_on)
			{
				write_user(user,ansi_tline);
				write_user(user,center("~CBÄÄÄ¯¯> ~FTRooms that you have clones in... ~CB<®®ÄÄÄ\n",80));
				write_user(user,ansi_bline);
			}
			else
			{
				write_user(user,ascii_tline);
				write_user(user,center("~FT-~CT=~CB> ~FGRooms that you have clones in... ~CB<~CT=~FT-\n",80));
				write_user(user,ascii_bline);
			}
		}
		writeus(user,"    ~CRRoom name~CB: ~FG%s\n",u->room->recap);
	}
	if (cnt == 0)
	{
		write_user(user,"~FR You don't have any clones...\n");
		return;
	}
	else
	{
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		writeus(user,"~CW- ~FGYou have a total of ~CY%d~FG ~FGclone%s..\n",cnt,cnt>1?"s":"");
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_line);
	}
}

/* Users swap rooms with their clones */
void clone_switch(User user,char *inpstr,int rt)
{
	User u;
	Room rm;
	char *rmname;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRroom clone is in~CB]");
		return;
	}
	rmname = get_room_full(NULL,inpstr);
	rm = get_room(rmname);
	if (rm == NULL)
	{
		rm = get_room_recap(inpstr);
		if (rm == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
	}
	for_users(u)
	{
		if (u->type == CloneType && u->room == rm && u->owner == user)
		{
			write_user(user,"~CW-~RS A mysterious force swallows this body so you seek refuge in another one...\n");
			u->room = user->room;
			user->room = rm;
			return;
		}
	}
	write_user(user,"~FT You don't have a clone in that room.\n");
}

/* set what you want your clone to hear, speech,swears,or nothing */
void clone_hear(User user,char *inpstr,int rt)
{
	Room rm;
	User u;

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
	if ((word_count < 3)
	  || (strcasecmp(word[2],"all")
	  &&    strcasecmp(word[2],"swears")
	  &&    strcasecmp(word[2],"nothing")))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRroom clone is in~CB] [~CRall~CW|~CRswears~CW|~CRnothing~CB]");
		return;
	}
	if ((rm = get_room(word[1])) == NULL)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	for (u = user_first ; u != NULL ; u = u->next)
	{
		if (u->type == CloneType && u->room == rm && u->owner == user)
		break;
	}
	if (u == NULL)
	{
		write_user(user,"~FMYou don't have a clone in that room...\n");
		return;
	}
	if (!strcasecmp(word[2],"all"))
	{
		u->clone_hear = Clone_All;
		write_user(user,"~FM Your clone will now hear everything in the room!\n");
		return;
	}
	if (!strcasecmp(word[2],"swears"))
	{
		u->clone_hear = Clone_Swears;
		write_user(user,"~FM Your clone will now hear swearing in the room!\n");
		return;
	}
	u->clone_hear = Clone_Nothing;
	write_user(user,"~FM Your clone will not hear anything...\n");
}

/*
   If word count<2 then it will do what .people did. list the users, and the
   sockets/sites they are connected on. Otherwise, get the given users site
*/
void site(User user,char *inpstr,int rt)
{
	User u;
	int total,logins,err=0;
	char portstr[16],sockstr[16],levname[20],*uname;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		total = 0;
		logins = 0;
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			write_user(user,center("~FTÄÄÄ¯¯> ~FMCurrent people online... ~FT<®®ÄÄÄ\n",80));
			write_user(user,"~FTÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍËÍÍÍÍÍÍÍËÍÍÍËÍÍÍËÍÍÑÍÍÍÍËÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»\n");
			write_user(user," ~FTName            ~FTº ~FGLevel ~FTº~CRLine~FTº~FGPort ~FTº~FR IP             ~FT| ~FRSite\n");
			write_user(user,"~FTÌÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÎÍÍÍÍÍÍÍÎÍÍÍÍÍÍÍÎÍÍØÍÍÍÍÎÍÍÍÍÎÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¹\n");
		}
		else
		{
			write_user(user,ascii_tline);
			write_user(user,center("~FT-~CT=~CB> ~FGCurrent people online... ~CB<~CT=~FT-\n",80));
			write_user(user,ascii_bline);
			write_user(user," ~FTName            ~FT| ~FGLevel ~FT|~CRLine~FT|~FGPort ~FT|~FR IP             ~FT| ~FRSite\n");
			write_user(user,ascii_line);
		}
		for_users(u)
		{
			if (u->type == CloneType) continue;
			if (u->port==port[0]) strcpy(portstr,"User ");
			else strcpy(portstr,"Staff");
			if (u->status & Jailed) strcpy(levname,"ARRESTED");
			else strcpy(levname,user->gender == Female?level[u->level].fname:level[u->level].mname);
			if (u->login)
			{
				if (user->ansi_on) writeus(user," ~CB[~FTLogin stage ~FG%2d~CB]~FTº~FG  n/a  ~FTº~CR%4d~FTº~FG%-5.5s~FTº~FR %-15.15s~FT|~FR %-23.23s\n",u->login,u->socket,portstr,u->ip_site,u->site);
				else writeus(user," ~CB[~FTLogin stage ~FG%2d~CB]~FT|~FG  n/a  ~FT|~CR%4d~FT|~FG%-5.5s~FT|~FR %-15.15s~FT| ~FR%-23.23s\n",u->login,u->socket,portstr,u->ip_site,u->site);
				logins++;
				continue;
			}
			++total;
			sprintf(sockstr,"%4d",u->socket);
			if (user->ansi_on) writeus(user," ~FT%-15.15s ~FTº~FG%-7.7s~FTº~CR%s~FTº~FG%-5.5s~FTº~FR %-15.15s~FT| ~FR%-23.23s\n",u->name,levname,sockstr,portstr,u->ip_site,u->site);
			else writeus(user," ~FT%-15.15s ~FT|~FG%-7.7s~FT|~CR%s~FT|~FG%-5.5s~FT|~FR %-15.15s~FT| ~FR%-23.23s\n",u->name,levname,sockstr,portstr,u->ip_site,u->site);
		}
		if (user->ansi_on) write_user(user,"~FTÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÊÍÍÍÍÍÍÍÊÍÍÍÍÊÍÍÍÊÍÍÏÍÍÍÍÊÍÍÍÍÍÊÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼\n");
		else write_user(user,ascii_tline);
		if (logins)
		{
			writeus(user,"  ~FGThere %s a total of ~FY%d ~FGlogin%s....\n",logins>1?"are":"is",logins,logins>1?"s":"");
		}
		writeus(user,"  ~FGThere %s a total of ~FY%d ~FGuser%s online...\n",total>1?"are":"is",total,total>1?"s":"");
		if (user->ansi_on) write_user(user,ansi_line);
		else write_user(user,ascii_bline);
		return;
	}
	/*
	  Now this is if word_count is > 2.. we try and display the site of
	  the given user providing that they actually exist.
	*/
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_name(user,uname)) != NULL)
	{
		writeus(user,"~FG %s ~CTis logged in from~CB: [~FT%s~CW:~FT%s~FM Port~CB:~FT%d~CB]\n",u->recap,u->site,u->ip_site,u->site_port);
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT Sorry, an error occurred, unable to get information.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temp. user object in site().\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	writeus(user,"~FG %s ~CTwas last logged in from~CB: [~FT%s~CB]\n",u->recap,u->last_site);
	destruct_user(u);
	destructed = 0;
}

/* Shows the system details of the talker */
void system_details(User user,char *inpstr,int rt)
{
	Room rm;
	User u;
	Telnet t;
	char bstr[40],minlogin[5],tstr[150];
	char *ca[] = { "NONE  ","IGNORE","REBOOT","SREBOOT" };
	int days,hours,mins,secs,telnets,rms,num_clones;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~CRðÄÄ¯¯> ~FG%s ~CYtalker running ~FG%s ~CYsystem stats. ~CR<®®ÄÄð\n",TalkerRecap,VERSION);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYtalker running ~FG%s ~FYsystem statistics. ~CB<~CT=~FT-\n",TalkerRecap,VERSION);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	/* get us some values */
	strcpy(bstr,ctime(&boot_time));
	secs  = (int)(time(0)-boot_time);
	days  = secs/86400;
	hours = (secs%86400)/3600;
	mins  = (secs%3600)/60;
	secs  = secs%60;
	if (!days && !hours && !mins) sprintf(tstr,"~CY%d ~FGsecond%s.",secs,secs>1?"s":"");
	else if (!days && !hours) sprintf(tstr,"~CY%d ~FGminute%s, and ~CY%d~FG second%s.",mins,mins>1?"s":"",secs,secs>1?"s":"");
	else if (!days) sprintf(tstr,"~CY%d ~FGhour%s, ~CY%d~FG minute%s, and ~CY%d ~FGsecond%s.",hours,hours>1?"s":"",mins,mins>1?"s":"",secs,secs>1?"s":"");
	else sprintf(tstr,"~CY%d ~FGday%s, ~CY%d~FG hour%s, ~CY%d  ~FGminute%s, and ~CY%d ~FGsecond%s.",days,days>1?"s":"",hours,hours>1?"s":"",mins,mins>1?"s":"",secs,secs>1?"s":"");
	num_clones = 0;		telnets = 0;
	for_users(u)
	{
		if (u->type == CloneType) num_clones++;
	}
	rms = 0;
	for_rooms(rm) ++rms;
	for_telnet(t) ++telnets;
	if (minlogin_level == -1) strcpy(minlogin,"NONE");
	else strcpy(minlogin,level[minlogin_level].name);
	/* Header parameters */
	writeus(user,"~FT   Process ID~CB: ~FY%d\n~FT   Talker booted~CB: ~FY%s~FT   Uptime~CB: %s\n",getpid(),bstr,tstr);
	writeus(user,"~FT   Ports ~CW[~CRmain~CW|~CRwiz~CW]~CB: ~FY%d~FG, & ~FY%d\n",port[0],port[1]);
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	writeus(user,"~FTMaximum users permitted~CB  =~FY %4.4d         ~FTNumber of users on~CB      =~FY %4.4d\n",max_users,num_of_users);
	writeus(user,"~FTMaximum clones permitted~CB =~FY %4.4d         ~FTNumber of clones on~CB     =~FY %4.4d\n",max_clones,num_clones);
	writeus(user,"~FTCurrent minlogin level~CB   =~FY %-11.11s  ~FTLogin idle time out~CB     =~FY %d ~CWsecs.\n",minlogin,login_idle_time);
	writeus(user,"~FTUser idle time out~CB       =~FY %4.4d ~CWsecs.   ~FTHeartbeat~CB               =~FY %4.4d\n",user_idle_time,heartbeat);
	writeus(user,"~FTWizport minlogin level~CB   =~FY %-11.11s  ~FTGatecrash level~CB         =~FY %s\n",level[wizport_level].name,level[gatecrash_level].name);
	writeus(user,"~FTTime out maxlevel~CB        =~FY %-11.11s  ~FTPrivate room min count~CB  =~FY %d\n",level[time_out_maxlevel].name,min_private_users);
	writeus(user,"~FTMessage lifetime~CB         =~FY %2.2d ~CWdays      ~FTMessage check time~CB      =~FY %02d~CW:~FY%02d\n",mesg_life,mesg_check_hour,mesg_check_min);
	writeus(user,"~FTNumber of rooms~CB          =~FY %4.4d         ~FTNumber of telnets~CB       =~FY %d\n",rms,telnets);
	writeus(user,"~FTIgnoring sigterm~CB         =~FY %-11.11s  ~FTEchoing password~CB        =~FY %s\n",noyes2[ignore_sigterm],noyes2[password_echo]);
	writeus(user,"~FTSwearing banned~CB          =~FY %-11.11s  ~FTTime out afks~CB           =~FY %s\n",noyes2[ban_swearing],noyes2[time_out_afks]);
	writeus(user,"~FTNew user prompt def.~CB     =~FY %-11.11s  ~FTFree Rooms~CB              =~FY %s\n",offon[prompt_def],noyes2[free_rooms]);
	writeus(user,"~FTNew user charecho def.~CB   =~FY %-11.11s  ~FTCrash action~CB            =~FY %s\n",offon[charecho_def],ca[crash_action]);
	writeus(user,"~FTTalker backup~CB            =~FY %-11.11s  ~FTTalker debugging~CB        =~FY %s\n",offon[backup_talker],offon[debug_talker]);
	writeus(user,"~FTWebpage generator~CB        =~FY %-11.11s  ~FTArrest swearers~CB         =~FY %s\n",offon[webpage_on],noyes2[arrest_swearers]);
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
}

/* Kill that fucking user who's pissing you off */
void kill_user(User user,char *inpstr,int type)
{
	User victim;
	char *name;
	int kill_num,i;

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
	if (user->tpromoted && user->orig_lev<WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	switch(type)
	{
		case 1:
		  if (word_count<2)
		  {
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CR-list~CB] [<~CR# of kill msg~CB>]");
			return;
		  }
		  if (!strncasecmp(word[1],"-list",2))
		  {
			for (i = 1 ; i < KILL_MESSAGES ; ++i)
			{
				sprintf(text,kill_text[i],"[victim]");
				writeus(user,"~FT%2.2d~CB]~RS %s",i,text);
			}
			return;
		  }
		  if ((victim = get_name(user,word[1])) == NULL)
		  {
			write_user(user,center(notloggedon,80));
			return;
		  }
		  if (user == victim)
		  {
			write_user(user,"~FMWhy would you want to kill yourself??\n");
			return;
		  }
		  if (victim->level >= user->level
		    && strcmp(user->name,TalkerOwner))
		  {
			write_user(user,"~FMYou can't kill someone of a greater or equal level then your own.\n");
			writeus(victim,"~FG%s~FM tried to kill you.\n",user->recap);
			return;
		  }
		  kill_num = atoi(word[2]);
		  if (!kill_num) kill_num = ran((unsigned)KILL_MESSAGES);
		  sprintf(text,"[KILL] - User: [%s] Victim: [%s]\n",user->name,victim->name);
		  write_log(1,LOG4,text);
		  write_record(victim,1,text);
		  write_user(user,"~CW-~RS You call upon the powers that be...\n");
		  sprintf(text,kill_text[kill_num],victim->recap);
		  write_room(NULL,text);
		  disconnect_user(victim,1);
		  write_room(NULL,"~CB:: ~FTAn evil laugh can be heard in the distance...\n");
		  return;
		case 2:
		  if (word_count<2)
		  {
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
			return;
		  }
		  if ((victim = get_name(user,word[1])) == NULL)
		  {
			write_user(user,center(notloggedon,80));
			return;
		  }
	  	  if (user == victim)
		  {
			write_user(user,"~FMWhy would you want to boot yourself??\n");
			return;
		  }
		  if (victim->level >= user->level
		    && strcmp(user->name,TalkerOwner))
		  {
			write_user(user,"~FMYou can't boot someone of a greater or equal level then your own!\n");
			writeus(victim,"~FG%s~FM tried to boot you off the talker.\n",user->recap);
			return;
		  }
		  sprintf(text,"[BOOT] - User: [%s] Victim: [%s]\n",user->name,victim->name);
		  write_log(1,LOG4,text);
		  write_record(victim,1,text);
		  WriteLevel(WIZ,user,"~FY %s~CR booted ~FY%s~FG off the talker.\n",name,victim->recap);
		  disconnect_user(victim,1);
		  return;
	}
}

/*
  Promote a user and temp promote a user, put it all into one function cuz I
  thought it would be easier, and look better.
*/
void promote(User user,char *inpstr,int type)
{
	User u;
	Room rm;
	char text2[ARR_SIZE],*name,*uname;
	int lev = 0,err = 0;

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
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	switch (type)
	{
		case 1:
		  if (word_count < 2 || word_count > 3)
		  {
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CRlevel~CB>]");
			return;
		  }
		  if (user->tpromoted && user->orig_lev<ADMIN)
		  {
			writeus(user,tempnodo,command[com_num].name);
			return;
		  }
		  if (word_count == 3)
		  {
			if ((lev = get_level(word[2])) == -1)
			{
				write_user(user,center(nosuchlev,80));
				return;
			}
		  }
		  /* Checks to see if the user is on right now */
		  if ((uname = get_user_full(user,word[1])) == NULL)
		  {
			write_user(user,center(nosuchuser,80));
			return;
		  }
		  if ((u = get_user(uname)))
		  {
			if (word_count == 3 && lev <= u->level)
			{
				write_user(user,"~FG Umm, I think ~CW'~CR.demote~CW'~FG would work better.\n");
				return;
			}
			if (word_count == 3 && lev > user->level)
			{
				write_user(user,"~FG You can't promote a user to a level greater then your own!\n");
				return;
			}
			if (u == user)
			{
				write_user(user,"~FG Well, that was rather pointless don't ya think? :P\n");
				return;
			}
			if (u->level >= user->level
			  && strcmp(user->name,TalkerOwner))
			{
				write_user(user,"~FG You can't promote someone to a higher level then your own!\n");
				return;
			}
			if (u->level == POWER && user->level<ADMIN)
			{
				write_user(user,"~FMYou cannot promote someone to the same level as your own!\n");
				return;
			}
			if (word_count == 3)
			{
				if (u->cloak_lev == u->level) u->cloak_lev = lev;
				u->level = lev;
			}
			else
			{
				if (u->cloak_lev == u->level) u->cloak_lev++;
				u->level++;
			}
			writeus(user,"~FG You deemed ~FY%s~FG worthy of level~CB: [~CT%s~CB]\n",u->recap,user->gender==Female?level[u->level].fname:level[u->level].mname);
			rm = user->room;
			sprintf(text,"~FY %s ~FGhas deemed ~FY%s~FG worthy of level~CB: [~CT%s~CB]\n",name,u->recap,level[u->level].name);
			write_room_except2(NULL,u,user,text);
			writeus(u,"~FY %s ~FGhas deemed you worthy of level~CB: [~CT%s~CB]\n",name,u->gender==Female?level[u->level].fname:level[u->level].mname);
			sprintf(text,"[PROMOTE]: User: [%s] Recipient: [%s] Level: [%s]\n",user->name,u->name,level[u->level].name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			clean_userlist(u->name);
			add_userlist(u->name,u->level);
			if (u->level >= WIZ && !is_staff(u))
			{
				writeus(user,"~FG Would you like to add '%s' to the staff_file?~CB:~RS ",u->name);
				strcpy(user->check,u->name);
				user->ignall_store = user->ignall;
				user->ignall = 1;
				user->misc_op = 35;
				no_prompt = 1;
			}
			return;
		  }
		  /* User not logged on, create a temporary user structure
		     change the level, then destruct it
		  */
		  if ((u = create_user()) == NULL)
		  {
			writeus(user,"~CW-~FT An error occurred in promoting.. try again later..\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in promote().\n");
			return;
		  }
		  err = LoadUser(u,uname);
		  if (err == 0)
		  {
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  else if (err == -1)
		  {
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (word_count == 3 && lev <= u->level)
		  {
			write_user(user,"~FG Umm, I think ~CW'~CR.demote~CW'~FG would work better.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (word_count == 3 && lev>user->level)
		  {
			write_user(user,"~FG You can't promote a user to a level greater then your own!\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (u->level >= user->level
		    && strcmp(user->name,TalkerOwner))
		  {
			write_user(user,"~FMYou can't promote someone to a higher level then your own!\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (u->level == OWNER)
		  {
			write_user(user,"~FM You can't temp promote someone to a level that doesn't exist.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (u->level == POWER && user->level < ADMIN)
		  {
			write_user(user,"~FMYou cannot promote someone to the same level as your own!\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (word_count == 3) u->level = lev;  else u->level++;
		  u->socket = -2;
		  strcpy(u->site,u->last_site);
		  SaveUser(u,0);
		  writeus(user,"~FGYou deemed ~FY%s~FG worthy of level~CB: [~CT%s~CB]\n",u->recap,user->gender==Female?level[u->level].fname:level[u->level].mname);
		  sprintf(text2,"~FG It seems ~FM%s~FG has deemed you worthy of level~CB: [~CR%s~CB]\n",user->recap,u->gender==Female?level[u->level].fname:level[u->level].mname);
		  send_oneline_mail(NULL,u->name,text2);
		  sprintf(text,"[PROMOTE] - User: [%s] Recipient: [%s] Level: [%s]\n",user->name,u->name,level[u->level].name);
		  write_log(1,LOG4,text);
		  write_record(u,1,text);
		  clean_userlist(u->name);
		  add_userlist(u->name,u->level);
		  if (u->level >= WIZ && !is_staff(u))
		  {
			writeus(user,"~FG Would you like to add '%s' to the staff_file?~CB:~RS ",u->name);
			strcpy(user->check,u->name);
			user->ignall_store = user->ignall;
			user->ignall = 1;
			user->misc_op = 35;
			no_prompt = 1;
		  }
		  destruct_user(u);
		  destructed=0;
		  return;
		case 2: /* temp promote */
		  if (word_count<2)
		  {
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
			return;
		  }
		  if (user->tpromoted && user->orig_lev < WIZ)
		  {
			writeus(user,tempnodo,command[com_num].name);
			return;
		  }
		  if ((u = get_name(user,word[1])) == NULL)
		  {
			write_user(user,center(notloggedon,80));
			return;
		  }
		  if (u == user)
		  {
			write_user(user,"~FMWell, that was rather pointless don't ya think? :P\n");
			return;
		  }
		  if (u->level >= user->level
		    && strcmp(user->name,TalkerOwner))
		  {
			write_user(user,"~FMYou can't temp promote someone to a higher level then your own.\n");
			return;
		  }
		  if (u->level == OWNER)
		  {
			write_user(user,"~FM You can't temp promote someone to a level that doesn't exist.\n");
			return;
		  }
		  if (u->tpromoted == 0)
		  {
			u->tpromoted = 1;
			u->orig_lev = u->level;
		  }
		  if (u->cloak_lev == u->level) u->cloak_lev++;
		  u->level++;
		  writeus(user,"~FG You have deemed ~FY%s~FG worthy of level~CB: [~CT%s~CB]~FG temporarily!\n",u->recap,user->gender==Female?level[u->level].fname:level[u->level].mname);
		  writeus(u,"~FG You have been temporarily deemed worthy of level~CB: [~CT%s~CB]\n",u->gender==Female?level[u->level].fname:level[u->level].mname);
		  WriteLevel(WIZ,user,"~FY %s~FG temporarily deemed ~FY%s~FG worthy of level~CB: [~CT%s~CB]\n",name,u->recap,level[u->level].name);
		  sprintf(text,"[TPROMOTE] - User: [%s] Recipient: [%s] Level: [%s]\n",user->name,u->name,level[u->level].name);
		  write_log(1,LOG4,text);
		  write_record(u,1,text);
		  return;
	}
}

/*
  demote a user and temp demote them if their being annoying by asking for
  promotions and what not, also if a user is temp promoted and demoted and
  or temp demoted it restores their original level.
*/
void demote(User user,char *inpstr,int type)
{
	User u;
	Room rm;
	char text2[ARR_SIZE],*name,*uname;
	int lev = 0,err = 0;

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
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	switch(type)
	{
		case 1:
		  if (word_count < 2 || word_count > 3)
		  {
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CRlevel~CB>]");
			return;
		  }
		  if (user->tpromoted && user->orig_lev<ADMIN)
		  {
			writeus(user,tempnodo,command[com_num].name);
			return;
		  }
		  if (word_count == 3)
		  {
			if ((lev = get_level(word[2])) == -1)
			{
				write_user(user,center(nosuchlev,80));
				return;
			}
		  }
		  /* Checks to see if the user is on right now */
		  if ((uname = get_user_full(user,word[1])) == NULL)
		  {
			write_user(user,center(nosuchuser,80));
			return;
		  }
		  if ((u = get_user(uname)))
		  {
			if (word_count == 3 && lev >= u->level)
			{
				write_user(user,"~FG Umm, I think ~CW'~CR.promote~CW'~FG would work better.\n");
				return;
			}
			if (u == user)
			{
				write_user(user,"~FMWell, that was rather pointless don't ya think? :P\n");
				return;
			}
			if (u->tpromoted)
			{
				u->level = u->orig_lev;
				u->tpromoted = 0;
				writeus(user,"~FR Restoring ~FM%s's ~FRlevel back to~CB: [~FT%s~CB]\n",u->recap,user->gender==Female?level[u->orig_lev].fname:level[u->orig_lev].mname);
				writeus(u,"~FR Your original level has been restored to~CB: [~FT%s~CB]\n",u->gender==Female?level[u->orig_lev].fname:level[u->orig_lev].mname);
				WriteLevel(WIZ,NULL,"~FM %s ~FRrestored ~FM%s's ~FRlevel back to~CB: [~FT%s~CB]\n",name,u->recap,level[u->orig_lev].name);
				sprintf(text,"[LEVEL RESTORE] - User: [%s] Victim: [%s] Level: [%s]\n",user->name,u->name,level[u->orig_lev].name);
				write_log(1,LOG4,text);
				write_record(u,1,text);
				return;
			}
			if (u->level == NEWBIE)
			{
				write_user(user,"~FM You can't demote a user past level NEWBIE!\n");
				return;
			}
			if (u->level >= user->level
			  && strcmp(user->name,TalkerOwner))
			{
				write_user(user,"~FM You can't demote someone of a equal or higher level then your own!\n");
				writeus(u,"~FG %s ~FMtried to demote you.\n",name);
				return;
			}
			if (word_count == 3)
			{
				if (u->cloak_lev == u->level) u->cloak_lev = lev;
				u->level = lev;
			}
			else
			{
				if (u->cloak_lev == u->level) u->cloak_lev--;
				u->level--;
			}
			writeus(user,"~FT %s~FM wasn't worthy so you demote them to level~CB: [~CT%s~CB]\n",u->recap,user->gender==Female?level[u->level].fname:level[u->level].mname);
			rm = user->room;
			user->room = NULL;
			WriteRoomExcept(NULL,u,"~FT %s ~FMwasn't worthy and demoted to level~CB: [~CT%s~CB]\n",u->recap,level[u->level].name);
			user->room = rm;
			writeus(u,"~FT %s ~FMdidn't find you worthy, so your demoted to level~CB: [~CT%s~CB]\n",name,u->gender==Female?level[u->level].fname:level[u->level].mname);
			sprintf(text,"[DEMOTE] - User: [%s] Victim: [%s] Level: [%s]\n",user->name,u->name,level[u->level].name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			clean_userlist(u->name);
			add_userlist(u->name,u->level);
			if (u->level < WIZ && is_staff(u))
			{
				writeus(user,"~FG Would you like to remove '%s' from the staff_file?~CB:~RS ",u->name);
				strcpy(user->check,u->name);
				user->misc_op = 36;
				user->ignall_store = user->ignall;
				user->ignall = 1;
				no_prompt = 1;
			}
			return;
		  }
		  /*
		    User not logged on, create a temporary user structure
		    change, then destruct it.
		  */
		  if ((u = create_user()) == NULL)
		  {
			writeus(user,"~CW-~FT Sorry.. an error occurred in the demotion. Try again later.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in demote().\n");
			return;
		  }
		  err = LoadUser(u,uname);
		  if (err == 0)
		  {
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  else if (err == -1)
		  {
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (word_count == 3 && lev >= u->level)
		  {
			write_user(user,"~FG Umm, I think ~CW'~CR.promote~CW'~FG would work better.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (u->level >= user->level
		    && strcmp(user->name,TalkerOwner))
		  {
			write_user(user,"~FMYou can't demote someone of a higher level then your own.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (u->level == NEWBIE)
		  {
			write_user(user,"~FMYou can't demote a user past level NEWBIE.\n");
			destruct_user(u);
			destructed = 0;
			return;
		  }
		  if (word_count == 3) u->level = lev;
		  else u->level--;
		  u->socket = -2;
		  strcpy(u->site,u->last_site);
		  SaveUser(u,0);
		  writeus(user,"~FT %s~FM wasn't worthy so you demote them to level~CB: [~CT%s~CB]\n",u->recap,user->gender==Female?level[u->level].fname:level[u->level].mname);
		  sprintf(text2,"~FM %s~FG has demoted you to level~CB: ~CB[~CR%s~CB]\n",user->recap,u->gender==Female?level[u->level].fname:level[u->level].mname);
		  send_oneline_mail(NULL,u->name,text2);
		  sprintf(text,"[DEMOTE] - User: [%s] Victim: [%s] Level: [%s]\n",user->name,u->name,level[u->level].name);
		  write_log(1,LOG4,text);
		  write_record(u,1,text);
		  clean_userlist(u->name);
		  add_userlist(u->name,u->level);
		  if (u->level<WIZ && is_staff(u))
		  {
			writeus(user,"~FG Would you like to remove '%s' from the staff_file?~CB:~RS ",u->name);
			strcpy(user->check,u->name);
			user->misc_op = 36;
			user->ignall_store = user->ignall;
			user->ignall = 1;
			destruct_user(u);
			destructed = 0;
			no_prompt = 1;
			return;
		  }
		  destruct_user(u);
		  destructed = 0;
		  return;
		case 2: /* temp demote */
		  if (word_count < 2)
		  {
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
			return;
		  }
		  if (user->tpromoted && user->orig_lev < WIZ)
		  {
			writeus(user,tempnodo,command[com_num].name);
			return;
		  }
		  if ((u = get_name(user,word[1])) == NULL)
		  {
			write_user(user,center(notloggedon,80));
			return;
		  }
		  if (u->level == NEWBIE)
		  {
			write_user(user,"~FMYou can't temp demote a user past the new user level.\n");
			return;
		  }
		  if (u == user)
		  {
			write_user(user,"~FMWell, that was rather pointless don't ya think? :P\n");
			return;
		  }
		  if (u->level >= user->level
		    && strcmp(user->name,TalkerOwner))
		  {
			write_user(user,"~FMYou can't temp demote someone of a higher level then your own!\n");
			return;
		  }
		  if (u->tpromoted)
		  {
			if (u->cloak_lev == u->level) u->cloak_lev = u->orig_lev;
			u->level = u->orig_lev;
			u->tpromoted = 0;
			writeus(user,"~FR Restoring ~FM%s's ~FRlevel back to~CB: [~FT%s~CB]\n",u->recap,user->gender==Female?level[u->orig_lev].fname:level[u->orig_lev].mname);
			writeus(u,"~FR Your original level has been restored to~CB: [~FT%s~CB]\n",u->gender==Female?level[u->orig_lev].fname:level[u->orig_lev].mname);
			WriteLevel(WIZ,user,"~FM%s ~FRrestored ~FM%s's ~FRlevel back to~CB: [~FT%s~CB]\n",name,u->recap,level[u->orig_lev].name);
			sprintf(text,"[LEVEL RESTORE] - User: [%s] Victim: [%s] Level: [%s]\n",user->name,u->name,level[u->orig_lev].name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			return;
		  }
		  if (u->cloak_lev == u->level) u->cloak_lev--;
		  u->orig_lev = u->level;
		  u->level--;
		  u->tpromoted = 1;
		  writeus(user,"~FT %s~FM was unworthy so they're demoted to level~CB: [~CT%s~CB]~FG temporarily!\n",u->recap,user->level==Female?level[u->level].fname:level[u->level].mname);
		  writeus(u,"~FG You have been temporarily deemed unworthy and demoted to level~CB: [~CT%s~CB]\n",u->gender==Female?level[u->level].fname:level[u->level].mname);
		  WriteLevel(WIZ,user,"~FY %s~FG temporarily deemed ~FY%s~FG unworthy and demoted them to level~CB: [~CT%s~CB]\n",name,u->recap,level[u->level].name);
		  sprintf(text,"[TDEMOTE] - User: [%s] Victim: [%s] Level: [%s]\n",user->name,u->name,level[u->level].name);
		  write_log(1,LOG4,text);
		  write_record(u,1,text);
		  return;
	}
}

/* Nuke that damn user */
void delete_user(User user,char *inpstr,int this_user)
{
	User u;
	char name[UserNameLen+1],*uname;
	int err = 0;

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
	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (this_user)
	{
		/*
		  Keep a copy of the name cuz user structure gets destructed in
		  disconnect_user()
		*/
		strcpy(name,user->name);
		write_user(user,center("~FMSorry you're suiciding...take care tho..\n",80));
		write_user(user,center("~CRACCOUNT DELETED.\n",80));
		WriteRoomExcept(user->room,user,"~FG%s~FR fades into absolute nothingness leaving us forever!\n",user->recap);
		write_log(1,LOG1,"[SUICIDE] - User: [%s]\n",user->name);
		disconnect_user(user,1);
		delete_files(name);
		clean_userlist(name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if (!strcasecmp(word[1],user->name))
	{
		write_user(user,"~FMUmm try .suicide...that'd probly work better :P\n");
		return;
	}
	if (get_name(user,word[1]) != NULL)
	{
		/* Kill the user if they are logged on */
		write_user(user,"~FMThat user is currently logged on, .kill them first.\n");
		return;
	}
	if ((u =create_user()) == NULL)
	{
		writeus(user,"~CW- ~FGAn error occurred in nuking a user.. Try again in a bit.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in delete_user().\n");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed=0;
		return;
	}
	strcpy(name,uname);
	err=LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level && strcmp(user->name,TalkerOwner))
	{
		write_user(user,"~FM You can't nuke a user of a equal or higher level then your own.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	destruct_user(u);
	destructed = 0;
	delete_files(uname);
	clean_userlist(uname);
	writeus(user,"\07~FG %s's~FR account has been deleted.\n",name);
	write_log(1,LOG4,"[NUKE] - User: [%s] Victim: [%s]\n",user->name,name);
}

/* List the bans that are currently active, be it new/users, or sites */
void listbans(User user,char *inpstr,int rt)
{
	Bans b;

	int type,cnt,i,hours,mins,days,secs;
	char buff[81],line[251],time_buff[81],temp[30];
	long ban_length;

	
	buff[0] = line[0] = time_buff[0] = temp[0] = '\0';
	i = cnt = type = days = hours = mins = secs = 0;
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
	if (!strncasecmp(word[1],"-sites",3))
	{
		cnt = 0;
		for_bans(b)
			if (b->type == SiteBan) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT There are no banned sites/domains.\n");
			return;
		}
		strcpy(buff,"sites/domains");
		type = SiteBan;
	}
	else if (!strncasecmp(word[1],"-new",2))
	{
		cnt = 0;
		for_bans(b)
			if (b->type == NewSBan) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT There are no banned sites/domains for new users.\n");
			return;
		}
		strcpy(buff,"sites/domains for new users");
		type = NewSBan;
	}
	else if (!strncasecmp(word[1],"-users",2))
	{
		cnt = 0;
		for_bans(b)
			if (b->type == UserBan) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT There are no banned users.\n");
			return;
		}
		strcpy(buff,"users");
		type = UserBan;
	}
	else if (!strncasecmp(word[1],"-who",2))
	{
		for_bans(b)
			if (b->type == WhoSBan) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT There are no banned sites/domains for doing 'w' at the login.\n");
			return;
		}
		strcpy(buff,"sites/domains for 'w' at the login");
		type = WhoSBan;
	}
	else if (!strncasecmp(word[1],"-swears",3))
	{
		i = 0;
		cnt = 0;
		while(swear_words[i][0] != '*')
		{
			if ((swear_words[0][0] != '\0') && i == 0)
			{
				if (user->ansi_on)
				{
					write_user(user,ansi_tline);
					write_user(user,center("~FTÄÄÄ¯¯> ~FMCurrent list of banned swear words... ~FT<®®ÄÄÄ\n",80));
					write_user(user,ansi_bline);
				}
				else
				{
					write_user(user,ascii_tline);
					write_user(user,center("~FT-~CT=~CB> ~FMCurrent list of banned swear words... ~CB<~CT=~FT-\n",80));
					write_user(user,ascii_bline);
				}
			}
			sprintf(line,"~FT%-15s",swear_words[i]);
			write_user(user,line);
			cnt++;
			if (cnt%5 == 0) write_user(user,"\n");
			++i;
		}
		if (cnt%5 != 0) write_user(user,"\n");
		if (i == 0) write_user(user,"~CW- ~FTThere are currently no banned swear words....\n");
		if (arrest_swearers) write_user(user,"\n~CW-~FT Users are ARRESTED if they swear.\n");
		else if (ban_swearing) write_user(user,"\n~CW-~FT Users are BANNED from using swear words.\n");
		else write_user(user,"\n~CW-~FT Swearing ban is currently OFF.\n");
		return;
	}
	else
	{
		writeus(user,usage,command[com_num].name,"~CB[~CR-sites~CW|~CR-users~CW|~CR-new~CW|~CR-who~CW|~CR-swears~CB]");
		return;
	}
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~FTÄÄÄ¯¯> ~FTCurrent list of banned %s... ~FT<®®ÄÄÄ\n",buff);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		sprintf(text,"~FT-~CT=~CB> ~FMCurrent list of banned %s... ~CB<~CT=~FT-\n",buff);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	cnt = 0;
	for_bans(b)
	{
		if (b->type != type) continue;
		ban_length = (time(0) - b->time_banned);
		strcpy(time_buff,"Banned for: ");
		days	= ban_length / 86400;
		hours	= (ban_length%86400)/3600;
		mins	= (ban_length%3600)/60;
		secs	= ban_length%60;
		if (days)
		{
			sprintf(temp,"%d day%s",days,days == 1?"":"s");
			strcat(time_buff,temp);
		}
		if (hours)
		{
			if (strstr(time_buff,"day")) strcat(time_buff,", ");
			sprintf(temp,"%d hour%s",hours,hours == 1?"":"s");
			strcat(time_buff,temp);
		}
		if (mins)
		{
			if (strstr(time_buff,"hour") || strstr(time_buff,"day"))
				strcat(time_buff,", ");
			sprintf(temp,"%d minute%s",mins,mins == 1?"":"s");
			strcat(time_buff,temp);
		}
		if (strstr(time_buff,"minute") || strstr(time_buff,"hour")
		  || strstr(time_buff,"day")) strcat(time_buff,", and ");
		sprintf(temp,"%d second%s",secs,secs == 1?"":"s");
		strcat(time_buff,temp);
		if (type == UserBan) sprintf(line,"~FT%-15.15s~CB: ~FG%s\n",b->who,time_buff);
		else sprintf(line,"~FGSite/Domain~CB:~FT %s\n~CB           : ~FG%s\n",b->who,time_buff);
		write_user(user,line);
		++cnt;
	}
	writeus(user,"\n~CW- ~FTTotal of %d ban%s found.\n",cnt,cnt == 1?"":"s");
	return;
}

/* ban a site, a new user site, or a specific user */
void ban(User user,char *inpstr,int rt)
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
	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 3)
	{
		write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -site ~CB[~CRsite to be banned~CB]\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -site ~FT.banmysite.com\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -site ~FT169.69.6.  ~CB[~CRomit last numbers~CB]\n");
		write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -new  ~CB[~CRsite to be banned~CB]\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -new  ~FT.banmysite.com\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -new  ~FT169.69.6.  ~CB[~CRomit last numbers~CB]\n");
		write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -who  ~CB[~CRsite to be banned~CB]\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -who  ~FT.banmysite.com\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -who  ~FT169.69.6.  ~CB[~CRomit last numbers~CB]\n");
		write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -user ~CB[~CRuser name~CB]\n");
		write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -user ~FTsquirt ~CB[~CRdon't actually do that ;)~CB]\n");
		return;
	}
	if (word[2][0] == '/' || word[2][0] == '*' || word[2][0] == '+'
	  || word[2][0] == '-' || word[2][0] == '?' || (word[2][0] == '.'
	  && (word[2][1] == '/' || (word[2][1] == '.'
	  && word[2][2] == '/'))))
	{
		write_user(user,"~CW- ~FTThat was an invalid site/user name.\n");
		return;
	}
	if (!strncasecmp(word[1],"-site",2))
	{
		ban_site(user,inpstr);
		return;
	}
	if (!strncasecmp(word[1],"-user",2))
	{
		ban_user(user,inpstr);
		return;
	}
	if (!strncasecmp(word[1],"-new",2))
	{
		ban_new(user,inpstr);
		return;
	}
	if (!strncasecmp(word[1],"-who",2))
	{
		ban_who(user);
		return;
	}

	/*
	   Tell the user to be specific if neither of the above options
	   were chosen
	*/
	write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -site ~CB[~CRsite to be banned~CB]\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -site ~FT.banmysite.com\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -site ~FT169.69.6.  ~CB[~CRomit last numbers~CB]\n");
	write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -new  ~CB[~CRsite to be banned~CB]\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -new  ~FT.banmysite.com\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -new  ~FT169.69.6.  ~CB[~CRomit last numbers~CB]\n");
	write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -who  ~CB[~CRsite to be banned~CB]\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -who  ~FT.banmysite.com\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -who  ~FT169.69.6.  ~CB[~CRomit last numbers~CB]\n");
	write_user(user,"~CW - ~FGCommand usage~CB: ~CR.ban -user ~CB[~CRuser name~CB]\n");
	write_user(user,"       ~CW- ~FMExample~CB: ~CR.ban -user ~FTsquirt ~CB[~CRdon't actually do that ;)~CB]\n");
	return;
}

/* ban a site */
void ban_site(User user,char *inpstr)
{
	Bans b;
	char host[81];

	gethostname(host,80);
	strtolower(inpstr);
	if (istrstr(word[2],host) || istrstr(word[2],"localhost")
	  || istrstr(word[2],"127.0.0.1"))
	{
		write_user(user,"~CW- ~FGYou can't ban the site that the talker is running on.\n");
		return;
	}
	b = find_ban(word[2],SiteBan);
	if (b != NULL)
	{
		write_user(user,"~CW-~FT That site/domain has already been banned.\n~CW-~FT To unban it, use the ~CW'~CR.unban~CW'~FT command.\n");
		return;
	}
	inpstr = remove_first(inpstr);
	inpstr = remove_first(inpstr);
	if ((strlen(inpstr) < 10) && (user->level < OWNER))
	{
		writeus(user,usage,command[com_num].name,"-site ~CB[~CRsite to ban~CB] [~CRVALID reason for the ban~CB]");
		return;
	}
	if (strlen(inpstr) > 80)
	{
		writeus(user,"~CW-~FT The reason is limited to 80 characters or less.\n");
		return;
	}
	if (istrstr(inpstr,"NULL"))
	{
		writeus(user,"~CW-~FT You can't NULLify the reason.\n");
		return;
	}
	b = new_ban(word[2],SiteBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred, unable to ban the site/domain.\n");
		return;
	}
	b->time_banned = time(0);
	if (strlen(inpstr)) strncpy(b->reason,inpstr,sizeof(b->reason)-1);
	else strcpy(b->reason,"N/A");
	strncpy(b->by,user->name,sizeof(b->by)-1);
	write_user(user,"~CW- ~FTSite/Domain has been ~CRBANNED~FT.\n");
	write_log(1,LOG4,"[SITE-BAN] - User: [%s] Site: [%s]\n",user->name,word[2]);
}

/* new user site ban */
void ban_new(User user,char *inpstr)
{
	Bans b;
	char host[81];

	gethostname(host,80);
	strtolower(inpstr);
	if (istrstr(word[2],host) || istrstr(word[2],"localhost")
	  || istrstr(word[2],"127.0.0.1"))
	{
		write_user(user,"~FGYou can't ban the site that the talker is running on.\n");
		return;
	}
	b = find_ban(word[2],NewSBan);
	if (b != NULL)
	{
		write_user(user,"~CW- ~FTThat site/domain has already been banned to new users.\n~CW-~FT To unban it use ~CW'~CR.unban~CW'~FG.\n");
		return;
	}
	inpstr = remove_first(inpstr);
	inpstr = remove_first(inpstr);
	if ((strlen(inpstr) < 10) && (user->level < OWNER))
	{
		writeus(user,usage,command[com_num].name,"-new ~CB[~CRsite to ban~CB] [~CRVALID reason for the ban~CB]");
		return;
	}
	if (strlen(inpstr) > 80)
	{
		writeus(user,"~CW-~FT The reason is limited to 80 characters or less.\n");
		return;
	}
	if (istrstr(inpstr,"null"))
	{
		writeus(user,"~CW-~FT You can't NULLify the reason.\n");
		return;
	}
	b = new_ban(word[2],NewSBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred, unable to ban the site/domain.\n");
		return;
	}
	b->time_banned = time(0);
	if (strlen(inpstr)) strncpy(b->reason,inpstr,sizeof(b->reason)-1);
	else strcpy(b->reason,"N/A");
	strncpy(b->by,user->name,sizeof(b->by)-1);
	write_user(user,"~CW- ~FTSite/Domain for new users has been ~CRBANNED~FT.\n");
	write_log(1,LOG4,"[NEWSITE-BAN] - User: [%s] Site: [%s]\n",user->name,word[2]);
}

/* who site ban */
void ban_who(User user)
{
	Bans b;
	char host[81];

	gethostname(host,80);
	strtolower(word[2]);
	if (istrstr(word[2],host) || istrstr(word[2],"localhost")
	  || istrstr(word[2],"127.0.0.1"))
	{
		write_user(user,"~CW- ~FGYou can't ban the site that the talker is running on.\n");
		return;
	}
	b = find_ban(word[2],WhoSBan);
	if (b != NULL)
	{
		write_user(user,"~CW- ~FTThat site/domain has already been banned for doing who's at the login\n~CW-~FT To unban it, use ~CW'~CR.unban~CW'~FG.");
		return;
	}
	b = new_ban(word[2],WhoSBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred, unable to ban the site/domain.\n");
		return;
	}
	b->time_banned = time(0);
	strcpy(b->reason,"N/A");
	strncpy(b->by,user->name,sizeof(b->by)-1);
	write_user(user,"~CW- ~FTSite/Domain has been ~CRBANNED~FT for doing who's at the login.\n");
	write_log(1,LOG4,"[WHOSITE-BAN] - User: [%s] Site: [%s]\n",user->name,word[2]);
}

/* user ban */
void ban_user(User user,char *inpstr)
{
	User u;
	Bans b;
	char *uname;
	int err = 0;

	if ((uname = get_user_full(user,word[2])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (!strcasecmp(user->name,word[2]))
	{
		write_user(user,"~CW- ~FTYou're on crack for wanting to ban yourself.\n");
		return;
	}
	if (!strcasecmp(TalkerOwner,uname))
	{
		write_user(user,"~CW- ~FTYou can't ban the owner of the talker.. shame on you..\n");
		return;
	}
	b = find_ban(uname,UserBan);
	if (b != NULL)
	{
		write_user(user,"~CW- ~FTThat user is already banned.\n~CW-~FT To unban them, use ~CW'~CR.unban~CW'~FG.");
		return;
	}
	inpstr = remove_first(inpstr);
	inpstr = remove_first(inpstr);
	if ((strlen(inpstr) < 10) && (user->level < OWNER))
	{
		writeus(user,usage,command[com_num].name,"-user ~CB[~CRuser to ban~CB] [~CRVALID reason for the ban~CB]");
		return;
	}
	if (strlen(inpstr) > 80)
	{
		write_user(user,"~CW-~FT Try to keep your reason to 80 characters or less.\n");
		return;
	}
	if (istrstr(inpstr,"NULL"))
	{
		write_user(user,"~CW-~FT You can't NULLify the reason.\n");
		return;
	}
	if ((u = get_user(uname)) != NULL) /* are they on right now? */
	{
		if (u->level >= user->level && strcasecmp(user->name,TalkerOwner))
		{
			write_user(user,"~CW- ~FTYou can't ban someone of a greater or equal level then you.\n");
			writeus(u,"~CW- ~FM%s~FT tried to ban you...\n",user->recap);
			return;
		}
	}
	else /* Not on.. */
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW- ~FGAn error occurred in your ban.. Please try again later.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temp. user object in ban_user()\n");
			return;
		}
		err = LoadUser(u,uname);
		if (err == 0)
		{
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		}
		if (err == -1)
		{
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			write_user(user,"~CW- ~FTIt appears as tho that user's userfile is corrupt. Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		if (u->level >= user->level
		  && strcasecmp(user->name,TalkerOwner))
		{
			write_user(user,"~CW- ~FTYou cannot ban a user of a greater or equal level then yours.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		destruct_user(u);
		destructed = 0;
	}
	b = new_ban(uname,UserBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred, unable to ban the user.\n");
		return;
	}
	b->time_banned = time(0);
	if (strlen(inpstr)) strncpy(b->reason,inpstr,sizeof(b->reason)-1);
	else strcpy(b->reason,"N/A");
	strncpy(b->by,user->name,sizeof(b->by)-1);
	writeus(user,"~CW- ~FT%s has been ~CRBANNED~FT.\n",uname);
	sprintf(text,"[USER-BAN] - User: [%s] Victim: [%s]\n",user->name,uname);
	write_log(1,LOG4,text);
	if (u != NULL)
	{
		write_record(u,1,text);
		write_user(user,"~CW- ~FTYou have been ~CRBANNED~FT.... Bye Bye..\n");
		disconnect_user(u,1);
	}
}

/* Gotta be able to undo the bans.. I guess.. ;-) */
void unban(User user,char *inpstr,int rt)
{
	char *args = "~CB[~CR-site~CW|~CR-new~CW|~CR-who~CW|~CR-user~CB] [~CRsite~CW|~CRip~CW|~CRname~CB]";

	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
		writeus(user,usage,command[com_num].name,args);
		return;
	}
	if (!strncasecmp(word[1],"-site",2))
	{
		unban_site(user);
		return;
	}
	if (!strncasecmp(word[1],"-user",2))
	{
		unban_user(user);
		return;
	}
	if (!strncasecmp(word[1],"-new",2))
	{
		unban_new(user);
		return;
	}
	if (!strncasecmp(word[1],"-who",2))
	{
		unban_who(user);
		return;
	}
	writeus(user,usage,command[com_num].name,args);
}

/* unban that site */
void unban_site(User user)
{
	Bans b;

	strtolower(word[2]);
	b = find_ban(word[2],SiteBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FTThat site/domain isn't currently banned.\n");
		return;
	}
	nuke_ban(b);
	write_user(user,"~CW- ~FTSite/Domain ban has been removed.\n");
	write_log(1,LOG4,"[SITE-UNBAN] - User: [%s] Site: [%s]\n",user->name,word[2]);
}

/* unban the new site ban */
void unban_new(User user)
{
	Bans b;

	strtolower(word[2]);
	b = find_ban(word[2],NewSBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FTThat site/domain isn't currently banned for new users.\n");
		return;
	}
	nuke_ban(b);
	write_user(user,"~CW- ~FTSite/Domain ban for new users has been removed.\n");
	write_log(1,LOG4,"[NEWSITE-UNBAN] - User: [%s] Site: [%s]\n",user->name,word[2]);
}

/* unban the who site ban */
void unban_who(User user)
{
	Bans b;

	b = find_ban(word[2],WhoSBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FTThat site/domain isn't currently banned for who's at the login.\n");
		return;
	}
	nuke_ban(b);
	write_user(user,"~CW- ~FTSite/Domain ban for doing who's at the login has been removed.\n");
	write_log(1,LOG4,"[WHO-UNBAN] - User: [%s] Site: [%s]\n",user->name,word[2]);
}

/* Unban the user ban */
void unban_user(User user)
{
	Bans b;

	strtolower(word[2]);
	word[2][0] = toupper(word[2][0]);
	b = find_ban(word[2],UserBan);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FTThat user isn't currently banned.\n");
		return;
	}
	nuke_ban(b);
	writeus(user,"~CW- ~FT%s has been unbanned.\n",word[2]);
	write_log(1,LOG4,"[USER-UNBAN] - User: [%s] Victim: [%s]\n",user->name,word[2]);
}

/* Arrest a user who is being harrasive or just a pain in the ass */
void arrest(User user,char *inpstr,int rt)
{
	User u;
	Room rm;
	char *name,text2[ARR_SIZE],*uname;
	int err = 0;

	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if ((u = get_user(uname)))
	{
		if (u == user)
		{
			write_user(user,"~FMWhy would you ever want to arrest yourself?\n");
			return;
		}
		if (u->level >= user->level && strcmp(user->name,TalkerOwner))
		{
			write_user(user,"~FMYou cannot arrest a user of a greater or equal level then your own.\n");
			writeus(u,"~FG%s ~FTattempted to arrest you.\n",name);
			return;
		}
		if (u->status & Jailed)
		{
			writeus(user,"~FG%s ~FMhas already been arrested.\n",u->recap);
			return;
		}
		writeus(user,"~FG You throw a set of handcuffs on ~FT%s~FG placing them under arrest!\n",u->recap);
		write_user(u,"~FG You have broken the rules of the talker and have been placed under arrest!\n");
		sprintf(text,"[ARREST] - User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status += Jailed;
		if ((rm = get_room(jail_room)) == NULL)
		{
			write_user(u,"~FYUnable to find the jail room, but you are still under arrest!\n");
			return;
		}
		move_user(u,rm,2);
		return;
	}
	/* The user isn't logged on */
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred in creating a user object.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in arrest().\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level && strcmp(user->name,TalkerOwner))
	{
		write_user(user,"~FM You can't nuke a user of a equal or higher level then your own.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level && strcmp(user->name,TalkerOwner))
	{
		write_user(user,"~FMYou cannot arrest a user of a greater or equal level then your own!\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->status & Jailed)
	{
		writeus(user,"~FG%s~FT is already under arrest.\n",u->recap);
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	u->status += Jailed;
	strcpy(u->site,u->last_site);
	SaveUser(u,0);
	writeus(user,"~FG You throw a set of handcuffs on ~FT%s~FG placing them under arrest.\n",u->recap);
	sprintf(text2,"~FM %s~FG has placed you under arrest.\n",user->recap);
	send_oneline_mail(NULL,u->name,text2);
	sprintf(text,"[ARREST] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
}

/* Unarrest the user now that they've apologized and all that shit */
void unarrest(User user,char *inpstr,int rt)
{
	User u;
	Room rm;
	char text2[ARR_SIZE],*uname;
	int err = 0;

	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u == user)
		{
			write_user(user,"~FMWhy would you ever want to arrest yourself?\n");
			return;
		}
		if (!(u->status & Jailed))
		{
			writeus(user,"~FG%s~FT isn't even under arrest.\n",u->recap);
			return;
		}
		writeus(user,"~FG You take your handcuffs reluctantly off of ~FT%s~FG...\n",u->recap);
		write_user(u,"~FG You have been forgiven and unarrested...\n");
		sprintf(text,"[UNARREST] - User: [%s] Jailbird: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status -= Jailed;
		if ((rm = get_room(main_room)) == NULL)
		{
			write_user(u,"~FYUnable to find the main room to warp you back to!!!\n");
			return;
		}
		move_user(u,rm,2);
		return;
	}
	/* The user isn't logged on */
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred in trying to create a temp. user object.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in unarrest().\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (!(u->status & Jailed))
	{
		writeus(user,"~FG%s~FT isn't even under arrest.\n",u->recap);
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	u->status -= Jailed;
	strcpy(u->site,u->last_site);
	SaveUser(u,0);
	writeus(user,"~FG You take your handcuffs reluctantly off of ~FT%s~FG...\n",u->recap);
	sprintf(text2,"~FM %s~FG released you from the hellish jail.\n",user->recap);
	send_oneline_mail(NULL,u->name,text2);
	sprintf(text,"[UNARREST] - User: [%s] Jailbird: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
}

/*
  Muzzle a user so that they can no longer use any commands that involve
  speech except for maybe .say.
*/
void muzzle(User user,char *inpstr,int rt)
{
	User u;
	char *name,text2[ARR_SIZE],*uname;
	int err = 0;

	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (user->invis == Invis) name = invisname; else name = user->recap;
	if ((u = get_user(uname)) != NULL)
	{
		if (u == user)
		{
			write_user(user,"~FMWhy would you ever want to muzzle yourself?\n");
			return;
		}
		if (u->level >= user->level && strcmp(user->name,TalkerOwner))
		{
			write_user(user,"~FMYou cannot muzzle a user of a greater or equal level then your own.\n");
			writeus(u,"~FG%s ~FTattempted to muzzle you!\n",user->recap);
			return;
		}
		if (u->status & Muzzled)
		{
			writeus(user,"~FG%s~FT has already been muzzled.\n",u->recap);
			return;
		}
		writeus(user,"~FG You slap a muzzle over ~FT%s's~FG face\n",u->recap);
		write_user(u,"~FG You have done something bad, and have been muzzled.\n");
		sprintf(text,"[MUZZLE] - User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status += Muzzled;
		return;
	}
	/* The user isn't logged on */
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred, try again in a bit.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in muzzle().\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level && strcmp(user->name,TalkerOwner))
	{
		write_user(user,"~FMYou cannot muzzle a user of a greater or equal level then your own!\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->status & Muzzled)
	{
		writeus(user,"~FG%s~FT has already been muzzled.\n",u->recap);
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	u->status += Muzzled;
	strcpy(u->site,u->last_site);
	SaveUser(u,0);
	writeus(user,"~FG You slap a muzzle over ~FT%s's~FG face.\n",u->recap);
	sprintf(text2,"~FM %s~FG has thrown a muzzle over your mouth.\n",user->recap);
	send_oneline_mail(NULL,u->name,text2);
	sprintf(text,"[MUZZLE] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
}

/* Unmuzzle the user now that they've apologized and all that shit */
void unmuzzle(User user,char *inpstr,int rt)
{
	User u;
	char text2[ARR_SIZE],*uname;
	int err = 0;

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
	if (user->tpromoted && user->orig_lev < WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u == user)
		{
			write_user(user,"~FMHaha, you can't unmuzzle yourself!\n");
			return;
		}
		if (!(u->status & Muzzled))
		{
			writeus(user,"~FG%s~FT isn't even muzzled.\n",u->recap);
			return;
		}
		writeus(user,"~FG You take out a knife and cut off ~FT%s's~FG muzzle...\n",u->recap);
		write_user(u,"~FG You have been forgiven and unmuzzled...\n");
		sprintf(text,"[UNMUZZLE] - User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status -= Muzzled;
		return;
	}
	/* The user isn't logged on */
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FGAn error occurred, unable to create temp. user object.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in unmuzzle().\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (!(u->status & Muzzled))
	{
		writeus(user,"~FG %s~FT isn't even muzzled.\n",u->recap);
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	u->status -= Muzzled;
	strcpy(u->site,u->last_site);
	SaveUser(u,0);
	writeus(user,"~FG You take out a knife and cut off ~FT%s's~FG muzzle...\n",u->recap);
	sprintf(text2,"~FM %s~FG takes out a knife and cuts your muzzle off.\n",user->recap);
	send_oneline_mail(NULL,u->name,text2);
	sprintf(text,"[UNMUZZLE] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed=0;
}

/*
  Silence a user, makes it so that the user can see everyone else's speech
  and what not, but when it comes to them talking all it does is
  write_user(user.... so noone else see's what they are saying
*/
void silence(User user,char *inpstr,int rt)
{
	User u;
	char *name,*uname;
	int err=0;

	if (user->tpromoted && user->orig_lev < ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if ((u = get_user(uname)) != NULL)
	{
		if (u->level >= user->level && user->level < OWNER)
		{
			write_user(user,"~CW- ~FTYou can't silence a user of a greater or equal level then your own.\n");
			writeus(u,"~CW-~FG %s~FT thought of trying to silence you.\n",user->recap);
			return;
		}
		if (u == user)
		{
			write_user(user,"~CW- ~FTIf you want to be quiet.. then shut up..\n");
			return;
		}
		if (u->status & Silenced)
		{
			writeus(user,"~CW- ~FT%s~FT has already been silenced.\n");
			return;
		}
		if (user->status & Silenced)
		{
			writeus(user,"~CW- ~FGYou mutter something under your breath.. silencing ~FT%s~FT...\n",u->recap);
			return;
		}
		writeus(user,"~CW- ~FGYou mutter something under your breath.. silencing ~FT%s~FT...\n",u->recap);
		sprintf(text,"[SILENCE] - User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status += Silenced;
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred in creating a temp. user object.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temp. user object in silence()\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level && user->level < OWNER)
	{
		write_user(user,"~CW- ~FTYou can't silence a user of a greater or equal level then your own.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->status & Silenced)
	{
		writeus(user,"~CW- ~FT%s~FT has already been silenced.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW- ~FGYou mutter something under your breath.. silencing ~FT%s~FT...\n",u->recap);
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	strcpy(u->site,u->last_site);
	u->status += Silenced;
	SaveUser(u,0);
	writeus(user,"~CW- ~FGYou mutter something under your breath.. silencing ~FT%s~FT...\n",u->recap);
	sprintf(text,"[SILENCE] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
}

/* Unsilence them after you feel you've had your fun with them ;) */
void unsilence(User user,char *inpstr,int rt)
{
	User u;
	char *uname;
	int err = 0;

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
	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u == user)
		{
			write_user(user,"~CW- ~FMYou must think I'm pretty stupid eh?\n");
			return;
		}
		if (!(u->status & Silenced))
		{
			writeus(user,"~CW- ~FT%s~FG hasn't even been silenced.\n",u->recap);
			return;
		}
		if (u->level >= user->level && user->level < OWNER)
		{
			write_user(user,"~CW- ~FTYou can't unsilence someone who's a higher level or the same as yours.\n");
			return;
		}
		if (user->status & Silenced)
		{
			writeus(user,"~CW- ~FGYou pull your gag out of ~FT%s%s ~FTmouth...\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
			return;
		}
		writeus(user,"~CW- ~FGYou pull your gag out of ~FT%s%s ~FTmouth...\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
		sprintf(text,"[UNSILENCE]: User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status -= Silenced;
		return;
	}
	if ((u = create_user()) == NULL)
	{
		writeus(user,"~CW- ~FTAn error occurred in creating a temp. user structure.\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		destruct_user(u);
		destructed=0;
		return;
	}
	if (!(u->status & Silenced))
	{
		writeus(user,"~CW- ~FT%s~FG hasn't even been silenced.\n",u->recap);
		destruct_user(u);
		destructed=0;
		return;
	}
	if (u->level >= user->level && user->level<OWNER)
	{
		write_user(user,"~CW- ~FTYou can't unsilence someone who's a higher level or the same as yours.\n");
		destruct_user(u);
		destructed=0;
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW- ~FGYou pull your gag out of ~FT%s%s ~FTmouth...\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	strcpy(u->site,u->last_site);
	u->status -= Silenced;
	SaveUser(u,0);
	writeus(user,"~CW- ~FGYou pull your gag out of ~FT%s%s ~FTmouth...\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
	sprintf(text,"[UNSILENCE]: User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
	return;
}

/* Broadcast an important message, ie any system information */
void broad_cast(User user,char *inpstr,int rt)
{

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	/* lil hack .bcast .message  -- does it a bit fancier.. */
	if (inpstr[0] == '.')
	{
		inpstr++;
		if (!inpstr[0])
		{
			writeus(user,usage,command[com_num].name,"~CB[~CR.message~CB]");
			return;
		}
		if (user->status & Silenced)
		{
			write_user(user,"~FG                                  \\`~FRo~FG`~FRo~FG/\n");
			write_user(user,"~FG                                ,^ ( ..)\n");
			write_user(user,"~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-~FGoOOo~CB-~FG'| \\   \\~CB=-~FGoOOo~CB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-]\n");
			write_user(user,"~FG                            ~CW````   ~FG\\ `^--^ ~CW''''\n");
			write_user(user,"~FG                                     \\ \\ \\\n");
			write_user(user,"~FG                                      ^--^\n");
			write_user(user,center(inpstr,80));
			write_user(user,"\n~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]\n");
			return;
		}
		write_room(NULL,"~FG                                  \\`~FRo~FG`~FRo~FG/\n");
		write_room(NULL,"~FG                                ,^ ( ..)\n");
		write_room(NULL,"~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-~FGoOOo~CB-~FG'| \\   \\~CB=-~FGoOOo~CB=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-]\n");
		write_room(NULL,"~FG                            ~CW````   ~FG\\ `^--^ ~CW''''\n");
		write_room(NULL,"~FG                                     \\ \\ \\\n");
		write_room(NULL,"~FG                                      ^--^\n");
		write_room(NULL,center(inpstr,80));
		write_room(NULL,"\n~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]\n");
		return;
	}
	else
	{
		if (user->invis) sprintf(text,"\07~FB---~CB===~FT[ ~CRBroadcast message ~FT]~CB===~FB---\n");
		else sprintf(text,"\07~FB---~CB===~FT[ ~CRBroadcast message from ~FG%s~FT ]~CB===~FB---\n",user->recap);
		if (user->status & Silenced)
		{
			write_user(user,center(text,80));
			write_user(user,"~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]\n");
			write_user(user,center(inpstr,80));
			write_user(user,"\n~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]\n");
			return;
		}
		force_listen = 1;
		write_room(NULL,center(text,80));
		write_room(NULL,"~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]\n");
		write_room(NULL,center(inpstr,80));
		write_room(NULL,"\n~CB[-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]\n");
		force_listen = 0;
		return;
	}
}

/* view a users arrest record, so you can keep tabs on them */
void view_record(User user,char *inpstr,int rt)
{
	User u;
	char filename[FSL],*uname;
	int err = 0;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u->level > user->level && user->level < OWNER)
		{
			write_user(user,"~FMYou can't view the record of a user of a greater or equal level then your own.\n");
			return;
		}
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~CRÄÄ¯> ~FT Viewing %s's ~FTrecord... ~CR<®ÄÄ\n",u->recap);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_line);
			sprintf(text,"~FT-~CT=~CB> ~FGViewing ~CM%s's~FG record...~CB<~CT=~FT-\n",u->recap);
			write_user(user,center(text,80));
			write_user(user,ascii_line);
		}
		sprintf(filename,"%s/%s/%s.R",UserDir,UserMisc,u->name);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  writeus(user," ~FG%s doesn't have a record to view.\n",u->recap);
			  return;
			case 1:
			  user->misc_op = 2;
		}
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred.. try again in a second.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in view_record().\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err==0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed = 0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level > user->level && user->level < OWNER)
	{
		write_user(user,"~FM You can't view the record of a user of a greater or equal level then your own.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~CRÄÄ¯> ~FT Viewing %s's ~FTrecord... ~CR<®ÄÄ\n",u->recap);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		sprintf(text,"~FT-~CT=~CB> ~FGViewing ~CM%s's~FG record...~CB<~CT=~FT-\n",u->recap);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	sprintf(filename,"%s/%s/%s.R",UserDir,UserMisc,u->name);
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  writeus(user," ~CB[~FG%s~CB]~FM does not have a record to view.\n",u->recap);
		  destruct_user(u);	destructed = 0;
		  return;
		case 1:
		  user->misc_op = 2;	destruct_user(u);	destructed = 0;
		  return;
	}
	destruct_user(u);
	destructed = 0;
	return;
}

/* gags a user so they cannot execute any commands what so ever, not even .q */
void gag(User user,char *inpstr,int rt)
{
	User u;
	char *name,*uname;
	int err=0;

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
	if (user->tpromoted && user->orig_lev<WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CR-private~CW|~CR-socials~CW|~CR-shouts~CB>]");
		return;
	}
	if (!strncasecmp(word[2],"-shouts",3))
	{
		gag_shouts(user);
		return;
	}
	if (!strncasecmp(word[2],"-socials",3))
	{
		gag_socials(user);
		return;
	}
	if (!strncasecmp(word[2],"-private",2))
	{
		gag_private(user);
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if ((u = get_user(uname)) != NULL)
	{
		if (u == user)
		{
			write_user(user,"~CW- ~FTNo.. that didn't work too well..\n");
			return;
		}
		if (u->level >= user->level && user->level<OWNER)
		{
			write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
			return;
		}
		if (u->status & Gagged)
		{
			if (user->status & Silenced)
			{
				writeus(user,"~CW-~FG %s~FT has had %s gag removed..\n",u->recap,gender1[user->gender]);
				return;
			}
			WriteRoom(NULL,"~CW-~FG %s~FT has had %s gag removed..\n",u->recap,gender1[user->gender]);
			strcpy(u->desc,"Forgiven for being bad...");
			sprintf(text,"[UNGAG] - User: [%s] Victim: [%s]\n",user->name,u->name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			u->status -= Gagged;
			return;
		}
		if (user->status & Silenced)
		{
			writeus(user,"~CW- ~FY%s~FG gets a gag shoved down %s throat.\n",u->recap,gender1[user->gender]);
			return;
		}
		WriteRoom(NULL,"~CW- ~FY%s~FG gets a gag shoved down %s throat.\n",u->recap,gender1[user->gender]);
		sprintf(text,"[GAG] - User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		u->status+=Gagged;
		strcpy(u->desc,"bad, bad, bad...");
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred in creating a temp user object.\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);
		destructed=0;
		return;
	}
	else if (err == -1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level && user->level < OWNER)
	{
		write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->status & Gagged)
	{
		if (user->status & Silenced)
		{
			writeus(user,"~CW-~FG %s~FT has had %s gag removed..\n",u->recap,gender1[user->gender]);
			destruct_user(u);
			destructed=0;
			return;
		}
		u->socket = -2;
		strcpy(u->site,u->last_site);
		u->status -= Gagged;
		writeus(user,"~CW-~FG %s~FT has had %s gag removed..\n",u->recap,gender1[user->gender]);
		SaveUser(u,0);
		strcpy(u->desc,"Forgiven for being bad...");
		sprintf(text,"~FY You have had your gag removed...\n");
		send_oneline_mail(NULL,u->name,text);
		sprintf(text,"[UNGAG] - User: [%s] Victim: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);
		write_record(u,1,text);
		destruct_user(u);
		destructed=0;
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW- ~FY%s~FG gets a gag shoved down %s throat.\n",u->recap,gender1[user->gender]);
		destruct_user(u);
		destructed=0;
		return;
	}
	u->socket = -2;
	strcpy(u->site,u->last_site);
	u->status -= Gagged;
	SaveUser(u,0);
	strcpy(u->desc,"bad, bad, bad...");
	writeus(user,"~CW- ~FY%s~FG gets a gag shoved down %s throat.\n",u->recap,gender1[user->gender]);
	sprintf(text,"~FY You have had a gag shoved down your throat..\n");
	send_oneline_mail(NULL,u->name,text);
	sprintf(text,"[GAG] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
	return;
}

/*
  Fake log out, I have made vast improvements on this since my original
  one, now it checks to see if the user is invisible or hidden first!
*/
void fake_logoff(User user,char *inpstr,int rt)
{
	Room rm;
	Telnet telnet;

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
	if (user->invis > Hidden)
	{
		logging_in = 1;
		if (user->temp_recap[0] != '\0') strcpy(user->recap,user->temp_recap);
		user->temp_recap[0] = '\0';
		if (user->predesc[0] != '\0')
		{
			if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s ~CB<=-\n",login_vis,user->predesc,user->recap,user->desc);
			else sprintf(text,"%s ~FY%s ~FY%s~FM %s ~CB<=-\n",login_vis,user->predesc,user->recap,user->desc);
		}
		else
		{
			if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s ~CB<=-\n",login_vis,user->recap,user->desc);
			else sprintf(text,"%s ~FY%s~FM %s ~CB<=-\n",login_vis,user->recap,user->desc);
		}
		write_user(user,text);
		write_room_except(NULL,user,text);
		if (user->desc[0] == '\'') sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
		else sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
		RecordLogin(text);
		WriteLevel(WIZ,user,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
		write_log(1,LOG2,"[LOGIN] - User: [%s] Site: [%s:%d] Port:[%d]\n",user->name,user->site,user->site_port,user->port);
		friend_check(user,1);
		wait_check(user);
		user->invis = 0;
		SaveUser(user,1);
		reset_junk(user);
		user->last_login = time(0);
		logging_in = 0;
		user->room = get_room(main_room);
		WriteRoomExcept(user->room,user,"~CW- ~FG%s~FR %s\n",user->recap,user->login_msg);
		return;
	}
	rm = user->room;
	logging_in = 1;
	switch(user->invis)
	{
		case 1 :
		  if (user->predesc[0])
		  {
			if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FG %s~FR%s~FM <=-\n",logout_inv,user->predesc,user->recap,user->desc);
			else sprintf(text,"%s ~FG%s~FG %s~FR %s~FM <=-\n",logout_inv,user->predesc,user->recap,user->desc);
		  }
		  else
		  {
			if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_inv,user->recap,user->desc);
			else sprintf(text,"%s ~FG%s ~FR%s~FM <=-\n",logout_inv,user->recap,user->desc);
		  }
		  write_user(user,text);
		  write_level(WIZ,user,text);
		  write_log(1,LOG2,"[LOGOFF] - User: [%s] [INVIS]\n",user->name);
		  break;
		case 2 :
		  strcpy(user->recap,user->temp_recap);
		  user->temp_recap[0] = '\0';
		  if (user->predesc[0])
		  {
			if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FG %s~FR%s~FM <=-\n",logout_hide,user->predesc,user->recap,user->desc);
			else sprintf(text,"%s ~FG%s~FG %s~FR %s~FM <=-\n",logout_hide,user->predesc,user->recap,user->desc);
		  }
		  else
		  {
			if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_hide,user->recap,user->desc);
			else sprintf(text,"%s ~FG%s ~FR%s~FM <=-\n",logout_hide,user->recap,user->desc);
		  }
		  write_user(user,text);
		  write_level(ADMIN,user,text);
		  write_log(1,LOG2,"[LOGOFF] - User: [%s] [HIDDEN]\n",user->name);
		  break;
		default:
		  WriteRoomExcept(rm,user,"~CW- ~FG%s ~FT%s\n",user->recap,user->logout_msg);
		  if (user->predesc[0])
		  {
			if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FG %s~FR%s~FM <=-\n",logout_vis,user->predesc,user->recap,user->desc);
			else sprintf(text,"%s ~FG%s~FG %s~FR %s~FM <=-\n",logout_vis,user->predesc,user->recap,user->desc);
		  }
		  else
		  {
			if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_vis,user->recap,user->desc);
			else sprintf(text,"%s ~FG%s ~FR%s~FM <=-\n",logout_vis,user->recap,user->desc);
		  }
		  write_user(user,text);
		  write_room_except(NULL,user,text);
		  friend_check(user,2);
		  write_log(1,LOG2,"[LOGOFF] - User: [%s]\n",user->name);
	}
	strcpy(user->temp_recap,user->recap);
	strcpy(user->recap,"~FM[~CR?~FM]");
	user->invis = Fakelog;
	SaveUser(user,1);
	reset_junk(user);
	reset_access(rm);
	telnet=NULL;
	if ((telnet = get_telnet(user)) != NULL) disconnect_telnet(telnet);
	destroy_user_clones(user);
	write_user(user,"~FG You are now hidden to everyone. Well.. except for GODS and above ;-)\n");
	logging_in = 0;
	if (webpage_on) who_webpage();
}

/*
   shackles a user to a room so that they cannot leave..kinda done like the
   muzzles, where it goes by level.
*/
void shackle(User user,char *inpstr,int type)
{
	User u;
	char *name,*rname;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user && user->level < OWNER)
	{
		writeus(user,"~FM Trying to ~FT%s~FM yourself ...now i know your crazy.\n",command[com_num].name);
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (u->invis == Invis) rname = invisname;  else rname = u->recap;
	switch (type)
	{
		case 1: 
		  if (u->status & Shackled)
		  {
			writeus(user,"~FT %s ~FMhas already been shackled.\n",u->recap);
			return;
		  }
		  if (u->level >= user->level && user->level<OWNER)
		  {
			write_user(user,"~FM You cannot shackle someone of a greater or equal level than your own!\n");
			writeus(u,"~FT %s~FY attempted to shackle you.\n",user->recap);
			return;
		  }
		  if (user->status & Silenced)
		  {
			writeus(user,"~FY You strap shackles onto ~FM%s~FY so they cannot leave the room.\n",u->recap);
			return;
		  }
		  writeus(user,"~FY You strap shackles onto ~FM%s~FY so they cannot leave the room.\n",u->recap);
		  WriteRoomExcept(user->room,user,"~FM %s ~FYstraps some shackles onto ~FM%s~FY binding them to this room.\n",name,rname);
		  u->status += Shackled;
		  sprintf(text,"[SHACKLE] - User: [%s] Victim: [%s]\n",user->name,u->name);
		  write_log(1,LOG4,text);
		  write_record(u,1,text);
		  return;
		case 2:
		  if (!(u->status & Shackled))
		  {
			writeus(user,"~FT %s ~FMisn't currently shackled.\n",u->recap);
			return;
		  }
		  if (u->level >= user->level && user->level < OWNER)
		  {
			write_user(user,"~FM You cannot unshackle someone of a greater or equal level than your own!\n");
			writeus(u,"~FT %s~FY attempted to unshackle you.\n",user->recap);
			return;
		  }
		  if (user->status & Silenced)
		  {
			writeus(user,"~FY You reluctantly take the keys and unshackle ~FM%s.\n",u->recap);
			return;
		  }
		  writeus(user,"~CY You reluctantly take the keys and unshackle ~FM%s\n",u->recap);
		  WriteRoomExcept(user->room,user,"~FM %s~FY reluctantly takes %s keys and unshackles ~FM%s~FY, setting %s free\n",name,gender1[user->gender],rname,gender3[user->gender]);
		  u->status -= Shackled;
		  sprintf(text,"[UNSHACKLE] - User: [%s] Victim: [%s]\n",user->name,u->name);
		  write_log(1,LOG4,text);
		  write_record(u,1,text);
	}   /* end of switch */
	return;
}

/*
   Makes a user visible again, if they are fake logged off, it appears as tho
   they are coming back online again...
*/
void make_visible(User user,char *inpstr,int rt)
{
	User u;
	char *name,*rname;

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
		write_user(user,"~FMI think .vis would probably be a bit easier...\n");
		return;
	}
	if (u->invis == 0)
	{
		write_user(user,"~FM That user is already visible..\n");
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (u->invis == Invis)  rname = invisname;  else rname = u->recap;
	if (u->level >= user->level && user->level < OWNER)
	{
		write_user(user,"~FM You cannot make a user of a greater or equal level then your own, visible.\n");
		writeus(u,"~FT %s~FY wanted to make you visible.\n",user->recap);
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW-~RS You call upon the mystical forces to make ~FY%s~RS appear...\n",u->recap);
		return;
	}
	writeus(user,"~CW-~RS You call upon the mystical forces to make ~FY%s~RS appear...\n",u->recap);
	switch(u->invis)
	{
		case 1 :
		  write_user(u,"~CW-~RS You appear in front of everyone seemingly out of nowhere...\n");
		  WriteRoomExcept(u->room,u,"~CW-~RS A mysterious fog appears, and out of it ~FT%s ~RSemerges...\n",u->recap);
		  break;
		case 2 :
		  strcpy(u->recap,u->temp_recap);
		  u->temp_recap[0] = '\0';
		  write_user(u,"~CW-~RS You appear in front of everyone seemingly out of nowhere...\n");
		  WriteRoomExcept(u->room,u,"~CW-~RS A mysterious fog appears, and out of it ~FT%s ~RSemerges...\n",u->recap);
		  break;
		case 3 :
		  write_user(u,"~CRYou are being forced to come back.\n");
		  strcpy(u->recap,u->temp_recap);
		  u->temp_recap[0] = '\0';
		  if (u->predesc[0] != '\0')
		  {
			if (u->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s ~CB<=-\n",login_vis,u->predesc,u->recap,u->desc);
			else sprintf(text,"%s ~FY%s ~FY%s~FM %s ~CB<=-\n",login_vis,u->predesc,u->recap,u->desc);
		  }
		  else
		  {
			if (u->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s ~CB<=-\n",login_vis,u->recap,u->desc);
			else sprintf(text,"%s ~FY%s~FM %s ~CB<=-\n",login_vis,u->recap,u->desc);
		  }
		  write_user(u,text);
		  write_room_except(NULL,u,text);
		  if (user->desc[0] == '\'') sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
		  else  sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
		  RecordLogin(text);
		  WriteLevel(WIZ,u,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",u->site,u->site_port);
		  write_log(1,LOG2,"[LOGIN] - User: [%s] Site: [%s:%d] Port: [%d]\n",u->name,u->site,u->site_port,u->port);
	  	  u->room=get_room(main_room);
		  SaveUser(u,1);
		  reset_junk(u);
		  u->last_login = time(0);
		  friend_check(u,1);
		  wait_check(u);
		  WriteRoomExcept(u->room,u,"~CW- ~FG%s~FR %s\n",u->recap,u->login_msg);
		  break;
	}
	sprintf(text,"[MAKEVIS] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	u->invis = 0;
}

/* Should be able to make a user invisible too */
void make_invis(User user,char *inpstr,int rt)
{
	User u;
	char *name,*rname;

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
		write_user(user,"~FM I think .invis would probly be a bit easier...\n");
		return;
	}
	if (u->invis)
	{
		write_user(user,"~FM That user is already invisible..\n");
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (u->invis == Invis) rname = invisname;  else rname = u->recap;
	if (u->level >= user->level && user->level < OWNER)
	{
		write_user(user,"~FM You cannot make a user of a greater or equal level then your own, invisible.\n");
		writeus(u,"~FT %s~FY wanted to make you invisible!\n",user->recap);
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW-~RS You call upon the forces to make ~FG%s~RS go away...\n",rname);
		return;
	}
	writeus(user,"~CW-~RS You call upon the forces to make ~FG%s~RS go away...\n",rname);
	write_user(u,"~CW-~RS The last thing you see is mysterious fog coming your way...\n");
	WriteRoomExcept(u->room,u,"~CT%s~RS gets swallowed by a mysterious fog...\n",u->recap);
	u->invis = 1;
	sprintf(text,"[MAKEINVIS] - User: [%s] Victim: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);
	write_record(u,1,text);
}

/* Writes something to the users record... */
void add_user_hist(User user,char *inpstr,int rt)
{
	User u;
	int on,err = 0;
	char *uname;

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
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRhistory to add~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) == NULL)
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~FG An error occurred.. Try back in a sec.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in add_user_hist()\n");
			return;
		}
		err = LoadUser(u,uname);
		if (err == 0)
		{
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		}
		else if (err == -1)
		{
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
		u->socket = -2;
		strcpy(u->site,u->last_site);
	}
	else on = 1;
	writeus(user,"~CW--~FG History to ~CW'~FT%s~CW'~FGbeen added.\n",u->recap);
	inpstr = remove_first(inpstr);
	sprintf(text,"[AddHistory (%s)]\n   -- %s\n",user->name,inpstr);
	write_record(u,1,text);
	write_log(1,LOG1,"[AddHistory]: User: [%s] Added To: [%s]\n",user->name,u->name);
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
	return;
}

/*
   Checks sites of all users or sites to see if anyone has the same site
   Based somewhat off of the PARIS samesite.. I originally used Andy's
   but kept on running into problems with it, so I decided to integrate
   my ideas as well as Arny's and see how this one comes up.
*/
void samesite(User user,char *inpstr,int stage)
{
	User u,u2 = NULL;
	Ulobj ul;
	int checked,same,err;
	char givensite[SiteNameLen+1],filename[FSL];

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
	checked = same = 0;
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CRpart of address to check~CB]");
		writeus(user,"~CW - ~FTExample~CB: ~CR.%s squirt ~CW(~FMFind all sites matching Squirt's.~CW)\n",command[com_num].name);
		writeus(user,"~CW - ~FTExample~CB: ~CR.%s .aol.com ~CW(~FMFind all sites with '.aol.com' it it~CW)\n",command[com_num].name);
		return;
	}
	for_users(u)		/* Save all online users details first. */
	{
		if (u->type == CloneType || u->login) continue;
		SaveUser(u,1);
		reset_junk(u);
	}
	/* First check to see if it's a user. */
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	if ((u = get_user(word[1])) == NULL)
	{
		sprintf(filename,"%s/%s.dat",UserDir,word[1]);
		if (file_exists(filename))
		{
			if ((u = create_user()) == NULL)
			{
				write_log(1,LOG1,"[ERROR] - Unable to create temp. user object in samesite().\n");
				write_user(user,"~CW-~FT An error occurred..aborting... Try again in a few minutes.\n");
				return;
			}
			err = LoadUser(u,word[1]);
			if (err == 0)
			{
				write_user(user,center(nosuchuser,80));
				destruct_user(u);
				destructed = 0;
				return;
			}
			if (err == -1)
			{
				write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",word[1]);
				write_user(user,"~CW-~FT That user's userfile is corrupted, please reset it.\n");
				destruct_user(u);
				destructed = 0;
				return;
			}
			strncpy(givensite,u->last_site,sizeof(givensite)-1);
			sprintf(text,"~CW - ~FTChecking user: ~CB[~CR%s~CB]~FT Site~CB: [~CR*%s*~CB]\n",u->recap,givensite);
			destruct_user(u);
			destructed = 0;
		}
	
		else
		{
			strncpy(givensite,word[1],sizeof(givensite)-1);
			sprintf(text,"~CW- ~FTChecking site~CB: [~RS*%s*~CB]\n",word[1]);
		}
	}
	else
	{
		strncpy(givensite,u->site,sizeof(givensite)-1);
		sprintf(text,"~CW - ~FTChecking user: ~CB[~CR%s~CB]~FT Site~CB: [~CR*%s*~CB]\n",u->recap,givensite);
	}
	write_user(user,text);
	if ((u = create_user()) == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Unable to create temp. user object in samesite().\n");
		write_user(user,"~CW-~FT An error occurred..aborting.. Please try again in a few minutes.\n");
		return;
	}
	for_list(ul)		/* Go through the user list. */
	{
		++checked;
		reset_vars(u);
		if ((u2 = get_user(ul->name)) != NULL) /* online user. */
		{
			/* Check both the ip site, and alphanumeric site */
			if (istrstr(u2->site,givensite)
			  || istrstr(u2->ip_site,givensite))
			{
				++same;
				writeus(user,"~CW --~FG Match found: User~CB: [~RS%-12s~CB] - ~FGSite~CB: [~RS%s~CB]\n",ul->name,u2->site);
			}
		}
		else
		{
			err = LoadUser(u,ul->name);
			if ((err == 0) || (err == -1))
			{
				write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted or missing.\n",ul->name);
				write_user(user,"~CW-~FT An error occurred.. aborting.\n");
				reset_vars(u);
				continue;
			}
			if (istrstr(u->last_site,givensite))
			{
				++same;
				writeus(user,"~CW --~FG Match found: User~CB: [~RS%-12s~CB] - ~FGSite~CB: [~RS%s~CB]\n",ul->name,u->last_site);
				reset_vars(u);
			}
		}
	}
	destruct_user(u);
	destructed = 0;
	if (same == 0) write_user(user,"~CW- ~FTNo matches were found.\n");
	else writeus(user,"~CW- ~FTChecked %d account%s. %d match%s found for ~CR%s\n",checked,checked>1?"s":"",same,same>1?"es":"",givensite);
	return;
}

/*
   Change a users login name
*/
void change_name(User user,char *inpstr,int rt)
{
	User u;
	int on = 0,err = 0;
	char file1[FSL],file2[FSL];
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
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRnew login name~CB]");
		return;
	}
	if (user->tpromoted && user->orig_lev<WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	strtolower(word[1]);
	strtolower(word[2]);
	word[1][0] = toupper(word[1][0]);
	word[2][0] = toupper(word[2][0]);
	if (!isalpha(word[2][0]))
	{
		write_user(user,"~FG You can only user letters and numbers in the name.\n");
		return;
	}
	for (i = 0; i < strlen(word[2]); ++i)
	{
		if (!isalnum(word[2][i]))
		{
			write_user(user,"~FG You can only use letters and numbers in the name.\n");
			return;
		}
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		sprintf(file1,"%s/%s.dat",UserDir,word[2]);
		if (file_exists(file1))
		{
			write_user(user,"~CR That name is currently taken.\n");
			return;
		}
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred, try back in a few minutes.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in change_name().\n");
			return;
		}
		err = LoadUser(u,word[1]);
		if (err == 0)
		{
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		}
		else if (err == -1)
		{
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",word[1]);
			write_user(user,"~CW-~FM That user's userfile is corrupted, Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		u->socket = -2;
		strcpy(u->site,u->last_site);
		on = 0;
	}
	else on = 1;
	if (u->level >= user->level && user->level < OWNER)
	{
		write_user(user,"~FM You cannot change the name of a user of a greater or equal level then your own!\n");
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	sprintf(file1,"%s/%s.dat",UserDir,word[1]);
	sprintf(file2,"%s/%s.dat",UserDir,word[2]);
	rename(file1,file2);
	sprintf(file1,"%s/%s/%s.M",UserDir,UserMesg,word[1]);
	sprintf(file2,"%s/%s/%s.M",UserDir,UserMesg,word[2]);
	rename(file1,file2);
	sprintf(file1,"%s/%s/%s.S",UserDir,UserMesg,word[1]);
	sprintf(file2,"%s/%s/%s.S",UserDir,UserMesg,word[2]);
	rename(file1,file2);
	sprintf(file1,"%s/%s/%s.P",UserDir,UserMesg,word[1]);
	sprintf(file2,"%s/%s/%s.P",UserDir,UserMesg,word[2]);
	rename(file1,file2);
	sprintf(file1,"%s/%s/%s.R",UserDir,UserMisc,word[1]);
	sprintf(file2,"%s/%s/%s.R",UserDir,UserMisc,word[2]);
	rename(file1,file2);
	strcpy(u->name,word[2]);
	strcpy(u->recap,word[2]);
	SaveUser(u,1);
	if (on) reset_junk(u);
	writeus(user,"~FG You changed ~CT%s%s ~FGname to ~CR%s\n",word[1],word[1][strlen(word[1])-1] == 's'?"'":"'s",word[2]);
	sprintf(text,"[RENAME] - User: [%s] Renamed: [%s] to [%s]\n",user->name,word[1],word[2]);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	clean_userlist(word[1]);
	add_userlist(u->name,u->level);
	if (on) writeus(u,"~CR %s ~FGhas changed your name to~CB: [~RS%s~CB]\n",user->name,u->name);
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
}

/*
   Saves all the users details who are currently logged on.
*/
void save_users(User user,char *inpstr,int rt)
{
	User u;
	int count;

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
	count = 0;
	for_users(u)
	{
		if (u->type == CloneType || u->login) continue;
		count++;
		SaveUser(u,1);
		reset_junk(u);
	}
	if (count != 0)
	{
		writeus(user,"~FR You have saved ~FM%d~FR user%s details...!\n",count,count>1?"s":"");
		write_log(1,LOG4,"[SAVE] User: [%s] - %d user%s saved.\n",user->name,count,count>1?"s":"");
	}
}

/* lets a user resign from being a staff member */
void resign_staff(User user,char *inpstr,int rt)
{
	FILE *f1, *f2;
	char filename[FSL],check[UserNameLen+1];

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
	if (user->tpromoted && user->orig_lev<WIZ)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	write_user(user,"~FM You resign from being staff, but however, you are left at the highest user\n");
	write_user(user,"~FM level. It was a pleasure having you as staff, and we will always be glad that\n");
	write_user(user,"~FM you chose to be a staff member on this talker!\n");
	user->level = POWER;
	WriteLevel(WIZ,user,"~FM %s~CR RESIGNED~FG from being a staff member.\n",user->recap);
	sprintf(text,"[RESIGNATION] - User: [%s]\n",user->name);
	write_log(1,LOG4,text);
	write_record(user,1,text);
	sprintf(filename,"%s/%s",DataDir,StaffFile);
	f1 = fopen(filename,"r");
	if (f1 == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Failed to remove '%s' from the staff file.\n",user->name);
		return;
	}
	f2 = fopen("templist","w");
	if (f2 == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Failed to remove '%s' from the staff file.\n",user->name);
		FCLOSE(f1);
		return;
	}
	fscanf(f1,"%s",check);
	while (!(feof(f1)))
	{
		if (strcasecmp(user->name,check))
			fprintf(f2,"%s\n",check);
		fscanf(f1,"%s",check);
	}
	FCLOSE(f1);
	FCLOSE(f2);
	unlink(filename);
	rename("templist",filename);
	return;
}

/* Deny a user certain commands */
void deny_command(User user,char *inpstr,int rt)
{
	User u;
	Commands cmd;
	char *uname;
	int cnt,cnt2,err,i,on;

	cnt = cnt2 = err = i = on = 0;
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
	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CRcommand name~CB>]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (!strcasecmp(user->name,uname))
	{
		write_user(user,"~CW-~FM You're on crack for trying to use this command on yourself.\n");
		return;
	}
	if ((u = get_user(uname)) == NULL)
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred, please try again in a few minutes.\n");
			write_log(1,LOG1,"[ERROR] - Unable to create temp. user object in deny_command().\n");
			return;
		}
		err = LoadUser(u,uname);
		if (err == 0)
		{
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		}
		else if (err == -1)
		{
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			write_user(user,"~CW-~FT That user's userfile appears to be corrupted. Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
		u->socket = -2;
		strcpy(u->site,u->last_site);
	}
	else on = 1;
	if (word_count < 3)
	{
		for (i = 0; i < 25; ++i)
		{
			if (user->denycom[i][0] == '\0') continue;
			if (++cnt == 1)
			{
				if (user->ansi_on) write_user(user,ansi_tline);
				else write_user(user,ascii_tline);
				sprintf(text,"~FT-~CT=~CB>~FG A list of denied commands for ~FY%s~FG.~CB<~CT=~FT-\n",u->recap);
				write_user(user,center(text,80));
				if (user->ansi_on) write_user(user,ansi_bline);
				else write_user(user,ascii_bline);
			}
			writeus(user,"~FT%-17.17s",user->denycom[i]);
			cnt2++;
			if (cnt2 >= 4)
			{
				write_user(user,"\n");
				cnt2 = 0;
			}
		}
		if (cnt2 > 0 && cnt2 < 4) write_user(user,"\n");
		i = 0;
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT That user has no denied commands.\n");
		}
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	strtolower(word[2]);
	if ((cmd = get_command(user,word[2])) == NULL)
	{
		writeus(user,wtf,word[2]);
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	/* Now we check to see if the command is allowed to them. */
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(user->allowcom[i],word[2]))
		{
			user->allowcom[i][0] = '\0';
			i = 0;
			writeus(user,"~CW-~FT You have removed the allowed ~CW'~FM%s~CW' ~FTcommand from ~FR%s.\n",word[2],u->recap);
			sprintf(text,"[DENY] - User: [%s] Command Unallowed From: [%s  (.%s)]\n",user->name,u->name,cmd->name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			if (on == 1)
			{
				writeus(u,"~CW-~FT You have had the allowed ~CW'~FM%s~CW'~FT taken away.\n",word[2]);
			}
			if (on == 0)
			{
				sprintf(text,"~CW- ~FT You have had the ~CW'~FM%s~CW'~FT command taken away from you.\n",cmd->name);
				send_oneline_mail(NULL,u->name,text);
				SaveUser(u,0);
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
	}
	if (cmd->min_lev > user->level)
	{
		write_user(user,"~FM You need the command before you can deny someone it.\n");
		if (on==0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(user->denycom[i],cmd->name))
		{
			write_user(user,"~FG That user already has that command denied from them.\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			} i = 0;
			return;
		}
	}
	for (i = 0; i < 25; ++i)
	{
		if (user->denycom[i][0] == '\0') /* This is where we store it */
		{
			strcpy(user->denycom[i],cmd->name);
			writeus(user,"~CW-~FT You have denied the ~CW'~FM%s~CW'~FT command from ~FR%s.\n",cmd->name,u->recap);
			sprintf(text,"[DENY] - User: [%s] Command Denied From: [%s  (.%s)]\n",user->name,u->name,cmd->name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			if (on == 1) writeus(u,"~CW-~FT You have had the ~CW'~FM%s~CW' ~FTcommand denied from you.\n",cmd->name);
			if (on == 0)
			{
				sprintf(text,"~CW- ~FT You have had the ~CW'~FM%s~CW'~FT command denied from you.\n",cmd->name);
				send_oneline_mail(NULL,u->name,text);
				SaveUser(u,0);
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
	}
	write_user(user,"~CW-~FG That user appears to have the max amount of denied commands.\n");
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
	return;
}

/* Allow a user certain commands */
void allow_command(User user,char *inpstr,int rt)
{
	User u;
	Commands cmd;
	char *uname;
	int cnt,cnt2,err,i,on;

	cnt = cnt2 = err = i = on = 0;
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
	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CRcommand name~CB>]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (!strcasecmp(user->name,uname))
	{
		write_user(user,"~CW-~FM You're on crack for trying to use this command on yourself.\n");
		return;
	}
	if ((u = get_user(uname)) == NULL)
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred in creating a user object.\n");
			write_log(1,LOG1,"[ERROR] - Unable to create temp. user object in deny_command().\n");
			return;
		}
		err = LoadUser(u,uname);
		if (err == 0)
		{
			write_user(user,center(nosuchuser,80));
			destruct_user(u);
			destructed = 0;
			return;
		}
		else if (err == -1)
		{
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			write_user(user,"~CW-~FT That user's userfile appears to be corrupted. Please reset it.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
		u->socket = -2;
		strcpy(u->site,u->last_site);
	}
	else on = 1;
	if (word_count < 3)	/* List out the allowed commands */
	{
		for (i = 0; i < 25; ++i)
		{
			if (user->allowcom[i][0] == '\0') continue;
			if (++cnt == 1)
			{
				if (user->ansi_on) write_user(user,ansi_tline);
				else write_user(user,ascii_tline);
				sprintf(text,"~FT-~CT=~CB>~FG A list of allowed commands for ~FY%s~FG.~CB<~CT=~FT-\n",u->recap);
				write_user(user,center(text,80));
				if (user->ansi_on) write_user(user,ansi_bline);
				else write_user(user,ascii_bline);
			}
			writeus(user,"~FT%-17.17s",user->allowcom[i]);
			cnt2++;
			if (cnt2 >= 4)
			{
				write_user(user,"\n");
				cnt2 = 0;
			}
		}
		if (cnt2 > 0 && cnt2 < 4) write_user(user,"\n");
		i = 0;
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT That user has no allowed commands.\n");
		}
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	strtolower(word[2]);
	if ((cmd = get_command(user,word[2])) == NULL)
	{
		writeus(user,wtf,word[2]);
		if (on==0)
		{
			destruct_user(u);
			destructed=0;
		}
		return;
	}
	/* Now we check to see if the command is allowed to them. */
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(user->denycom[i],word[2]))
		{
			user->denycom[i][0] = '\0';
			i = 0;
			writeus(user,"~CW-~FT You have allowed the denied ~CW'~FM%s~CW' ~FTcommand to ~FR%s.\n",word[2],u->recap);
			sprintf(text,"[ALLOW] - User: [%s] Command Re-Allowed To: [%s  (.%s)]\n",user->name,u->name,cmd->name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			if (on == 1)
			{
				writeus(u,"~CW-~FT You have access to your denied ~CW'~FM%s~CW'~FT command.\n",word[2]);
			}
			if (on == 0)
			{
				sprintf(text,"~CW- ~FT You have had the ~CW'~FM%s~CW'~FT command given back to you.\n",cmd->name);
				send_oneline_mail(NULL,u->name,text);
				SaveUser(u,0);
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
	}
	if (cmd->min_lev > user->level)
	{
		write_user(user,"~FM You need the command before you can deny someone it.\n");
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(user->allowcom[i],cmd->name))
		{
			write_user(user,"~FG That user already has that command allowed to them.\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			} i = 0;
			return;
		}
	}
	for (i = 0; i < 25; ++i)
	{
		if (user->allowcom[i][0] == '\0')
		{
			strcpy(user->allowcom[i],cmd->name);
			writeus(user,"~CW-~FT You have allowed the ~CW'~FM%s~CW'~FT command to ~FR%s.\n",cmd->name,u->recap);
			sprintf(text,"[ALLOW] - User: [%s] Command Allowed To: [%s  (.%s)]\n",user->name,u->name,cmd->name);
			write_log(1,LOG4,text);
			write_record(u,1,text);
			if (on == 1) writeus(u,"~CW-~FT You have had the ~CW'~FM%s~CW' ~FTcommand allowed to you.\n",cmd->name);
			if (on == 0)
			{
				sprintf(text,"~CW- ~FT You have had the ~CW'~FM%s~CW'~FT command allowed to you.\n",cmd->name);
				send_oneline_mail(NULL,u->name,text);
				SaveUser (u,0);
				destruct_user(u);
				destructed = 0;
			}
			sprintf(text,"[ALLOW] - User: [%s] Command Allowed To: [%s:%s]\n",user->name,u->name,cmd->name);
			return;
		}
	}
	write_user(user,"~CW-~FG That user appears to have the max amount of allowed commands.\n");
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
	return;
}

/* Force a user to execute a certain command */
void force(User user,char *inpstr,int rt)
{
	User u;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRcommand~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FG Umm why would you want to force yourself to do something?!\n");
		return;
	}
	if (!strncasecmp(word[2],"cap",3))
	{
		write_user(user,"~CW-~FG You can't force someone to .capture..\n");
		return;
	}
	inpstr = remove_first(inpstr);
	if (u->level >= user->level && user->level < OWNER)
	{
		write_user(user,"~FR You cannot force someone of a greater or equal level then your own!\n");
		writeus(u,"~FM %s ~FTattempted to force you to~CB: [~RS%s~CB]\n",user->recap,inpstr);
		return;
	}
	clear_words();
	word_count = wordfind(inpstr);
	sprintf(text,"%s\n",inpstr);
	write_user(u,text);
	writeus(user,"~FG You force ~FT%s~FG to execute~CB: [~RS%s~CB]\n",u->recap,inpstr);
	sprintf(text,"[FORCE] - User: [%s] Victim: [%s] Execute: [%s]\n",user->name,u->name,inpstr);
	write_log(1,LOG4,text);
	write_record(u,1,text);
	exec_com(u,inpstr);
	return;
}

/* Warn an annoying user in case they are being harassive or some shit */
void warn_user(User user,char *inpstr,int rt)
{
	User u;
	char *name;

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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRwhy you're warning them~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FG Umm whats the point of warning yourself??!\n");
		return;
	}
	inpstr = remove_first(inpstr);
	if (u->level >= user->level && user->level<OWNER)
	{
		write_user(user,"~FR You cannot warn someone of a greater or equal level then your own.\n");
		writeus(u,"~FM%s ~FTattempted to warn you for~CB: [~RS%s~CB]\n",name,inpstr);
		return;
	}
	writeus(u,"\07 ~CB[~CR~LIWARNING~RS~FG from~FT %s~CB]~FT %s\n",name,inpstr);
	sprintf(text," ~FT%s~FG warns~FT %s~FG for~CB: [~RS%s~CB]\n",name,u->recap,inpstr);
	write_user(user,text);
	write_level(WIZ,user,text);
	sprintf(text,"[WARN] - User: [%s] Warned: [%s] For: [%s]\n",user->name,u->name,inpstr);
	write_log(1,LOG4,text);
	write_record(u,1,text);
}

/*
   This will display to the user the getname variable.. if auto_auth is set
   to ON, then when the user logs on their user->realname flag will be set
   providing the server they are connecting from runs an identd
*/
void get_real_name(User user,char *inpstr,int rt)
{
	User u;

	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
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
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}

	if (!strcasecmp(u->realname,"unknown") || u->realname[0] == '\0') writeus(user,"~CR%s%s~FG login server isn't running an identd.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
	else writeus(user,"~CR%s%s ~FGserver name appears to be~CB: [~RS%s~CB]\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s",u->realname);
	return;
}

/* Create a new user */
void make_newuser(User user,char *inpstr,int rt)
{
	User u;
	char filename[FSL];
	size_t i = 0;

	if (word_count<3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRnew user name~CB] [~CRpassword for user~CB]");
		return;
	}
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
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	if (!isalpha(word[1][0]))
	{
		write_user(user,"~CW-~FT You can only use letters in the first part of the name.\n");
		return;
	}
	for (i = 0; i<strlen(word[1]); ++i)
	{
		if (!isalnum(word[1][i]))
		{
			write_user(user,"~FG You can only user letters and numbers in the name.\n");
			return;
		}
	}
	if (strlen(word[1]) > UserNameLen)
	{
		write_user(user,"~FG That name is a wee bit long dontcha think?\n");
		return;
	}
	if (strlen(word[1]) < 3)
	{
		write_user(user,"~FG Ummm that name is rather short.\n");
		return;
	}
	sprintf(filename,"%s/%s.dat",UserDir,word[1]);
	if (file_exists(filename))
	{
		write_user(user,"~FG There is already an account with that name!\n");
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred.. Try again in a few minutes.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temp. user object in make_newuser().\n");
		return;
	}
	strcpy(u->name,word[1]);
	strcpy(u->pass,(char *)crypt(word[2],salt));
	strcpy(u->desc,def_desc);
	strcpy(u->in_phr,def_inphr);
	strcpy(u->out_phr,def_outphr);
	strcpy(u->recap,u->name);
	strcpy(u->webpage,def_url);
	strcpy(u->bday,def_bday);
	strcpy(u->married_to,"Noone.");
	if (free_rooms) u->has_room = 1;  else u->has_room = 0;
	strcpy(u->login_msg,def_lognmsg);
	strcpy(u->logout_msg,def_logtmsg);
	strcpy(u->last_site,"never.logged.on");
	sprintf(text,"%s@rt2.talker",u->name);
	strcpy(u->email,text);
	u->char_echo = charecho_def;
	u->prompt=prompt_def;
	u->predesc[0] = '\0';
	u->lev_rank[0] = '\0';
	SaveUser(u,0);
	write_log(1,LOG1,"[NEW-USER] - User: [%s] Created: [%s]\n",user->name,word[1]);
	writeus(user,"~FT New user ~FR%s~FT has been created Password~CB: [~CR%s~CB]\n",word[1],word[2]);
	add_userlist(u->name,USER);
	destruct_user(u);
	destructed = 0;
	return;
}

/* Counts the all the memory that the talker is currently using up */
void memory_count(User user,char *inpstr,int rt)
{
	User	u;
	Room	rm;
	Ulobj	ul;
	Rmlobj	rl;
	Telnet	telnet;
	Pgame	game;
	Editor	e;
	Socials soc;
	Commands cmd;
#ifdef HAVE_BSHIP
	Bship	bs = bships_first;
#endif
	float	kb, megs, bytes, totmem;
	long	memused, size;

	memused = size = totmem = 0;
	kb = megs = bytes = 0;
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~CRðÄÄ¯¯> ~FG%s ~CYtalker running ~FG%s ~CYmemory count! ~CR<®®ÄÄð\n",TalkerRecap,VERSION);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYtalker running ~FG%s ~FYmemory count! ~CB<~CT=~FT-\n",TalkerRecap,VERSION);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	/* Figure out the size of each */

	size = sizeof(struct user_struct);
	for_users(u)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG      User Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct userlist_struct);
	for_list(ul)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG User List Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct room_struct);
	for_rooms(rm)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG      Room Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct rmlist_struct);
	for_rmlist(rl)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG Room List Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct cmd_struct);
	for_comms(cmd)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG   Command Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct soc_struct);
	for_socials(soc)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG    Social Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct input_buff);
	for_users(u)
	{
		if (u->buffer == NULL) continue;
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FGInput Buff Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct conv_buff);
	for_users(u)
	{
		if (u->away_buff != NULL)
		{
			memused += size;
			totmem  += size;
		}
		if (u->tbuff != NULL)
		{
			memused += size;
			totmem  += size;
		}
	}
	for_rooms(rm)
	{
		if (rm->revbuff != NULL)
		{
			memused += size;
			totmem  += size;
		}
	}
	if (shoutbuff != NULL)
	{
		memused += size;
		totmem  += size;
	}
	if (wizbuff != NULL)
	{
		memused += size;
		totmem  += size;
	}
	if (channelbuff != NULL)
	{
		memused += size;
		totmem  += size;
	}
	if (debugbuff != NULL)
	{
		memused += size;
		totmem  += size;
	}
	if (loginbuff != NULL)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG Rev. Buff Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct editor_struct);
	for_edit(e)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG    Editor Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct telnet_struct);
	for_telnet(telnet)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG    Telnet Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct poker_player);
	for_users(u)
	{
		if (u->pop == NULL) continue;
		memused += size;
		totmem  += size;
	}
	size = sizeof(struct poker_game);
	for_game(game)
	{
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG     Poker Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct chess_struct);
	for_users(u)
	{
		if (u->chess == NULL) continue;
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG     Chess Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct checkers_struct);
	for_users(u)
	{
		if (u->checkers == NULL) continue;
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG  Checkers Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

	size = sizeof(struct bjack_struct);
	for_users(u)
	{
		if (u->bjack == NULL) continue;
		memused += size;
		totmem  += size;
	}
	writeus(user,"~FG Blackjack Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;

#ifdef HAVE_BSHIP
	size = sizeof(struct bship_struct);
	while (bs != NULL)
	{
		memused += size;
		totmem  += size;
		bs = bs->next;
	}
	writeus(user,"~FGBattleship Structure~CB: ~CY%7ld ~FGbytes ~FT--------- ~FGAllocated~CB: ~CY%7ld ~FGbytes.\n",size,memused);
	memused = 0;
#else
	writeus(user,"~FGNo Battleship support found.\n");
#endif
	megs  = totmem / 1048576;
	kb    = totmem / 1024;
	bytes = totmem;

	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~CRðÄÄ¯¯> ~FG%s ~CYis in total, allocating ~CT%.2f~CY bytes. ~CR<®®ÄÄð\n",TalkerRecap,totmem);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		if (megs >= 1) sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYis in total, allocating ~CR%.2f ~FYmegabytes. ~CB<~CT=~FT-\n",TalkerRecap,megs);
		else if (kb >= 1) sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYis in total, allocating ~CR%.2f ~FYkilobytes. ~CB<~CT=~FT-\n",TalkerRecap,kb);
		else sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYis in total, allocating ~CR%.2f ~FYbytes. ~CB<~CT=~FT-\n",TalkerRecap,bytes);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
}

/* Forces a backup of the talkers files */
void force_backup(User user,char *inpstr,int rt)
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
	if (!backup_files()) return;
	else
	{
		write_room(NULL,"      ~CB[~CRSYSTEM~CB]~FG Talker backup has been FORCED.\n");
		writeus(user,"~FG Backing up talker files, for details see ~CW'~CR.view -backuplog~CW'\n");
		write_log(1,LOG1,"[FORCE-BACKUP] - User: [%s]\n",user->name);
		return;
	}
}

/* END OF STAFF.C */
