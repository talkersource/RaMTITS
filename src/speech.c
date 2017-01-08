/*
   speech.c
    The source in this file contains the functions for all of the speech
    commands, such as .tell, .say, .echo, etc.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Mike Irving, Neil Robertson, Andrew Collington,
	and Cusack.

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
#define _SPEECH_C
  #include "include/needed.h"
#undef _SPEECH_C

/*
   I have organized these commands via normal says, shouts, or private mesg's
*/
/* Lets start with the normal says and shit first */
void say(User user,char *inpstr,int rt)
{
	Room rm;
	char *type,*name,*tstr;

	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (user->status & Gagged)
	{
		write_user(user,center("~FT-~CT=~CB> ~FTYou're gagged, you can't do ANYTHING... ~CB<~CT=~FT-\n",80));
		return;
	}
	if ((word_count<2 && user->mode) || !strlen(inpstr))
	{
		writeus(user,usage,command[com_num].name[0]=='\0'?".say":command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	tstr = ColourStrip(inpstr);
	type = smiley_type(tstr,1);
	if (user->talk)
	{
		if (user->mode == 0 && istrstr(word[0],"say"))
		{
			send_message(user,type,inpstr);
			return;
		}
	}
	rm = user->room;
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CY%s ~CG%ss%s~RS %s\n",name,type,user->sstyle,inpstr);
		else sprintf(text,"~CY%s ~CG%ss%s~RS %s%s\n",name,type,user->sstyle,hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if (user->type == CloneType)
	{
		if (!user->hat) writeus(user,"~CB[~FM%s~CB]: ~CY%s ~CG%ss%s~RS %s\n",user->room->recap,name,type,user->sstyle,inpstr);
		else writeus(user,"~CB[~FM%s~CB]: ~CY%s ~CG%ss%s~RS %s%s\n",user->room->recap,name,type,user->sstyle,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) writeus(user,"~CY%s ~CG%ss%s~RS %s\n",name,type,user->sstyle,inpstr);
		else writeus(user,"~CY%s ~CG%ss%s~RS %s%s\n",name,type,user->sstyle,hatcolor[user->hat],inpstr);
	}
	if (!user->hat) sprintf(text,"~CY%s ~CG%ss%s~RS %s\n",name,type,user->sstyle,inpstr);
	else sprintf(text,"~CY%s ~CG%ss%s~RS %s%s\n",name,type,user->sstyle,hatcolor[user->hat],inpstr);
	write_room_except(user->room,user,text);
	record(user->room,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(user->room,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Emote something -- added a small patch so it can see "'"'s */
void emote(User user,char *inpstr,int rt)
{
	Room rm;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRemoted message~CB]");
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->talk)
	{
		if (user->mode == 0 && istrstr(word[0],"emote"))
		{
			send_message(user,NULL,inpstr);
			return;
		}
	}
	rm = user->room;
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (user->status & Silenced)
	{
		if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
		  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
		{
			if (!user->hat) sprintf(text,"~FM%s~FT%s\n",name,inpstr);
			else sprintf(text,"~FM%s~FT%s%s\n",name,hatcolor[user->hat],inpstr);
		}
		else
		{
			if (!user->hat) sprintf(text,"~FM%s ~FT%s\n",name,inpstr);
			else sprintf(text,"~FM%s ~FT%s%s\n",name,hatcolor[user->hat],inpstr);
		}
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
	  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
	{
		if (!user->hat) sprintf(text,"~FM%s~FT%s\n",name,inpstr);
		else sprintf(text,"~FM%s~FT%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~FM%s ~FT%s\n",name,inpstr);
		else sprintf(text,"~FM%s ~FT%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	record(rm,text);
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s",user->room->recap,text);
	else write_user(user,text);
	write_room_except(rm,user,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(rm,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* echo something */
void send_echo(User user,char *inpstr,int rt)
{
	Room rm;
	User u;

	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRechoed text~CB]");
		return;
	}
	for_users(u)
	{
		if (u == user) continue;
		if (u->login || u->type == CloneType) continue;
		if (istrstr(ColourStrip(inpstr),"http://")) continue;
		if (istrstr(ColourStrip(inpstr),u->name))
		{
			write_user(user,"~FM You cannot echo other users names.\n");
			return;
		}
	}
	rm = user->room;
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CW-~RS %s\n",inpstr);
		else sprintf(text,"~CW- %s%s\n",hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	write_monitor(user,0,user->room);
	if (!user->hat) sprintf(text,"~CW-~RS %s\n",inpstr);
	else sprintf(text,"~CW- %s%s\n",hatcolor[user->hat],inpstr);
	record(rm,text);
	write_user(user,text);
	write_room_except(rm,user,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(rm,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Think something */
void think(User user,char *inpstr,int rt)
{
	Room rm;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	alter_speech(inpstr,user->speech);
	rm = user->room;
	if (word_count < 2)
	{
		if (user->status & Silenced)
		{
			sprintf(text,"~FG%s ~FTattempts to think of something...but fails...as always.\n",name);
			if (user->type == CloneType)
				writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
			else
				write_user(user,text);
			return;
		}
		else
		{
			sprintf(text,"~FG%s ~FTattempts to think of something...but fails...as always.\n",name);
			record(rm,text);
			if (user->type == CloneType)
				writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
			else
				write_user(user,text);
			write_room_except(rm,user,text);
			return;
		}
	}
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~FY%s~FG thinks ~FT. o O (~FY %s ~FT)\n",name,inpstr);
		else sprintf(text,"~FY%s~FG thinks ~FT. o O ( %s%s ~FT)\n",name,hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~FY%s~FG thinks ~FT. o O (~FY %s ~FT)\n",name,inpstr);
	else sprintf(text,"~FY%s~FG thinks ~FT. o O ( %s%s ~FT)\n",name,hatcolor[user->hat],inpstr);
	record(rm,text);
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
	else
		write_user(user,text);
	write_room_except(rm,user,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(rm,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Sing something */
void sing(User user,char *inpstr,int rt)
{
	Room rm;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	rm = user->room;
	if (word_count < 2)
	{
		if (user->status & Silenced)
		{
			sprintf(text,"~FG%s ~FTbreaks out in song... making everyone run for their earplugs.\n",name);
			if (user->type == CloneType)
				writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
			else
				write_user(user,text);
			return;
		}
		else
		{
			sprintf(text,"~FG%s ~FTbreaks out in song... making everyone run for their earplugs.\n",name);
			if (user->type == CloneType)
				writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
			else
				write_user(user,text);
			write_room_except(rm,user,text);
			record(rm,text);
			return;
		}
	}
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~FY%s~FG sings ~FTo/~(~FR %s ~FT)~\\o\n",name,inpstr);
		else sprintf(text,"~FY%s~FG sings ~FTo/~( %s%s ~FT)~\\o\n",name,hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~FY%s~FG sings ~FTo/~(~FR %s ~FT)~\\o\n",name,inpstr);
	else sprintf(text,"~FY%s~FG sings ~FTo/~( %s%s ~FT)~\\o\n",name,hatcolor[user->hat],inpstr);
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
	else
		write_user(user,text);
	write_room_except(rm,user,text);
	record(rm,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(rm,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* direct a comment to another user in the room */
void to_user(User user,char *inpstr,int rt)
{
	Room rm;
	User u;
	char *type,*name,*rname,*tstr;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count<3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRmesssage~CB]");
		return;
	}
	if ((u = get_clone_here(user->room,word[1])) == NULL)
	{
		if ((u = get_name(user,word[1])) == NULL)
		{
			write_user(user,center(notloggedon,80));
			return;
		}
	}
	if (u == user)
	{
		write_user(user,"~FM Why the heck are you trying to direct a comment to yourself?\n");
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (u->invis == Invis) rname = invisname;  else rname = u->recap;
	if (u->afk)
	{
		if (u->afk_mesg[0]) writeus(user,"~CW- ~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard~CB:~FT %s\n",rname,u->afk_mesg);
		else writeus(user,"~CW- ~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard at the moment!\n",rname);
		write_user(user,"~CW- ~FGIf you would like to leave them a message ~CW'~FR.tell~CW'~FG them!\n");
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~CW- ~FT%s~FG is using the editor at the moment...\n",u->recap);
		write_user(user,"~CW- ~FGIf you would like to leave them a message ~CW'~FR.tell~CW'~FG them!\n");
		return;
	}
	rm = user->room;
	if (rm != u->room)
	{
		writeus(user,"~FT %s ~FGisn't in the same room as you.\n",u->recap);
		write_user(user,text);
		return;
	}
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = remove_first(inpstr);
	tstr = ColourStrip(inpstr);
	type = smiley_type(tstr,3);
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~FM%s ~FR%s ~FM%s%s~FY %s\n",name,type,rname,user->sstyle,inpstr);
		else sprintf(text,"~FM%s ~FR%s ~FM%s%s~FY %s%s\n",name,type,rname,user->sstyle,hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~FM%s ~FR%s ~FM%s%s~FY %s\n",name,type,rname,user->sstyle,inpstr);
	else sprintf(text,"~FM%s ~FR%s ~FM%s%s~FY %s%s\n",name,type,rname,user->sstyle,hatcolor[user->hat],inpstr);
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
	else
		write_user(user,text);
	write_room_except(user->room,user,text);
	record(rm,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(rm,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Show a command */
void show(User user,char *inpstr,int rt)
{

	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRcommand~CB]");
		return;
	}
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~FMType ~FT--~FG==~CB> ~RS%s\n",inpstr);
		else sprintf(text,"~FMType ~FT--~FG==~CB> %s%s\n",hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~FMType ~FT--~FG==~CB> ~RS%s\n",inpstr);
	else sprintf(text,"~FMType ~FT--~FG==~CB> %s%s\n",hatcolor[user->hat],inpstr);
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
	else
		write_user(user,text);
	write_room_except(user->room,user,text);
	record(user->room,text);
}

/* write something to everyone in the room except for a given user */
void mutter(User user,char *inpstr,int rt)
{
	Room rm;
	User u;
	char *name,*rname;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CR-male~CW|~CR-female~CB] [~CRmessage~CB]");
		return;
	}
	rm = user->room;
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = remove_first(inpstr);
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (!strncasecmp(word[1],"-male",2))
	{
		for_users(u)
		{
			if (u->login || u->room != user->room
			  || u->gender == Male || is_enemy(u,user)
			  || u == user || u->type == CloneType) continue;
			if (!user->hat) sprintf(text,"~CM%s ~FRmutters to females only%s~RS %s\n",name,user->sstyle,inpstr);
			else sprintf(text,"~CM%s ~FRmutters to females only%s %s%s\n",name,user->sstyle,hatcolor[user->hat],inpstr);
			write_user(u,text);
			if (user->speech == 1)
			{
				alter_speech(inpstr,1);
				writeus(u,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
			}
		}
		if (!user->hat) sprintf(text,"~FR You mutter to the females only%s ~RS%s\n",user->sstyle,inpstr);
		else sprintf(text,"~FR You mutter to the females only%s %s%s\n",user->sstyle,hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	else if (!strcasecmp(word[1],"-female"))
	{
		for_users(u)
		{
                	if (u->login || u->room != user->room
			  || u->gender == Female || is_enemy(u,user)
			  || u == user || u->type == CloneType) continue;
			if (!user->hat) sprintf(text,"~CM%s ~FRmutters to males only%s~RS %s\n",name,user->sstyle,inpstr);
			else sprintf(text,"~CM%s ~FRmutters to males only%s %s%s\n",name,user->sstyle,hatcolor[user->hat],inpstr);
			write_user(u,text);
			if (user->speech == 1)
			{
				alter_speech(inpstr,1);
				writeus(u,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
			}
		}
		if (!user->hat) sprintf(text,"~FR You mutter to the males only%s ~RS%s\n",user->sstyle,inpstr);
		else sprintf(text,"~FR You mutter to the males only%s %s%s\n",user->sstyle,hatcolor[user->hat],inpstr);
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u->invis == Invis) rname = invisname; else rname = u->recap;
	if (u == user)
	{
		write_user(user,"~FM You have problems....muttering about yourself..\n");
		return;
	}
	if (u->room != user->room)
	{
		write_user(user,"~CW-~FT It usually helps if the user is in the same room as you.\n");
		return;
	}
	if (!user->hat) sprintf(text,"~CM%s ~FRmutters to everyone but ~CT%s%s~RS %s\n",name,rname,user->sstyle,inpstr);
	else sprintf(text,"~CM%s ~FRmutters to everyone but ~CT%s%s %s%s\n",name,rname,user->sstyle,hatcolor[user->hat],inpstr);
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
	else
		write_user(user,text);
	u->room = NULL;     /* So we don't write to the mutter to user. */
	write_room_except(user->room,user,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(user->room,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
	u->room = rm;      /* Give them their room back. */
}

/*
   Sends a tell/pemote/pthink/psing to multiple users.. it's all in one comm
   just because having a bunch of different commands for it is rather point-
   less. I do have to admit, this code isn't entirely mine.. I just mainly
   made some bug fixes and modifications to a great piece of code. (mods I
   made are I made it for more then one command (.tell, .pemote, .pthink etc
   and fixed a nasty bug that would cause it to crash w/ a bad amount of
   arguments given to the command.)
*/
void multi_message(User user,char *str,int type)
{
	User u[10];
	char names[10][250],*type1,*type2,*tstr,save[ARR_SIZE*2];
	char list[10*(250)-2],*msg,*substr,*name,*token=NULL,temp[80],temp2[80];
	int i = 0,cnt = 0, sent_busy = 0;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	type1 = NULL;  type2 = NULL;
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
		revtell(user,NULL,0);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (type == 1)
	{
		tstr = ColourStrip(str);
		type1 = smiley_type(tstr,2);
		type2 = smiley_type(tstr,3);
	}
	switch (type) /* We use this switch to find out which cmd is used. */
	{
		case 1: /* .tell   */
		  sprintf(temp,"~CB:: ~FY%s ~FG%s you%s",name,type2,user->sstyle);
		  break;
		case 2: /* .pemote */
		  sprintf(temp,"~CB:: ~FY%s",name);
		  break;
		case 3: /* .pthink */
		  sprintf(temp,"~CB:: ~FY%s~FG thinks ~FT. o O (",name);
		  break;
		case 4: /* .psing  */
		  sprintf(temp,"~CB:: ~FY%s~FG sings o/~(",name);
		  break;
	}
	strcpy(save,str);
	for (i = 0; i < 10; i++) names[i][0] = '\0';
	for (i = 0; i < 10; i++) u[i] = NULL;
	if (!(substr = (char *)strtok(save," ")))
	{
		writeus(user,"~FG Check out ~CW'~CR.h .%s~CW' ~FGfor help with this command.\n",command[com_num].name);
		return;
	}
	msg = (substr+strlen(substr)+1);  while (isspace(*msg)) msg++;
	if (!strlen(msg))
	{
		write_user(user,"~FG What exactly do you want to send to them?\n");
		return;
	}
	if (!(token=(char *)strtok(substr,",")))
	{
		write_user(user,"~FG You kind of need who you want to specify the message to.\n");
		return;
	}
	i = 0;
	do
	{
		int j,dupe = 0;
		for (j = 0; j < i; j++) if (!strcasecmp(names[j],token)) dupe++;
		if (!dupe) strcpy(names[i++],token);
	} while (((token = (char *)strtok(NULL,", ")) != NULL) && (i<10));
	cnt = i;
	for (i = 0; i < 10; i++)
	{
		if (strlen(names[i]))
		{
			if ((u[i] = get_name(user,names[i])) == NULL)
			{
				write_user(user,center(notloggedon,80));
				cnt--;
			}
			else if (!strcasecmp(u[i]->name,user->name))
			{
				write_user(user,"~FG Don't be silly, you can't send private messages to yourself.\n");
				cnt--;
				u[i] = NULL;
			}
			else
			{
				int j,k;
				for (j = 0; j < 10; j++)
				{
					if (u[j])
					{
						for (k = j+1; k < 10; k++)
						{
							if (u[k]
							  && u[j] == u[k])
							{
								u[k] = NULL;
								cnt--;
							}
						}
					}
				}
			}
		}
	}
	for (i = 0; i < 10; i++)
	{
		if (u[i])
		{
			if (is_enemy(u[i],user) && user->level<OWNER)
			{
				writeus(user,"~CR %s ~FGwishes not to talk to you.\n",u[i]->recap);
				u[i] = NULL;
				cnt--;
			}
			else if (is_enemy(user,u[i]) && user->level<OWNER)
			{
				writeus(user,"~FG What's the point of having %s as an enemy if you want to talk to %s\n",u[i]->recap,gender2[u[i]->gender]);
				u[i] = NULL;
				cnt--;
			}
			else if ((u[i]->ignore & Tells) && user->level<OWNER)
			{
				writeus(user,"~CR %s~FG is ignoring private messages at the moment..\n",u[i]->recap);
				u[i] = NULL;
				cnt--;
			}
			else if (is_busy(u[i]))
			{
				writeus(user,"~FT %s ~FGis currently busy at the moment....\n",u[i]->recap);
				writeus(user,"~FG But the message has been recorded and %s will be told to .rev -t\n",gender3[u[i]->gender]);
				if (user->status & Silenced) return;
				if (type != 2) sprintf(text,"%s %s%s",temp,user->hat?hatcolor[user->hat]:"~RD",msg);
				else
				{
					if ((strchr("\'",msg[0]) || strchr("\'",msg[1]))
					  || (strchr("~",msg[0]) && strchr("\'",msg[3])))
					  sprintf(text,"%s%s%s",temp,user->hat?hatcolor[user->hat]:"~RD",msg);
					else sprintf(text,"%s %s%s",temp,user->hat?hatcolor[user->hat]:"~RD",msg);
				}
				if (type == 3) strcat(text," ~FT)\n");
				else if (type==4) strcat(text," ~FT)~\\o\n");
				else strcat(text,"\n");
				u[i]->chkrev = 1;
				record_tell(u[i],text);
				u[i] = NULL;  cnt--;
				sent_busy = 1;
			}
			else if (u[i]->ignall && user->level<OWNER)
			{
				writeus(user,"~CR %s~FG is ignoring everyone at the moment..\n",u[i]->recap);
				u[i] = NULL;
				cnt--;
			}
		}
	}
	if (cnt == 0)
	{
		if (sent_busy == 1) return;
		writeus(user,"~FG Failed to send multiple messages.. check ~CW'~CR.help %s~CW'\n",command[com_num].name);
		return;
	}
	if (cnt >= 2)
	{
		char *pos = (char *)&list;
		*pos++ = '(';
		for (i = 0; i < 10; i++)
		{
			if (u[i])
			{
				strcpy(pos,u[i]->recap);
				pos += strlen(u[i]->recap);
				strcpy(pos,", ");
				pos += 2;
			}
		}
		*(pos-2) = ')';
		*(pos-1) = '\0';
	}
	else
	{
		char *pos = (char *)&list;
		int i = 0,found = 0;
		while (i<10 && !found)
		{
			if (u[i])
			{
				found++;
				strcpy(pos,u[i]->recap);
				pos += strlen(u[i]->recap);
				pos = '\0';
			}
			else i++;
		}
	}
	alter_speech(msg,user->speech);
	if (user->drunk >= 10) alter_speech(msg,5);
	msg = replace_macros(user,msg);
	switch (type)
	{
		case 1:
		  sprintf(temp2,"~CB:: ~FTYou %s ~FG%s%s",type1,list,user->sstyle);
		  break;
		case 2:
		  sprintf(temp2,"~CB:: ~FTTo ~FG%s%s ~FR%s",list,user->sstyle,name);
		  break;
		case 3:
		  sprintf(temp2,"~CB:: ~FGYou think to ~FY%s ~FT. o O (",list);
		  break;
		case 4:
		  sprintf(temp2,"~CB:: ~FGYou sing to ~FY%s ~FTo/~(",list);
		  break;
	}
	if (user->status & Silenced)
	{
		if (type != 2) sprintf(text,"%s %s%s",temp2,user->hat?hatcolor[user->hat]:"~RD",msg);
		else
		{
			if ((strchr("\'",msg[0]) || strchr("\'",msg[1]))
			  || (strchr("~",msg[0]) && strchr("\'",msg[3])))
			  sprintf(text,"%s%s%s",temp2,user->hat?hatcolor[user->hat]:"~RD",msg);
			else sprintf(text,"%s %s%s",temp2,user->hat?hatcolor[user->hat]:"~RD",msg);
		}
		if (type == 3) strcat(text," ~FT)\n");
		else if (type == 4) strcat(text," ~FT)~\\o\n");
		else strcat(text,"\n");
		write_user(user,text);
		record_tell(user,text);
		return;
	}
	for (i = 0;i < 10; i++)
	{
		if (u[i])
		{
			if (type != 2) sprintf(text,"%s %s%s",temp,user->hat?hatcolor[user->hat]:"~RD",msg);
			else
			{
				if ((strchr("\'",msg[0]) || strchr("\'",msg[1]))
				  || (strchr("~",msg[0]) && strchr("\'",msg[3])))
				  sprintf(text,"%s%s%s",temp,user->hat?hatcolor[user->hat]:"~RD",msg);
				else sprintf(text,"%s %s%s",temp,user->hat?hatcolor[user->hat]:"~RD",msg);
			}
			if (type == 3) strcat(text," ~FT)\n");
			else if (type == 4) strcat(text," ~CT)~\\o\n");
			else strcat(text,"\n");
			write_user(u[i],text);
			record_tell(u[i],text);
		}
	}
	if (type != 2) sprintf(text,"%s %s%s",temp2,user->hat?hatcolor[user->hat]:"~RD",msg);
	else
	{
		if ((strchr("\'",msg[0]) || strchr("\'",msg[1]))
		  || (strchr("~",msg[0]) && strchr("\'",msg[3])))
		  sprintf(text,"%s%s%s",temp2,user->hat?hatcolor[user->hat]:"~RD",msg);
		else sprintf(text,"%s %s%s",temp2,user->hat?hatcolor[user->hat]:"~RD",msg);
	}
	if (type==3) strcat(text," ~FT)\n");
	else if (type==4) strcat(text," ~FT)~\\o\n");
	else strcat(text,"\n");
	write_user(user,text);
	record_tell(user,text);
	return;
}

/* sends an echo to a user...can really screw up the recipient ;) */
void private_echo(User user,char *inpstr,int rt)
{
	User u;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRecho message~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FMUmmm why would you want to echo something to yourself?\n");
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	inpstr = remove_first(inpstr);
	if (is_enemy(u,user) && user->level<OWNER)
	{
		write_user(user,"~FG You are blocked from sending that person private messages!\n");
		return;
	}
	if (u->afk)
	{
		if (u->afk_mesg[0]) writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard~CB:~FT %s\n",u->recap,u->afk_mesg);
		else writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard~FG at the moment!\n",u->recap);
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
		return;
	}
	if (u->ignall && user->level<OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring everyone at the moment.\n",u->recap);
		return;
	}
	if (u->ignore & Tells && user->level<OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring tells right now...\n",u->recap);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CW[ ~FRPrivate echo to ~FG%s ~CW]%s~FT %s\n",u->recap,user->sstyle,inpstr);
		else sprintf(text,"~CW[ ~FRPrivate echo to ~FG%s ~CW]%s %s%s\n",u->recap,user->sstyle,hatcolor[user->hat],inpstr);
		record_tell(user,text);
		write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CW[ ~FRPrivate echo to ~FG%s ~CW]%s~FT %s\n",u->recap,user->sstyle,inpstr);
	else  sprintf(text,"~CW[ ~FRPrivate echo to ~FG%s ~CW]%s %s%s\n",u->recap,user->sstyle,hatcolor[user->hat],inpstr);
	record_tell(user,text);
	write_user(user,text);
	if (u->level >= POWER && u->monitor)
	{
		sprintf(text,"~CB-=[~CT %s~CB ]=-\n",name);
		write_user(u,text);
		record_tell(u,text);
	}
	if (!user->hat) sprintf(text,"~CB:: ~FT%s\n",inpstr);
	else sprintf(text,"~CB:: %s%s\n",hatcolor[user->hat],inpstr);
	write_user(u,text);
	record_tell(u,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		writeus(u,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* lets a user review their tells */
void revtell(User user,char *inpstr,int rt)
{
	int i,cnt = 0;
	char temp[ARR_SIZE*2], search[ARR_SIZE*2];

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	cnt = 0;
	if (!strncasecmp(word[2],"-search",2))
	{
		cnt = 0;
		inpstr = remove_first(inpstr);
		inpstr = remove_first(inpstr);
		strcpy(search,inpstr);
		if (!strlen(inpstr))
		{
			writeus(user,usage,command[com_num].name,"-tells -search ~CB[~CRpattern~CB]");
			return;
		}
		strcpy(search,ColourStrip(search));
		for (i = 0; i < NUM_LINES; ++i)
		{
			temp[0] = '\0';
			if (user->tbuff->conv[i] == NULL) continue;
			strcpy(temp,ColourStrip(user->tbuff->conv[i]));
			if (!istrstr(temp,search)) continue;
			++cnt;
		}
		if (cnt == 0)
		{
			writeus(user,"~CW-~FT No occurrences of ~CW\"~FY%s~CW\"~FT were found in your private msg buffer.\n",search);
			return;
		}
		writeus(user,"~CW-~FT Private message buffer containing ~CW\"~FY%s~CW\"\n",search);
		write_user(user,ascii_tline);
		for (i = 0; i < NUM_LINES; ++i)
		{
			temp[0] = '\0';
			if (user->tbuff->conv[i] == NULL) continue;
			strcpy(temp,ColourStrip(user->tbuff->conv[i]));
			if (!istrstr(temp,search)) continue;
			write_user(user,user->tbuff->conv[i]);
		}
		write_user(user,ascii_bline);
		writeus(user,"~CW-~FT Total of ~FY%d ~FTline%s found.\n",cnt,cnt == 1?"":"s");
		return;
	}
	for (i = 0; i < NUM_LINES; ++i) if (user->tbuff->conv[i] != NULL) ++cnt;
	if (cnt)
	{
		if (user->ansi_on) sprintf(text,"~FBÄÄÄ~CB¯¯~FT> ~CYReview of all your private messages ~FT<~CB®®~FBÄÄÄ\n");
		else sprintf(text,"~FT-~CT=~CB> ~FYReview of all your private messages... ~FT]~CB=~FT-\n");
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_convbuff(user,user->tbuff);
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
	}
	else write_user(user,"\n ~FGYou need someone to send you a private message first\n");
}

/* beeps a user -can either use it by itself or with a message */
void send_beep(User user,char *inpstr,int rt)
{
	User u;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CRmessage~CB>]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		writeus(user,"~FM Ummm.... \07... Happy?\n");
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	inpstr = remove_first(inpstr);
	if (is_enemy(u,user) && user->level<OWNER)
	{
		write_user(user,"~FG You are blocked from sending that person private messages!\n");
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
		return;
	}
	if (u->ignall && user->level<OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring everyone at the moment.\n",u->recap);
		return;
	}
	if (u->ignore & Beeps && user->level<OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring beeps right now...\n",u->recap);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (word_count < 3)
		{
			sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG to ~FM%s\n",u->recap);
			write_user(user,text);
			record_tell(user,text);
			return;
		}
		else
		{
			if (!user->hat) sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG to~FM %s~CB:~FT %s\n",u->recap,inpstr);
			else sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG to~FM %s~CB:~FT %s%s\n",u->recap,hatcolor[user->hat],inpstr);
			write_user(user,text);
			record_tell(user,text);
			return;
		}
	}
	if (word_count<3)
	{
		sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG to ~FM%s\n",u->recap);
		write_user(user,text);
		record_tell(user,text);
		sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG from ~FM%s\n",name);
		write_user(u,text);
		record_tell(u,text);
		return;
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG to~FM %s~CB:~FT %s\n",u->recap,inpstr);
		else sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG to~FM %s~CB:~FT %s%s\n",u->recap,hatcolor[user->hat],inpstr);
		write_user(user,text);
		record_tell(user,text);
		if (!user->hat) sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG from~FM %s~CB:~FT %s\n",name,inpstr);
		else sprintf(text,"~CB[\07~CRBEEEEEEP~CB]~FG from~FM %s~CB:~FT %s%s\n",name,hatcolor[user->hat],inpstr);
		write_user(u,text);
		record_tell(u,text);
	}
}

/* Start of all shouted speech commands, ie .shout, .semote */
void shout(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW -~FT Shouts have been disabled.\n");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CB#~FG %s ~FYshouts%s ~FR%s\n",name,user->sstyle,inpstr);
		else sprintf(text,"~CB#~FG %s ~FYshouts%s %s%s\n",name,user->sstyle,hatcolor[user->hat],inpstr);
		write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CB#~FG %s ~FYshouts%s ~FR%s\n",name,user->sstyle,inpstr);
	else sprintf(text,"~CB#~FG %s ~FYshouts%s %s%s\n",name,user->sstyle,hatcolor[user->hat],inpstr);
	write_user(user,text);
	write_room_except(NULL,user,text);
	record_shout(text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Do a shout emote */
void shout_emote(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW -~FT Shouts have been disabled.\n");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
		  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
		{
			if (!user->hat) sprintf(text,"~CB#~FG %s~FR%s\n",name,inpstr);
			else sprintf(text,"~CB#~FG %s%s%s\n",name,hatcolor[user->hat],inpstr);
		}
		else
		{
			if (!user->hat) sprintf(text,"~CB#~FG %s ~FR%s\n",name,inpstr);
			else sprintf(text,"~CB#~FG %s %s%s\n",name,hatcolor[user->hat],inpstr);
		}
		write_user(user,text);
		return;
	}
	if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
	  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
	{
		if (!user->hat) sprintf(text,"~CB#~FG %s~FR%s\n",name,inpstr);
		else sprintf(text,"~CB#~FG %s%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB#~FG %s ~FR%s\n",name,inpstr);
		else sprintf(text,"~CB#~FG %s %s%s\n",name,hatcolor[user->hat],inpstr);
	}
	write_user(user,text);
	write_room_except(NULL,user,text);
	record_shout(text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Echo's a shout */
void shout_echo(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRtext to shout echo~CB]");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW - ~FTShouts have been disabled.\n");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CB# ~CW- ~FT%s\n",inpstr);
		else sprintf(text,"~CB# ~CW- %s%s\n",hatcolor[user->hat],inpstr);
		write_user(user,text);
		return;
	}
	write_monitor(user,0,NULL);
	if (!user->hat) sprintf(text,"~CB# ~CW- ~FT%s\n",inpstr);
	else sprintf(text,"~CB# ~CW- %s%s\n",hatcolor[user->hat],inpstr);
	write_user(user,text);
	write_room_except(NULL,user,text);
	record_shout(text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Shouts a users thought */
void shout_think(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW - ~FTShouts have been disabled.\n");
		return;
	}
	if (word_count < 2)
	{
		if (user->status & Silenced)
		{
			writeus(user,"~CB# ~FG%s ~FTattempts to think of something...but fails...as always.\n",name);
			return;
		}
		else
		{
			sprintf(text,"~CB# ~FG%s ~FTattempts to think of something...but fails...as always.\n",name);
			write_user(user,text);
			write_room_except(NULL,user,text);
			record_shout(text);
			return;
		}
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CB# ~FY%s~FG thinks ~FT. o O (~FY %s ~FT)\n",name,inpstr);
		else sprintf(text,"~CB# ~FY%s~FG thinks ~FT. o O ( %s%s ~FT)\n",name,hatcolor[user->hat],inpstr);
		write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CB# ~FY%s~FG thinks ~FT. o O (~FY %s ~FT)\n",name,inpstr);
	else sprintf(text,"~CB# ~FY%s~FG thinks ~FT. o O ( %s%s ~FT)\n",name,hatcolor[user->hat],inpstr);
	write_user(user,text);
	write_room_except(NULL,user,text);
	record_shout(text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Gotta be able to shout a song too */
void shout_sing(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW - ~FTShouts have been disabled.\n");
		return;
	}
	if (word_count < 2)
	{
		if (user->status & Silenced)
		{
			writeus(user,"~CB# ~FG%s ~FTbreaks out in song... making everyone run for their earplugs.\n",name);
			return;
		}
		else
		{
			sprintf(text,"~CB# ~FG%s ~FTbreaks out in song... making everyone run for their earplugs.\n",name);
			write_user(user,text);
			write_room_except(NULL,user,text);
			record_shout(text);
			return;
		}
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CB# ~FY%s~FG sings ~FTo/~(~FR %s ~FT)~\\o\n",name,inpstr);
		else sprintf(text,"~CB# ~FY%s~FG sings ~FTo/~( %s%s ~FT)~\\o\n",name,hatcolor[user->hat],inpstr);
		write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CB# ~FY%s~FG sings ~FTo/~(~FR %s ~FT)~\\o\n",name,inpstr);
	else sprintf(text,"~CB# ~FY%s~FG sings ~FTo/~( %s%s ~FT)~\\o\n",name,hatcolor[user->hat],inpstr);
	write_user(user,text);
	write_room_except(NULL,user,text);
	record_shout(text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Shows a welcoming message in a nice little header */
void welcome_user(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRwelcome message~CB]");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW - ~FTShouted messages have been disabled.\n");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~FT-=<>=-=<~FR %s~FT >=-=<>=-\n",inpstr);
		else sprintf(text,"~FT-=<>=-=< %s%s~FT >=-=<>=-\n",hatcolor[user->hat],inpstr);
		write_user(user,center(text,80));
		return;
	}
	write_monitor(user,0,NULL);
	if (!user->hat) sprintf(text,"~FT-=<>=-=<~FR %s~FT >=-=<>=-\n",inpstr);
	else sprintf(text,"~FT-=<>=-=< %s%s~FT >=-=<>=-\n",hatcolor[user->hat],inpstr);
	write_room(NULL,center(text,80));
	record_shout(center(text,80));
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Yell something */
void yell(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis==Invis) name=invisname;  else name=user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	if (shout_ban == 1)
	{
		write_user(user,"~CW - ~FTShouts have been disabled.\n");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CB# ~FR%s~FG yells~RD: ~CT%s\n",name,inpstr);
		else sprintf(text,"~CB# ~FR%s~FG yells~RD: %s%s\n",name,hatcolor[user->hat],inpstr);
		write_user(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CB# ~FR%s~FG yells~RD:~CT %s\n",name,inpstr);
	else sprintf(text,"~CB# ~FR%s~FG yells~RD: %s%s\n",name,hatcolor[user->hat],inpstr);
	write_user(user,text);
	write_room_except(NULL,user,text);
	record_shout(text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(NULL,user,"~CB#   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Sends a message to your friends who are online */
void friend_tell(User user,char *inpstr,int rt)
{
	char *name,*tstr;
	char *type,*type2;

	if (user->invis == Invis) name=invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	tstr = ColourStrip(inpstr);
	type = smiley_type(tstr,2);
	type2 = smiley_type(tstr,3);
	if (user->status & Silenced)
	{
		if (!user->hat) sprintf(text,"~CB[~FMFriend~CB]~FR You %s ~FRyour friends%s~FG %s\n",type,user->sstyle,inpstr);
		else sprintf(text,"~CB[~FMFriend~CB]~FR You %s ~FRyour friends%s %s%s\n",type,user->sstyle,hatcolor[user->hat],inpstr);
		write_user(user,text);
		record_tell(user,text);
		return;
	}
	if (!user->hat) sprintf(text,"~CB[~FMFriend~CB]~FR You %s ~FRyour friends%s~FG %s\n",type,user->sstyle,inpstr);
	else sprintf(text,"~CB[~FMFriend~CB]~FR You %s ~FRyour friends%s %s%s\n",type,user->sstyle,hatcolor[user->hat],inpstr);
	write_user(user,text);
	record_tell(user,text);
	if (!user->hat) sprintf(text,"~CB[~FMFriend~CB]~FT %s %s ~FTyou%s~FG %s\n",name,type2,user->sstyle,inpstr);
	else sprintf(text,"~CB[~FMFriend~CB]~FT %s %s ~FTyou%s %s%s\n",name,type2,user->sstyle,hatcolor[user->hat],inpstr);
	write_friends(user,text);
	if (user->speech==1)
	{
		alter_speech(inpstr,1);
		sprintf(text,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
		write_friends(user,text);
	}
}

/* Emote to your friends */
void friend_emote(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if (user->status & Silenced)
	{
		if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
		  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
		{
			if (!user->hat) sprintf(text,"~CB[~FMFriend~CB] ~CM%s~FR%s\n",name,inpstr);
			else sprintf(text,"~CB[~FMFriend~CB] ~CM%s%s%s\n",name,hatcolor[user->hat],inpstr);
		}
		else
		{
			if (!user->hat) sprintf(text,"~CB[~FMFriend~CB] ~CM%s ~FR%s\n",name,inpstr);
			else sprintf(text,"~CB[~FMFriend~CB] ~CM%s %s%s\n",name,hatcolor[user->hat],inpstr);
		}
		write_user(user,text);
		record_tell(user,text);
		return;
	}
	if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
	  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
	{
		if (!user->hat) sprintf(text,"~CB[~FMFriend~CB] ~CM%s~FR%s\n",name,inpstr);
		else sprintf(text,"~CB[~FMFriend~CB] ~CM%s%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[~FMFriend~CB] ~CM%s ~FR%s\n",name,inpstr);
		else sprintf(text,"~CB[~FMFriend~CB] ~CM%s %s%s\n",name,hatcolor[user->hat],inpstr);
	}
	write_user(user,text);
	record_tell(user,text);
	if ((strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
	  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
	{
		if (!user->hat) sprintf(text,"~CB[~FMFriend~CB] ~CM%s~FR%s\n",name,inpstr);
		else sprintf(text,"~CB[~FMFriend~CB] ~CM%s%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[~FMFriend~CB] ~CM%s ~FR%s\n",name,inpstr);
		else sprintf(text,"~CB[~FMFriend~CB] ~CM%s %s%s\n",name,hatcolor[user->hat],inpstr);
	}
	write_friends(user,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		sprintf(text,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
		write_friends(user,text);
	}
}

/* Print out greeting in large letters */
void greet(User user,char *inpstr,int rt)
{
	char pbuff[ARR_SIZE],temp[8];
	int len,lc,c,i,j;
	char *clr[]={ "~CR","~CG","~CY","~CB","~CT","~CM","~FR","~FG","~FY","~FB","~FT","~FM" };

	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	if (strlen(inpstr)>11)
	{
		write_user(user,"~FG Greeted messages can only be 11 characters long.\n");
		return;
	}
	write_room(user->room,"\n");
	write_monitor(user,0,user->room);
	len=strlen(inpstr);
	if (len > 11) len = 11;
	for (i = 0; i < 5; ++i)
	{
		pbuff[0] = '\0';
		temp[0] = '\0';
		for (c = 0; c < len; ++c)
		{
			if (isupper(inpstr[c]) || islower(inpstr[c]))
			{
				lc = tolower(inpstr[c])-'a';
				if ((lc >= 0) && (lc<27))
				{
					for (j = 0; j < 5; ++j)
					{
						if (biglet[lc][i][j])
						{
							sprintf(temp,"%s#",clr[rand()%12]);
							strcat(pbuff,temp);
						}
						else strcat(pbuff," ");
					}
					strcat(pbuff,"  ");
				}
			}
			/* Is it a character from ! to @ */
			if (isprint(inpstr[c]))
			{
				lc = inpstr[c]-'!';
				if ((lc>=0) && (lc<32))
				{
					for (j = 0; j < 5; ++j)
					{
						if (bigsym[lc][i][j])
						{
							sprintf(temp,"%s#",clr[rand()%12]);
							strcat(pbuff,temp);
						}
						else strcat(pbuff," ");
					}
					strcat(pbuff,"  ");
				}
			}
		}
		sprintf(text,"%s\n",pbuff);
		if (user->status & Silenced)
		{
			write_user(user,text);
			return;
		}
		write_room(user->room,text);
	}
}

/* Function that sends the message if user has opened up a call connection */
void send_message(User user,char *type,char *str)
{
	User u;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if ((u = get_user(user->talk_to)) == NULL)
	{
		write_user(user,"~FR Unable to find who your call is linked to...resetting.\n");
		user->talk = 0;
		user->talk_to[0] = '\0';
		return;
	}
	str = replace_macros(user,str);
	if (type != NULL)
	{
		if (!user->hat) sprintf(text,"~CB[~FTPHONE~CB] ~CY%s ~CG%ss%s~RS %s\n",name,type,user->sstyle,str);
		else sprintf(text,"~CB[~FTPHONE~CB] ~CY%s ~CG%ss%s~RS %s%s\n",name,type,user->sstyle,hatcolor[user->hat],str);
		write_user(user,text);
		record_tell(user,text);
		write_user(u,text);
		record_tell(u,text);
		return;
	}
	if ((strchr("\'",str[0]) || strchr("\'",str[1]))
	  || (strchr("~",str[0]) && strchr("\'",str[3])))
	{
		if (!user->hat) sprintf(text,"~CB[~FTPHONE~CB] ~CM%s~FT%s\n",name,str);
		else sprintf(text,"~CB[~FTPHONE~CB] ~CM%s~FT%s%s\n",name,hatcolor[user->hat],str);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[~FTPHONE~CB] ~CM%s ~FT%s\n",name,str);
		else sprintf(text,"~CB[~FTPHONE~CB] ~CM%s ~FT%s%s\n",name,hatcolor[user->hat],str);
	}
	write_user(user,text);
	record_tell(user,text);
	write_user(u,text);
	record_tell(u,text);
	return;
}

/* Lets a user ring another user so that they can talk */
void phone(User user,char *inpstr,int rt)
{
	User u;
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (user->notify)
	{
		write_user(user,"~FG Someone has already tried to ring you...use ~CW'~CR.pickup~CW'\n");
		return;
	}
	if (user->talk)
	{
		write_user(user,"~FG You are already on the phone!!!\n");
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
	if (u==user)
	{
		write_user(user,"~FG You can't phone your own number.\n");
		return;
	}
	if (u->talk || u->notify)
	{
		writeus(user,"~FG Ringing ~FY%s%s~FG line......uh oh...it's busy.\n",u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (is_enemy(u,user) && user->level < OWNER)
	{
		write_user(user,"~FG That user would rather not have you phone them.\n");
		return;
	}
	if (is_enemy(user,u) && user->level < OWNER)
	{
		write_user(user,"~FG You can't call up your enemies.\n");
		return;
	}
	if (u->afk)
	{
		if (u->afk_mesg[0]) writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard~CB:~FT %s\n",u->recap,u->afk_mesg);
		else writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard at the moment!\n",u->recap);
		write_user(user,"~FGIf you would like to leave them a message ~CW'~CR.tell~CW'~FG them!\n");
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
		write_user(user,"~FGIf you would like to leave them a message ~CW'~CR.tell~CW'~FG them!\n");
		return;
	}
	if (u->ignall && user->level<OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring everyone at the moment.\n",u->recap);
		return;
	}
	if (u->ignore & Tells && user->level<OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring private messages right now...\n",u->recap);
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~FT You punch in ~FM%s%s~FT number, and let it ring....\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
		write_user(user,"~FT Awaiting a connection...........\n");
		return;
	}
	writeus(user,"~FT You punch in ~FM%s%s~FT number, and let it ring....\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
	write_user(user,"~FT Awaiting a connection...........\n");
	writeus(u,"~CB[~CRRING~CB] - [~CRRING~CB] ~FY%s~CM is dialing your number....\n",name);
	write_user(u,"~CB[~CRRING~CB] - [~CRRING~CB] ~CMTo answer your phone, type ~CW'~CR.answer~CW'\n");
	u->notify = 1;
	strcpy(user->talk_to,u->name);
	strcpy(u->talk_to,user->name);
}

/* Gotta answer the phone */
void answer_call(User user,char *inpstr,int rt)
{
	User u;

	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->gpriv)
	{
		write_user(user,"~CW- ~FGYou have had private message commands taken away from you.\n");
		return;
	}
	if (!user->notify)
	{
		write_user(user,"~FG You pick up the phone and get.....a DIALTONE!!!\n");
		return;
	}
	if ((u = get_user(user->talk_to)) == NULL)
	{
		write_user(user,"~FG Guess the person who called you left....\n");
		user->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (u->status & Jailed)
	{
		writeus(user,"~FY %s~FG is stuck in jail and has already used up their phone call.\n",u->recap);
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (u->status & Muzzled)
	{
		writeus(user,"~FY %s ~FGhas a muzzle strapped to their mouth and cannot talk.\n",u->recap);
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (is_enemy(u,user) && user->level < OWNER)
	{
		write_user(user,"~FG You are blocked from sending that person private messages.\n");
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (is_enemy(user,u) && user->level < OWNER)
	{
		write_user(user,"~CW-~FT You can't talk to your enemies..\n");
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (u->afk)
	{
		if (u->afk_mesg[0]) writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard~CB:~FT %s\n",u->recap,u->afk_mesg);
		else writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard at the moment!\n",u->recap);
		write_user(user,"~FGIf you would like to leave them a message ~CW'~CR.tell~CW'~FG them.\n");
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
		write_user(user,"~FGIf you would like to leave them a message ~CW'~CR.tell~CW'~FG them!\n");
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (u->ignall && user->level < OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring everyone at the moment.\n",u->recap);
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (u->ignore & Tells && user->level < OWNER)
	{
		writeus(user,"~FT%s~FR is ignoring private messages right now...\n",u->recap);
		user->talk_to[0] = u->talk_to[0] = '\0';
		user->notify = 0;
		return;
	}
	if (user->status & Silenced)
	{
		write_user(user,"~CB[~FTPHONE~CB] ~FT Connection has been established...\n");
		return;
	}
	u->talk = user->talk = 1;
	write_user(user,"~CB[~FTPHONE~CB] ~FT Connection has been established...\n");
	write_user(u,"~CB[~FTPHONE~CB] ~FT Connection has been established...\n");
	user->notify=0;
}

/* Disconnect the phone call now */
void hangup_phone(User user,char *inpstr,int rt)
{
	User u;

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->talk == 0)
	{
		write_user(user,"~FG You pick up your phone and then hang it up again.\n");
		return;
	}
	if ((u = get_user(user->talk_to)) == NULL)
	{
		write_user(user,"~FG Guess the person who called you left....\n");
		user->talk_to[0] = '\0';
		user->talk = 0;
		return;
	}
	sprintf(text,"~CB[~FTPHONE~CB]~FT This phone call has been terminated.\n");
	write_user(u,text);
	write_user(user,text);
	u->talk_to[0] = user->talk_to[0] = '\0';
	u->talk = user->talk = 0;
}

/*
   User says something to the channel, something like a wiztell but sends to
   the user level and above.
*/
void channel_say(User user,char *inpstr,int rt)
{
	char *type,*name,*tstr;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	tstr = ColourStrip(inpstr);
	type = smiley_type(tstr,1);
	if (!user->hat) sprintf(text,"~CB[~FMChannel~CB] ~FT%s ~FM%ss%s~RS %s\n",name,type,user->sstyle,inpstr);
	else sprintf(text,"~CB[~FMChannel~CB] ~FT%s ~FM%ss%s %s%s\n",name,type,user->sstyle,hatcolor[user->hat],inpstr);
	if (user->status & Silenced)
	{
		write_user(user,text);
		return;
	}
	write_user(user,text);
	write_level(USER,user,text);
	record_channel(text);
	if (user->speech==1)
	{
		alter_speech(inpstr,1);
		WriteLevel(USER,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Gotta be able to do an emote in a channel to I suppose */
void channel_emote(User user,char *inpstr,int rt)
{
	char *name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (user->gshout)
	{
		write_user(user,"~CW- ~FGYou have had shout commands taken away from you.\n");
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmessage~CB]");
		return;
	}
	if (ban_swearing && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (arrest_swearers && contains_swearing(inpstr) && user->level<GOD)
	{
		auto_arrest(user,1);
		return;
	}
	inpstr = replace_macros(user,inpstr);
	alter_speech(inpstr,user->speech);
	if (user->drunk >= 10) alter_speech(inpstr,5);
	if ((strchr("'\'",inpstr[0]) || strchr("\'",inpstr[1]))
	  || (strchr("~",inpstr[0]) && strchr("\'",inpstr[3])))
	{
		if (!user->hat) sprintf(text,"~CB[~FMChannel~CB]~FT %s~FY%s\n",name,inpstr);
		else sprintf(text,"~CB[~FMChannel~CB]~FT %s~FY%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[~FMChannel~CB]~FT %s ~FY%s\n",name,inpstr);
		else sprintf(text,"~CB[~FMChannel~CB]~FT %s ~FY%s%s\n",name,hatcolor[user->hat],inpstr);
	}
	write_user(user,text);
	write_level(USER,user,text);
	record_channel(text);
	if (user->speech==1)
	{
		alter_speech(inpstr,1);
		WriteLevel(USER,user,"~CB   [~FMTranslation~CB] ~RS%s\n",inpstr);
	}
}

/* Alters the speech according to whatever type the user->speech is set to */
void alter_speech(char *str,int type)
{
	int i,j,len,low,p;
	char *str2 = NULL;

	i = j = len = low = 0;
	len = strlen(str);
	switch(type)
	{
		case 1:      /* Scrambled */
		  for (i = 0; i < len; i++)
		  {
			if(!isalpha(str[i])) continue;
			if (islower(str[i])) low = 1;
			else low = 0;
			str[i] = (toupper(str[i])-'A'+13)%26+'A';
			if (low) str[i] += ('a'-'A');
		  }
		  break;
		case 2:    /* Shifted */
		  for (i = 0; i < len; i++)
		  {
			if (j++%2) str[i] = toupper(str[i]);
			else str[i] = tolower(str[i]);
		  }
		  break;
		case 3:   /* reversed */
		  len = strlen(str)-1;
		  low = len/2;
		  for(i = 0; i <= low; i++)
		  {
			j = str[i];
			str[i] = str[len-i];
			str[len-i] = j;
		  }
		  break;
		case 4: /* Professor Pither (archie comics) */
		  str = str; /* Doesn't work yet */
		  break;
		case 5: /* Drunk.. not a speech type... users can get drunk. */
		  if (strlen(str)>ARR_SIZE+500) return; /* prevent buffer overflow. */
		  str2 = (char *)malloc(ARR_SIZE*2);
		  strcpy(str2,str);
		  *str = '\0';
		  for (i = 0, p = 0; i < len; i++,p++)
		  {
			switch(str2[i])
			{
				case 'a':
				case 's':
				  if (str2[i+1]!='h')
				  {
					str[(p++)] = str2[i];
					str[p] = 'h';
					if (str2[i] == str2[i+1]) i++;
				  }
				  else str[i] = str2[i];  break;
				case '\'':
				  str[p] = str[p-1];
				  str[p-1] = str2[i];  break;
				case 'r':
				  if ((1+(int) (10.0*rand()/(RAND_MAX+1.0)))<5)
				  {
					str[(p++)] = str2[i];
					str[p] = str2[i];
				  }
				  else str[p] = str2[i];  break;
				case 'w':
				  if ((1+(int) (10.0*rand()/(RAND_MAX+1.0)))<5)
				  {
					str[(p++)] = str2[i];
					str[p] = 'u';
				  }
				  else str[p] = str2[i];
				  break;
				case 'e':
				  if (str2[i-2] == 't' && str2[i-1] == 'h')
					str[p] = '\'';
				  else str[p] = str2[i];
				  break;
				default:
				  str[p] = str2[i];
				  break;
			}
		  }
		  str[p] = '\0';
		  free(str2);
		  break;
		default:
		  return;
	}
}

/* Lets a user set their speech */
void set_speech(User user,char *inpstr,int rt)
{
	int type = 0,i = 0;

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRtype of speech~CW|~CR-list~CB]");
		return;
	}
	if (!strcasecmp(word[1],"-list"))
	{
		write_user(user,center("~FT-~CT=~CB> ~FGDifferent type of speeches ~CB<~CT=~FT-\n",80));
		for (i = 0; i < NumSpeech; ++i)
		{
			writeus(user," ~CT%d~CB  =~FM %s\n",i,speech_style[i]);
		}
		return;
	}
	type = atoi(word[1]);
	if (type < 0)
	{
		write_user(user,"~FG That is an invalid speech type.\n");
		return;
	}
	if (type > (NumSpeech-1))
	{
		write_user(user,"~FG There is no such speech type.\n");
		return;
	}
	user->speech = type;
	write_user(user,"~FG Ok, you change your speech\n");
	return;
}

/*
   This should be in general.c but I was stupid one night and put it in here
   by mistake and I really don't feel like rewriting it... oh well.. maybe
   if you're more energetic you can.
*/
/*
   Lets a user net marry another user, w/ their equivalent vows, as long as
   the other user consents to it
*/
void net_marry(User user,char *inpstr,int stage)
{
	User u,u2;
	Room rm,rm2;
	int cnt=0;
	char worh[15],*uname;

	if (stage == 0)
	{
		if (is_jailed(user))
		{
			status_text(user,2,command[com_num].name);
			return;
		}
		if (word_count < 2)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [<~CRyour personal vows~CB>]");
			return;
		}
		if ((uname = get_user_full(user,user->married_to)) != NULL)
		{
			writeus(user,"~FG Shame on you, you're already married to ~FT%s.\n",uname);
			return;
		}
		rm = user->room;
		if ((u = get_name(user,word[1])) == NULL)
		{
			write_user(user,center(notloggedon,80));
			return;
		}
		if (u == user)
		{
			write_user(user,"~CW-~FT I'm sorry.. I can't let you marry yourself..\n");
			return;
		}
		if ((uname = get_user_full(user,u->married_to)) != NULL)
		{
			writeus(user,"~FG That person is already married to ~FT%s.\n",uname);
			return;
		}
		if ((rm2 = get_room(wedd_room)) == NULL)
		{
			write_user(user,"~FG Failed to find the wedding room...sorry.\n");
			return;
		}
		if (strcasecmp(rm->name,rm2->name))
		{
			writeus(user,"~FT You must be in the ~FM%s~FT room in order to get married!\n",rm2->name);
			return;
		}
		if (u->room != rm2)
		{
			writeus(user,"~FT Your future spouse must be in the ~FM%s~FT with you.\n",rm2->name);
			return;
		}
		for_users(u2)
		{
			if (u2->login || u2->type==CloneType) continue;
			if (u2->room != rm2) continue;
			cnt++;
		}
		if (cnt <= 2)
		{
			write_user(user,"~FG There need to be some witnesses in this room in order to make it legal!\n");
			return;
		}
		if (word_count > 2)
		{
			inpstr = remove_first(inpstr);
			strncpy(user->vows,inpstr,sizeof(user->vows)-1);
		}
		strcpy(u->check,user->name);
		strcpy(user->check,u->name);
		writeus(u,"~FG %s~FM wants to marry you... will you accept? ~CB[~CRY~CW|~CRN~CB]:~RS ",user->recap);
		writeus(user,"~CW-~FT Hang on... seeing if %s ~FTwants to marry you.\n",u->recap);
		u->misc_op = 28;
		return;
	}
	if (stage == 1)
	{
		if ((u = get_user(user->check)) == NULL)
		{
			write_user(user,"~FG Crappy deal, your spouse left ya at the alter.\n");
			user->check[0] = user->vows[0] = '\0';
			return;
		}
		WriteRoom(user->room,"~FG  Dearly beloved, we are gathered here today, to unite ~FM%s ~FGand ~FM%s"
				     "~FG together in holy matrimony.  If there are any objections as to why this wedding should"
				     "~FG not take place, please speak now, or forever hold your peace.\n\n",user->recap,u->recap);
		strcpy(worh,"thing");
		if (u->gender == Female) strcpy(worh,"wife");
		else if (u->gender == Male) strcpy(worh,"husband");
		if (user->vows[0] == '\0')
		{
			WriteRoom(user->room,"~FG Do you ~FM%s ~FGtake ~FM%s ~FGto be your lawfully wedded %s?"
					     "~FG To have and to hold and to cherish for all time?\n",u->recap,user->recap,worh);
			write_user(u,"~CB[~CRYes or No~CB]:~RS ");
			u->misc_op = 31;
			return;
		}
		else
		{
			WriteRoom(user->room,"  ~FT%s ~FGmakes the following vows to ~FT%s~FG...\n",user->recap,u->recap);
			WriteRoom(user->room," %s\n\n",user->vows);
			WriteRoom(user->room,"~FG  Do you ~FM%s ~FGtake ~FM%s ~FGto be your lawfully wedded %s?\n"
					     "~FG To have and to hold and to cherish for all time?\n",u->recap,user->recap,worh);
			write_user(u,"~CB[~CRYes or No~CB]:~RS ");
			u->misc_op=31;
			return;
		}
		return;
	}
	if (stage == 2)
	{
		if ((u = get_user(user->check)) == NULL)
		{
			write_user(user,"~FG Crappy deal, your spouse left ya at the alter.\n");
			user->check[0] = user->vows[0] = '\0';
			return;
		}
		WriteRoom(user->room,"~FG Then by the power vested in this talker, as the good Lord as our witness,"
				     "~FG I now pronounce ~FM%s and ~FM%s ~FGman and wife.\n",user->recap,u->recap);
		write_room(NULL,ascii_line);
		WriteRoom(NULL,"~FT  Would everyone please congratulate ~FY%s ~FTand ~FY%s~FT, for they have just"
			       "~FT professed their love for each other, and have been united together in"
			       "~FT holy matrimony...\n",user->recap,u->recap);
		write_room(NULL,ascii_line);
		strcpy(user->married_to,u->name);
		strcpy(u->married_to,user->name);
		return;
	}
}

/* Gotta let them get a divorce as well... */
void net_divorce(User user,char *inpstr,int rt)
{
	User u;

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if (istrstr(user->married_to,"noone")
	  || istrstr(user->married_to,"divorce"))
	{
		write_user(user,"~FG You can't get divorced if your not even married.\n");
		return;
	}
	if ((u = get_user(user->married_to)) == NULL)
	{
		write_user(user,"~FG Your spouse has to be online in order to get a divorce.\n");
		return;
	}
	writeus(u,"~FG %s ~FTis divorcing you... Personally I am sorry about this, and I"
		  "~FT hope that you can overcome this loss, it shall be difficult, but"
		  "~FT everything will work out for the best, have faith...\n",user->recap);
	write_user(user,"~FM The divorce has been finalized...\n");
	strcpy(user->married_to,"Divorced");
	strcpy(u->married_to,"Divorced");
}

/* END OF SPEECH.C */
