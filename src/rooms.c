/*
  rooms.c
    The code in this file deals with the inner workings of rooms, such as
    .look, .go, and other room commands.


    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Mike Irving, and Neil Robertson.

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
#define _ROOMS_C
  #include "include/needed.h"
#undef _ROOMS_C

/* Print out the map */
void map(User user,char *inpstr,int rt)
{
	Room rm;

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
	rm = user->room;
	/* Shows a map file for each different room->map, suit to fit */
	if (!strcasecmp(rm->map,"cove") || !strcasecmp(rm->map,"users"))
	{
		sprintf(file,"%s",MAP1);
		if (!(show_screen(user)))
		  write_user(user,center("~FT-~CT=~CB> ~FMThere is no map... ~CB<~CT=~FT-\n",80));
		return;
	}
	/* I'll add another one here to show how its done... */
	if (!strcasecmp(rm->map,"staff"))
	{
		sprintf(file,"%s",MAP2);
		if (!(show_screen(user)))
		  write_user(user,center("~FT-~CT=~CB> ~FMThere is no map... ~CB<~CT=~FT-\n",80));
		return;
	}
	write_user(user,center("~FT-~CT=~CB> ~FMThere is no map... ~CB<~CT=~FT-\n",80));
}

/* go to the users personal room.. */
void goto_myroom(User user,char *inpstr,int rt)
{
	Room rm;

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
	if (user->status & Shackled)
	{
		write_user(user,"~FG Those shackles prevent you from even going to your own room...\n");
		return;
	}
	if (user->has_room == 0 && user->level<WIZ && free_rooms == 0)
	{
		write_user(user,"~FG Sorry, you don't have your own personal room.\n");
		return;
	}
	home_room(user);
	rm = get_room(user->name);
	if (rm == NULL)
	{
		write_user(user,"~FG Sorry, some sort of error occurred while trying to load your room.\n");
		return;
	}
	if (rm == user->room)
	{
		write_user(user,"~FG You are already in your own room.....\n");
		return;
	}
	if (user->invis == 0 && rm->hidden == 0)
	{
		WriteRoomExcept(user->room,user," ~FT%s ~FM%s ~FGto %s room...\n",user->recap,user->out_phr,gender1[user->gender]);
	}
	if (rm->hidden) move_user(user,rm,4);
	else move_user(user,rm,3);
}

/* Creates USER rooms.. */
void home_room(User user)
{
	Room room;
	int i,err = 0;

	if ((room = get_room(user->name)) == NULL)
	{
		if ((room = create_room()) == NULL)
		{
			write_user(user,"~FG Unable to build your room at this time.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create personal room in home_room()\n",0,LOG1);
			return;
		}
		strcpy(room->name,user->name);
		err = LoadRoom(room,user->name);
		if (err == 0 || err == -1)
		{
			strcpy(room->link[0],main_room);
			strcpy(room->recap,user->name);
			if ((user->tpromoted && user->orig_lev<WIZ) || user->level<WIZ)
			{
				room->access = PERSONAL;
				strcpy(room->map,"users");
			}
			else
			{
				room->access = STAFF_ROOM;
				strcpy(room->map,"staff");
			}
			room->no_buff = 0;
			room->hidden = user->hide_room;
			room->level = NEWBIE;
			strcpy(room->owner,user->name);
			sprintf(text,"~FG%s%s ~FMroom.",user->name,user->name[strlen(user->name)-1] == 's'?"'":"'s");
			strcpy(room->topic,text);
			for (i=0;i<MaxAtmos;++i) room->atmos[i][0] = '\0';
			i = 0;
			for (i=0;i<MaxKeys;++i)  room->key[i][0] = '\0';
			add_link(user,room,room->link[0]);
			SaveRoom(room);
			add_roomlist(room);
		}
	}
	return;
}

/* kicks a user out of a users personal room.. */
void kick_out(User user,char *inpstr,int rt)
{
	User u;
	Room rm;
	char *name,*rname;

	rm = user->room;
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
	if (strcmp(rm->owner,user->name))
	{
		write_user(user,"~FG This isn't your room, you can't kick anyone out..\n");
		return;
	}
	if ((u = get_clone(word[1])) == NULL)
	{
		if ((u = get_name(user,word[1])) == NULL)
		{
			write_user(user,center(notloggedon,80));
			return;
		}
	}
	if (user->pop != NULL)
	{
		write_user(user,"~FG You can't kick someone out in the middle of poker.\n");
		return;
	}
	if (u == user)
	{
		write_user(user,"~FG Why would you want to kick yourself out?\n");
		return;
	}
	if (u->room != rm)
	{
 		writeus(user,"~FG It would help if ~CT%s~FG was in the same room as you.\n",u->recap);
		return;
	}
	if (u->type == CloneType)
	{
		writeus(u->owner,"~CW-~FT %s~FG is kicking your clone out of the ~FT%s~FG, destroying your clone...\n",user->recap,rm->recap);
		writeus(user,"~CTYou remove ~FG%s~CT from your room.\n",u->recap);
		destruct_alpha(u);
		destruct_user(u);
		return;
	}
	if ((get_room(main_room)) == NULL)
	{
		write_user(user,"~FGSorry, can't find the main room right now....\n");
		return;
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (u->invis == Invis) rname = invisname;  else rname = u->recap;
	writeus(user,"~CTYou remove ~FG%s~CT from your room.\n",u->recap);
	write_user(u,"~CG You have been ~CRREMOVED~CG from this room.\n");
	if (rm->hidden) move_user(u,get_room(main_room),5);
	else move_user(u,get_room(main_room),2);
	sprintf(text,"~FT%s~FM throws ~FT%s~FM out of %s room, yelling~CW \"~FYGET THE **** OUTTA HERE!~CW\"\n",name,rname,gender1[user->gender]);
	if (rm->hidden == 0)
	{
		write_room(NULL,text);
		record_shout(text);
	}
	return;
}

/* Moves user between rooms */
void go(User user,char *inpstr,int rt)
{
	Room rm,rm2,rm3=NULL;
	int i;
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
	if (user->status & Shackled)
	{
		write_user(user,"~FG Those shackles prevent you from going anywhere.\n");
		return;
	}
	if (word_count < 2)
	{
		rm2 = user->room;
		rm = get_room(main_room);
		if (rm == NULL)
		{
			write_user(user,"~FG Unable to warp you to the main room...\n");
			return;
		}
		if (user->pop)
		{
			write_user(user,"~FG You should leave your poker game first.\n");
			return;
		}
		if (user->room == rm)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CW<~CRroom name~CW>~CB]");
			return;
		}
		if (rm2->access >= INV_MALE)
		{
			if (!strcmp(rm2->owner,user->name))
			{
				write_user(user,"~FG Your room's access has been restored.\n");
				if (rm2->temp_access) user->room->access = user->room->temp_access;
				rm2->temp_access = 0;
			}
		}
		if (rm2->hidden) move_user(user,rm,5);
		else move_user(user,rm,0);
		destruct_room(rm2);
		return;
	}
	if (strlen(inpstr)>RoomNameLen)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	if (user->room->hidden)
	{
		write_user(user,"~FG You MUST type ~CW'~CR.go~CW' ~FGby itself to go to another room.\n");
		return;
	}
	if (!strcasecmp(word[1],"staff"))
	{
		rm2 = user->room;
		if (rm2->hidden)
		{
			write_user(user,"~FG You MUST type ~CW'~CR.go~CW' ~FGby itself to go to another room.\n");
			return;
		}
		if (user->level<WIZ)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
		if (user->room == get_room(staff_room))
		{
			write_user(user,"~FG You are already in the staff room.\n");
			return;
		}
		if ((rm = get_room(staff_room)) == NULL)
		{
			write_user(user,"~CW-~FT There was an error in finding the staff room.\n");
			write_log(1,LOG1,"[ERROR] - I wasn't able to find the staff room.\n");
			return;
		}
		if (rm2->access >= INV_MALE)
		{
			if (!strcmp(rm2->owner,user->name))
			{
				write_user(user,"~FG Your room's access has been restored.\n");
				if (rm2->temp_access) user->room->access = user->room->temp_access;
				rm2->temp_access = 0;
			}
		}
		move_user(user,rm,2);
		destruct_room(rm2);
		return;
	}
	if ((rmname = get_room_full(user,inpstr)) == NULL)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	if ((rm = get_room(rmname)) == NULL)
	{
		if ((rm = create_room()) == NULL)
		{
			write_user(user,"~FG Unable to move you to that room.\n");
			return;
		}
		strcpy(rm->name,rmname);
		if ((LoadRoom(rm,rmname)) < 1)
		{
			write_user(user,center(nosuchroom,80));
			destruct_room(rm);
			return;
		}
	}
	if (rm == user->room)
	{
		write_user(user,"~FG Silly, your already here...\n");
		return;
	}
	if (rm->access == INV_MALE)
	{
		rm2 = user->room;
		if (user->gender != Male)
		{
			if (strcmp(rm->owner,user->name))
			{
				write_user(user,"~FG That room is currently only open to males.\n");
				return;
			}
			move_user(user,rm,1);
			destruct_room(rm2);
			return;
		}
		else
		{
			move_user(user,rm3,1);
			destruct_room(rm2);
			return;
		}
	}
	if (rm->access == INV_FEM)
	{
		rm2 = user->room;
		if (user->gender != Female)
		{
			if (strcmp(rm->owner,user->name))
			{
				write_user(user,"~FG That room is currently only open to females.\n");
				return;
			}
			move_user(user,rm,1);
			destruct_room(rm2);
			return;
		}
		else
		{
			move_user(user,rm3,1);
			destruct_room(rm2);
			return;
		}
	}
	rm2 = user->room;
	if (rm->hidden)
	{
		if (user->level < rm->level)
		{
			WriteRoom(rm,"~CW-~FT %s~FG is trying to open the door.\n",user->recap);
			write_user(user,center(nosuchroom,80));
			destruct_room(rm);
			return;
		}
		if (rm->access == STAFF_ROOM || rm->access == PERSONAL)
		{
			if (strcmp(rm->owner,user->name) && user->level < OWNER)
			{
				if (user->invite_room != rm && !has_room_key(user,rm))
				{
					WriteRoom(rm,"~CW-~FT %s~FG is trying to open the door.\n",user->recap);
					write_user(user,center(nosuchroom,80));
					destruct_room(rm);
					return;
				}
			}
		}
		move_user(user,rm,4);
		destruct_room(rm2);
		return;
	}
	if (rm->access == PERSONAL || rm->access == STAFF_ROOM)
	{
		if (strcmp(rm->owner,user->name) && user->level<OWNER)
		{
			if (user->invite_room != rm && !has_room_key(user,rm))
			{
				writeus(user,"~FG The ~FT%s~FG room is a personal room in which you don't own.\n",rm->recap);
				return;
			}
		}
		if (rm->hidden) move_user(user,rm,4);
		else move_user(user,rm,0);
		destruct_room(rm2);
		return;
	}
	for (i=0;i<MaxLinks;++i)
	{
		if (strcasecmp(rm2->link[i],rm->name) && strcasecmp(rm->link[i],rm2->name))
		{
			if (user->level<WIZ && !user->carriage)
			{
				writeus(user,"~FG The ~FT%s~FG room isn't connected to this one.\n",rm->recap);
				return;
			}
			move_user(user,rm,2);
			destruct_room(rm2);
			return;
		}
		move_user(user,rm,1);
		destruct_room(rm2);
		return;
	}
	if (user->room->access>=INV_MALE)
	{
		if (!strcmp(user->room->owner,user->name))
		{
			write_user(user,"~FG Your room's access has been restored!\n");
			if (user->room->temp_access) user->room->access = user->room->temp_access;
			user->room->temp_access = 0;
		}
	}
	if (rm->hidden) move_user(user,rm,4);
	else move_user(user,rm,0);
	destruct_room(rm2);
}

/* Sets the room access public or private */
void set_room_access(User user,char *inpstr,int lock)
{
	User u;
	Room rm;
	char *name;
	int cnt;

	if (user == NULL) return;
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
	if (word_count<2) rm = user->room;
	else
	{
		if (user->level<gatecrash_level)
		{
			rm = user->room;
		}
		if ((rm = get_room(word[1])) == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
	}
	if (rm->access != PUBLIC && rm->access != PRIVATE
	  && rm->access != STAFF_ROOM)
	{
		write_user(user,"~FG Unable to change access in this room.\n");
		return;
	}
	if (lock == 0 && rm->access == PUBLIC)
	{
		if (rm==user->room) write_user(user,"~FG This room is already unlocked.\n");
		else write_user(user,"~FG That room is already unlocked.\n");
		return;
	}
	if (rm->access >= INV_MALE)
	{
		if (rm==user->room) write_user(user,"~FG Unable to change access in this room.\n");
		else write_user(user,"~FG Unable to change that rooms access.\n");
		return;
	}
	if (lock)
	{
		if (rm->access == PRIVATE)
		{
			if (rm==user->room) write_user(user,"~FG This room is already is locked.\n");
			else write_user(user,"~FG That room is already locked.\n");
			return;
		}
		cnt = 0;
		for_users(u)
		{
			if (u->room == rm) ++cnt;
		}
		if (cnt<min_private_users && user->level<ignore_mp_level)
		{
			write_user(user,"~FG Why would you want to lock yourself in a room?\n");
			return;
		}
		write_user(user,"~FG You turn the latch on the door, locking it.\n");
		if (rm == user->room)
		{
			WriteRoomExcept(rm,user,"~FG %s~FM turns the latch on the door locking it tight.\n",name);
		}
		else write_room(rm,"~FG The latch is turned on the door, locking it tight.\n");
		if (rm->access == STAFF_ROOM) rm->lock=1;
		rm->access = PRIVATE;
		return;
	}
	write_user(user,"~FG You turn the latch and unlock the door.\n");
	if (rm == user->room)
	{
		WriteRoomExcept(rm,user,"~FG %s~FM turns the latch on the door and unlocks it.\n",name);
	}
	else write_room(rm,"~FG The latch is turned on the door, unlocking it.\n");
	if (rm->access == STAFF_ROOM) rm->lock=0;
	else rm->access = PUBLIC;
	for_users(u)
	{
		if (u->invite_room == rm) u->invite_room=NULL;
	}
	clear_convbuff(rm->revbuff);
}

/* Knock on a locked door */
void knock(User user,char *inpstr,int rt)
{
	Room rm;
	int i;
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
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRroom name~CB]");
		return;
	}
	if (strlen(inpstr)>RoomNameLen)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	if ((rmname = get_room_full(user,inpstr)) == NULL)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	if ((rm = get_room(rmname)) == NULL)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	if (rm->hidden && user->level<rm->level)
	{
		write_user(user,center(nosuchroom,80));
		WriteRoom(rm,"~CW- ~FT%s ~FGis knocking on the door.\n",user->recap);
		return;
	}
	if (rm == user->room)
	{
		writeus(user,"~FM Your already in the ~FR%s~FM silly...\n",rm->recap);
		return;
	}
	if (has_room_key(user,rm))
	{
		writeus(user,"~FG You don't need to knock.. you have a key to the %s.\n",rm->recap);
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW-~RS You knock on the door to the ~FY%s~RS room.\n",rm->recap);
		return;
	}
	if (rm->access == PERSONAL || rm->access == STAFF_ROOM)
	{
		writeus(user,"~CW-~RS You knock on the door to the ~FY%s~RS room.\n",rm->recap);
		WriteRoom(rm,"~CW-~FT %s~RS is knocking on the door...\n",user->recap);
		return;
	}
	for (i=0;i<MaxLinks;++i)
	  if (!strcasecmp(user->room->link[i],rm->name) || user->carriage) goto GOT_IT;
	writeus(user,"~FY Would kind of help if the ~FG%s~FY was joined to this one.\n",rm->recap);
	return;

	GOT_IT:
	  if (!(rm->access & PRIVATE))
 	  {
		writeus(user,"~FG Don't need to knock, the ~FT%s ~FGis unlocked.\n",rm->recap);
		return;
	  }
	  writeus(user,"~CW-~RS You knock on the door to the ~FY%s~RS room.\n",rm->recap);
	  WriteRoom(rm,"~CW-~FT %s~RS is knocking on the door...\n",user->recap);
}

/* Invite a user into a locked room */
void invite(User user,char *inpstr,int rt)
{
	User u;
	Room rm;
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
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmale~CW|~CRfemale~CW|~CRall~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
	return;
	}
	rm = user->room;
	if (rm->access == PUBLIC || rm->access == FIXED_PUBLIC)
	{
		write_user(user,"~FGThis room is currently unlocked..\n");
		return;
	}
	if (rm->owner[0] && strcmp(rm->owner,"NULL"))
	{
		if (strcmp(user->name,rm->owner))
		{
			writeus(user,"~FGThe ~FT%s~FG room is NOT your personal room.\n",rm->recap);
			return;
		}
	}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (!strcasecmp(word[1],"female"))
	{
		if (rm->hidden)
		{
			write_user(user,"~FG You can't use this option in hidden rooms.\n");
			return;
		}
		for_users(u)
		{
			if (u->login || u==user || u->room==rm
			  || u->gender==Male || u->type==CloneType
			  || (u->status & Shackled)) continue;
			writeus(u,"~CB[~FMInvite~CB] - ~FT%s ~FGhas invited any females to the %s.\n",user->recap,rm->recap);
			writeus(u,"~CB[~FMInvite~CB] - ~FGTo go there, type ~CW'~CR.join %s~CW'\n",user->name);
		}
		write_user(user,"~CB[~FMInvite~CB] - ~FGYou invite all females in.\n");
		if (rm->temp_access) rm->access = rm->temp_access;
		rm->temp_access = rm->access;
		rm->access = INV_FEM;
		return;
	}
	if (!strcasecmp(word[1],"male"))
	{
		if (rm->hidden)
		{
			write_user(user,"~FG You can't use this option in hidden rooms.\n");
			return;
		}
		for_users(u)
		{
			if ((u->gender & Female) || u->type==CloneType
			  || (u->status & Shackled) || u->invite_room == rm
			  || u->login || u == user || u->room == rm) continue;
			writeus(u,"~CB[~FMInvite~CB] - ~FT%s ~FGhas invited any males to the %s.\n",user->recap,rm->recap);
			writeus(u,"~CB[~FMInvite~CB] - ~FGTo go there, type ~CW'~CR.join %s~CW'\n",user->name);
		}
		write_user(user,"~CB[~FMInvite~CB] - ~FGYou invite all males in.\n");
		if (rm->temp_access) rm->access = rm->temp_access;
		rm->temp_access = rm->access;
		rm->access = INV_MALE;
		return;
	}
	if (!strcasecmp(word[1],"all"))
	{
		if (rm->hidden)
		{
			write_user(user,"~FG You can't use this option in hidden rooms.\n");
			return;
		}
		for_users(u)
		{
			if (u->login || u == user || u->room == rm
			  || u->type == CloneType || (u->status & Shackled)
			  || u->invite_room == rm) continue;
			writeus(u,"~CB[~FMInvite~CB] - ~FT%s ~FGhas invited anyone to the %s.\n",user->recap,rm->recap);
			writeus(u,"~CB[~FMInvite~CB] - ~FGTo go there, type ~CW'~CR.join %s~CW'\n",user->name);
		}
		write_user(user,"~CB[~FMInvite~CB] - ~FGYou invite everyone in.\n");
		if (rm->temp_access) rm->access = rm->temp_access;
		rm->temp_access = rm->access;
		rm->access = INV_ALL;
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FG Do you invite yourself to parties too?\n");
		return;
	}
	if (u->status & Shackled)
	{
		writeus(user,"~FT %s~FG is shackled and cannot leave the ~FT%s\n",u->recap,u->room->recap);
		return;
	}
	if (has_room_key(u,rm))
	{
		writeus(user,"~FT %s~FG has a key for this room.. they don't need to be invited.\n",u->recap);
		return;
	}
	if (u->room == rm)
	{
		writeus(user,"~FT %s~FG is already in here... try out .look ;-)\n",u->recap);
		return;
	}
	if (u->invite_room == rm)
	{
		writeus(user,"~FT %s~FG has already been invited into this room.\n",u->recap);
		return;
	}
	writeus(user,"~CB[~FMInvite~CB] - ~FGYou invite ~FT%s ~FGin.\n",u->recap);
	writeus(u,"~CB[~FMInvite~CB] - ~FT%s ~FGhas invited you into the ~FY%s\n",name,rm->recap);
	writeus(u,"~CB[~FMInvite~CB] - ~FTTo go there, type ~CW'~CR.join %s~CW'\n",user->name);
	sprintf(text,"~CB[~FMInvite~CB] - ~FT%s ~FGhas been invited in by ~FT%s~FG.\n",u->recap,name);
	write_room_except(user->room,user,text);
	record(user->room,text);
	u->invite_room = rm;
}

/* Uninvites a user from a room */
void uninvite(User user,char *inpstr,int rt)
{
	User u;
	Room rm;
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
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRmale~CW|~CRfemale~CW|~CRall~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	rm = user->room;
	if (rm->access == PUBLIC)
	{
		write_user(user,"~FGThis room is currently unlocked..\n");
		return;
	}
	if (rm->owner[0] != '\0' && strcmp(rm->owner,"NULL"))
	{
		if (strcmp(user->name,rm->owner))
		{
			writeus(user,"~FG The ~FT%s~FG room isn't your room.\n",rm->recap);
			return;
		}
	}
	if (user->invis == Invis) name = invisname; else name = user->recap;
	if (!strcasecmp(word[1],"female"))
	{
		if (rm->hidden)
		{
			write_user(user,"~FG You can't use this option in hidden rooms.\n");
			return;
		}
		for_users(u)
		{
			if (u->login || u == user || u->room == rm
			  || (u->gender == Male) || u->type == CloneType
			  || (u->status & Shackled)) continue;
			writeus(u,"~CB[~FTUninvite~CB] - ~FY%s ~FMhas uninvited females from the ~FY%s\n",name,rm->recap);
		}
		write_user(user,"~CB[~FTUninvite~CB] - ~FGYou uninvite females from this room.\n");
		if (rm->temp_access == 0) return;
		rm->access = rm->temp_access;  rm->temp_access = 0;
		return;
	}
	if (!strcasecmp(word[1],"male"))
	{
		if (rm->hidden)
		{
			write_user(user,"~FG You can't use this option in hidden rooms.\n");
			return;
		}
		for_users(u)
		{
			if (u->login || u == user || u->room == rm
			  || (u->gender & Female) || u->type == CloneType
			  || (u->status & Shackled)) continue;
			writeus(u,"~CB[~FTUninvite~CB] - ~FY%s ~FMhas uninvited males from the ~FY%s\n",name,rm->recap);
		}
		write_user(user,"~CB[~FTUninvite~CB] - ~FGYou uninvite males from this room.\n");
		if (rm->temp_access == 0) return;
		rm->access = rm->temp_access;  rm->temp_access = 0;
		return;
	}
	if (!strcasecmp(word[1],"all"))
	{
		if (rm->hidden)
		{
			write_user(user,"~FG You can't use this option in hidden rooms.\n");
			return;
		}
		for_users(u)
		{
			if (u->login || u == user || u->room == rm
			  || u->type==CloneType || (u->status & Shackled))
			  continue;
			writeus(u,"~CB[~FTUninvite~CB] - ~FY%s ~FMhas uninvited everyone from the ~FY%s\n",name,rm->recap);
		}
		write_user(user,"~CB[~FTUninvite~CB] - ~FGYou uninvite everyone from this room.\n");
		if (rm->temp_access == 0) return;
		rm->access = rm->temp_access;  rm->temp_access = 0;
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~FG Why would you want to uninvite yourself?\n");
		return;
	}
	if (u->invite_room == rm)
	{
		u->invite_room = NULL;
		writeus(u,"~CB[~FTUninvite~CB] - ~FY%s ~FMhas taken away your invitation to the %s.\n",name,rm->recap);
		writeus(user,"~CB[~FTUninvite~CB] - ~FY%s's invitation has been taken away.\n",u->recap);
		return;
	}
	writeus(user,"~FG %s isn't even invited into this room.\n",u->recap);
}

/*
  Sets the topic for the room, an added feature, I removed the .toplock
  command, and replaced it with just .topic -lock, you can now -lock and
  also have a topic with it, ie: .top -l Welcome to this Talker would set
  the topic to "My Talker" and lock it at the users level.
*/
void set_topic(User user,char *inpstr,int rt)
{
	Room rm;
	int lockit;
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
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	rm = user->room;  lockit = 0;
	if (word_count < 2)
	{
		if (rm->topic[0] != '\0')
		{
			writeus(user,"~FT Topic currently set to~CB: [~RS%s~CB]\n",rm->topic);
			return;
		}
		write_user(user,"~FT No topic has been currently set.\n");
		return;
	}
	if (user->level < rm->topic_lock)
	{
		writeus(user,"~FG The topic is currently locked at level ~CB[~CT%s~CB]\n",user->gender==Female?level[rm->topic_lock].fname:level[rm->topic_lock].mname);
		return;
	}
	/* See if we want to lock it. */
	if (!strncasecmp(word[1],"-lock",2))
	{
		if (user->level < POWER)
		{
			write_user(user,"~FG You can't lock topics.\n");
			return;
		}
		if (rm->topic_lock)
		{
			write_user(user,"~FM Topic lock removed.\n");
			WriteRoomExcept(rm,user,"~FT %s~FG removes the topic lock for the current topic.\n",name);
			rm->topic_lock = 0;
			SaveRoom(rm);
			return;
		}
		lockit = 1;
	}
	/* We're just locking the topic. */
	if (word_count == 2 && lockit)
	{
		if (rm->topic[0] == '\0')
		{
			write_user(user,"~FG What's the point of locking a non-existant topic?\n");
			return;
		}
		writeus(user,"~CB[~FRTopic-lock~CB] - ~FGTopic locked at level ~FT%s\n",level[user->level].name);
		WriteRoomExcept(rm,user,"~CB[~FRTopic-lock~CB] - ~FM%s~FG locked the topic at level ~FT%s\n",name,level[user->level].name);
		rm->topic_lock = user->level;
		SaveRoom(rm);
		return;
	}
	if (lockit) inpstr = remove_first(inpstr);
	if (strlen(inpstr) > TopicLen)
	{
		write_user(user,"~FMSorry that topic is a wee bit too long!\n");
		return;
	}
	writeus(user,"~FMYou set the topic to~CB: [~RS%s~CB]\n",inpstr);
	WriteRoomExcept(rm,user,"~FY%s~FG sets the topic to~CB: [~RS%s~CB]\n",name,inpstr);
	strncpy(rm->topic,inpstr,sizeof(rm->topic)-1);
	if (lockit)
	{
		writeus(user,"~CB[~FRTopic-lock~CB] - ~FGTopic locked at level ~FT%s\n",level[user->level].name);
		WriteRoomExcept(rm,user,"~CB[~FRTopic-lock~CB] - ~FM%s~FY locked the topic at level ~FT%s\n",name,level[user->level].name);
		rm->topic_lock = user->level;
		SaveRoom(rm);
		return;
	}
	else rm->topic_lock = 0;
	SaveRoom(rm);
}

/* Wiz moves a user to the room they're in or to the specified room */
void staff_move(User user,char *inpstr,int rt)
{
	User u;
	Room rm,rm2;
	char *name,*rmname;
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
	rm2 = user->room;
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CW<~CRroom name~CW>~CB]");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (user == u)
	{
		write_user(user,"~FG See those two legs at the bottom of your body...use them.\n");
		return;
	}
	if (u->level >= user->level && user->level<OWNER)
	{
		write_user(user,"~FG Sorry, moving other staff members isn't allowed.\n");
		writeus(u,"~FT %s~FG tried to .move you.\n",user->recap);
		return;
	}
	if (u->room->hidden)
	{
		rm = get_room(main_room);
		if (rm == NULL)
		{
			write_user(user,"~CW-~FT I couldn't find the main room.. sorry.\n");
			return;
		}
		inpstr = remove_first(inpstr);
		if (strcasecmp(inpstr,main_room))
		{
			write_user(user,"~CW-~FT You can only move users in hidden rooms to the main room.\n");
			return;
		}
		move_user(u,rm,5);
		write_user(u,"~CW-~FT You have been moved to the main room.\n");
		writeus(user,"~CW-~FT You move '~FG%s~FT' to the %s.\n",u->recap,rm->recap);
		write_log(1,LOG4,"[MOVE] - User: [%s] Person moved: [%s] Room: [%s]\n",user->name,u->name,rm->name);
		return;
	}
	else if (word_count < 3) rm = user->room;
	else
	{
		inpstr = remove_first(inpstr);
		if ((rmname = get_room_full(user,inpstr)) == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
		if ((rm = get_room(rmname)) == NULL)
		{
			if ((rm = create_room()) == NULL)
			{
				write_user(user,"~FG Sorry, an error occurred..\n");
				return;
			}
			strcpy(rm->name,inpstr);
			err = LoadRoom(rm,inpstr);
			if (err == 0)
			{
				write_user(user,center(nosuchroom,80));
				destruct_room(rm);
				return;
			}
			if (err == -1)
			{
				write_log(1,LOG1,"[ERROR] - Room '%s' seems to be corrupted.\n");
				writeus(user,"~CW-~FT An error occurred.. aborting.\n");
				destruct_room(rm);
				return;
			}
		}
	}
	if ((rm->access == PERSONAL || rm->access == STAFF_ROOM)
	  && user->level < OWNER)
	{
		if (strcmp(rm->owner,user->name))
		{
			write_user(user,"~FT You can't be moving people into rooms that aren't yours.\n");
			destruct_room(rm);
			return;
		}
	}
	if (rm == u->room)
	{
		writeus(user,"~FG %s ~FTis already in the ~FG%s\n",u->recap,rm->recap);
		return;
	}
	write_user(user,"~FR You wheel out your catapult....\n");
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (rm->hidden == 0) WriteRoomExcept(user->room,user,"~FT %s~FG wheels out %s catapult...\n",name,gender1[user->gender]);
	if (u->room->access >= INV_MALE)
	{
		if (!strcmp(u->room->owner,u->name))
		{
			write_user(u,"~FG Your room's access has been restored.\n");
			if (u->room->temp_access) u->room->access = u->room->temp_access;
			u->room->temp_access = 0;
		}
	}
	write_log(1,LOG4,"[MOVE] - User: [%s] Person moved: [%s] Room: [%s]\n",user->name,u->name,rm->name);
	if (rm->hidden) move_user(u,rm,4);
	else move_user(u,rm,2);
	destruct_room(rm2);
}

/* Join another user */
void join(User user,char *inpstr,int rt)
{
	Room rm,rm2;
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
		write_user(user,"~FG You can't join yourself.\n");
		return;
	}
	if (user->status & Shackled)
	{
		write_user(user,"~FM Don't those shackles just suck?\n");
		return;
	}
	rm = u->room;	rm2 = user->room;
	if (rm == user->room)
	{
		write_user(user,"~FG Why would you want to join someone who you're already with?\n");
		return;
	}
	if (rm2->hidden)
	{
		write_user(user,"~FG You can't join anyone while in a hidden room.\n");
		return;
	}
	if (rm->hidden)
	{
		if (user->invite_room != rm && !has_room_key(user,rm)
		  && strcmp(rm->owner,user->name))
		{
			write_user(user,center(notloggedon,80));
			return;
		}
		move_user(user,rm,4);
		destruct_room(rm2);
		return;
	}
	if (rm->access == INV_MALE)
	{
		if (user->gender != Male)
		{
			if (strcmp(rm->owner,user->name))
			{
				write_user(user,"~FG That room is currently only open to males!\n");
				return;
			}
			move_user(user,rm,1);
			destruct_room(rm2);
			return;
		}
		else
		{
			move_user(user,rm,1);
			destruct_room(rm2);
			return;
		}
	}
	if (rm->access == INV_FEM)
	{
		if (user->gender != Female)
		{
			if (strcmp(rm->owner,user->name))
			{
				write_user(user,"~FG That room is currently only open to females!\n");
				return;
			}
			move_user(user,rm,3);
			destruct_room(rm2);
			return;
		}
		else
		{
			move_user(user,rm,3);
			destruct_room(rm2);
			return;
		}
	}
	if (rm->access == PERSONAL || rm->access == STAFF_ROOM)
	{
		if (strcmp(rm->owner,user->name) && user->level<OWNER)
		{
			if (user->invite_room != rm && !has_room_key(user,rm))
			{
				writeus(user,"~FG The ~FT%s~FG isn't your personal room.\n",rm->recap);
				WriteRoom(rm,"~FG You can hear the frantic cries of ~FT%s~FG wanting in.\n",user->recap);
				return;
			}
		}
	}
	if (rm->access == PRIVATE && (!(user->invite_room == rm)))
	{
		writeus(user,"~FG The ~FT%s~FG room is currently locked and you're not invited in.\n",rm->recap);
		WriteRoom(rm,"~FG You can hear the frantic cries of ~FT%s~FG wanting in.\n",user->recap);
		return;
	}
	if (user->invis == 0)
	{
		if (user->predesc[0]) sprintf(text,"~FT %s ~FT%s ~FM%s to join ~FT%s ~FMin the ~FT%s.\n",user->predesc,user->recap,user->out_phr,u->recap,rm->recap);
		else sprintf(text,"~FT %s ~FM%s to join ~FT%s ~FMin the ~FT%s.\n",user->recap,user->out_phr,u->recap,rm->recap);
		write_room_except(user->room,user,text);
	}
	if (user->room->access >= INV_MALE)
	{
		if (!strcmp(rm2->owner,user->name))
		{
			write_user(user,"~FG Your room's access has been restored.\n");
			if (user->room->temp_access) user->room->access = user->room->temp_access;
			user->room->temp_access = 0;
		}
	}
	move_user(user,rm,3);
	destruct_room(rm2);
}

/* Toggles the review buffer for the room on/off */
void nobuff(User user,char *inpstr,int rt)
{
	Room rm;

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
	rm = user->room;
	if (rm->no_buff)
	{
		write_user(user,"~FG Review buffer has been enabled...\n");
		rm->no_buff = 0;
		return;
	}
	write_user(user,"~FG Review buffer has been disabled...\n");
	rm->no_buff = 1;
	SaveRoom(rm);
}

/* Display the details of the room */
void look(User user,char *inpstr,int rt)
{
	Room	rm;
	User	u;
	FILE	*fp;
	char	filename[FSL],line[ARR_SIZE+2],null[1],acsstr[40],rmname[100],
		temp[50],rmlevel[50];
	char	*afk = "[~CRA.~CRF.~CRK~RS]";
	char	*sline = "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
	int	i,exits,users,cnt,ccnt;

	if (user == NULL) return;
	rm = user->room;
	acsstr[0] = rmname[0] = '\0';
	cnt = ccnt = 0;
	write_user(user,ascii_line);
	sprintf(filename,"%s/%s/%s.R",RoomDir,Descript,rm->name);
	fp = fopen(filename,"r");
	if (fp == NULL)
	{
		write_user(user,center("~CR No description for this room.\n",80));
		if (rm->access == PERSONAL || rm->access == STAFF_ROOM)
			write_user(user,center(" To create one, use ~CW'~CR.edit~CW'\n",80));
		else if (user->level >= WIZ)
			write_user(user,center(" To create one, use ~CW'~CR.entroom~CW'\n",80));
	}
	else
	{
		fgets(line,ARR_SIZE-1,fp);
		while(!feof(fp))
		{
			write_user(user,line);
			fgets(line,ARR_SIZE-1,fp);
		}
		FCLOSE(fp);
	}
	write_user(user,ascii_line);
	cnt = colour_com_count(rm->recap);
	switch (rm->access)
	{
		case PUBLIC       :
		  sprintf(rmname,"~FT%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  if (rm->hidden) strcpy(acsstr,"~FBHIDDEN              ");
		  else strcpy(acsstr,"~FTUNLOCKED            ");
		  break;
		case PRIVATE      :
		  sprintf(rmname,"~FR%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  strcpy(acsstr,"~FRLOCKED              ");
		  break;
		case FIXED_PUBLIC :
		  sprintf(rmname,"~FG%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  strcpy(acsstr,"~FGFIXED PUBLIC        ");
		  break;
		case FIXED_PRIVATE:
		  sprintf(rmname,"~CR%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  strcpy(acsstr,"~CRFIXED PRIVATE       ");
		  break;
		case PERSONAL     :
		case STAFF_ROOM   :
		  sprintf(rmname,"~CY%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  if (rm->hidden) strcpy(acsstr,"~CYPERSONAL-HIDDEN     ");
		  else strcpy(acsstr,"~CYPERSONAL            ");
		  break;
		case INV_MALE     :
		  sprintf(rmname,"~FY%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  strcpy(acsstr,"~FYMALES ONLY          ");
		  break;
		case INV_FEM      :
		  sprintf(rmname,"~FY%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  strcpy(acsstr,"~FYFEMALES ONLY        ");
		  break;
		case INV_ALL      :
		  sprintf(rmname,"~FT%-*.*s",20+cnt*3,20+cnt*3,rm->recap);
		  strcpy(acsstr,"~FTPUBLIC              ");
		  break;
	}
	writeus(user," ~FGRoom Name~CB: [ %s ~CB]                   ~FGMessage Count~CB:  [ ~CR%2d ~CB ]\n",rmname,rm->mesg_cnt);
	sprintf(text," ~FGAccess   ~CB: [ %s ~CB]",acsstr);
	if (rm->owner[0] != '\0' && strcmp(rm->owner,"NULL"))
	{
		sprintf(temp,"                   ~FGOwner~CB: [ ~CR%-12.12s ~CB]\n",rm->owner);
		strcat(text,temp);
	}
	else if (rm->hidden
	  && (rm->access != STAFF_ROOM || rm->access != PERSONAL))
	{
		if (user->gender == Female) strncpy(rmlevel,level[rm->level].fname,sizeof(rmlevel)-1);
		else strncpy(rmlevel,level[rm->level].mname,sizeof(rmlevel)-1);
		sprintf(temp,"                   ~FGLevel~CB: [ ~CR%-12.12s ~CB]\n",rmlevel);
		strcat(text,temp);
	}
	else strcat(text,"\n");
	write_user(user,text);
	write_user(user,center(sline,80));
	write_user(user,center("~FGPossible Exits:\n",80));
	exits = cnt = 0;
	text[0] = '\0';
	for (i = 0 ; i < MaxLinks ; ++i)
	{
		if (rm->link[i][0] != '\0')
		{
			if (cnt == 0) sprintf(text,"                        ");
			sprintf(temp,"~FT%-20s   ",rm->link[i]);
			strcat(text,temp);
			++exits;  ++cnt;
			if (cnt%2 == 0)
			{
				strcat(text,"\n");
				write_user(user,text);
				text[0] = '\0';
				cnt = 0;
			}
		}
	}
	if (cnt == 1)
	{
		write_user(user,text);
		write_user(user,"\n");
	}
	if (exits == 0) write_user(user,center("~CB=-= ~FTThere are no exits out...~CB =-=\n",80));
	write_user(user,center(sline,80));
	users = 0;  null[0] = '\0';
	for_ausers(u)
	{
		if (u->type == CloneType) continue;
		if ((u->invis && u->level>user->level) || u->room != rm
		  || u == user || u->login
		  || (u->invis == Hidden && user->level<ADMIN)
		  || (u->invis == Fakelog && user->level<GOD)) continue;
		++users;
	}
	text[0] = '\0';
	cnt = 0;
	if (users)
	{
		write_user(user,center("~FGOther users in this room:\n",80));
		for_ausers(u)
		{
			if (u->type == CloneType) continue;
			if ((u->invis && u->level > user->level)
			  || u->room != rm || u == user || u->login
			  || (u->invis == Hidden && user->level<ADMIN)
			  || (u->invis == Fakelog && user->level<GOD)) continue;
			if (cnt==0) sprintf(text,"                  ");
			if (u->afk && u->invis>Invis) sprintf(line,"~FY  %-13s %s",ColourStrip(u->temp_recap),afk);
			else if (u->afk) sprintf(line,"~FY  %-13s %s",ColourStrip(u->recap),afk);
			else if (u->invis>Invis) sprintf(line,"~FY  %-21s",ColourStrip(u->temp_recap));
			else sprintf(line,"~FY  %-21s",ColourStrip(u->recap));
			++cnt;
			switch (u->invis)
			{
				case 1 : line[3]='I';  break;
				case 2 : line[3]='H';  break;
				case 3 : line[3]='F';  break;
				default: line[3]=' ';  break;
			}
			strcat(text,line);
			if (cnt%2 == 0)
			{
				strcat(text,"\n");
				write_user(user,text);
				text[0] = '\0';
				cnt = 0;
			}
		}
		if (cnt == 1)
		{
			write_user(user,text);
			write_user(user,"\n");
		}
		write_user(user,center(sline,80));
	}
	write_user(user,center("~FGThe current topic is~CB:\n",80));
	if (rm->topic[0] != '\0')
	{
		sprintf(text,"%s\n",rm->topic);
		write_user(user,center(text,80));
	}
	else
	{
		sprintf(text,"%s\n",def_topic);
		write_user(user,center(text,80));
	}
	write_user(user,center(sline,80));
}

/* Lets a user check out the description of another room */
void room_look(User user,char *inpstr,int rt)
{
	Room rm,rm2;
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
	rm2 = user->room;
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRroom name~CB]");
		return;
	}
	if ((rmname = get_room_full(user,inpstr)) == NULL)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	if ((rm = get_room(rmname)) == NULL)
	{
		if ((rm = create_room()) == NULL)
		{
			write_user(user,"~FG Unable to look into that room.\n");
			return;
		}
		strcpy(rm->name,rmname);
		if ((LoadRoom(rm,rmname)) < 1)
		{
			write_user(user,center(nosuchroom,80));
			destruct_room(rm);
			return;
		}
	}
	/* we do things the easy way now ;) */
	user->room = rm;
	look(user,NULL,0);
	user->room = rm2;
	destruct_room(rm);
	return;
}

/* assigns that atmospherics to the certain room */
void atmos_assign(Room rm)
{
	int i = 1;

	for (i = 1; i < (MaxAtmos+1);++i)
	{
		if (atmos[i][0] == '\0') continue; /* whoops.. lil bug.. ;) */
		strncpy(rm->atmos[i],atmos[i],sizeof(rm->atmos[1])-1);
	}
}

/* Saves the atmos */
int save_atmos(Room rm)
{
	FILE *fp;
	char filename[FSL],temp[ARR_SIZE+1];
	int i=0;

	if (rm == NULL) return(0);
	sprintf(filename,"%s/%s/%s.A",RoomDir,AtmosFiles,rm->name);
	if ((fp=fopen(filename,"w")) == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Unable to open file [%s.A] to write in save_atmos()!\n",rm->name);
		return(0);
	}
	for (i = 0 ; i < MaxAtmos ; ++i)
	{
		strncpy(temp,rm->atmos[i],AtmosLen);
		fprintf(fp,"%s\n",temp);
	}
	FCLOSE(fp);
	return(1);
}

void reset_access(Room rm)
{
	User u;
	int cnt;

	if (rm == NULL || rm->access != PRIVATE) return;
	cnt = 0;
	for_users(u)
	{
		if (u->room == rm) ++cnt;
	}
	if (cnt < min_private_users)
	{
		write_room(rm,center("~FT-~CT=~CB>~FG The lock on the door turns, unlocking it. ~CB<~CT=~FT-\n",80));
		rm->access = PUBLIC;
		/* Reset any invitations */
		for_users(u)
		{
			if (u->invite_room == rm) u->invite_room = NULL;
		}
		write_room(rm,center("~FT-~CT=~CB>~FG Review buffer has been cleared.. ~CB<~CT=~FT-\n",80));
		clear_convbuff(rm->revbuff);
		if (rm->no_buff)
		{
			write_room(rm,center("~FT-~CT=~CB>~FG Review buffer has been enabled. ~CB<~CT=~FT-\n",80));
			rm->no_buff = 0;
		}
	}
}

/* Called by go() and move(), as well as many other commands */
void move_user(User user,Room rm,int teleport)
{
	User u;
	Room old_room;
	Telnet t;

	old_room = user->room;
	if (teleport != 2 && !has_room_access(user,rm))
	{
		write_user(user,"~CW- ~FTThat room is currently locked, and you're not invited.\n");
		return;
	}
	/* Reset invite */
	if (user->invite_room == rm) user->invite_room = NULL;
	if (teleport != 4 || teleport != 5)
	{
		if (user->invis == Invis)
		{
			write_room(rm,invisenter);
			write_room_except(user->room,user,invisleave);
			goto SKIP;
		}
		if (user->invis >= Hidden) goto SKIP;
	}
	switch (teleport)
	{
		case 1 :
		  WriteRoom(rm,"~CW - ~FT%s~FY comes flying through the air and lands in front of you.\n",user->recap);
		  WriteRoomExcept(old_room,user,"~CW - ~FT%s~FY pulls the lever on %s catapult and gets launched.\n",user->recap,gender1[user->gender]);
		  goto SKIP;
		  break;
		case 2 :
		  write_user(user,"~CW -~FT You are forced into the catapult as someone pulls the lever.\n");
		  WriteRoom(rm,"~CW - ~FT%s ~FYcomes flying through the air and lands in front of you.\n",user->recap);
		  WriteRoomExcept(old_room,user,"~CW - ~FT%s~FY is forced onto the catapult as someone pulls the lever.\n",user->recap);
		  goto SKIP;
		  break;
		case 3 :
		  if (user->predesc[0] != '\0') sprintf(text,"~CW - ~FT%s ~FT%s ~FY%s\n",user->predesc,user->recap,user->in_phr);
		  else sprintf(text,"~CW - ~FT%s ~FY%s\n",user->recap,user->in_phr);
		  write_room(rm,text);
		  goto SKIP;
		  break;
		case 4 :
		  logging_in = 1;
		  WriteRoom(rm,"~CW - ~FT%s~FY comes flying through the air and lands in front of you.\n",user->recap);
		  switch (user->invis)
		  {
			case 1 :
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s~FM <=-\n",logout_inv,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s~FR %s~FM <=-\n",logout_inv,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_inv,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s~FR %s~FM <=-\n",logout_inv,user->recap,user->desc);
			  }
			  write_level(WIZ,user,text);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s] [INVIS]\n",user->name);
			  break;
			case 2 :
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s~FM <=-\n",logout_hide,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s~FR %s~FM <=-\n",logout_hide,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_hide,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s~FR %s~FM <=-\n",logout_hide,user->recap,user->desc);
			  }
			  write_level(ADMIN,user,text);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s] [HIDDEN]\n",user->name);
			  break;
			case 3 :
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s~FM <=-\n",logout_fake,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s~FR %s~FM <=-\n",logout_fake,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_fake,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s~FR %s~FM <=-\n",logout_fake,user->recap,user->desc);
			  }
			  write_level(GOD,user,text);
			  break;
			default:
			  WriteRoomExcept(old_room,user,"~CW-~FG %s~FT %s\n",user->recap,user->logout_msg);
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s~FM <=-\n",logout_vis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s~FR %s~FM <=-\n",logout_vis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s~FM <=-\n",logout_vis,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s~FR %s~FM <=-\n",logout_vis,user->recap,user->desc);
			  }
			  WriteRoomExcept(NULL,user,text);
			  friend_check(user,2);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s]\n",user->name);
			  break;
		  }
		  logging_in = 0;
		  SaveUser(user,1);
		  reset_junk(user);
		  t = NULL;
		  if ((t = get_telnet(user)) != NULL) disconnect_telnet(t);
		  destroy_user_clones(user);
		  user->room = rm;
		  look(user,NULL,0);
		  reset_access(old_room);
		  return;
		case 5 :
		  logging_in = 1;
		  WriteRoomExcept(old_room,user,"~CW - ~FT%s~FY pulls the lever on %s catapult and gets launched.\n",user->recap,gender1[user->gender]);
		  switch(user->invis)
		  {
			case 1 :
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_invis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s ~FM%s~CB <=-\n",login_invis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s ~CB<=-\n",login_invis,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FM%s ~CB<=-\n",login_invis,user->recap,user->desc);
			  }
			  write_level(WIZ,user,text);
			  write_room(user->room,"~CW - ~FTA mystical being is now amongst you.\n");
			  sprintf(text,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  write_level(WIZ,user,text);
			  write_log(1,LOG2,"[LOGIN-I] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->ip_site,user->site_port,user->port);
			  break;
			case 2 :
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_hide,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s ~FM%s~CB <=-\n",login_hide,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s ~CB<=-\n",login_hide,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FM%s ~CB<=-\n",login_hide,user->recap,user->desc);
			  }
			  write_level(ADMIN,user,text);
			  sprintf(text,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  write_level(ADMIN,user,text);
			  write_log(1,LOG2,"[LOGIN-H] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->ip_site,user->site_port,user->port);
			  break;
			case 3 :
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_fake,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s ~FM%s~CB <=-\n",login_fake,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s ~CB<=-\n",login_fake,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FM%s ~CB<=-\n",login_fake,user->recap,user->desc);
			  }
			  write_level(GOD,user,text);
			  sprintf(text,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  write_level(GOD,user,text);
			  break;
			default:
			  if (user->predesc[0] != '\0')
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_vis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s ~FM%s~CB <=-\n",login_vis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s ~CB<=-\n",login_vis,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FM%s ~CB<=-\n",login_vis,user->recap,user->desc);
			  }
			  write_room_except(NULL,user,text);
			  if (user->desc[0] == '\'') sprintf(text,"~FT%s ~FY%s~RS%s\n",long_date(0),user->recap,user->desc);
			  else sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
			  RecordLogin(text);
			  sprintf(text,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  write_level(WIZ,user,text);
			  write_log(1,LOG2,"[LOGIN] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->ip_site,user->site_port,user->port);
			  friend_check(user,1);
			  wait_check(user);
			  break;
		  }
		  logging_in = 0;
		  SaveUser(user,1);
		  reset_junk(user);
		  user->last_login = time(0);
		  user->room = rm;
		  look(user,NULL,0);
		  if (user->invis == 0)
		  {
			WriteRoomExcept(rm,user,"~CW - ~FG%s~FR %s\n",user->recap,user->login_msg);
		  }
		  return;
		default:
		  if (user->predesc[0] != '\0') sprintf(text,"~CW - ~FT%s ~FT%s~FY %s\n",user->predesc,user->recap,user->in_phr);
		  else sprintf(text,"~CW - ~FT%s ~FY%s\n",user->recap,user->in_phr);
		  write_room(rm,text);
		  if (user->predesc[0] != '\0') sprintf(text,"~CW - ~FT%s ~FT%s~FY %s to the %s.\n",user->predesc,user->recap,user->out_phr,rm->recap);
		  else sprintf(text,"~CW - ~FT%s ~FY%s to the %s.\n",user->recap,user->out_phr,rm->recap);
		  write_room_except(old_room,user,text);
		  goto SKIP;
		  break;
	}

	SKIP:
	  user->room = rm;
	  look(user,NULL,0);
	  reset_access(old_room);
	  if (user->follow_by[0] != '\0')
	  {
		if ((u = get_user(user->follow_by)) == NULL)
		{
			user->follow_by[0] = '\0';
			return;
		}
	  move_user(u,rm,1);
	  }
}

void room_key(User user,char *inpstr,int rt)
{
	Room rm;
	int i,used,cnt,found;
	char *rmname,*uname;

	i = used = cnt = found = 0;
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW|~CR-list~CB]");
		return;
	}
	rmname = get_room_full(user,user->name);
	if (rmname == NULL)
	{
		if (user->has_room == 0 && free_rooms == 1)
		{
			user->has_room = 1;
			home_room(user);
		}
		else if ((user->has_room == 0 && free_rooms == 0)
		  && ((user->tpromoted && user->orig_lev < WIZ)
		  || user->level < WIZ))
		{
			write_user(user,"~CW- ~FT You don't even have a personal room.\n");
			return;
		}
		else
		{
			user->has_room = 1;
			home_room(user);
		}
		rmname = get_room_full(user,user->name);
		/* if room is still NULL, an error somewhere */
		if (rmname == NULL)
		{
			write_user(user,"~CW-~FT Sorry, an error occurred in finding your room.\n");
			return;
		}
	}
	else home_room(user);
	rm = get_room(rmname);
	if (rm == NULL)
	{
		write_user(user,"~CW-~FT Sorry.. an error occurred in finding your room.\n");
		return;
	}
	if (!strncasecmp(word[1],"-list",2))
	{
		if (user->keys == 0)
		{
			write_user(user,"~CW-~FG You have no keys.. so noone has a key to this room.\n");
			return;
		}
		for (i = 0 ; i < user->keys ; ++i)
		{
			if (rm->key[i][0] != '\0')
			{
				if (++used == 1)
				{
					if (user->ansi_on) write_user(user,ansi_tline);
					else write_user(user,ascii_tline);
					write_user(user,center("~FT-~CT=~CB>~FG A list of users who have keys to your room. ~CB<~CT=~FT-\n",80));
					if (user->ansi_on) write_user(user,ansi_bline);
					else write_user(user,ascii_bline);
				}
				writeus(user,"\t%s\t",rm->key[i]);
				++cnt;
				if (cnt%4 == 0)
				{
					write_user(user,"\n");
					cnt = 0;
				}
			}
		}
		if (cnt > 0 && cnt < 4) write_user(user,"\n");
		if (used == 0) write_user(user,"~CW-~FT Noone has a key to this room.\n");
		else writeus(user,"~CW-~FT You have given %d key%s away out of your %d.\n",used,used>1?"s":"",user->keys);
		return;
	}
	/* First check to see if we're removing a user. */
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	if (!strcasecmp(user->name,word[1]))
	{
		write_user(user,"~CW-~FT You don't really need to give yourself a key to this room.\n");
		return;
	}
	for (found = 0 ; found < user->keys ; ++found)
	{
		if (!strcasecmp(rm->key[found],word[1]))
		{
			/* We found a user who has a key.. remove them */
			writeus(user,"~CW-~FT Alright.. you take ~FG%s%s ~FTkey away.\n",rm->key[found],rm->key[found][strlen(rm->key[found])-1]=='s'?"'":"'s");
			rm->key[found][0] = '\0';
			SaveRoom(rm);
			return;
		}
	}
	/* Ok.. now we go and see if we're adding a user. */
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	for (i = 0 ; i < user->keys ; ++i)
	{
		if (rm->key[i][0] == '\0')
		{
			if (i > MaxKeys)
			{
				user->keys = MaxKeys;
				write_user(user,"~CW-~FT You've given away all of your keys.\n");
				return;
			}
			strncpy(rm->key[i],uname,sizeof(rm->key[0])-1);
			writeus(user,"~CW-~FY %s~FG has been given a key to your room.\n",uname);
			sprintf(text,"~FT %s~FG has given you a key to %s room.\n",user->invis>Invis?user->temp_recap:user->recap,gender1[user->gender]);
			write_user(get_user(uname),text);
			SaveRoom(rm);
			return;
		}
	}
	write_user(user,"~CW-~FG You've given away all of your room's keys.\n");
	return;
}

/*
   See if a user has access to a room. If room is fixed private then it is
   considered a wiz's room so grant permission to any user of WIZ and above.
*/
int has_room_access(User user, Room rm)
{
	if ((rm->access == PRIVATE)
	  && user->level < gatecrash_level
	  && user->invite_room != rm
	  && !((rm->access & FIXED) && user->level >= WIZ)) return(0);
	return(1);
}

/* END OF ROOMS.C */
