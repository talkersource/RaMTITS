/*
  general.c
    Code for general functions through out the talker, stuff like
    .examine, .stats, etc.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Mike Irving, Reddawg, Cygnus, Andrew Collington,
	and Neil Robertson.

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
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define _GENERAL_C
  #include "include/needed.h"
#undef _GENERAL_C
#include "include/genlib.h"
#include <unistd.h>
extern char *crypt __P ((__const char *__key, __const char *__salt));

/* Sets the users description */
void set_desc(User user,char *inpstr,int rt)
{
	char temp[ARR_SIZE];

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,"~FG Your current description is~CB: [~RS%s~CB]\n",user->desc);
		return;
	}
	strcpy(temp,ColourStrip(inpstr));
	if (strlen(temp) > UserDescLen)
	{
		write_user(user,"~FG That description is too long.\n");
		return;
	}
	if (strlen(inpstr)>80)
	{
		write_user(user,"~FG That description is to long.\n");
		return;
	}
	strcpy(user->desc,inpstr);
	if (user->predesc[0]) writeus(user,"~FGYou will now be known as~CB: ~RS%s~RS %s%s~RS%s\n",user->predesc,user->invis>Invis?user->temp_recap:user->recap,user->desc[0] == '\''?"":" ",user->desc);
	else writeus(user,"~FGYou will now be known as~CB: ~RS %s%s~RS%s\n",user->invis>Invis?user->temp_recap:user->recap,user->desc[0] == '\''?"":" ",user->desc);
}

/* Wake up some user just idling away */
void wake(User user,char *inpstr,int rt)
{
	User u;
	char *name;

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
	if (user->invis == Invis) name = invisname; else name = user->recap;
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-tfu",2))
	{
		if ((u = get_name(user,word[2])) == NULL)
		{
			write_user(user,center(notloggedon,80));
			return;
		}
		if (u == user)
		{
			write_user(user,"~FG If you're trying to wake yourself aren't you already awake?\n");
			return;
		}
		if (is_enemy(u,user))
		{
			write_user(user,"~FG You can't wake up someone who has made you an enemy.\n");
			return;
		}
		if (is_enemy(user,u))
		{
			write_user(user,"~FG You can't wake up someone who is your enemy.\n");
			return;
		}
		if (user->editor.editing)
		{
			writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
			return;
		}
		writeus(user,"~FG You bellow at ~CW'~FT%s~CW' ~FGto wake the fuck up.\n",u->recap);
		writeus(u,"~FR%s ~FGwants you to~CB:\n",name);
		write_user(u,"\07~CR       |    |  /\\   |/  .--   --+-- .  . .--  .-- .  . .-- |/  .  . .-.\n");
		write_user(u,"\07~CR       \\ /\\ / /~~\\  |\\  |-      |   |--| |-   |-  |  | |   |\\  |  | |-'\n");
		write_user(u,"\07~CR        `  ' '    ` ` ` `--     `   `  ' `--  `   `--' `-- ` ` `--' `\n");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FG If your trying to wake yourself aren't you already awake?\n");
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
		return;
	}
	if (is_enemy(u,user))
	{
		write_user(user,"~FG You can't wake up someone who has made you an enemy.\n");
		return;
	}
	if (is_enemy(user,u))
	{
		write_user(user,"~FG You can't wake up someone who is your enemy.\n");
		return;
	}
	writeus(u,"~FR%s ~FGwants you to~CB:\n",name);
	write_user(u,"~CRWW             WW                         UU    UU         !!!\n");
	write_user(u,"~CR WW     W     WW  AAA     KK  KK EEEEEE   UU    UU PPPPPP  !!!\n");
	write_user(u,"~CR  WW   WWW   WW  AA AA    KK KK  EE       UU    UU PP   PP !!!\n");
	write_user(u,"~CR   WW WW WW WW  AA   AA   KKKK   EEEE     UU    UU PPPPPP  !!!\n");
	write_user(u,"~CR    WWW   WWW  AAAAAAAAA  KK KK  EE       UUU  UUU PP      ...\n");
	write_user(u,"~CR\07     W     W  AA       AA KK  KK EEEEEE   UUUUUUUU P       !!!\n");
	writeus(user,"~FMYou yell at ~FR%s~FM to wake up!\n",u->recap);
}

/* set user visibility vis/invis/hidden */
void visibility(User user,char *inpstr,int type)
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
	switch(type)
	{
		case 0:
		  switch(user->invis)
		  {
			case 0:
			  write_user(user,"~FG You are already visible.\n");
			  break;
			case 1: 
			case 2:
			  write_user(user,"~CW-~RS You are no longer invisible.\n");
			  if (user->invis == Hidden) strcpy(user->recap,user->temp_recap);
			  user->temp_recap[0] = '\0';
			  WriteRoomExcept(user->room,user,"~FY%s ~RSappears suddenly, seemingly out of nowhere...\n",user->recap);
			  user->invis = 0;
			  break;
			case 3:
			  logging_in = 1;
			  write_user(user,"~FG You are no longer fake logged off.\n");
			  strcpy(user->recap,user->temp_recap);
			  user->temp_recap[0] = '\0';
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FY %s~FM%s~CB <=-\n",login_vis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s~FY %s~FM %s~CB <=-\n",login_vis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s~CB <=-\n",login_vis,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s~FM %s~CB <=-\n",login_vis,user->recap,user->desc);
			  }
			  write_room_except(NULL,user,text);
			  if (user->desc[0] == '\'') sprintf(text,"~FT%s ~FY%s~RS%s\n",long_date(0),user->recap,user->desc);
			  else sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
			  RecordLogin(text);
			  WriteLevel(WIZ,user,"~FT    Connecting from~CW: ~CT%s~CW:~CM%d\n",user->site,user->site_port);
			  write_level(WIZ,user,text);
			  friend_check(user,1);
			  wait_check(user);
			  write_log(1,LOG2,"[LOGIN] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->site,user->site_port,user->port);
			  user->room=room_first;
			  SaveUser(user,1);
		  	  reset_junk(user);
			  user->last_login=time(0);
			  user->invis=0;
			  logging_in=0;
			  break;
		  }
		  break;
		case 1:
		  if (user->invis)
		  {
			write_user(user,"~FG You are already invisible!\n");
			return;
		  }
		  write_user(user,"~CW-~RS A mysterious fog envelopes you, making you invisible to others...\n");
		  sprintf(text,"~CT%s~RS is blanketed by a mysterious fog...\n",user->recap);
		  write_room_except(user->room,user,text);
		  user->invis = Invis;
		  break;
		case 2:
		  if (user->invis > Hidden)
		  {
			write_user(user,"~FR You are currently fake logged off...hiding from EVERYONE!\n");
			return;
		  }
		  if (user->invis == Hidden)
		  {
			write_user(user,"~FY Ok, you are no longer hidden.\n");
			strcpy(user->recap,user->temp_recap);
			user->temp_recap[0] = '\0';
			user->invis = 0;
			return;
		  }
		  write_user(user,"~FR Ok, you are now hidden from everyone.\n");
		  strcpy(user->temp_recap,user->recap);
		  strcpy(user->recap,"~FM[~FG?~FM]");
		  user->invis=Hidden;
		  break;
	}
	return;
}

/* Shows a users stats */
void stats(User user,char *inpstr,int rt)
{
	User	u;
	char	inv_room[RoomNameLen+1],lev_name[20],email[ARR_SIZE],
		*name,icqstr[ARR_SIZE],*uname;
	int	on = 0,earning = 0,err = 0;

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
		u = user;
		on = 1;
	}
	else
	{
		if ((uname = get_user_full(user,word[1])) == NULL)
		{
			write_user(user,center(nosuchuser,80));
			return;
		}
		if ((u = get_user(uname)) == NULL)
		{
			if ((u = create_user()) == NULL)
			{
				write_user(user,"~CW-~FT An error occurred, try again in a second.\n");
				write_log(1,LOG1,"[ERROR] - Unable to create temporary user object in stats()\n");
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
				write_user(user,"~CW-~FT That user's userfile seems to be corrupted..\n");
				destruct_user(u);
				destructed = 0;
				return;
			}
			on = 0;
		}
		else on = 1;
	}
	if (on == 0)
	{
		name = u->recap;
		earning = 0;
	}
	else
	{
		switch(u->invis)
		{
			case 0:
			case 1:
			  name = u->recap; earning=u->earning;
			  break;
			case 2:
			case 3:
			  name = u->temp_recap;
			  earning = 0;
			  break;
			default:
			  name = u->recap;
			  earning=0;
		}
	}
	if (user->ansi_on)
	{
		write_user(user,"~FTษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n");
		if (on == 0 || u->invis >= Hidden)
		{
			sprintf(text,"~FRฤฤฤฤฏ> ~FMCurrently ~CRoffline~CB: [~RS%s ~RS%s%s~RS%s ~CB] ~FR<ฎฤฤฤฤ\n",u->predesc[0]?u->predesc:"",name,u->desc[0] == '\''?"":" ",u->desc);
			write_user(user,center(text,80));
		}
		else
		{
			sprintf(text,"~FRฤฤฤฤฏ> ~FMCurrently ~CGonline~CB: [~RS%s ~RS%s%s~RS%s ~CB]~FR <ฎฤฤฤฤด\n",u->predesc[0]?u->predesc:"",name,u->desc[0] == '\''?"":" ",u->desc);
			write_user(user,center(text,80));
		}
		write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n");
	}
	else
	{
		write_user(user,ascii_tline);
		if (on == 0 || u->invis>Invis)
		{
			sprintf(text,"~FB--~CB==~FT> ~FMCurrently ~CRoffline~CB: [~RS%s ~RS%s%s~RS%s ~CB]~FT <~CB==~FB--\n",u->predesc[0]?u->predesc:"",name,u->desc[0] == '\''?"":" ",u->desc);
			write_user(user,center(text,80));
		}
		else
		{
			sprintf(text,"~FB--~CB==~FT> ~FMCurrently ~CGonline~CB: [~RS%s ~RS%s%s~RS%s ~CB]~FT <~CB==~FB--\n",u->predesc[0]?u->predesc:"",name,u->desc[0] == '\''?"":" ",u->desc);
			write_user(user,center(text,80));
		}
		write_user(user,ascii_bline);
	}
	strcpy(lev_name,user->gender==Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
	if (u->status & Jailed) strcpy(lev_name,"ARRESTED");

	if (u->hide_email == 1 && (user->level<WIZ && u != user)) sprintf(email,"~CB[~FMHIDDEN~CB]");
	else if (u->hide_email && (user->level >= WIZ || u == user)) sprintf(email,"%s ~CB[~FMH~CB]",u->email);
	else sprintf(email,"%s",u->email);

	if (u->icqhide && (user->level<WIZ && u != user)) sprintf(icqstr,"~CB[~FMHIDDEN~CB]");
	else if (u->icqhide && (user->level>=WIZ || u==user)) sprintf(icqstr,"%ld ~CB[~FMH~CB]",u->icq);
	else sprintf(icqstr,"%ld",u->icq);

	if (u->invite_room == NULL) strcpy(inv_room,"[ Nowhere ]");
	else strcpy(inv_room,u->invite_room->name);
	writeus(user," ~FGAge~CB              : ~FY%2d      ~FR-==<>==-"
		     " ~FGGender~CB           : ~FY%s\n",u->age,gendert[u->gender]);
	writeus(user," ~FGBirthday~CB         : ~FY%-5.5s   ~FR-==<>==-"
		     " ~FGICQ Number~CB       : ~FY%s\n",u->bday,icqstr);
	writeus(user," ~FGAllowing fights~CB  : ~FY%-3.3s     ~FR-==<>==-"
		     " ~FGAllowing follows~CB : ~FY%s\n",yesno1[u->no_fight],yesno1[u->no_follow]);
	writeus(user," ~FGEarning money~CB    : ~FY%-3.3s     ~FR-==<>==-"
		     " ~FGUnread s-mail~CB    : ~FY%s\n",noyes1[earning],noyes2[has_unread_mail(u)]);
	writeus(user," ~FGAmount of chips~CB  : ~FT%6ld~FR  -==<>==-"
		     " ~FGIn the bank~CB      : ~FT%ld\n",u->chips,u->dep_chips);
	writeus(user," ~FGAmount of money~CB  : ~FT%6ld~FR  -==<>==-"
		     " ~FGIn the bank~CB      : ~FT%ld\n",u->money,u->dep_money);
	writeus(user," ~FGTotal logins~CB     : ~FY%6d~FR  -==<>==-"
		     " ~FGLevel~CB            : ~FY%s\n"
		     " ~FGInvited to~CB       : ~FY%s\n",u->logons,lev_name,inv_room);
	writeus(user," ~FGEmail address~CB    : ~FY%s\n",email);
	writeus(user," ~FGHomepage URL~CB     : ~FY%s\n",u->webpage);
	if (u == user || user->level>=WIZ)
	{
		if (user->ansi_on)
		{
			write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n");
			write_user(user,"~FTบ                     ~FR ฤฤฤฤฏฏ> ~FYUser only stats ~FR<ฎฎฤฤฤฤ                         ~FTบ\n");
			write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n");
		}
		else
		{
			write_user(user,ascii_tline);
			write_user(user,center("~FT--~CT==~CB> ~FYUser only stats ~CB<~CT==~FT--\n",80));
			write_user(user,ascii_bline);
		}
		writeus(user," ~FGColour~CB           : ~FY%-3.3s     ~FR-==<>==-"
			     " ~FGCharacter echo~CB   : ~FY%s\n",offon[u->colour],offon[u->char_echo]);
		writeus(user," ~FGIgnoring all~CB     : ~FY%-3.3s     ~FR-==<>==-"
			     " ~FGArrested~CB         : ~FY%s\n",noyes2[u->ignall],(u->status & Jailed)?"YES":"NO ");
		writeus(user," ~FGMuzzled~CB          : ~FY%-3.3s     ~FR-==<>==-"
			     " ~FGGagged~CB           : ~FY%s\n",(u->status & Muzzled)?"YES":"NO",(u->status & Gagged)?"YES":"NO");
		writeus(user," ~FGShouts Gagged~CB    : ~FY%-3.3s     ~FR-==<>==-"
			     " ~FGSocials Gagged~CB   : ~FY%s\n",noyes2[u->gshout],noyes2[u->gsocs]);
		writeus(user," ~FGTells Gagged~CB     : ~FY%-3.3s     ~FR-==<>==-"
			     " ~FGAnsi Screens~CB     : ~FY%s\n",noyes2[u->gpriv],offon[u->ansi_on]);
		writeus(user," ~FGIgnoring~CB         : ~FY%s  %s  %s  %s  %s\n",
			(u->ignore & Tells) ? "Tells":"     ",
			(u->ignore & Shouts) ? "Shouts":"      ",
			(u->ignore & Friends) ? "Friends":"       ",
			(u->ignore & Pictures) ? "Pics ":"     ",
			(u->ignore & Channels) ? "Channels":"");
		if ((u->ignore & Atmos) || (u->ignore & Logons)
		  || (u->ignore & Figlets) || (u->ignore & Wizard)
		  || (u->ignore & Swears))
		{
			writeus(user," ~FG        ~CB         : ~FY%s  %s  %s  %s  %s\n",
				(u->ignore & Atmos) ? "Atmos":"     ",
				(u->ignore & Logons) ? "Logons":"      ",
				(u->ignore & Figlets) ? "Figlets":"       ",
				(u->ignore & Wizard) ? "Wiz  ":"     ",
				(u->ignore & Swears) ? "Swearing":"");

		}
		if ((u->ignore & Beeps) || (u->ignore & Blinks)
		  || (u->ignore & Execall) || (u->ignore & Alert)
		  || (u->ignore & Quotd))
		{
			writeus(user," ~FG        ~CB         : ~FY%s  %s  %s  %s  %s\n",
				(u->ignore & Beeps) ? "Beeps":"     ",
				(u->ignore & Blinks) ? "Blinks":"      ",
				(u->ignore & Execall) ? "Execall":"       ",
				(u->ignore & Alert) ? "Alert":"     ",
				(u->ignore & Quotd) ? "Quotd":"");

		}
		if (u != user)
		{
			writeus(user," ~FGSilenced~CB         : ~FY%s\n",(u->status & Silenced)?"YES":"NO");
		}
		if (on == 0) 
			sprintf(text," ~FGLast site~CB        : ~CR%s\n",u->last_site);
		else
			sprintf(text," ~FGCurrent Site~CB     : ~CR%s\n",u->site);
		write_user(user,text);

	}
	if (user->ansi_on) write_user(user,"~FTศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n");
	else write_user(user,ascii_bline);
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
}

/* lets a user examine another user */
void examine(User user,char *inpstr,int wrap)
{
	User	u,u2;
	FILE	*fp;
	char	filename[FSL],lev_name[20];
	char	*name,line[ARR_SIZE+2],icqstr[70],*uname;
	int	new_mail,days,hours,mins,timelen,days2,hours2,
		mins2,days3,hours3,mins3,idle,err, in_html = 0;

	new_mail = days = hours = mins = timelen = days2 = hours2 = 0;
	mins2 = days3 = hours3 = idle = err = 0;
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (word_count < 2) strcpy(word[1],user->name);
	if (wrap == 0)
	{
		if ((uname = get_user_full(user,word[1])) == NULL)
		{
			write_user(user,center(nosuchuser,80));
			return;
		}
		if ((u = get_user(uname)) == NULL)
		{
			if ((u = create_user()) == NULL)
			{
				write_user(user,"~CW-~FT An error occurred, please try again in a few minutes.\n");
				write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in examine()");
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
				write_user(user,"~CW-~FT That user's userfile seems to be corrupted..\n");
				destruct_user(u);
				destructed = 0;
				return;
			}
			u2 = NULL;
		}
		else u2=u;
	}
	if (wrap)
	{
		if ((u = get_user(user->check)) == NULL)
		{
			if ((u = create_user()) == NULL)
			{
				write_user(user,"~CW-~FG An error occurred unable to finish examination.\n");
				write_log(0,LOG1,"[ERROR] - Unable to create temp. user object in examine()");
				return;
			}
			err = LoadUser(u,user->check);
			if (err == 0)
			{
				write_user(user,center(nosuchuser,80));
				destruct_user(u);
				destructed = 0;
				return;
			}
			else if (err == -1)
			{
				write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",user->check);
				write_user(user,"~CW-~FT That users userfile seems to be corrupted..\n");
				destruct_user(u);
				destructed = 0;
				return;
			}
			u2 = NULL;
		}
		else u2 = u;
	}
	days	= u->total_login/86400;
	hours	= (u->total_login%86400)/3600;
	mins	= (u->total_login%3600)/60;
	timelen	= (int)(time(0)-u->last_login);
	days2	= timelen/86400;
	hours2	= (timelen%86400)/3600;
	mins2	= (timelen%3600)/60;

	strcpy(lev_name,u->gender==Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
	if (u->lev_rank[0]) strcpy(lev_name,u->lev_rank);
	if (u->status & Jailed) strcpy(lev_name,"ARRESTED");

	if (u->icqhide && (user->level<WIZ && u!=user)) sprintf(icqstr,"~CB[~FMHIDDEN~CB]");
	else if (u->icqhide && (user->level>=WIZ || u==user)) sprintf(icqstr,"%ld ~CB[~FMH~CB]",u->icq);
	else sprintf(icqstr,"%ld",u->icq);

	if (has_unread_mail(u)) new_mail = 1;
	else new_mail = 0;
	switch(u->invis)
	{
		case 0:
		case 1:
		  name = u->recap;
		  break;
		case 2:
		case 3:
		  name = u->temp_recap;
		  break;
		default:
		  name = u->recap;
	}
	if (wrap == 0)
	{
		if (user->ansi_on)
		{
			write_user(user,"~FTษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n");
			if (u->predesc[0]) sprintf(text,"~FRฤฤฤฤฏ> ~FM%s ~FM%s%s~FG%s~FR <ฎฤฤฤฤ\n",u->predesc,name,u->desc[0] == '\''?"":" ",u->desc);
			else sprintf(text,"~FRฤฤฤฤฏ> ~FM%s%s~FG%s~FR <ฎฤฤฤฤ\n",name,u->desc[0] == '\''?"":" ",u->desc);
			write_user(user,center(text,80));
			write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n");
		}
		else
		{
			write_user(user,ascii_tline);
			if (u->predesc[0]) sprintf(text,"~FB--~CB==~FT> ~FM%s ~FM%s%s~RS%s~FT <~CB==~FB--\n",u->predesc,name,u->desc[0] == '\''?"":" ",u->desc);
			else sprintf(text,"~FT--~CT==~CB> ~FM%s%s~FG%s~FR ~CB<~CT==~FT--\n",name,u->desc[0] == '\''?"":" ",u->desc);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
	}
	if (u2 == NULL || ((u->invis > Invis) && u != user))
	{
		if (wrap == 0)
		{
			days3	= u->last_login_len/86400;
			hours3	= (u->last_login_len%86400)/3600;
			mins3	= (u->last_login_len%3600)/60;
			writeus(user,"   ~FGAge         ~CB : ~FT%2d~FR         -==<>==-~FG   Been examined~CB  : ~FT%d ~FYtime%s\n",u->age,u->examined,u->examined>1?"s":"");
			writeus(user,"   ~FGGender      ~CB : ~FT%-8.8s~FR   -==<>==-   ~FGMarried to~CB     : ~FT%s\n",gendert[u->gender],u->married_to);
			writeus(user,"   ~FGLevel alias ~CB : ~FT%-8.8s~FR   -==<>==-   ~FGICQ Number ~CB    : ~FT%s\n",lev_name,icqstr);
			writeus(user,"   ~FGLast login  ~CB : ~FT%s",ctime((time_t *)&(u->last_login)));
			if (days2 == 0)
			{
				if (hours2 == 0) sprintf(text,"   ~FGWhich was   ~CB : ~FT%d~FY minute%s ago.\n",mins2,mins2>1?"s":"");
				else sprintf(text,"   ~FGWhich was   ~CB : ~FT%d ~FYhour%s, and ~FT%d~FY minute%s ago.\n",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"");
			}
			else sprintf(text,"   ~FGWhich was   ~CB : ~FT%d ~FYday%s, ~FT%d~FY hour%s, and ~FT%d ~FYminute%s ago.\n",days2,days2>1?"s":"",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"");
			write_user(user,text);
			if (days3 == 0)
			{
				if (hours3 == 0) sprintf(text,"   ~FGWas on for  ~CB : ~FT%d ~FYminute%s.\n",mins3,mins3>1?"s":"");
				else sprintf(text,"   ~FGWas on for  ~CB : ~FT%d ~FYhour%s, and ~FT%d~FY minute%s.\n",hours3,hours3>1?"s":"",mins3,mins3>1?"s":"");
			}
			else sprintf(text,"   ~FGWas on for  ~CB : ~FT%d ~FYday%s, ~FT%d ~FYhour%s, and ~FT%d~FY minute%s.\n",days3,days3>1?"s":"",hours3,hours3>1?"s":"",mins3,mins3>1?"s":"");
			write_user(user,text);
			if (days == 0)
			{
				if (hours == 0) sprintf(text,"   ~FGTotal login ~CB : ~FT%d ~FYminute%s.\n",mins,mins>1?"s":"");
				else sprintf(text,"   ~FGTotal login ~CB : ~FT%d ~FYhour%s, and ~FT%d ~FYminute%s.\n",hours,hours>1?"s":"",mins,mins>1?"s":"");
			}
			else sprintf(text,"   ~FGTotal login ~CB : ~FT%d ~FYday%s, ~FT%d ~FYhour%s, and ~FT%d~FY minute%s.\n",days,days>1?"s":"",hours,hours>1?"s":"",mins,mins>1?"s":"");
			write_user(user,text);
			if (user->level >= WIZ)
			{
				writeus(user,"   ~FGLast site   ~CB : ~FT%s\n",u->last_site);
			}
			if (new_mail)
			{
				writeus(user,"    ~FG%s ~FThas some mail that needs reading.\n",u->invis>Invis?u->temp_recap:u->recap);
			}
			if (user->profile)
			{
				if (user->ansi_on)
				{
					write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\n");
					write_user(user,"~FTบ      ~CRPress ~CW[~FTEnter~CW]~CR to see the users profile, or ~CW[~FTQ~CW]~CR + ~CW[~FTEnter~CW]~CR to exit~FT        บ\n");
					write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\n");
				}
				else
				{
					write_user(user,ascii_tline);
					write_user(user,center("~FR--==> ~CRPress ~CW[~FTEnter~CW]~CR to see the users profile, or ~CW[~FTQ~CW]~CR + ~CW[~FTEnter~CW]~CR to exit~FR <==--\n",80));
					write_user(user,ascii_bline);
				}
			}
			else
			{
				if (user->ansi_on) write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\n");
				else write_user(user,ascii_line);
			}
			u->examined++;
			if (u == NULL)
			{
				u->socket = -2;
				strcpy(u->site,u->last_site);
				SaveUser(u,0);
			}
			if (user->profile)
			{
				strcpy(user->check,u->name);
				user->misc_op=18;
				no_prompt=1;
			}
			else
			{
				strcpy(user->check,u->name);
				examine(user,NULL,1);
			}
			if (u->invis > Invis) return;
			else
			{
				destruct_user(u);
				destructed = 0;
				return;
			}
		}
		if (wrap)
		{
			sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,u->name);
			if ((fp = fopen(filename,"r")) == NULL)
				write_user(user,center("~CR Tell this user to use .entpro.\n",80));
			else
			{
				fgets(line,ARR_SIZE,fp);
				while(!feof(fp))
				{
					if (!strncasecmp(line,"[HTML]",6))
					{
						in_html = 1;
						fgets(line,ARR_SIZE,fp);
						continue;
					}
					if (!strncasecmp(line,"[/HTML]",7))
					{
						in_html = 0;
						fgets(line,ARR_SIZE,fp);
						continue;
					}
					if (in_html == 0) write_user(user,line);
					fgets(line,ARR_SIZE,fp);
				}
				FCLOSE(fp);
			}
			user->check[0]	= '\0';
			user->misc_op	= 0;
			if (u->invis > Invis) return;
			else
			{
				destruct_user(u);
				destructed = 0;
				return;
			}
		}
		return;
	}
	if (wrap == 0)
	{
		if (u != user && !u->ignall && user->invis < Invis)
		{
			writeus(u,"~FG %s ~FYis peeking at your profile.\n",user->recap);
		}
		idle = (int)(time(0)-u->last_input)/60;
		writeus(user,"  ~FG Age         ~CB : ~FT%2d~FR         -==<>==-~FG   Been examined~CB  : ~FT%d ~FYtime%s\n",u->age,u->examined,u->examined>1?"s":"");
		writeus(user,"  ~FG Gender      ~CB : ~FT%-8.8s~FR   -==<>==-   ~FGMarried to ~CB    : ~FT%s\n",gendert[u->gender],u->married_to);
		writeus(user,"  ~FG Level alias ~CB : ~FT%-8.8s~FR   -==<>==-   ~FGICQ Number ~CB    : ~FT%s\n",lev_name,icqstr);
		writeus(user,"  ~FG On since    ~CB : ~FT%s",ctime((time_t *)&u->last_login));
		if (days2 == 0)
		{
			if (hours2 == 0) sprintf(text,"   ~FGOn for      ~CB : ~FT%d ~FYminute%s.\n",mins2,mins2>1?"s":"");
			else sprintf(text,"   ~FGOn for      ~CB : ~FT%d ~FYhour%s, and ~FT%d ~FYminute%s.\n",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"");
		}
		else sprintf(text,"   ~FGOn for      ~CB : ~FT%d ~FYday%s, ~FT%d ~FYhour%s, and ~FT%d ~FYminute%s.\n",days2,days2>1?"s":"",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"");
		write_user(user,text);
		if (idle)
		{
			if (u->afk)
			{
				writeus(user,"   ~FGIdling for  ~CB : ~FT%d ~FYminute%s ~CB[~CRA~CW.~CRF~CW.~CRK~CB]\n",idle,idle>1?"s":"");
				if (u->afk_mesg[0] != '\0')
				{
					writeus(user,"   ~FGAFK Message ~CB : ~FT%s\n",u->afk_mesg);
				}
			}
			else
			{
				writeus(user,"   ~FGIdling for  ~CB : ~FT%d~FY minute%s.\n",idle,idle>1?"s":"");
			}
		}
		if (days == 0)
		{
			if (hours == 0) sprintf(text,"   ~FGTotal login ~CB : ~FT%d ~FYminute%s.\n",mins,mins>1?"s":"");
			else sprintf(text,"   ~FGTotal login ~CB : ~FT%d ~FYhour%s, and ~FT%d ~FYminute%s.\n",hours,hours>1?"s":"",mins,mins>1?"s":"");
		}
		else sprintf(text,"   ~FGTotal login ~CB : ~FT%d ~FYday%s, ~FT%d ~FYhour%s, and ~FT%d~FY minute%s.\n",days,days>1?"s":"",hours,hours>1?"s":"",mins,mins>1?"s":"");
		write_user(user,text);
		if ((user->level >= WIZ) || (u==user))
		{
			writeus(user,"   ~FGCurrent site~CB : ~FT%s\n",u->site);
			writeus(user,"   ~FGCurrent IP  ~CB : ~FT%-15.15s       ~FGPort~CB : ~FT%d\n",u->ip_site,u->site_port);
		}
		if (new_mail)
		{
			writeus(user,"   ~FG%s ~FThas some mail that needs reading.\n",u->invis>Invis?u->temp_recap:u->recap);
		}
		if (user->profile)
		{
			if (user->ansi_on)
			{
				write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\n");
				write_user(user,"~FTบ      ~CRPress ~CW[~FTEnter~CW]~CR to see the users profile, or ~CW[~FTQ~CW]~CR + ~CW[~FTEnter~CW]~CR to exit~FT        บ\n");
				write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\n");
			}
			else
			{
				write_user(user,ascii_tline);
				write_user(user,center("~FR--==> ~CRPress ~CW[~FTEnter~CW]~CR to see the users profile, or ~CW[~FTQ~CW]~CR + ~CW[~FTEnter~CW]~CR to exit~FR <==--\n",80));
				write_user(user,ascii_bline);
			}
		}
		else
		{
			if (user->ansi_on) write_user(user,"~FTฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน\n");
			else write_user(user,ascii_line);
		}
		u->examined++;
		if (user->profile)
		{
			strcpy(user->check,u->name);
			user->misc_op	= 18;
			no_prompt	= 1;
		}
		else
		{
			strcpy(user->check,u->name);
			examine(user,NULL,1);
		}
	}
	if (wrap)
	{
		sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,u->name);
		if ((fp = fopen(filename,"r")) == NULL)
		{
			write_user(user,center("~CR Tell this user to use .entpro.\n",80));
			return;
		}
		else
		{
			fgets(line,ARR_SIZE,fp);
			while(!feof(fp))
			{
				if (!strncasecmp(line,"[HTML]",6))
				{
					in_html = 1;
					fgets(line,ARR_SIZE,fp);
					continue;
				}
				if (!strncasecmp(line,"[/HTML]",7))
				{
					in_html = 0;
					fgets(line,ARR_SIZE,fp);
					continue;
				}
				if (in_html == 0) write_user(user,line);
				fgets(line,ARR_SIZE,fp);
			}
			FCLOSE(fp);
		}
		return;
	}
}

/* Toggle ignore all on/off */
void toggle_ignall(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname; else name = user->recap;
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->ignall == 0)
	{
		WriteRoomExcept(user->room,user,"~FG%s ~FRis to cool for everyone and starts ignoring everything.\n",name);
		write_user(user,"~CTYou are now ignoring everything.\n");
		user->ignall = 1;
		return;
	}
	WriteRoomExcept(user->room,user,"~FG%s ~FRdecides %s isn't that cool and starts listening again.\n",name,gender3[user->gender]);
	write_user(user,"~CTYou are no longer ignoring everything.\n");
	user->ignall = 0;
	user->ignore = 0;
}

/* lets a user go afk */
void afk(User user,char *inpstr,int rt)
{
	int i;

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
	i = ran((unsigned)NUM_AFK_MESGS);
	if (word_count>1)
	{
		if (!strncasecmp(word[1],"-lock",2))
		{
			inpstr = remove_first(inpstr);
			if (strlen(inpstr) > AfkMesgLen)
			{
				write_user(user,"~FR Sorry, that afk message is a tad bit too long!\n");
				return;
			}
			write_user(user,"~FGYou are now afk with your screen ~CRLOCKED~FG. Enter your password to unlock it!\n");
			if (inpstr[0]) strcpy(user->afk_mesg,inpstr);
			user->afk = 2;
			return;
		}
		else if (!strncasecmp(word[1],"-cls",2))
		{
			inpstr = remove_first(inpstr);
			if (strlen(inpstr) > AfkMesgLen)
			{
				write_user(user,"~FR Sorry, that afk message is a tad bit too long!\n");
				return;
			}
			write_user(user,"~CR Clearing your screen now!\n");
			cls(user,NULL,0);
			if (inpstr[0])
			{
				strcpy(user->afk_mesg,inpstr);
				write_user(user,"~FR Your afk message has been set!\n");
			}
		}
		else
		{
			if (strlen(inpstr) > AfkMesgLen)
			{
				write_user(user,"~FR Sorry, that afk message is a tad bit too long!\n");
				return;
			}
			if (inpstr[0]) strcpy(user->afk_mesg,inpstr);
		}
	}
	if (user->invis == 0)
	{
		if (user->afk_mesg[0]) sprintf(text," ~CB-- ~FM%s ~FY%s ~CB[~FRA~CW.~FRF~CW.~FRK~CB]\n",user->recap,user->afk_mesg);
		else sprintf(text,afk_text[i],user->recap);
		write_room(user->room,text);
		write_log(1,LOG1,"%s went AFK\n",user->name);
	}
	strcpy(user->temp_desc,user->desc);
	strcpy(user->desc,"~CW[~CRA~CW]~FTway ~CW[~CRF~CW]~FTrom ~CW[~CRK~CW]~FTeyboard");
	user->ignall_store = user->ignall;
	user->ignall = 1;
	user->afk = 1;
}

/* Clears the users screen */
void cls(User user,char *inpstr,int sock)
{
	int i;

	if (sock == 0 && user != NULL) sock = user->socket;
	else sock = -1;
	for (i = 0 ; i < 5 ; ++i)
	{
		if (user!=NULL) write_user(user,"\n\n\n\n\n\n\n\n\n\n\n");
		else write(sock,"\n\n\n\n\n\n\n\n\n\n\n",strlen("\n\n\n\n\n\n\n\n\n\n\n"));
	}
	if (sock != -1) write(sock,"\033[2J",strlen("\033[2J"));
}

/* Lets a user delete their account */
void suicide(User user,char *inpstr,int rt)
{

	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRyour password~CB]");
		return;
	}
	if (strcasecmp((char *)crypt(word[1],salt),user->pass))
	{
		write_user(user,"~FR You have entered the wrong password.\n");
		return;
	}
	write_user(user,"~CB~LI\07[~CRWARNING~CB] ~FMThis will delete your account.\n");
	write_user(user,"~FR Are you sure you want to do this? ~CB[~CRy~CW|~CRn~CB]:~RS ");
	user->misc_op = 13;
	no_prompt = 1;
}

/* Lets a user change their password */
void change_pass(User user,char *inpstr,int rt)
{
	User u;
	char *name,*uname;
	int err = 0;

	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 3)
	{
		if (user->level < GOD) writeus(user,usage,command[com_num].name,"~CB[~CRold password~CB] [~CRnew password~CB]");
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRold password~CB] [~CRnew password~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CRnew password~CB] [~CRnew password~CB] [~CRuser~CB]");
			return;
		}
	}
	if (strlen(word[2]) < 3)
	{
		write_user(user,"~FTWouldn't you want a password someone couldn't crack??\n");
		return;
	}
	if (strlen(word[2])>PassLen)
	{
		write_user(user,"~FTTry a password that you could remember eh :P\n");
		return;
	}
	/* Changing their own password */
	if (word_count == 3)
	{
		if (!strcasecmp(word[2],user->name))
		{
			write_user(user,"~FRYou cannot have the same password as your user name!\n");
			return;
		}
		if (strcmp((char *)crypt(word[1],salt),user->pass))
		{
			write_user(user,"~FR You have entered the wrong password.\n");
			return;
		}
		if (!strcmp(word[1],word[2]))
		{
			write_user(user,"~FR Those two passwords are the same...\n");
			return;
		}
		strcpy(user->pass,(char *)crypt(word[2],salt));
		SaveUser(user,0);
		cls(user,NULL,0);
		write_user(user,"~FY Your password has been successfully changed!\n");
		return;
	}
	/* Change someone elses */
	if (user->level < GOD)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRold password~CB] [~CRnew password~CB]");
		return;
	}
	word[3][0] = toupper(word[3][0]);
	if (!strcasecmp(word[3],user->name))
	{
		write_user(user,"~FG You cannot change your own password with the ~CRuser~FR option.\n");
		return;
	}
	if (strcasecmp(word[1],word[2]))
	{
		write_user(user,"~CW-~FT Passwords don't match.\n");
		return;
	}
	if ((uname = get_user_full(user,word[3])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u->level >= user->level)
		{
			write_user(user,"~FG You cannot change the password of a user of greater or equal level then your own.\n");
			return;
		}
		if (!strcasecmp(word[1],word[3]))
		{
			write_user(user,"~FG You cannot change the password to the users name!\n");
			return;
		}
		strcpy(u->pass,(char *)crypt(word[2],salt));
		cls(user,NULL,0);
		writeus(user,"~FM%s%s ~FYpassword has been successfully changed.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
		if (user->invis == Invis) name = invisname;
		else name = user->recap;
		writeus(u,"~FY Your password has been changed by ~FT%s~FY.\n",name);
		write_log(1,LOG4,"[PASSWORD] - User: [%s] Changed: [%s%s password]\n",user->name,u->name,u->name[strlen(u->name)-1] == 's'?"'":"'s");
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred.. Try again in a few minutes.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in change_pass().\n");
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
		write_user(user,"~CW-~FT That user's userfile seems to be corrupted..\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->level >= user->level)
	{
		write_user(user,"~FG You cannot change the password of a user of greater or equal level then your own!\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	strcpy(u->site,u->last_site);
	strcpy(u->pass,(char *)crypt(word[2],salt));
	SaveUser(u,1);
	cls(user,NULL,0);
	writeus(user,"~FM%s%s ~FYpassword has been successfully changed.\n",uname,uname[strlen(uname)-1] == 's'?"'":"'s");
	write_log(1,LOG1,"[PASSWORD] - User: [%s] Changed: [%s%s password]\n",user->name,uname,uname[strlen(uname)-1] == 's'?"'":"'s");
	destruct_user(u);
	destructed = 0;
}

/* Lets a user try and ask for forgiveness */
void beg_wizzes(User user,char *inpstr,int rt)
{

	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRwhy you should be forgiven.~CB]");
		return;
	}
	if (user->status == 0)
	{
		write_user(user,"~FR You don't have to be forgiven for anything.\n");
		return;
	}
	WriteLevel(WIZ,user,"~CB [~FG%s~FT begs~CB]:~FM %s\n",user->recap,inpstr);
	write_user(user," ~FMYou beg the wizzes for forgiveness.\n");
}

/* Lets a user start listening to things again */
void listen_up(User user,char *inpstr,int rt)
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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (!strncasecmp(word[1],"all",2) && (user->ignore || user->ignall))
	{
		write_user(user,"~CG You decide to listen again!\n");
		user->ignore = 0;
		user->ignall = 0;
		return;
	}
	if (!strncasecmp(word[1],"all",2))
	{
		write_user(user,"~FR You aren't currently ignoring anything.\n");
		return;
	}
	if (!strncasecmp(word[1],"tells",2) && (user->ignore & Tells))
	{
		write_user(user,"~FG Ok, you start listening to private messages again!\n");
		user->ignore -= Tells;
		return;
	}
	if (!strncasecmp(word[1],"tells",2))
	{
		write_user(user,"~FY You aren't currently ignoring private messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"shouts",2) && (user->ignore & Shouts))
	{
		write_user(user,"~FG Ok, you start listening to shouted messages again!\n");
		user->ignore -= Shouts;
		return;
	}
	if (!strncasecmp(word[1],"shouts",2))
	{
		write_user(user,"~FY You aren't currently ignoring shouts.\n");
		return;
	}
	if (!strncasecmp(word[1],"logons",2) && (user->ignore & Logons))
	{
		write_user(user,"~FG Ok, you start listening to login messages again.\n");
		user->ignore -= Logons;
		return;
	}
	if (!strncasecmp(word[1],"logons",2))
	{
		write_user(user,"~FY You aren't currently ignoring shouts.\n");
		return;
	}
	if (!strncasecmp(word[1],"beeps",2) && (user->ignore & Beeps))
	{
		write_user(user,"~FG Ok, you start listening to beeping messages again.\n");
		user->ignore -= Beeps;
		return;
	}
	if (!strncasecmp(word[1],"beeps",2))
	{
		write_user(user,"~FY You aren't currently ignoring beeps.\n");
		return;
	}
	if (!strncasecmp(word[1],"pics",2) && (user->ignore & Pictures))
	{
		write_user(user,"~FG Ok, you start listening to pictures.\n");
		user->ignore -= Pictures;
		return;
	}
	if (!strncasecmp(word[1],"pics",2))
	{
		write_user(user,"~FY You aren't currently ignoring pictures.\n");
		return;
	}
	if (!strncasecmp(word[1],"wiz",2) && (user->ignore & Wizard))
	{
		write_user(user,"~FG Ok, you start listening to wiz messages again.\n");
		user->ignore -= Wizard;
		return;
	}
	if (!strncasecmp(word[1],"wiz",2))
	{
		write_user(user,"~FY You aren't currently ignoring wiz messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"atmos",2) && (user->ignore & Atmos))
	{
		write_user(user,"~FG Ok, you start listening to atmospherics again.\n");
		user->ignore -= Atmos;
		return;
	}
	if (!strncasecmp(word[1],"atmos",2))
	{
		write_user(user,"~FY You aren't currently ignoring atmospheres.\n");
		return;
	}
	if (!strncasecmp(word[1],"friends",2) && (user->ignore & Friends))
	{
		write_user(user,"~FG Ok, you start listening to friend messages again.\n");
		user->ignore -= Friends;
		return;
	}
	if (!strncasecmp(word[1],"friends",2))
	{
		write_user(user,"~FY You aren't currently ignoring your friends.\n");
		return;
	}
	if (!strncasecmp(word[1],"channel",2) && (user->ignore & Channels))
	{
		write_user(user,"~FG Ok, you start listening to the users channel again.\n");
		user->ignore -= Channels;
		return;
	}
	if (!strncasecmp(word[1],"channel",2))
	{
		write_user(user,"~FY You aren't currently ignoring the users channel.\n");
		return;
	}
	if (!strncasecmp(word[1],"swears",2) && (user->ignore & Swears))
	{
		write_user(user,"~FG Ok, you are listening to swear words again.\n");
		user->ignore -= Swears;
		return;
	}
	if (!strncasecmp(word[1],"swears",2))
	{
		write_user(user,"~FG You currently aren't ignoring swear words.\n");
		return;
	}
	if (!strncasecmp(word[1],"blinkies",2) && (user->ignore & Blinks))
	{
		write_user(user,"~FG Ok, you will now see those annoying blinkies.\n");
		user->ignore -= Blinks;
		return;
	}
	if (!strncasecmp(word[1],"blinkies",2))
	{
		write_user(user,"~FG You currently aren't ignoring those annoying blinkies.\n");
		return;
	}
	if (!strncasecmp(word[1],"figlets",2) && (user->ignore & Figlets))
	{
		write_user(user,"~FG Ok, you will now see figlet messages.\n");
		user->ignore -= Figlets;
		return;
	}
	if (!strncasecmp(word[1],"figlets",2))
	{
		write_user(user,"~FG You currently aren't ignoring figlets.\n");
		return;
	}
	if (!strncasecmp(word[1],"alert",3) && (user->ignore & Alert))
	{
		write_user(user,"~FG Ok, you will now be alerted when your friends log on/off.\n");
		user->ignore -= Alert;
		return;
	}
	if (!strncasecmp(word[1],"alert",3))
	{
		write_user(user,"~FG You currently aren't ignoring friend alerts..\n");
		return;
	}
	if (!strncasecmp(word[1],"execall",2) && (user->ignore & Execall))
	{
		write_user(user,"~FG Ok, you will now participate when someone uses .execall.\n");
		user->ignore -= Execall;
		return;
	}
	if (!strncasecmp(word[1],"execall",2))
	{
		write_user(user,"~FG You currently are participating when someone uses .execall.\n");
		return;
	}
	if (!strncasecmp(word[1],"quotd",2) && (user->ignore & Quotd))
	{
		write_user(user,"~FG Ok, you will now see the quote of the day upon logging in.\n");
		user->ignore -= Quotd;
		return;
	}
	if (!strncasecmp(word[1],"quotd",2))
	{
		write_user(user,"~FG You currently aren't ignoring the quote of the day.\n");
		return;
	}
	if (user->level >= WIZ)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRtells~CW|~CRshouts~CW|~CRlogons~CW|~CRbeeps~CW|~CRchannel~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRpics~CW|~CRwiz~CW|~CRatmos~CW|~CRfriends~CW|~CRswears~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRfiglets~CW|~CRblinkies~CW|~CRalert~CW|~CRexecall~CW|~CRquotd~CB]");
	}
	else
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRtells~CW|~CRshouts~CW|~CRlogons~CW|~CRbeeps~CW|~CRchannel~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRpics~CW|~CRatmos~CW|~CRfriends~CW|~CRswears~CW|~CRquotd~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRfiglets~CW|~CRblinkies~CW|~CRalert~CW|~CRexecall~CB]");
	}
}

/* Gotta be able to ignore everything too! */
void ignore(User user,char *inpstr,int rt)
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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		if (user->level >= WIZ)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRtells~CW|~CRshouts~CW|~CRlogons~CW|~CRbeeps~CW|~CRchannel~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CRpics~CW|~CRwiz~CW|~CRatmos~CW|~CRfriends~CW|~CRswears~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CRfiglets~CW|~CRblinkies~CW|~CRalert~CW|~CRexecall~CW|~CRquotd~CB]");
		}
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRtells~CW|~CRshouts~CW|~CRlogons~CW|~CRbeeps~CW|~CRchannel~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CRpics~CW|~CRatmos~CW|~CRfriends~CW|~CRswears~CW|~CRquotd~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CRfiglets~CW|~CRblinkies~CW|~CRalert~CW|~CRexecall~CB]");
		}
		writeus(user,"~FR  Ignoring private messages ~CB[~FM%s~CB]\n",(user->ignore & Tells)?"YES":"NO ");
		writeus(user,"~FR  Ignoring shouted messages ~CB[~FM%s~CB]\n",(user->ignore & Shouts)?"YES":"NO ");
		writeus(user,"~FR  Ignoring logons           ~CB[~FM%s~CB]\n",(user->ignore & Logons)?"YES":"NO ");
		writeus(user,"~FR  Ignoring beeps            ~CB[~FM%s~CB]\n",(user->ignore & Beeps)?"YES":"NO ");
		writeus(user,"~FR  Ignoring pictures         ~CB[~FM%s~CB]\n",(user->ignore & Pictures)?"YES":"NO ");
		writeus(user,"~FR  Ignoring atmospherics     ~CB[~FM%s~CB]\n",(user->ignore & Atmos)?"YES":"NO ");
		writeus(user,"~FR  Ignoring friend messages  ~CB[~FM%s~CB]\n",(user->ignore & Friends)?"YES":"NO ");
		writeus(user,"~FR  Ignoring channel messages ~CB[~FM%s~CB]\n",(user->ignore & Channels)?"YES":"NO ");
		writeus(user,"~FR  Ignoring swear words      ~CB[~FM%s~CB]\n",(user->ignore & Swears)?"YES":"NO ");
		writeus(user,"~FR  Ignoring figlets          ~CB[~FM%s~CB]\n",(user->ignore & Figlets)?"YES":"NO ");
		writeus(user,"~FR  Ignoring blinkies         ~CB[~FM%s~CB]\n",(user->ignore & Blinks)?"YES":"NO ");
		writeus(user,"~FR  Ignoring alert messages   ~CB[~FM%s~CB]\n",(user->ignore & Alert)?"YES":"NO ");
		writeus(user,"~FR  Ignoring execall commands ~CB[~FM%s~CB]\n",(user->ignore & Execall)?"YES":"NO ");
		writeus(user,"~FR  Ignoring quote of the day ~CB[~FM%s~CB]\n",(user->ignore & Quotd)?"YES":"NO ");
		if (user->level >= WIZ) writeus(user,"~FR  Ignoring wizard messages  ~CB[~FM%s~CB]\n",(user->ignore & Wizard)?"YES":"NO ");
		return;
	}
	if (!strncasecmp(word[1],"tells",2) && (user->ignore & Tells))
	{
		write_user(user,"~FG You are already ignoring private messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"tells",2))
	{
		write_user(user,"~FR Ok, you are now ignoring private messages.\n");
		user->ignore += Tells;
		return;
	}
	if (!strncasecmp(word[1],"shouts",2) && (user->ignore & Shouts))
	{
		write_user(user,"~FG You are already ignoring shout messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"shouts",2))
	{
		write_user(user,"~FR Ok, you are now ignoring shouted messages.\n");
		user->ignore += Shouts;
		return;
	}
	if (!strncasecmp(word[1],"logons",2) && (user->ignore & Logons))
	{
		write_user(user,"~FG You are already ignoring logons.\n");
		return;
	}
	if (!strncasecmp(word[1],"logons",2))
	{
		write_user(user,"~FR Ok, you are now ignoring logons.\n");
		user->ignore += Logons;
		return;
	}
	if (!strncasecmp(word[1],"beeps",2) && (user->ignore & Beeps))
	{
		write_user(user,"~FG You are already ignoring beeping messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"beeps",2))
	{
		write_user(user,"~FR Ok, you are now ignoring beeps.\n");
		user->ignore += Beeps;
		return;
	}
	if (!strncasecmp(word[1],"pics",2) && (user->ignore & Pictures))
	{
		write_user(user,"~FG You are already ignoring pictures.\n");
		return;
	}
	if (!strncasecmp(word[1],"pics",2))
	{
		write_user(user,"~FR Ok, you are now ignoring pictures.\n");
		user->ignore += Pictures;
		return;
	}
	if (!strncasecmp(word[1],"wiz",2) && (user->ignore & Wizard))
	{
		write_user(user,"~FG You are already ignoring wiz messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"wiz",2))
	{
		write_user(user,"~FR Ok, you are now ignoring wiz messages.\n");
		user->ignore += Wizard;
		return;
	}
	if (!strncasecmp(word[1],"atmos",2) && (user->ignore & Atmos))
	{
		write_user(user,"~FG You are already ignoring atmospherics.\n");
		return;
	}
	if (!strncasecmp(word[1],"atmos",2))
	{
		write_user(user,"~FR Ok, you are now ignoring atmospheres.\n");
		user->ignore += Atmos;
		return;
	}
	if (!strncasecmp(word[1],"friends",2) && (user->ignore & Friends))
	{
		write_user(user,"~FG You are already ignoring your friends.\n");
		return;
	}
	if (!strncasecmp(word[1],"friends",2))
	{
		write_user(user,"~FR Ok, you are now ignoring your friends.\n");
		user->ignore += Friends;
		return;
	}
	if (!strncasecmp(word[1],"channel",2) && (user->ignore & Channels))
	{
		write_user(user,"~FG You are already ignoring channel messages.\n");
		return;
	}
	if (!strncasecmp(word[1],"channel",2))
	{
		write_user(user,"~FR Ok, you are now ignoring the users channel.\n");
		user->ignore += Channels;
		return;
	}
	if (!strncasecmp(word[1],"swears",2) && (user->ignore & Swears))
	{
		write_user(user,"~FG You are already ignoring swear words.\n");
		return;
	}
	if (!strncasecmp(word[1],"swears",2))
	{
		write_user(user,"~FR Ok, you are now ignoring swear words.\n");
		user->ignore += Swears;
		return;
	}
	if (!strncasecmp(word[1],"figlets",2) && (user->ignore & Figlets))
	{
		write_user(user,"~FG You are already ignoring figlets.\n");
		return;
	}
	if (!strncasecmp(word[1],"figlets",2))
	{
		write_user(user,"~FR Ok, you are now ignoring figlet messages.\n");
		user->ignore += Figlets;
		return;
	}
	if (!strncasecmp(word[1],"blinkies",2) && (user->ignore & Blinks))
	{
		write_user(user,"~FG You are already ignoring blinkies.\n");
		return;
	}
	if (!strncasecmp(word[1],"blinkies",2))
	{
		write_user(user,"~FR Ok, you are now ignoring annoying blinks.\n");
		user->ignore += Blinks;
		return;
	}
	if (!strncasecmp(word[1],"alert",2) && (user->ignore & Alert))
	{
		write_user(user,"~FG You are already ignoring friend alerts..\n");
		return;
	}
	if (!strncasecmp(word[1],"alert",2))
	{
		write_user(user,"~FR Ok, you are now ignoring alerts when your friends log on/off.\n");
		user->ignore += Alert;
		return;
	}
	if (!strncasecmp(word[1],"execall",2) && (user->ignore & Execall))
	{
		write_user(user,"~FG You are already ignoring when people use .execall\n");
		return;
	}
	if (!strncasecmp(word[1],"execall",2))
	{
		write_user(user,"~FR Ok, you will no longer participate when people use .execall\n");
		user->ignore += Execall;
		return;
	}
	if (!strncasecmp(word[1],"quotd",2) && (user->ignore & Quotd))
	{
		write_user(user,"~FG You are already ignoring the quote of the day.\n");
		return;
	}
	if (!strncasecmp(word[1],"quotd",2))
	{
		write_user(user,"~FR Ok, you will no longer see the quote of the day when you login.\n");
		user->ignore += Quotd;
		return;
	}
	if (user->level >= WIZ)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRtells~CW|~CRshouts~CW|~CRlogons~CW|~CRbeeps~CW|~CRchannel~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRpics~CW|~CRwiz~CW|~CRatmos~CW|~CRfriends~CW|~CRswears~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRfiglets~CW|~CRblinkies~CW|~CRalert~CW|~CRexecall~CW|~CRquotd~CB]");
	}
	else
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRtells~CW|~CRshouts~CW|~CRlogons~CW|~CRbeeps~CW|~CRchannel~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRpics~CW|~CRatmos~CW|~CRfriends~CW|~CRswears~CW|~CRquotd~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRfiglets~CW|~CRblinkies~CW|~CRalert~CW|~CRexecall~CB]");
	}
}

/* Lets a user call out for help if they need it */
void helpme(User user,char *inpstr,int rt)
{
	User u;
	int cnt = 0;

	if (user->level >= WIZ)
	{
		write_user(user,"~FR Why does a staff member need help?!?\n");
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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	for_users(u)
	{
		if (u->tpromoted && u->orig_lev<WIZ) continue;
		if (u->level<WIZ) continue;
		cnt++;
	}
	if (cnt == 0)
	{
		write_user(user,"~FR There are currently no staff members online..sorry!\n");
		return;
	}
	WriteLevel(WIZ,user,"\07~CB[~FTW~CB][~FMHelp Request~CB] - ~FT%s ~FGrequests some help.\n",user->recap);
	write_user(user,"~CW- ~FTYour help call has been sent, help is on the way :-)\n");
}

/* Lets a user decide if they want to be followed or not! */
void nofollow(User user,char *inpstr,int rt)
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
	if (user->no_follow)
	{
		write_user(user,"~FG Ok, you are allowing people to follow you again.\n");
		user->no_follow = 0;
		return;
	}
	if (user->follow_by[0] != '\0')
	{
		if ((u = get_user(user->follow_by)) == NULL)
		{
			write_user(user,"~FR Ok, you are no longer allowing people to follow you.\n");
			user->follow_by[0] = '\0';
			user->no_follow = 1;
			return;
		}
		writeus(u,"~FT%s~FM did not want to be followed anymore..so you are no longer following them.\n",user->recap);
		writeus(user,"~FG %s~FR is no longer following you.\n",u->recap);
		write_user(user,"~FR Ok, you are no longer allowing people to follow you.\n");
		u->following[0] = '\0';
		user->follow_by[0] = '\0';
		user->no_follow = 1;
		return;
	}
	write_user(user,"~FR Ok, you are no longer allowing people to follow you.\n");
	user->no_follow = 1;
}

/* Lets a user toggle ignoring fights */
void nofight(User user,char *inpstr,int rt)
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
	if (user->no_fight)
	{
		write_user(user,"~FG Ok, you are now accepting fighting challenges.\n");
		user->no_fight = 0;
		return;
	}
	write_user(user,"~FR You are no longer accepting fighting challenges.\n");
	user->no_fight = 1;
}

/* Lets a user start or stop earning money */
void earn_money(User user,char *inpstr,int rt)
{
	char *name;
	int i;

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
	if (user->invis == Invis) name = invisname;
	else name = user->recap;
	if (user->earning)
	{
		write_user(user,"~FR Ok, you stop earning money.\n");
		WriteRoomExcept(user->room,user,"~FT%s ~FYstops earning money.\n",name);
		user->earning = 0;
		return;
	}
	i = ran((unsigned)NUM_EARNINGS);
	writeus(user,earn_phrases[i],"You","");
	if (user->port != port[1] && istrstr(user->autoexec,"earn"))
	{
		WriteRoomExcept(user->room,user,earn_phrases[i],name,"s");
	}
	user->earning = 1;
}

/* Lets a user start following or stop following a user */
void follow(User user,char *inpstr,int rt)
{
	User u,u2;
	char *name;

	if (user->invis == Invis) name = invisname;
	else name = user->recap;
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CR-reset~CB]");
		if (user->following[0] != '\0') writeus(user,"~CW-~FT You are currently following~CB: ~FG%s\n",user->following);
		return;
	}
	if (user->status & Shackled)
	{
		write_user(user,"~FM Shackled people cannot follow other people.\n");
		return;
	}
	if (!strcasecmp(word[1],"-reset"))
	{
		if (user->following[0] == '\0')
		{
			write_user(user,"~FR You currently aren't following anyone!\n");
			return;
		}
		write_user(user,"~FT Ok, you are no longer following anyone!\n");
		if ((u = get_user(user->following)) == NULL)
		{
			user->following[0]='\0';
			return;
		}
		writeus(u,"~FT %s ~FYstops following you.\n",name);
		user->following[0] = '\0';
		u->follow_by[0] = '\0';
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~CW- ~FT Why would you want to follow yourself?\n");
		return;
	}
	if (u->no_follow)
	{
		write_user(user,"~FR That user is not allowing anyone to follow them!\n");
		return;
	}
	if (user->following[0] != '\0')
	{
		if ((u2 = get_user(user->following)) == NULL)
		{
			write_user(user,"~FR It appears you already were following someone, but they left...resetting follow stats.\n");
			user->following[0] = '\0';
			return;
		}
		writeus(user,"~FG You are already following ~FM%s\n",u2->recap);
		return;
	}
	if (user->room != u->room)
	{
		write_user(user,"~FT You have to be in the same room as the user first.\n");
		return;
	}
	if (!strcasecmp(user->follow_by,u->name))
	{
		write_user(user,"~CW-~FT Tsk, tsk, following someone who's following you could be bad...\n");
		return;
	}
	writeus(user,"~FT You start following~CB: [~FM%s~CB]\n",u->recap);
	writeus(u,"~FG%s ~FMstarts to follow you.\n",name);
	strcpy(user->following,u->name);
	strcpy(u->follow_by,user->name);
}

/* Ask the mystic eight ball a question */
void eight_ball(User user,char *inpstr,int rt)
{
	char *name;
	int i;

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
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRa yes or no question~CB]");
		return;
	}
	if (!strstr(inpstr," ") || !strstr(inpstr,"?"))
	{
		write_user(user,"~FG It would help if a question was asked.\n");
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->invis == Invis) name = invisname; else name = user->recap;
	i = ran((unsigned)NUM_BALL_LINES);
	inpstr[0] = toupper(inpstr[0]);
	writeus(user,"~CY You ask the mystical eightball~CB:~FG %s\n",inpstr);
	WriteRoomExcept(user->room,user,"~CY%s~FG asks the mystical eightball~CB: ~FT%s\n",name,inpstr);
	WriteRoom(user->room,"~FY The eightball speaks and says~CB: ~FR%s\n",ball_text[i]);
}

/* Lets a user review the logins, or see when a user was last logged on */
void last(User user,char *inpstr,int rt)
{
	User u;
	int i,cnt,err = 0;
	int timelen,days,hours,mins,hours2,mins2;
	char *uname;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		cnt = 0;
		for (i = 0; i < 10;++i)
		{
			if (loginbuff->conv[i]!=NULL) ++cnt;
		}
		if (cnt == 0)
		{
			write_user(user,"~CW- ~FTIt appears as tho noone has been online~CW :~(\n");
			return;
		}
		if (user->ansi_on) sprintf(text,"~FMฤฤฤฏฏ>~CY The last logins on ~FR%s ~FM<ฎฎฤฤฤ\n",TalkerRecap);
		else sprintf(text,"~FT-~CT=~CB> ~CYThe last logins on ~FR%s ~CB<~CT=~FT-\n",TalkerRecap);
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_convbuff(user,loginbuff);
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
		return;
	}
	word[1][0] = toupper(word[1][0]);
	if (!strcasecmp(word[1],user->name))
	{
		write_user(user,"~FG Don't be silly, you are already logged on.\n");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		write_user(user,"~FM That user is already logged on.\n");
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~FG Sorry an error occurred while trying to use this command.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in last()\n");
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
	else if (err==-1)
	{
		write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
		write_user(user,"~CW-~FT That user's userfile seems to be corrupted..\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	timelen	= (int)(time(0)-u->last_login);
	days	= timelen/86400;
	hours	= (timelen%86400)/3600;
	mins	= (timelen%3600)/60;
	hours2	= u->last_login_len/3600;
	mins2	= (u->last_login_len%3600)/60;
	writeus(user,"~CW - ~FR%s ~FTwas last logged on~CB: ~FY%s",u->recap,ctime((time_t *)&(u->last_login)));
	if (days == 0 && hours == 0) sprintf(text," ~CW-~FT Which was~CB: [~FY%d ~FGminute%s ago.~CB]\n",mins,mins>1?"s":"");
	else if (days == 0) sprintf(text,"~CW - ~FTWhich was~CB: [~FY%d~FG hour%s and ~FY%d~FG minute%s ago.~CB]\n",hours,hours>1?"s":"",mins,mins>1?"s":"");
	else sprintf(text,"~CW - ~FTWhich was~CB: [~FY%d ~FGday%s, ~FY%d~FG hour%s and ~FY%d ~FGminute%s ago.~CB]\n",days,days>1?"s":"",hours,hours>1?"s":"",mins,mins>1?"s":"");
	write_user(user,text);
	if (hours2 == 0 && mins2 == 0) sprintf(text,"~CW - ~FR%s~FT didn't even stay on long enough to say hi.\n",u->recap);
	else if (hours2 == 0) sprintf(text,"~CW - ~FTAnd they were on for~CB: ~CB[~FY%d~FG minute%s.~CB]\n",mins2,mins2>1?"s":"");
	else sprintf(text,"~CW - ~FTAnd they were on for~CB: [~FY%d ~FGhour%s, and ~FY%d ~FGminute%s.~CB]\n",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"");
	write_user(user,text);
	destruct_user(u);
	destructed = 0;
}

/* Shows a user a list of their friends currently online */
void friend_who(User user,char *inpstr,int rt)
{
	Room rm;
	User u;
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
	for_ausers(u)
	{
		if (u == NULL || u->login || u->type == CloneType) continue;
		if (u->invis && u->level > user->level) continue;
		if (u->invis == Hidden && user->level < ADMIN) continue;
		if (u->invis == Fakelog && user->level < GOD) continue;
		rm = u->room;
		if (!is_friend(user,u)) continue;
		if (++cnt == 1)
		{
			if (user->ansi_on)
			{
				write_user(user,ansi_tline);
				sprintf(text,"~FRฤฤฤฏฏ>~CY Friends of yours on ~FM%s ~FR<ฎฎฤฤฤ\n",long_date(0));
				write_user(user,center(text,80));
				write_user(user,ansi_bline);
			}
			else
			{
				write_user(user,ascii_tline);
				sprintf(text,"~FT-~CT=~CB> ~FGFriends of yours on ~FM%s ~CB<~CT=~FT-\n",long_date(0));
				write_user(user,center(text,80));
				write_user(user,ascii_bline);
			}
		}
		if (u->predesc[0]) sprintf(text,"~CR[~FT %s~FT %s~FM %s~CR ]~FG is currently in the~CB: %s\n",u->predesc,u->recap,u->desc,rm->name);
		else sprintf(text,"~CR[~FT %s~FM %s~CR ]~FG is currently in the~CB: %s\n",u->recap,u->desc,rm->name);
		write_user(user,text);
	}
	if (cnt == 0) write_user(user,"~CW- ~FGYou have no friends online right now.\n");
	return;
}

/* Lets a user see how much money they have or another user has */
void show_money(User user,char *inpstr,int rt)
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
	if (word_count < 2)
	{
		if (user->money == 0 && user->dep_money == 0) sprintf(text,"~CW - ~CTYou don't have any money on hand OR in the bank.\n");
		else if (user->money == 0 && user->dep_money) sprintf(text,"~CW - ~CTYou have no money on hand, but you have ~FG%ld~CT dollar%s in the bank.\n",user->dep_money,user->dep_money>1?"s":"");
		else if (user->money && user->dep_money == 0) sprintf(text,"~CW - ~CTYou currently have ~FG%ld ~CTdollar%s on hand and no money in the bank.\n",user->money,user->money>1?"s":"");
		else sprintf(text,"~CW - ~CTYou currently have ~FG%ld ~CTdollar%s on hand and ~FG%ld ~CTdollar%s in the bank.\n",user->money,user->money>1?"s":"",user->dep_money,user->dep_money>1?"s":"");
		write_user(user,text);
		if (user->chips == 0 && user->dep_chips == 0) sprintf(text,"~CW - ~CTYou have no chips on hand or in the bank.\n");
		else if (user->chips == 0 && user->dep_chips) sprintf(text,"~CW - ~CTYou have no chips on hand, but you have ~FM%ld~CT chip%s in the bank.\n",user->dep_chips,user->dep_chips>1?"s":"");
		else if (user->chips && user->dep_chips == 0) sprintf(text,"~CW - ~CTYou currently have ~FM%ld~CT chip%s on hand and no chips in the bank.\n",user->chips,user->chips>1?"s":"");
		else sprintf(text,"~CW - ~CTYou currently have ~FM%ld~CT chip%s on hand and ~FM%ld ~CTchip%s in the bank.\n",user->chips,user->chips>1?"s":"",user->dep_chips,user->dep_chips>1?"s":"");
		write_user(user,text);
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_name(user,uname)) != NULL)
	{
		if (u->money == 0 && u->dep_money == 0) sprintf(text,"~CW - ~FR%s ~CThas no money on hand or in the bank.\n",u->recap);
		else if (u->money == 0 && u->dep_money) sprintf(text,"~CW - ~FR%s ~CThas no money on hand, but they have ~FG%ld~CT dollar%s in the bank.\n",u->recap,u->dep_money,u->dep_money>1?"s":"");
		else if (u->money && u->dep_money == 0) sprintf(text,"~CW - ~FR%s ~CTcurrently has ~FG%ld ~CTdollar%s on hand and no money in the bank.\n",u->recap,u->money,u->money>1?"s":"");
		else sprintf(text,"~CW - ~FR%s ~CTcurrently has ~FG%ld ~CTdollar%s on hand and ~FG%ld ~CTdollar%s in the bank.\n",u->recap,u->money,u->money>1?"s":"",u->dep_money,u->dep_money>1?"s":"");
		write_user(user,text);
		if (u->chips == 0 && u->dep_chips == 0) sprintf(text,"~CW - ~FR%s ~CThas no chips on hand or in the bank.\n",u->recap);
		else if (u->chips == 0 && u->dep_chips) sprintf(text,"~CW - ~FR%s ~CThas no chips on hand, but they have ~FM%ld~CT chip%s in the bank.\n",u->recap,u->dep_chips,u->dep_chips>1?"s":"");
		else if (u->chips && u->dep_chips == 0) sprintf(text,"~CW - ~FR%s ~CTcurrently has ~FM%ld~CT chip%s on hand and no chips in the bank.\n",u->recap,u->chips,u->chips>1?"s":"");
		else sprintf(text,"~CW - ~FR%s ~CTcurrently has ~FM%ld~CT chip%s on hand and ~FM%ld ~CTchip%s in the bank.\n",u->recap,u->chips,u->chips>1?"s":"",u->dep_chips,u->dep_chips>1?"s":"");
		write_user(user,text);
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred, please try again in a few minutes.\n");
		write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in show_money()\n");
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
		write_user(user,"~CW-~FT That user's userfile seems to be corrupted..\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->money == 0 && u->dep_money == 0) writeus(user,"~CW - ~FR%s ~CThas no money on hand or in the bank.\n",u->recap);
	else if (u->money == 0 && u->dep_money) writeus(user,"~CW - ~FR%s ~CThas no money on hand, but they have ~FG%ld~CT dollar%s in the bank.\n",u->recap,u->dep_money,u->dep_money>1?"s":"");
	else if (u->money && u->dep_money == 0) writeus(user,"~CW - ~FR%s ~CTcurrently has ~FG%ld ~CTdollar%s on hand and no money in the bank.\n",u->recap,u->money,u->money>1?"s":"");
	else writeus(user,"~CW - ~FR%s ~CTcurrently has ~FG%ld ~CTdollar%s on hand and ~FG%ld ~CTdollar%s in the bank.\n",u->recap,u->money,u->money>1?"s":"",u->dep_money,u->dep_money>1?"s":"");
	if (u->chips == 0 && u->dep_chips == 0) writeus(user,"~CW - ~FR%s ~CThas no chips on hand or in the bank.\n",u->recap);
	else if (u->chips == 0 && u->dep_chips) writeus(user,"~CW - ~FR%s ~CThas no chips on hand, but they have ~FM%ld~CT chip%s in the bank.\n",u->recap,u->dep_chips,u->dep_chips>1?"s":"");
	else if (u->chips && u->dep_chips == 0) writeus(user,"~CW - ~FR%s ~CTcurrently has ~FM%ld~CT chip%s on hand, and no chips in the bank.\n",u->recap,u->chips,u->chips>1?"s":"");
	else writeus(user,"~CW - ~FR%s ~CTcurrently has ~FM%ld~CT chip%s on hand and ~FM%ld ~CTchip%s in the bank.\n",u->recap,u->chips,u->chips>1?"s":"",u->dep_chips,u->dep_chips>1?"s":"");
	destruct_user(u);
	destructed = 0;
}

/* Lets a user read a called upon text file */
void text_read(User user,char *inpstr,int rt)
{
	FILE *fp;
	char filename[FSL],*c;

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
		sprintf(filename,"%s/%s",MiscDir,TextList);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,center("~FR There is no current list of text files right now...\n",80));
			  break;
			case 1:
			  user->misc_op = 2;
		}
		return;
	}
	/* Check for illegal shit, ie /etc/passwd */
	c = word[1];
	while(*c)
	{
		if (*c == '.' || *c++ == '/')
		{
			write_user(user,"~CW-~FT It appears as tho there isn't a text file with that name.\n");
			write_log(1,LOG1,"[Illegal FileName] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[1],command[com_num].name);
			return;
		}
	}
	sprintf(filename,"%s/%s",TextDir,word[1]);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		write_user(user,"~CW - ~FGNo current text file with that name.\n");
		return;
	}
	FCLOSE(fp);
	sprintf(text,"~FT-~CT=~CB> ~FGThe ~FM[~FR%s~FM]~FG text file ~CB<~CT=~FT-\n",word[1]);
	write_user(user,center(text,80));
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  break;
		case 1:
		  user->misc_op = 2;
	}
	return;
}

/* Lets a user see all the users in the same room as the given user */
void with_user(User user,char *inpstr,int rt)
{
	User u,u2;
	Room rm;
	int cnt = 0;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
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
	rm = u->room;
	for_users(u2)
	{
		if (u2->room != rm) continue;
		if (++cnt == 1)
		{
			if (user->ansi_on)
			{
				write_user(user,"~FMีอออออออออออออออออออออออออออออออออออออธ\n");
				writeus(user,"  ~CYList of users in the~CB:~FM %s\n",rm->name);
				write_user(user,"~FMิอออออออออออออออออออออออออออออออออออออพ\n");
			}
			else
			{
				write_user(user,"~FM=-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
				writeus(user,"  ~CYList of users in the~CB:~FM %s\n",rm->name);
				write_user(user,"~FM=-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
			}
		}
		writeus(user,"  ~FT-- ~FY%s\n",u2->recap);
	}
	if (cnt)
	{
		if (user->ansi_on) write_user(user,"~FMิอออออออออออออออออออออออออออออออออออออพ\n");
		else write_user(user,"~FM=-==-==-==-==-==-==-==-==-==-==-==-==-=\n");
	}
	else
	{
		writeus(user,"~CW-~FT Noone is with~FY %s.\n",u->recap);
	}
	return;
}

/* Shows out the picture list */
void picture_list(User user,char *inpstr,int rt)
{
	char filename[FSL];

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
	sprintf(filename,"%s/%s",MiscDir,PicList);
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  write_user(user,center("~FR There is no current list of pictures right now...\n",80));
		  break;
		case 1:
		  user->misc_op = 2;
	}
}

/* View a picture */
void view_picture(User user,char *inpstr,int rt)
{
	FILE *fp;
	char filename[FSL],line[ARR_SIZE],*c;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRpicture name~CB]");
		return;
	}
	/* Check for illegal shit, ie /etc/passwd */
	c = word[1];
	while(*c)
	{
		if (*c == '.' || *c++ == '/')
		{
			write_user(user,"~CW-~FT It appears as tho there isn't a picture file with that name.\n");
			write_log(1,LOG1,"[Illegal FileName] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[1],command[com_num].name);
			return;
		}
	}
	sprintf(filename,"%s/%s",PicDir,word[1]);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		write_user(user,"~CW - ~FGNo current picture with that name.\n");
		return;
	}
	sprintf(text,"~FT-~CT=~CB> ~FGThe ~FR%s~FG picture. ~CB<~CT=~FT-\n",word[1]);
	write_user(user,center(text,80));
	fgets(line,ARR_SIZE-1,fp);
	while(!feof(fp))
	{
		write_user(user,line);
		fgets(line,ARR_SIZE-1,fp);
	}
	FCLOSE(fp);
}

/* Displays a picture to the room */
void room_picture(User user,char *inpstr,int rt)
{
	FILE *fp;
	char filename[FSL],line[ARR_SIZE],*c,*name;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRpicture name~CB]");
		return;
	}
	/* Check for illegal shit, ie /etc/passwd */
	c = word[1];
	while(*c)
	{
		if (*c == '.' || *c++ == '/')
		{
			write_user(user,"~CW-~FT It appears as tho there isn't a picture file with that name.\n");
			write_log(1,LOG1,"[Illegal FileName] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[1],command[com_num].name);
			return;
		}
	}
	sprintf(filename,"%s/%s",PicDir,word[1]);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		write_user(user,"~CW - ~FGNo current picture with that name.\n");
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	sprintf(text,"~FT-~CT=~CB> ~FM%s ~FGshows everyone the: ~FR%s~FG picture. ~CB<~CT=~FT-\n",name,word[1]);
	write_room(user->room,center(text,80));
	fgets(line,ARR_SIZE-1,fp);
	while(!feof(fp))
	{
		write_room(user->room,line);
		fgets(line,ARR_SIZE-1,fp);
	}
	FCLOSE(fp);
}

/* Sends a picture to another user */
void picture_tell(User user,char *inpstr,int rt)
{
	User u;
	FILE *fp;
	char filename[FSL],line[ARR_SIZE],*c,*name,*rname;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRpicture name~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (user->invis == Invis) name = invisname;   else name = user->recap;
	if (u->invis == Invis) rname = invisname;   else rname = u->recap;
	if (u == user)
	{
		write_user(user,"~FG Your crazy for trying to send yourself a picture.\n");
		return;
	}
	if (u->afk)
	{
		if (u->afk_mesg[0]) sprintf(text,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]eyboard~CB:~FT %s\n",rname,u->afk_mesg);
		else sprintf(text,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard at the moment\n",rname);
		write_user(user,text);
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s ~FGis using the editor at the moment...\n",rname);
		return;
	}
	if (is_enemy(u,user) && user->level < OWNER)
	{
		write_user(user,"~FG That person does not want any pictures from you!\n");
		return;
	}
	if (u->ignall && user->level < OWNER)
	{
		writeus(user,"~FT%s ~FRis ignoring everyone at the moment.\n",rname);
		return;
	}
	if ((u->ignore & Pictures) && user->level<GOD)
	{
		writeus(user,"~FT%s ~FRis ignoring pictures right now...\n",rname);
		return;
	}
	if ((u->ignore & Tells) && user->level<GOD)
	{
		writeus(user,"~FT%s ~FRis ignoring private messages right now...\n",rname);
		return;
	}
	if (user->status & Silenced)
	{
		sprintf(text,"~FT-~CT=~CB> ~FGYou show ~FM%s~FG the: ~FM[~FR%s~FM]~FG picture. ~CB<~CT=~FT-\n",rname,word[2]);
		write_user(user,center(text,80));
		return;
	}
	/* Check for illegal shit, ie /etc/passwd */
	c = word[2];
	while(*c)
	{
		if (*c == '.' || *c++ == '/')
		{
			write_user(user,"~CW-~FT It appears as tho there isn't a picture file with that name.\n");
			write_log(1,LOG1,"[Illegal FileName] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[1],command[com_num].name);
			return;
		}
	}
	sprintf(filename,"%s/%s",PicDir,word[2]);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		write_user(user,"~CW - ~FGNo current picture with that name.\n");
		return;
	}
	sprintf(text,"~FT-~CT=~CB> ~FGYou show ~FM%s~FG the: ~FR%s~FG picture. ~CB<~CT=~FT-\n",rname,word[2]);
	write_user(user,center(text,80));
	sprintf(text,"~FT-~CT=~CB> ~FM%s ~FGshows you the: ~FM[~FR%s~FM] ~FGpictures. ~CB<~CT=~FT-\n",name,word[2]);
	write_user(u,center(text,80));
	fgets(line,ARR_SIZE-1,fp);
	while(!feof(fp))
	{
		write_user(u,line);
		fgets(line,ARR_SIZE-1,fp);
	}
	FCLOSE(fp);
	write_user(user,"~CTThe picture has been sent.\n");
}

/* Shows the talkers version..please do not alter this... */
void what_version(User user,char *inpstr,int rt)
{
	char bstr[40],srbstr[40],temp[51];
	int secs,hours,mins,days,weeks,secs2,mins2,hours2,days2,weeks2;

	strcpy(bstr,ctime(&boot_time));
	if (sreboot_time != 0) strcpy(srbstr,ctime(&sreboot_time));
	secs	= (int)(time(0)-boot_time);
	weeks	= secs/604800;
	days	= (secs%604800)/86400;
	hours	= (secs%86400)/3600;
	mins	= (secs%3600)/60;
	secs	= secs%60;
	secs2	= (int)(time(0)-sreboot_time);
	weeks2	= secs2/604800;
	days2	= (secs2%604800)/86400;
	hours2	= (secs2%86400)/3600;
	mins2	= (secs2%3600)/60;
	secs2	= secs2%60;
	write_user(user,"~CB[` `]~FG------------------------------------------------------------~CB[` `]\n");
	write_user(user,"~CB ` `                                                              ~CB` `\n");
	writeus(user,"~CW  - ~FY %s ~FTtalker running ~FM%s\n",TalkerRecap,VERSION);
	writeus(user,"~CW  - ~FT Total users ~CB: ~FY%d  ~CR-=<>=-~FT New users this boot~CB: ~FY%d\n",user_count,new_users);
	write_user(user,"~CW  - ~FT Total logins~CB:\n");
	writeus(user,"~CW     -~FT This month~CB: ~FY%7ld ~CW-- ~FTToday~CB: ~FY%ld\n",ulogins[2],ulogins[1]);
	writeus(user,"~CW     -~FT This year ~CB: ~FY%7ld ~CW-- ~FTTotal~CB: ~FY%ld\n",ulogins[0],ulogins[3]);
	write_user(user,"~CW  - ~FT New users~CB:\n");
	writeus(user,"~CW     -~FT This month~CB: ~FY%7ld ~CW-- ~FTToday~CB: ~FY%ld\n",newlog[2],newlog[1]);
	writeus(user,"~CW     -~FT Total     ~CB: ~FY%7ld\n",newlog[0]);
	writeus(user,"~CW  -~FT  Booted on~CB: ~FY%s",bstr);
	sprintf(text,"~CW  -~FT  Uptime~CB: ");
	if (weeks)
	{
		sprintf(temp,"~CY%d ~FGweek%s",weeks,weeks == 1?"":"s");
		strcat(text,temp);
	}
	if (days)
	{
		if (strstr(text,"week")) strcat(text,", ");
		sprintf(temp,"~CY%d ~FGday%s",days,days == 1?"":"s");
		strcat(text,temp);
	}
	if (hours)
	{
		if (strstr(text,"day") || strstr(text,"week")) strcat(text,", ");
		sprintf(temp,"~CY%d ~FGhour%s",hours,hours == 1?"":"s");
		strcat(text,temp);
	}
	if (mins)
	{
		if (strstr(text,"hour") || strstr(text,"day")
		 || strstr(text,"week")) strcat(text,", ");
		sprintf(temp,"~CY%d ~FGminute%s",mins,mins == 1?"":"s");
		strcat(text,temp);
	}
	if (strstr(text,"minute") || strstr(text,"hour")
	  || strstr(text,"day") || strstr(text,"week")) strcat(text,", and ");
	sprintf(temp,"~CY%d ~FGsecond%s",secs,secs == 1?"":"s");
	strcat(text,temp);
	strcat(text,"\n");
	write_user(user,text);
	if (sreboot_time == 0)
		write_user(user,"~CW  -~FT  No seamless reboot since talker booted.\n");
	else
	{
		writeus(user,"~CW  -~FT  Last seamless reboot date~CB: ~FY%s",srbstr);
		sprintf(text,"~CW  -~FT  Last SReboot~CB: ");
		if (days2)
		{
			sprintf(temp,"~CY%d~FG day%s",days2,days2 == 1?"":"s");
			strcat(text,temp);
		}
		if (hours2)
		{
			if (strstr(text,"day")) strcat(text,", ");
			sprintf(temp,"~CY%d~FG hour%s",hours2,hours2 == 1?"":"s");
			strcat(text,temp);
		}
		if (mins2)
		{
			if (strstr(text,"hour") || strstr(text,"day"))
				strcat(text,", ");
			sprintf(temp,"~CY%d~FG minute%s",mins2,mins2 == 1?"":"s");
			strcat(text,temp);
		}
		if (strstr(text,"minute") || strstr(text,"hour")
		  || strstr(text,"day")) strcat(text,", and ");
		sprintf(temp,"~CY%d ~FGsecond%s",secs2,secs2 == 1?"":"s");
		strcat(text,temp);
		strcat(text,"\n");
		write_user(user,text);
	}
	writeus(user,"~CW  - ~FT Current system time~CB: ~FY%s\n",long_date(2));
	write_user(user,"~CB[` `]~FG------------------------------------------------------------~CB[` `]\n");
	write_user(user,"~CB ` `                                                              ~CB` `\n");
}

/* List the socials out */
void socials(User user,char *inpstr,int wrap)
{
	Socials soc;
	int com,cnt,tot,lines;
	char temp[51];

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
	if (wrap == 0)
	{
		user->ignall_store = user->ignall;
		user->ignall = 1;
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~FRฤฤฤฤฏฏ> ~FG%s ~FYSocial listing ~FR<ฎฎฤฤฤฤ\n",TalkerRecap);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FB--~CB==~FT[ ~FG%s ~FYSocial listing ~FT]~CB==~FB--\n",TalkerRecap);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		text[0]='\0';
		soc = first_social;
		strcpy(user->check,first_social->name);
	}
	if (wrap) /* Find where we are. */
	{
		text[0] = '\0';
		soc = first_social;
		while (soc != NULL)
		{
			if (strcasecmp(soc->name,user->check)) soc = soc->next;
			else break;
		}
	}
	lines = tot = cnt = com = 0;
	while (soc != NULL)
	{
		sprintf(temp,"  ~FY%-12s",soc->name);
		strcat(text,temp);
		++cnt;
		if (cnt >= 5)
		{
			strcat(text,"\n");
			write_user(user,text);
			text[0] = '\0';
			cnt = 0;
			++lines;
		}
		soc = soc->next;
		if (user->rows != 0 && lines >= user->rows)
		{
			user->misc_op = 34;
			user->check[0] = '\0';
			strcpy(user->check,soc->name);
			write_user(user,continue2);
			return;
		}
		if (cnt == 0) strcat(text,"");
	}
	if (cnt)
	{
		strcat(text,"\n");
		write_user(user,text);
		++lines;
		text[0] = '\0';
	}
	soc = first_social;
	while (soc != NULL)
	{
		soc = soc->next;
		++com;
		++tot;
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	writeus(user,"~FG  There are currently ~CR%d~FG social%s available to you. \n",com,com>1?"s":"");
	write_user(user,"~FG  Please remember that all socials start with a ~CW'~CR.~CW' ~FGor a ~CW'~CR,~CW'~FG.\n");
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
	user->misc_op = 0;
	rev_away(user);
}

/* Shows a user the list of staff that are currently online */
void wiz_who(User user,char *inpstr,int rt)
{
	User u;
	int cnt,cnt1,cnt2;
	char uname[251],rmname[81],lev_name[16];

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
	cnt = cnt1 = cnt2 = 0;
	for_users(u)
	{
		if (u->login) continue;
		if (u->type == CloneType || u->invis >= Invis
		  || u->room->hidden) continue;
		if (u->tpromoted && u->orig_lev<WIZ) continue;
		if (u->cloak_lev < WIZ) continue;
		if (++cnt == 1)
		{
			if (user->ansi_on) write_user(user,ansi_tline);
			else write_user(user,ascii_tline);
			sprintf(text,"~FT-~CT=~CB> ~CYStaff members present on %s ~CB<~CT=~FT-\n",long_date(0));
			write_user(user,center(text,80));
			if (user->ansi_on) write_user(user,ansi_bline);
			else write_user(user,ascii_bline);
		}
		strcpy(lev_name,user->gender == Female?level[u->cloak_lev].fname:level[u->cloak_lev].mname);
		strcpy(uname,u->recap);
		strcpy(rmname,u->room->recap);
		cnt1 = colour_com_count(uname);
		cnt2 = colour_com_count(rmname);
		writeus(user,"~CW- ~FTStaff Member~CB: [~RS%-*s~CB]~CW - ~FTRoom~CB: [~RS%-*s~CB]~CW - ~FTLevel~CB: [~FR%-10.10s~CB]\n",12+cnt1*3,uname,15+cnt2*3,rmname,lev_name);
	}
	if (cnt == 0) write_user(user,"~CW-~FT There are no staff members on presently.\n");
	else
	{
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		sprintf(text,"  ~FGThere %s ~CR%d~FG staff member%s on currently.\n",cnt == 1?"is":"are",cnt,cnt == 1?"":"s");
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
	}
}

/* Wait for a certain user */
void waitfor_user(User user,char *inpstr,int rt)
{
	User u;
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
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CR-reset~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-reset",2))
	{
		if (user->waiting[0])
		{
			write_user(user,"~FG You are no longer waiting for anyone.\n");
			user->waiting[0] = '\0';
			return;
		}
		else write_user(user,"~FG You aren't currently waiting for anyone.\n");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)))
	{
		write_user(user,"~FT Have you done a .who lately to see who's on?!?!\n");
		return;
	}
	strcpy(user->waiting,uname);
	writeus(user,"~FT You will be notified when ~CR%s~FT logs onto the talker.\n",user->waiting);
}

/* Lets a user contribute an answer to a riddle, or see the riddle so far! */
void riddle(User user,char *inpstr,int rt)
{
	FILE *fp;
	char filename[FSL];

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
		sprintf(filename,"%s/riddle",MiscDir);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,"~CW-~RS There isn't a riddle at this point in time.\n");
			  break;
			case 1:
			  user->misc_op = 2;
		}
		return;
	}
	if (!strcasecmp(word[1],"-results"))
	{
		if (user->level < WIZ)
		{
			sprintf(filename,"%s/riddle",MiscDir);
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,"~CW-~RS There isn't a riddle at this point in time.\n");
				  break;
				case 1:
				  user->misc_op = 2;
			}
			return;
		}
		sprintf(filename,"%s/rid_answers",MiscDir);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,"~CW- ~RSNoone has submitted any answers at this point in time.\n");
			  break;
			case 1:
			  user->misc_op=2;
		}
		return;
	}
	if (!strcasecmp(word[1],"-submit"))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CR-submit~CB [~CRyour answer~CB]");
			return;
		}
		sprintf(filename,"%s/riddle",MiscDir);
		if ((fp = fopen(filename,"r")) == NULL)
		{
			write_user(user,"~CW-~RS There is no current riddle right now.\n");
			return;
		}
		FCLOSE(fp);
		sprintf(filename,"%s/rid_answers",MiscDir);
		if ((fp = fopen(filename,"a")) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred in your request.. please try again in a few minutes.\n");
			write_log(0,LOG1,"[ERROR] - Unable to open file to append in riddle()\n");
			return;
		}
		inpstr = remove_first(inpstr);
		sprintf(text,"~FM From ~FT%s~CB: ~RS%s\n\n",user->name,inpstr);
		fputs(text,fp);
		FCLOSE(fp);
		write_user(user,"~FT Your answer has been received, check the text files for results in the days to come.\n");
		write_log(1,LOG1,"[RIDDLE] - User [%s] has submitted an answer.\n",user->name);
		return;
	}
	if (!strcasecmp(word[1],"-add"))
	{
		sprintf(filename,"%s/riddle",MiscDir);
		if (user->level < ADMIN)
		{
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,"~CW-~RS There isn't a riddle at this point in time.");
				  break;
				case 1:
				  user->misc_op = 2;
			}
			return;
		}
		if (file_exists(filename))
		{
			writeus(user,"~CW-~RS You have to use the .%s -clear first to clear the current riddle.\n",command[com_num].name);
			return;
		}
		memset(filename,0,sizeof(filename));
		user->misc_op = 50;
		enter_riddle(user,NULL,0);
		return;
	}
	if (!strcasecmp(word[1],"-clear"))
	{
		if (user->level < ADMIN)
		{
			sprintf(filename,"%s/riddle",MiscDir);
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,"~CW-~RS There isn't a riddle at this point in time.");
				  break;
				case 1:
				  user->misc_op = 2;
			}
			return;
		}
		sprintf(filename,"%s/riddle",MiscDir);
		unlink(filename);
		sprintf(filename,"%s/rid_answers",MiscDir);
		unlink(filename);
		write_user(user,"~CW-~RS Riddle files have been deleted...\n");
		return;
	}
	sprintf(filename,"%s/riddle",MiscDir);
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  write_user(user,"~CW-~RSThere isn't a riddle at this point in time!\n");
		  break;
		case 1:
		  user->misc_op=2;
	}
}

/* Finds a user from a certain pattern */
void find_user(User user,char *inpstr,int wrap)
{
	Ulobj ul;
	int lines,cnt,total,lev,uselev;
	char temp[ARR_SIZE],search[ARR_SIZE];

	if (user == NULL) return;
	lines = cnt = total = lev = uselev = 0;
	text[0] = temp[0] = '\0';
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		if (wrap == 0)
		{
			user->ignall_store = user->ignall;
			user->ignall = 1;
			sprintf(text,"~FT-~CT=~CB> ~FGA full list of users who come on %s ~CB<~CT=~FT-\n",TalkerRecap);
			write_user(user,center(text,80));
			write_user(user,ascii_line);
			text[0] = '\0';
			ul = ulist_first;
			strcpy(user->check,ulist_first->name);
		}
		if (wrap) /* find where we are */
		{
			text[0] = '\0';
			ul = ulist_first;
			while (ul != NULL)
			{
				if (strcasecmp(ul->name,user->check))
					ul = ul->next;
				else break;
			}
		}
		lines = cnt = total = 0;
		while (ul != NULL)
		{
			sprintf(temp," %-12s ",ul->name);
			strcat(text,temp);
			++cnt;
			if (cnt >= 5)
			{
				strcat(text,"\n");
				write_user(user,text);
				cnt = 0;
				++lines;
				text[0] = temp[0] = '\0';
			}
			ul = ul->next;
			if (user->rows != 0 && lines >= user->rows)
			{
				user->check[0] = '\0';
				strcpy(user->check,ul->name);
				write_user(user,continue1);
				user->misc_op = 37;
				return;
			}
			if (cnt == 0) strcat(text,"");
		}
		if (cnt)
		{
			strcat(text,"\n");
			write_user(user,text);
			text[0] = '\0';
		}
		for_list(ul) {  ++total;  }
		write_user(user,ascii_tline);
		writeus(user,"~FG There were a total of %d users found.\n",total);
		write_user(user,ascii_bline);
		user->misc_op = 0;
		user->ignall = user->ignall_store;
		rev_away(user);
		return;
	}
	strcpy(search,word[1]);
	if ((lev = get_level(search)) == -1) uselev = 0;
	else uselev = 1;
	if (uselev)
	{
		sprintf(text,"~FT-~CT=~CB> ~FGUsers at level~CB: [~CR%s~CB] ~CB<~CT=~FT-\n",user->gender==Female?level[lev].fname:level[lev].mname);
		write_user(user,center(text,80));
		text[0] = '\0';
	}
	else
	{
		sprintf(text,"~FT-~CT=~CB> ~FGUsers with pattern~CB: [~CR%s~CB]~FG in their name. ~CB<~CT=~FT-\n",word[1]);
		write_user(user,center(text,80));
		text[0] = '\0';
	}
	write_user(user,ascii_line);
	for_list(ul)
	{
		if (uselev)
		{
			if (ul->lev != lev) continue;
		}
		else
		{
			if (!istrstr(ul->name,word[1])) continue;
		}
		sprintf(temp," %-12s",ul->name);
		++cnt;
		++total;
		if (cnt >= 5)
		{
			strcat(text,temp);
			strcat(text,"\n");
			write_user(user,text);
			cnt = 0;
			text[0] = '\0';
			temp[0] = '\0';
		}
		else strcat(text,temp);
	}
	if (cnt > 0 && cnt < 5)
	{
		strcat(text,"\n");
		write_user(user,text);
	}
	if (total)
	{
		write_user(user,ascii_tline);
		if (lev!=-1) writeus(user,"~FG Out of ~CR%d ~FGusers %d %s at level %s.\n",user_count,total,total>1?"were":"was",user->gender==Female?level[lev].fname:level[lev].mname);
		else writeus(user,"~FG Out of ~CR%d ~FGusers %d had %s in their name.\n",user_count,total,word[1]);
		write_user(user,ascii_bline);
	}
	else write_user(user,"~FG No matches found.. see ~CW'~CR.h finduser~CW'\n");
	return;
}

/*
   Vote command, For votes, do NOT create any files but the "votefile"
   Make sure you always do a '.vote -clear' when starting a vote
*/
void vote(User user,char *inpstr,int rt)
{
	FILE *fp,*fp2;
	int a,b,c;
	char line[ARR_SIZE],filename[FSL];

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
	/* Gotta check to see if the file exists, no matter what */
	sprintf(filename,"%s/votetals",MiscDir);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		fp = fopen(filename,"w");
		if (fp != NULL) fputs("0\n0\n0\n",fp);
	}
	FCLOSE(fp);
	if (word_count < 2)
	{
		sprintf(filename,"%s/votefile",MiscDir);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,center("~FT-~CT=~CB> ~FRThere is nothing to vote on. ~CB<~CT=~FT-\n",80));
			  break;
			case 1:
			  user->misc_op=2;
		}
		return;
	}
	if ((!strcmp(inpstr,"1")) || (!strcmp(inpstr,"2"))
	  || (!strcmp(inpstr,"3")))
	{
		/* Check to see if there is actually anything to vote on. */
		sprintf(filename,"%s/votefile",MiscDir);
		if (!file_exists(filename))
		{
			write_user(user,"~CW-~FT There isn't even anything to vote on.\n");
			return;
		}
		filename[0] = '\0';
		sprintf(filename,"%s/voters",MiscDir);
		if ((fp = fopen(filename,"r")) == NULL)
		{
			goto ADD;
		}
		while(!feof(fp))
		{
			fscanf(fp,"%s\n",line);
			if (!strcasecmp(line,user->name))
			{
				write_user(user,"~FG You have already voted on this current topic.\n");
				FCLOSE(fp);
				return;
			}
			line[0] = '\0';
		}
		FCLOSE(fp);
		ADD:
		  sprintf(filename,"%s/votetals",MiscDir);
		  if ((fp2 = fopen(filename,"r")) == NULL)
		  {
			write_user(user,"~FT Unable to read the tallies!\n");
			return;
		  }
		  fscanf(fp2,"%s\n",line);
		  a = atoi(line);
		  line[0] = '\0';
		  fscanf(fp2,"%s\n",line);
		  b = atoi(line);
		  line[0] = '\0';
		  fscanf(fp2,"%s\n",line);
		  c = atoi(line);
		  line[0] = '\0';
		  FCLOSE(fp2);
		  if ((fp = fopen(filename,"w")) == NULL)
		  {
			write_user(user,"~FT Unable to write to tally voting file.\n");
			a = b = c = 0;
			return;
		  }
		  if (!strcmp(inpstr,"1")) a++;
		  else if (!strcmp(inpstr,"2")) b++;
		  else c++;
		  sprintf(line,"%d\n%d\n%d\n",a,b,c);
		  fputs(line,fp);
		  line[0] = '\0';
		  FCLOSE(fp);
		  sprintf(filename,"%s/voters",MiscDir);
		  if ((fp2 = fopen(filename,"a")) == NULL)
		  {
			write_user(user,"~FT Unable to append to voters file.\n");
			return;
		  }
		  fputs(user->name,fp2);
		  fputs("\n",fp2);
		  FCLOSE(fp2);
		  writeus(user,"~FG Thank you for your vote ~FT%s~FG =)\n",user->recap);
		  write_log(1,LOG1,"[Vote] - User: [%s]\n",user->name);
		  return;
	}
	else if ((!strcasecmp(inpstr,"-clear")) && (user->level >= ADMIN))
	{
		sprintf(filename,"%s/voters",MiscDir);
		unlink(filename);
		sprintf(filename,"%s/votefile",MiscDir);
		unlink(filename);
		write_user(user,"~CR Voting topic deleted...\n");
		sprintf(filename,"%s/votetals",MiscDir);
		if ((fp2 = fopen(filename,"w")) == NULL)
		{
			write_user(user,"~FG Reset voters file, but can't reset tallies!\n");
			return;
		}
		fputs("0\n0\n0\n",fp2);
		FCLOSE(fp2);
		write_user(user,"~CR Voters and tallies have been cleared.\n");
		write_log(1,LOG1,"%s RESETTED the vote tallies!\n",user->name);
		return;
	}
	else if ((!strcasecmp(inpstr,"-enter")) && (user->level >= ADMIN))
	{
		/* Check to see if there is actually anything to vote on. */
		sprintf(filename,"%s/votefile",MiscDir);
		if (file_exists(filename))
		{
			write_user(user,"~CW-~FT There is already a votefile.. use ~CR.vote -clear~FT first.\n");
			return;
		}
		filename[0] = '\0';
		inpstr[0] = '\0';
		enter_vote(user,NULL,0);
		return;
	}
	else if ((!strcasecmp(inpstr,"-results")) && (user->level >= WIZ))
	{
		/* Check to see if there is actually anything to vote on. */
		sprintf(filename,"%s/votefile",MiscDir);
		if (!file_exists(filename))
		{
			write_user(user,"~CW-~FT There isn't even anything to vote on.\n");
			return;
		}
		filename[0] = '\0';
		write_user(user,"~FG Current vote tally~CB:\n");
		sprintf(filename,"%s/votetals",MiscDir);
		if ((fp2 = fopen(filename,"r")) == NULL)
		{
			write_user(user,"~FG Noone has voted yet :~(\n");
			return;
		}
		line[0] = 0;
		fscanf(fp2,"%s\n",line);
		writeus(user,"~FG 1~CB] ~FY%s ~FGvotes.\n",line);
		line[0] = '\0';
		fscanf(fp2,"%s\n",line);
		writeus(user,"~FG 2~CB] ~FY%s ~FGvotes\n",line);
		line[0] = '\0';
		fscanf(fp2,"%s\n",line);
		writeus(user,"~FG 3~CB] ~FY%s ~FGvotes.\n",line);
		line[0] = '\0';
		FCLOSE(fp2);
		return;
	}
	else	
	{
		write_user(user,"~FG That is an invalid option, try 1, 2, or 3.\n");
		return;
	}
}

/* Lets a talker see the talker list, or add a new one */
void talkers(User user,char *inpstr,int rt)
{
	char filename[FSL];

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
	if (!strcasecmp(word[1],"list"))
	{
		sprintf(filename,"%s/%s",DataDir,TalkerFile);
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~CRฤฤฏ> ~FGThe ~CR%s ~FGtalker list. ~CR<ฎฤฤ\n",TalkerRecap);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FT-~CT=~CB> ~FGThe ~CR%s ~FGtalker list. ~CB<~CT=~FT-\n",TalkerRecap);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,center("~FR Noone has added any talkers to the list.\n",80));
			  return;
			case 1:
			  user->misc_op = 2;
		}
		return;
	}
	if (!strcasecmp(word[1],"add"))
	{
		write_user(user,"~FGEnter the name of the talker~CB:~RS ");
		user->misc_op = 25;
		return;
	}
	writeus(user,usage,command[com_num].name,"~CB[~CRlist~CW|~CRadd~CB]");
	return;
}

/* Lets a user exchange the color of their hat for a new one */
void hat_colour(User user,char *inpstr,int rt)
{
	int which;

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
	which = 0;
	if (word_count < 2)
	{
		for (which = 1 ; which < NumHats ; ++which)
		{
			writeus(user,"  %2d~CB = %sthis colour\n",which,hatcolor[which]);
		}
		writeus(user,usage,command[com_num].name,"~CB[~CR# of hat you want~CB]");
		return;
	}
	which = atoi(word[1]);
	if (user->hat == 0)
	{
		write_user(user,"~FT You don't even have a hat to trade in.\n");
		return;
	}
	if (which < 1)
	{
		write_user(user,"~FT That's an invalid hat colour.\n");
		return;
	}
	if (which>(NumHats-1))
	{
		write_user(user,"~FT That is an invalid hat colour.\n");
		return;
	}
	user->hat = which;
	writeus(user,"~FG Your hat is now this %scolour~FG.\n",hatcolor[user->hat]);
}

/* Lets a user see what the current session is */
void check_session(User user,char *inpstr,int rt)
{
	int hours,mins,secs,total;
	char filename[FSL],*name;

	if (user->invis == Invis) name = invisname; else name = user->recap;
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
		if (session[0])
		{
			writeus(user,"~FG The current session is~CB: [~CR%s~CB]\n",session);
			if (user->ansi_on) write_user(user,ansi_tline);
			else write_user(user,ascii_tline);
			sprintf(filename,"%s/sessionfile",MiscDir);
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,center("~FR Noone has commented on the current session!\n",80));
				  return;
				case 1:
				  user->misc_op=2;
			}
			return;
		}
		else
		{
			write_user(user,"~CY It looks as though there is no current session.\n");
			return;
		}
	}
	if (!strcasecmp(word[1],"-who"))
	{
		if (session[0])
		{
			total = time(0)-sessiontime;
			hours = total/3600;
			mins  = (total%3600)/60;
			secs  = (total%60)/60;
			if (hours)
			{
				sprintf(text,"~CT %s ~FTset the session ~FY%d ~FThour%s, ~FY%d ~FTminute%s, and ~FY%d~FT second%s ago.\n",sessionwho,hours,hours == 1?"":"s",mins,mins == 1?"":"s",secs,secs == 1?"":"s");
			}
			else
			{
				if (mins == 0) sprintf(text,"~CT %s ~FTset the session ~FY%d ~FT second%s ago.\n",sessionwho,secs,secs == 1?"":"s");
				else sprintf(text,"~CT %s ~FTset the session ~FY%d ~FTminute%s, and ~FY%d~FT second%s ago.\n",sessionwho,mins,mins == 1?"":"s",secs,secs == 1?"":"s");
			}
			write_user(user,text);
			return;		
		}
		else
		{
			write_user(user,"~CY It looks as though there is no current session.\n");
			return;
		}
	}
	if (time(0)<sessiontime+600 && strcmp(user->name,sessionwho))
	{
		total = (sessiontime+600)-time(0);
		secs = total%60;
		mins = (total-secs)/60;
		writeus(user,"~FT The session may be reset in~CB: [~FY%d ~FTminute%s and ~FY%d ~FTsecond%s.~CB]\n",mins,mins>1?"s":"",secs,secs>1?"s":"");
		return;
	}
	if (strlen(inpstr)>100)
	{
		write_user(user,"~FG That session is a tad long don't ya think?\n");
		return;
	}
	strcpy(session,inpstr);
	sprintf(filename,"%s/sessionfile",MiscDir);
	unlink(filename);
	writeus(user,"~FG You set the session to~CB: [~FM%s~CB]\n",inpstr);
	sprintf(text,"~FT-~CT=~CB> ~FG%s ~FMhas set the session to~CB: ~FM%s ~CB<~CT=~FT-\n",name,inpstr);
	write_level(USER,user,center(text,80));
	strcpy(sessionwho,user->name);
	sessiontime = time(0);
}

/* Lets a user comment on the session */
void comment(User user,char *inpstr,int rt)
{
	FILE *fp;
	int total_len,cur_len,line_pos,clr_count;
	char filename[FSL],*name;

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
	if (session[0] == '\0')
	{
		write_user(user,"~FG There isn't currently a session...\n");
		return;
	}
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRyour comment on the session~CB]");
		return;
	}
	sprintf(filename,"%s/sessionfile",MiscDir);
	if ((fp = fopen(filename,"a")) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred.. try again in a bit.\n");
		return;
	}
	sprintf(text,"  ~FG%-*s  ~CB: ~FM%s\n\r",UserNameLen+1,user->name,ColourStrip(inpstr));
	line_pos = 0;
	clr_count = colour_com_count(text);
	for (cur_len = 0,total_len = (strlen(text) - clr_count) ; cur_len<total_len ; cur_len++,line_pos++)
	{
		putc(text[cur_len],fp);
		if (line_pos%80 == 0 && line_pos != 0)
		{
			putc('\n',fp);
			putc('\r',fp);
		fprintf(fp,"                   ~FM");
		line_pos = 18;
		}
	}
	FCLOSE(fp);
	writeus(user,"~FG Your comment to the session~CB: ~RS%s\n",inpstr);
	WriteLevel(USER,user,"~CB[~FMChannel~CB] ~CT%s ~CYhas .comment'd on the session.\n",name);
}

/*
   Lets a user see what aliases they have, or set them... No longer based
   on the Amnuts alias' figured they should be real alias' so a user can
   virtually make up their own commands. :-) So now.. more based on the
   look and feel of the PARIS macro system... Also I figured I would add
   some extra options, such as $1 $2..
*/
void aliases(User user,char *inpstr,int rt)
{
	Socials s;
	Commands c;
	char aliasname[21];
	int i,cnt,found;
	size_t x = 0;

	i = cnt = found = 0;
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
		write_user(user,"~CW- ~FTYour current list of aliases~CB:\n");
		write_user(user,"~CB[~CR##~CB]~FT Name ~FM=-=-=-=-=-=-=-=-= ~FTAlias\n");
		for (i = 0; i < 25; ++i)
		{
			if (user->alias.find[i][0] == '\0') continue;
			writeus(user,"~CB[~CR%2d~CB] ~FT%-20s ~FM= ~FT%s\n",i,user->alias.find[i],user->alias.alias[i]);
			++cnt;
		}
		if (user->level < POWER) writeus(user,"~CW- ~CR%d~FT alias' set out of %d max.\n",cnt,(25-10));
		else writeus(user,"~CW- ~CR%d~FT aliases set out of %d max.\n",cnt,25);
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"");
		writeus(user,usage,command[com_num].name,"~CB[~CRalias name~CB] [~CRalias~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CR# of alias~CB] ~CR-delete");
		write_user(user,"~CW - ~FGFor more help, see ~CW'~CR.help alias~CW'\n");
		return;
	}
	/* See if we're deleting one. */
	if (isdigit(word[1][0]) && !strncasecmp(word[2],"-delete",2))
	{
		i = atoi(word[1]);
		if (i<0 || i>25 || user->alias.find[i][0] == '\0')
		{
			write_user(user,"~CW- ~FTYou don't have an alias set for that number.\n");
			return;
		}
		user->alias.find[i][0] = '\0';
		user->alias.alias[i][0]='\0';
		writeus(user,"~CW-~FT Alias number %d has been deleted.\n",i);
		return;
	}
	/* Make sure it's valid. */
	for (x = 0; x < strlen(word[1]); ++x)
	{
		if (!isalpha(word[1][x]))
		{
			writeus(user,"~CW- ~CR%s~FT is an invalid alias name.\n",word[1]);
			return;
		}
	}
	for_comms(c)
	{
		if (!strcasecmp(word[1],c->name))
		{
			found = 1;
			break;
		}
	}
	if (found == 0)
	{
		for_socials(s)
		{
			if (!strcasecmp(word[1],s->name))
			{
				found = 1;
				break;
			}
		}
	}
	if (found)
	{
		writeus(user,"~CW- ~FT%s ~FGis already a command or social name...\n",word[1]);
		return;
	}
	for (i = 0; i < 25; ++i)
	{
		if (!strcmp(user->alias.find[i],word[1]))
		{
			writeus(user,"~CW- ~FGYou already have an alias set for ~CR%s\n",word[1]);
			return;
		}
	}
	if (strlen(word[1])>20)
	{
		writeus(user,"~CW- ~FT%s is too long of an alias name.\n",word[1]);
		return;
	}
	strncpy(aliasname,word[1],sizeof(aliasname)-1);
	inpstr = remove_first(inpstr);
	if (strlen(inpstr)>sizeof(user->alias.alias[0])-1)
	{
		writeus(user,"~CW- ~FT%s ~FGis too long of an alias.\n",word[2]);
		return;
	}
	/* Find a free slot. */
	for (i = 0; i < 25; ++i)
	{ 
		if (user->alias.find[i][0] == '\0') break;
	}
	if (user->level < POWER && i >= (25-10))
	{
		write_user(user,"~CW- ~FTYou already have the max alias' set for your level.\n");
		return;
	}
	if (i > 25)
	{
		write_user(user,"~CW- ~FTYou already have the max amount alias' set.\n");
		return;
	}
	strncpy(user->alias.find[i],aliasname,sizeof(user->alias.find[0])-1);
	strncpy(user->alias.alias[i],inpstr,sizeof(user->alias.alias[0])-1);
	writeus(user,"~CW- ~FTYou set an alias for ~FR%s ~FTto execute ~FR%s.\n",user->alias.find[i],user->alias.alias[i]);
	return;
}

/*
   lets a user netsex another user. This gets rather hardcore..thats why I
   put in the age check
*/
void net_sex(User user,char *inpstr,int rt)
{
	User u;
	char *name;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if (user->age < 18)
	{
		write_user(user,"~FG Sorry you are not of legal age to use this command!\n");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u->age < 18)
	{
		write_user(user,"~FG Sorry, that user is under age...I can't risk lawsuits. :P\n");
		return;
	}
	if (user->invis == Invis) name = invisname; else name = user->recap;
	if (u == user)
	{
		write_user(user,"~FG Ewwww. Please...take that to a different talker you sicko! :P\n");
		WriteRoomExcept(user->room,user,"~FM %s~FG just tried to netsex themself!!!!!!!!\n",name);
		return;
	}
	if (u->gender == 0)
	{
		writeus(user,"~FT You should tell ~FR%s~FT to set their gender.\n",u->recap);
		return;
	}
	if (is_enemy(u,user) && user->level<OWNER)
	{
		write_user(user,"~FG That person does not want any netsex from you.\n");
		return;
	}
	if ((u->ignore & Tells) && user->level<GOD)
	{
		writeus(user,"~FT%s ~FRis ignoring private messages right now...\n",u->recap);
		return;
	}
	if (u->afk)
	{
		if (u->afk_mesg[0]) sprintf(text,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]eyboard~CB:~FT %s\n",u->recap,u->afk_mesg);
		else  sprintf(text,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard at the moment\n",u->recap);
		write_user(user,text);
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s ~FGis using the editor at the moment...\n",u->recap);
		return;
	}
	if (u->ignall && user->level<OWNER)
	{
		writeus(user,"~FT%s ~FRis ignoring everyone at the moment!\n",u->recap);
		return;
	}
	if (user->gender==Female)	/* Woman using the .netsex command */
	{
		if (u->gender==Female)	/* mmmmm 2 women ;) */
		{
			writeus(u,"~FT%s ~FTslowly slides your shirt over your head, massaging your breasts gently\n",name);
			write_user(u,"~FTas she brings your shirt over your head. From there she reaches around your\n");
			write_user(u,"~FTback and slides your bra off. Bringing a mouth to one of your breasts, taking\n");
			write_user(u,"~FTyour nipple into your mouth, tugging gently on it with her teeth. She reaches\n");
			write_user(u,"~FTdown and begins to rub between your legs, feeling the dampness begin to\n");
			writeus(u,"~FTto show through your pants.  From there, %s ~FTtugs off your pants and underwear\n",name);
			write_user(u,"~FTand begins to rub more faster, trying to bring you to a climax.  Feeling\n");
			write_user(u,"~FTyourself start to give a little, you let out a soft moan.  Hearing this moan,\n");
			writeus(u,"~FT%s ~FTmoves her head down, kissing your body as you go down. Once she reaches\n",name);
			write_user(u,"~FTyour damp cunt, she leans forward and kisses the hood of your clit.  From there\n");
			write_user(u,"~FTshe sticks a finger deep inside of you, while teasing your clit with her\n");
			write_user(u,"~FTtongue.  Feeling more bold, she places another finger inside of you and\n");
			write_user(u,"~FTproceeds to fuck you with her fingers, bringing you to an orgasm on her face!\n");
			write_user(user,"~FG Your erotic netsex has been sent.\n");
			return;
		}
		if (u->gender==Male)	/* Female performing netsex on a guy */
		{
			if (user->condoms == 0)
			{
				write_user(user,"~FG You need condoms in order to have sex..we can't risk pregnancy!!\n");
				return;
			}
			writeus(u,"~FT%s ~FTapproaches you with a sly gleam in her eyes, walking towards you with\n",name);
			write_user(u,"~FTher hips swinging from left to right.  As she pauses in front of you, she\n");
			write_user(u,"~FTleans forward and pulls her shirt over her, giving you an eyeful of her supple\n");
			write_user(u,"~FTbreasts.  She stops right in front of you, and kneels down before you. She\n");
			write_user(u,"~FTslowly undoes the belt in which holds your pants up, and lets them drop from\n");
			write_user(u,"~FTyour waste as she stairs in marvel at the size of which you offer her.  She \n");
			write_user(u,"~FTkisses the head of your solid cock, pushes you down, so you are in the sitting\n");
			write_user(u,"~FTposition, and from there, she straddles your lap, and slowly places your\n");
			write_user(u,"~FTmanhood inside of her.  She starts riding you more and more wild, making your\n");
			write_user(u,"~FTorgasm build up deep inside your balls, until at last you let forth a stream\n");
			write_user(u,"~FTof cum deep inside of her!\n");
			write_user(user,"~FG Your erotic netsex has been sent!\n");
			user->condoms--;
			return;
		}
		return;
	}
	if (user->gender == Male)	/* Its a male user .netsexing someone */
	{
		if (user->condoms == 0)
		{
			write_user(user,"~FG You need condoms in order to have sex..we can't risk pregnancy or disease...\n");
			return;
		}
		if (u->gender==Male)	/* Ugh... no comment */
		{
			/* don't want no guys netsexing me... DO NOT REMOVE! */
			if (!strcasecmp(u->name,"squirt"))
			{
				write_user(user,"~FG Sorry, Squirt ain't into the man on man type thing.\n");
				return;
			}
			writeus(u,"~FT%s ~FTwinks at you from across the room as he walks towards you, rubbing\n",name);
			write_user(u,"~FThis crotch, while licking his lips as he approaches you.  He runs his hands\n");
			write_user(u,"~FTalong your firm chest, bringing them lower towards your semi-hard cock, tugging\n");
			write_user(u,"~FTit roughly through your jeans.  He drops down to his knees, and kisses it\n");
			write_user(u,"~FTthrough the fabric of your jeans, making your dick twitch slightly.  He reaches\n");
			write_user(u,"~FTup and undoes the belt in which holds your jeans in place and watches in awe\n");
			write_user(u,"~FTas they drop down, revealing your rock hard boner.  He moves to engulf it into\n");
			write_user(u,"~FThis mouth, and takes the whole thing in, sucking hard, trying to squeeze you\n");
			write_user(u,"~FTdry.  From their he puts one of his condoms on your cock, and guides you into\n");
			write_user(u,"~FThis eager asshole, and starts milking your cock for all its worth until you\n");
			write_user(u,"~FTspunk a mighty load deep inside his eager ass.\n");
			write_user(user,"~FG Your erotic netsex has been sent!\n");
			user->condoms--;
			return;
		}
		if (u->gender == Female)  /* much much much much better ;-) */
		{
			writeus(u,"~FT%s ~FTwalks up to you, and whispers in your ear of how much he would like to\n",name);
			write_user(u,"~FTget to know you on a more personal level.  So you get up and lead him to a more\n");
			write_user(u,"~FTprivate area, where it would be just the two of you.  In that area, you watch\n");
			write_user(u,"~FThim as he slowly peels off his clothes for you in a most erotic way.  Just\n");
			write_user(u,"~FTbefore he removes his shorts, he approaches you, and places your hand on the\n");
			write_user(u,"~FTbulge which is concealed underneath them. You stroke it for a second or two,\n");
			write_user(u,"~FTbefore he finally steps out of his shorts standing there as naked as he was\n");
			write_user(u,"~FTthe day he was born.  He then slowly slides your shirt over your head, and\n");
			write_user(u,"~FTunclasps the bra in which holds your beatiful bouncy breasts up.  From there\n");
			write_user(u,"~FThe slides a condom slowly over his cock, and moves between your legs, and tugs\n");
			write_user(u,"~FTdown on your pants, freeing them and your sweet and wet cunt.  He nudges his\n");
			write_user(u,"~FTmanhood slowly into you at first, then thrusts into you all at once, making a\n");
			write_user(u,"~FTsmall moan come out of your lips.  He continues pushing himself in and out of\n");
			write_user(u,"~FTyou until finally he lets loose a load of cum...\n");
			write_user(user,"~FG Your erotic netsex has been sent!\n");
			user->condoms--;
			return;
		}
		return;
	}
}

void staff_list(User user,char *inpstr,int rt)
{
	Ulobj	ul;
	int	cnt = 0, i = 0;
	char	temp[81],lev_clr[5];

	sprintf(file,"stafflist");
	show_screen(user);
	for_list(ul)
	{
		if (ul->lev < WIZ) continue;
		++cnt;
	}
	if (cnt == 0)
	{
		write_user(user,"~CW-~FT There are no staff members at this time.\n");
		return;
	}
	write_user(user,"~FM,-----------------.------------.     .-----------------.------------.\n");
	write_user(user,"~FM|~FT Name~CB:           ~FM|~FG Level~CB:~FM     |     | ~FTName~CB:           ~FM|~FG Level~CB:     ~FM|\n");
	write_user(user,"~FM|~CB --------------- ~FM|~CB ---------- ~FM|     | ~CB--------------- ~FM| ~CB---------- ~FM|\n");
	cnt = 0;
	memset(text,0,sizeof(text)-1);
	for_list(ul)
	{
		if (ul->lev < WIZ) continue;
		switch (ul->lev)
		{
			case WIZ  :
			  strncpy(lev_clr,"~FR",sizeof(lev_clr)-1);
			  break;
			case ADMIN:
			  strncpy(lev_clr,"~FT",sizeof(lev_clr)-1);
			  break;
			case GOD  :
			  strncpy(lev_clr,"~FY",sizeof(lev_clr)-1);
			  break;
			case OWNER:
			  strncpy(lev_clr,"~RD",sizeof(lev_clr)-1);
			  break;
			default	  :
			  strncpy(lev_clr,"~FT",sizeof(lev_clr)-1);
			  break;
		}
		sprintf(temp,"~FM| %s%-15.15s ~FM| ~FG%-10.10s ~FM|",lev_clr,ul->name,level[ul->lev].name);
		strcat(text,temp);
		++cnt;
		if (cnt%1 == 0)
			strcat(text,"     ");
		if (cnt%2 == 0 && cnt != 0)
		{
			strcat(text,"\n");
			write_user(user,text);
			for (i = 0; i < sizeof(text)-1; ++i) text[i] = '\0';
			for (i = 0; i < sizeof(temp)-1; ++i) temp[i] = '\0';
		}
	}
	if (cnt%2 != 0)
	{
		write_user(user,text);
		write_user(user,"~FM|                 |            |\n");
	}
	write_user(user,"~FM`-----------------'------------'     `-----------------'------------'\n");
	writeus(user,"~CW-~FT There %s a total of %d staff member%s.\n",cnt == 1?"is":"are",cnt,cnt == 1?"":"s");
	return;
}

/* END OF GENERAL.C */
