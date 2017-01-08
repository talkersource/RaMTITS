/*
  objects.c
    The code in that handles the creation of users, rooms, and buffers,
    as well as destructs them, and returns an object based on another
    variable.

    The source within this file contains Copyrights 1998 - 2001 from
 	Rob Melhuish, Andrew Collington, Tim van der Leeuw, and Neil
 	Robertson.

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
#include <ctype.h>
#include "include/ident.h"
#define _OBJECTS_SOURCE
  #include "include/needed.h"
#undef _OBJECTS_SOURCE

/* Create user/clone object */
User create_user(void)
{
	User user;

	if ((user = (User)malloc(sizeof(struct user_struct))) == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in create_user()\n");
		return(NULL);
	}
	memset(user,0,sizeof(struct user_struct));
	/* append into linked list. */
	if (user_first == NULL)
	{
		user_first = user;
		user->prev = NULL;
	}
	else
	{
		user_last->next = user;
		user->prev	= user_last;
	}
	user->next	= NULL;
	user_last	= user;
	/* Init the user structure. */
	do
	{
		id_count = ran(1000);
	} while (get_userid(id_count) != NULL);
	user->id	= id_count;
	user->type	= UserType;	user->name[0]	= '\0';
	user->socket	= -1;		user->port	= 0;
	user->attempts	= 0;		user->login	= 0;
	user->site_port	= 0;		user->site[0]	= '\0';
	user->ip_site[0]= '\0';		user->ident_id	= AWAITING_IDENT;
	user->away_buff	= NULL;		user->tbuff	= NULL;
	user->buffer	= NULL;
	strcpy(user->realname,IDENT_NOTYET);
	reset_vars(user);
	user->last_input = time(0);
	return(user);
}

/*
   Puts the user into an alphabetical assortment. Based on Andy's assortment,
   of commands.. I thought it might work for users.. turned out it did :)
*/
void alpha_sort(User user)
{
	User tmp;
	int inserted;

	inserted=0;
	if (user_afirst == NULL)
	{
		user_afirst = user;	user->aprev = NULL;
		user->anext = NULL;	user_alast  = user;
	}
	else
	{
		tmp = user_afirst;
		while (tmp != NULL)
		{
			/* Insert as first item in the list */
			if ((strcmp(user->name,tmp->name)<0)
			  && tmp==user_afirst)
			{
				user_afirst->aprev = user;
				user->aprev	   = NULL;
				user->anext	   = user_afirst;
				user_afirst	   = user;
				inserted=1;
			}
			/* insert it into the middle of the list somewhere */
			else if (strcmp(user->name,tmp->name)<0)
			{
				tmp->aprev->anext = user;
				user->aprev	  = tmp->aprev;
				tmp->aprev	  = user;
				user->anext	  = tmp;
				inserted=1;
			}
			if (inserted) break;
			tmp=tmp->anext;
		}
		/* Otherwise insert at the end of the list. */
		if (inserted == 0)
		{
			user_alast->anext	= user;
			user->aprev		= user_alast;
			user->anext		= NULL;
			user_alast		= user;
		}
	}
	return;
}

/* Destruct a regular user object. */
void destruct_user(User user)
{

	if (user->buffer != NULL) release_inputbuff(user);
	/* remove from linked list */
	if (user == user_first)
	{
		user_first = user->next;
		if (user == user_last) user_last = NULL;
		else user_first->prev = NULL;
	}
	else
	{
		user->prev->next = user->next;
		if (user == user_last)
		{
			user_last	= user->prev;
			user_last->next = NULL;
		}
		else user->next->prev = user->prev;
	}
	last_uid = user->id;
	--id_count;
	user->socket = -1;
	memset(user,0,sizeof(struct user_struct));
	free((User)user);
	destructed = 1;
}

/* We *SHOULD* remove alpha sorted users from the linked lists as well ;-) */
void destruct_alpha(User user)
{

	if (user == NULL) return;
	if (user == user_afirst)
	{
		user_afirst = user->anext;
		if (user == user_alast) user_alast = NULL;
		else user_afirst->aprev = NULL;
	}
	else
	{
		user->aprev->anext = user->anext;
		if (user == user_alast)
		{
			user_alast	  = user->aprev;
			user_alast->anext = NULL;
		}
		else user->anext->aprev = user->aprev;
	}
}

/*
   Adds the users name and level to a userlist in its own
   linked list so we can handle everything with greater ease.
*/
int add_userlist(char *name,int lev)
{
	Ulobj add;

	if ((add=(Ulobj)malloc(sizeof(struct userlist_struct)))==NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in add_userlist()\n");
		return(0);
	}
	memset(add,0,sizeof(struct userlist_struct));
	if (ulist_first == NULL)
	{ 
		ulist_first	= add;
		add->prev	= NULL;
	}
	else
	{
		ulist_last->next = add;
		add->prev	 = ulist_last;
	}
	add->next = NULL;		ulist_last = add;
	++user_count;
	strcpy(add->name,name);
	add->lev=lev;
	return(1);
}

/* And clean it up.. */
int clean_userlist(char *name)
{
	Ulobj ulist;
	int gotit;

	ulist = ulist_first;
	gotit = 0;
	while (ulist != NULL)
	{
		if (!strcmp(ulist->name,name))
		{
			gotit=1;
			break;
		}
		ulist = ulist->next;
	}
	if (gotit == 0) return(0);
	/* Remove from linked list */
	if (ulist == ulist_first)
	{
		ulist_first = ulist->next;
		if (ulist == ulist_last) ulist_last = NULL;
		else ulist_first->prev = NULL;
	}
	else
	{
		ulist->prev->next = ulist->next;
		if (ulist == ulist_last)
		{
			ulist_last	 = ulist->prev;
			ulist_last->next = NULL;
		}
		else ulist->next->prev = ulist->prev;
	}
	memset(ulist,0,sizeof(struct userlist_struct));
	free(ulist);
	--user_count;
	return(1);
}

/* Destroy all clones belonging to given user. */
void destroy_user_clones(User user)
{
	User u;

	for_users(u)
	{
		if (u->type==CloneType && u->owner==user)
		{
			WriteRoomExcept(u->room,u,"~FT %s ~RSwatches helplessly as a mysterious fog swallows %s.\n",user->recap,gender2[user->gender]);
			destruct_user(u);
		}
	}
}

/* Create a room object */
Room create_room(void)
{
	Room room;

	if ((room=(Room)malloc(sizeof(struct room_struct)))==NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in create_room()\n");
		return(NULL);
	}
	memset(room,0,sizeof(struct room_struct));
	/* Append into linked list */
	if (room_first == NULL)
	{
		room_first = room;
		room->prev = NULL;
	}
	else
	{
		room_last->next = room;
		room->prev	= room_last;
	}
	room->next = NULL;
	room_last  = room;
	room->revbuff = NULL;
	reset_room(room);
	return(room);
}

/* Resets the rooms variables */
void reset_room(Room room)
{
	int i;

	room->name[0]	= '\0';		room->topic[0]	 = '\0';
	room->owner[0]	= '\0';		room->map[0]	 = '\0';
	room->access	= -1;		room->topic_lock = 0;
	room->mesg_cnt	= 0;		room->no_buff	 = 0;
	room->lock	= 0;
	if (room->revbuff == NULL) room->revbuff=newconv_buffer();
	for (i=0;i<MaxLinks;++i) room->link[i][0]='\0';
	for (i=0;i<MaxKeys;++i) room->key[i][0]='\0';
	for (i=0;i<MaxAtmos;++i) room->atmos[i][0]='\0';
	return;
}

/* Destruct the room as well. */
void destruct_room(Room room)
{
	int cnt;

	if (room == NULL) return;
	/*
	   We leave our three main rooms alone ALL the time, otherwise it
	   could seriously fuck things up
	*/
	if (boot_up == 0)
	{
		if (!strcasecmp(room->name,main_room)) return;
		if (!strcasecmp(room->name,idle_room)) return;
		if (!strcasecmp(room->name,jail_room)) return;
	}
	cnt = count_room_users(room);
	if (cnt) return; /* if users in the room, don't destruct it. */
	/* now we remove it from the linked list. */
	if (room == room_first)
	{
		room_first = room->next;
		if (room == room_last) room_last = NULL;
		else room_first->prev = NULL;
	}
	else
	{
		room->prev->next = room->next;
		if (room == room_last)
		{
			room_last	= room->prev;
			room_last->next	= NULL;
		}
		else room->next->prev = room->prev;
	}
	if (room->revbuff) delete_convbuff(room->revbuff);
	memset(room,0,sizeof(struct room_struct));
	free(room);
}

/*
   Adds the room to a roomlist in its own linked list so we can handle
   everything with greater ease.
*/
int add_roomlist(Room room)
{
	Rmlobj add;

	if (room == NULL) return(0);
	if ((add=(Rmlobj)malloc(sizeof(struct rmlist_struct)))==NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in add_roomlist()\n");
		return(0);
	}
	memset(add,0,sizeof(struct rmlist_struct));
	if (rlist_first == NULL)
	{
		rlist_first	= add;
		add->prev	= NULL;
	}
	else
	{
		rlist_last->next = add;
		add->prev	 = rlist_last;
	}
	add->next  = NULL;
	rlist_last = add;
	strcpy(add->name,room->name);
	strcpy(add->map,room->map);
	add->access=room->access;
	return(1);
}

/* And clean it up.. */
int clean_roomlist(char *name)
{
	Rmlobj rlist;
	int gotit;

	rlist = rlist_first;
	gotit=0;
	while (rlist!=NULL)
	{
		if (!strcmp(rlist->name,name))
		{
			gotit=1;
			break;
		}
		rlist = rlist->next;
	}
	if (gotit == 0) return(0);
	/* Remove from linked list */
	if (rlist == rlist_first)
	{
		rlist_first = rlist->next;
		if (rlist == rlist_last) rlist_last = NULL;
		else rlist_first->prev = NULL;
	}
	else
	{
		rlist->prev->next = rlist->next;
		if (rlist == rlist_last)
		{
			rlist_last	 = rlist->prev;
			rlist_last->next = NULL;
		}
		else rlist->next->prev = rlist->prev;
	}
	memset(rlist,0,sizeof(struct rmlist_struct));
	free(rlist);
	return(1);
}

/* Add a command into the command linked list. */
int add_command(int cmd_id)
{
	int inserted,i;
	Commands cmd,tmp;

	if ((cmd=(Commands)malloc(sizeof(struct cmd_struct)))==NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in add_command().\n");
		return(0);
	}
	memset(cmd,0,sizeof(struct cmd_struct));
	/* Throw it in the linked list */
	inserted = 0;
	strcpy(cmd->name,command[cmd_id].name);
	if (first_command == NULL)
	{
		first_command	= cmd;
		cmd->prev	= NULL;
		cmd->next	= NULL;
		last_command	= cmd;
	}
	else
	{
		tmp = first_command;
		inserted = 0;
		while (tmp != NULL)
		{
			/* Insert as first item in the list */
			if ((strcmp(cmd->name,tmp->name)<0)
			  && tmp == first_command)
			{
				first_command->prev = cmd;
				cmd->prev	= NULL;
				cmd->next	= first_command;
				first_command	= cmd;
				inserted	= 1;
			}
			/* otherwise put it somewhere in the middle */
			else if (strcmp(cmd->name,tmp->name)<0)
			{
				tmp->prev->next	= cmd;
				cmd->prev	= tmp->prev;
				tmp->prev	= cmd;
				cmd->next	= tmp;
				inserted	= 1;
			}
			if (inserted) break;
			tmp = tmp->next;
		}
		/* Insert at the end of the list */
		if (inserted == 0)
		{
			last_command->next = cmd;
			cmd->prev	= last_command;
			cmd->next	= NULL;
			last_command	= cmd;
		}
	}
	cmd->id		= cmd_id;
	cmd->min_lev	= command[cmd_id].lev;
	cmd->type	= command[cmd_id].type;
	cmd->funct	= command[cmd_id].funct;
	cmd->args	= command[cmd_id].args;
	cmd->count	= 0;
	cmd->disabled	= 0;
	for (i = 0 ; i < 20 ; ++i)
	{
		if (!strcmp(cmd->name,disabled[i])) cmd->disabled = 1;
	}
	return(1);
}

/*
  Should be able to destruct the command as well, even tho I've never used
  this function.. I suppose it could be useful in it's own little way.
*/
int destruct_command(int cmd_id)
{
	Commands cmd;

	/* find the command we wanna destruct */
	cmd = first_command;
	while (cmd != NULL)
	{
		if (cmd->id == cmd_id) break;
		cmd = cmd->next;
	}
	if (cmd == first_command)
	{
		first_command = cmd->next;
		if (cmd == last_command) last_command = NULL;
		else first_command->prev = NULL;
	}
	else
	{
		cmd->prev->next = cmd->next;
		if (cmd==last_command)
		{
			last_command	   = cmd->prev;
			last_command->next = NULL;
		}
		else cmd->next->prev = cmd->prev;
	}
	memset(cmd,0,sizeof(struct cmd_struct));
	free(cmd);
	return(1);
}

/* Add a social into the social linked list. */
int add_social(char *name)
{
	Socials soc;

	if ((soc=(Socials)malloc(sizeof(struct soc_struct)))==NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in add_social().\n");
		return(0);
	}
	memset(soc,0,sizeof(struct soc_struct));
	/* Throw it in the linked list */
	if (first_social == NULL)
	{
		first_social	= soc;
		soc->prev	= NULL;
	}
	else
	{
		last_social->next = soc;
		soc->prev	  = last_social;
	}
	soc->next	= NULL;
	last_social	= soc;
	strcpy(soc->name,name);
	return(1);
}

/* Should be able to destruct the social as well. */
int destruct_social(char *name)
{
	Socials soc;

	/* find the social we wanna destruct */
	soc = first_social;
	while (soc != NULL)
	{
		if (!strcasecmp(soc->name,name)) break;
		soc = soc->next;
	}
	if (soc == first_social)
	{
		first_social = soc->next;
		if (soc == last_social) last_social = NULL;
		else first_social->prev = NULL;
	}
	else
	{
		soc->prev->next = soc->next;
		if (soc == last_social)
		{
			last_social	  = soc->prev;
			last_social->next = NULL;
		}
		else soc->next->prev = soc->prev;
	}
	memset(soc,0,sizeof(struct soc_struct));
	free(soc);
	return(1);
}

/* Get user pointer from name. */
User get_user(char *name)
{
	User u;

	for_users(u)
	{
		if (u->login || u->type == CloneType) continue;
		if (!strcasecmp(u->name,name)) return(u);
	}
	return(NULL);
}

/* Get clone from name. */
User get_clone(char *name)
{
	User u;

	for_users(u)
	{
		if (u->login || u->type != CloneType) continue;
		if (!strcasecmp(u->owner->name,name)) return(u);
	}
	return(NULL);
}

/* Get clone from name in a certain room. */
User get_clone_here(Room rm,char *name)
{
	User u;

	for_users(u)
	{
		if (u->login || u->type != CloneType) continue;
		if (u->room != rm) continue;
		if (!strcasecmp(u->owner->name,name)) return(u);
	}
	return(NULL);
}

/* Get user pointer from id #. */
User get_userid(int id_number)
{
	User u;

	for_users(u)
	{
		if (u->id == id_number) return(u);
	}
	return(NULL);
}

/*
  Searches for a users name, something like above, except if there are
  multiple matches found then it lets the user know all of the diff.
  matches.. This is based off of the amnuts getname function.
*/
User get_name(User user,char *name)
{
	User u,last;
	int found=0;
	char name2[UserNameLen+2],text2[ARR_SIZE];

	strncpy(name2,name,sizeof(name2)-1);
	name2[0] = toupper(name2[0]);
	text[0]	 = '\0';
	last	 = NULL;
	for_users(u)
	{
		if (!strcmp(u->name,name2))
		{
			/*
			   We have the exact name here.. but we need to make
			   sure and check to see the user->invis flags. We
			   can't have someone .t'ing someone who's fake
			   logged off, can we?
			*/
			if (u->invis == Fakelog && user->level<GOD)
			  return(NULL);
			if (u->invis == Hidden && user->level<ADMIN)
			  return(NULL);
			if (u->invis == Invis && user->level<u->level)
			  return(NULL);
			if ((u->room->hidden && !has_room_key(user,u->room)
			  && u->room != user->room)
			  && user->level<GOD
			  && u->room != user->invite_room) return(NULL);
			return(u);
		}
	}
	for_users(u)
	{
		if (u->type == CloneType || u->login) continue;
		if (u->invis == Fakelog && user->level<GOD) continue;
		if (u->invis == Hidden && user->level<ADMIN) continue;
		if (u->invis == Invis && user->level<u->level) continue;
		if ((u->room->hidden && !has_room_key(user,u->room)
		  && u->room != user->room) && user->level<GOD
		  && u->room != user->invite_room)
		  continue;
		if (ainstr(u->name,name2)!=-1)
		{
			strcat(text,u->name);
			strcat(text,"   ");
			found++;
			last = u;
		}
	}
	if (found == 0) return(NULL);
	if (found>1)
	{
		write_user(user,"~CB[~FG Possible matches~CB: ]\n");
		sprintf(text2,"  ~FT%s\n",text);
		write_user(user,text2);
		return(NULL);
	}
	return(last);
}

/*
   Now we'll go through the user list, search for the name of the users,
   and return the user name, providing there aren't more then one matches
   found.. we do it as a char * type, because we are returning the name
   of a user rather then the actual user object.
*/
char *get_user_full(User user,char *name)
{
	Ulobj ul,last;
	int found=0,cnt=0;
	char name2[UserNameLen+2],text2[ARR_SIZE];

	strncpy(name2,name,UserNameLen);  strtolower(name2);
	name2[0] = toupper(name2[0]);	  text[0] = text2[0]='\0';
	last	 = NULL;
	for_list(ul)
	{
		 if (!strcmp(ul->name,name2)) return(ul->name);
	}
	for_list(ul)
	{
		if (ainstr(ul->name,name2)!=-1)
		{
			++cnt;
			strcat(text,ul->name);
			if (cnt%5 == 0)
			{
				strcat(text,"\n~FT  ");
				cnt=0;
			}
			strcat(text,"   ");
			found++;
			last = ul;
		}
	}
	if (found == 0) return(NULL);
	if (found>1)
	{
		if (user != NULL)
		{
			write_user(user,"~CB[~FG Possible matches~CB: ]\n");
			sprintf(text2,"  ~FT%s\n",text);
			write_user(user,text2);
		}
		return(NULL);
	}
	return(last->name);
}

/* Get a room object based from a name */
Room get_room(char *name)
{
	Room rm;

	if (name == (char *)NULL) return(NULL);
	for_rooms(rm)
	{
		if (!strcasecmp(rm->name,name))
		  return(rm);
	}
	return(NULL);
}

/* get a room object based on the recapped name */
Room get_room_recap(char *name)
{
	Room rm;
	char *tmpname;

	if (name == (char *)NULL) return(NULL);
	for_rooms(rm)
	{
		tmpname = ColourStrip(rm->recap);
		if (!strncasecmp(name,tmpname,strlen(name)))
			return(rm);
	}
	return(NULL);
}

/*
  Now we'll go through the room list, search for the name of the rooms,
  and return the room name, providing there aren't more then one matches
  found.. we do it as a char * type, because we are returning the name
  of a room rather then the actual room object.
*/
char *get_room_full(User user,char *name)
{
	Rmlobj rl,last;
	Room rm;
	int found=0,cnt=0;
	char name2[RoomNameLen+2],text2[ARR_SIZE];

	strncpy(name2,name,RoomNameLen);  strtolower(name2);
	name2[0] = toupper(name2[0]);	  text[0] = text2[0]='\0';
	last	 = NULL;
	/* Quick check to see if we're dealing with a room recap. */
	for_rooms(rm)
	{
		if (!strcasecmp(ColourStrip(rm->recap),name)) return(rm->name);
	}
	for_rmlist(rl)
	{
		if (!strcmp(rl->name,name2)) return(rl->name);
	}
	for_rmlist(rl)
	{
		if (ainstr(rl->name,name2)!=-1)
		{
			strcat(text,rl->name);
			++cnt;
			if (cnt%4 == 0)
			{
				strcat(text,"\n~FT  ");
				cnt = 0;
			}
			strcat(text,"   ");
			found++;
			last = rl;
		}
	}
	if (found == 0) return(NULL);
	if (found>1)
	{
		write_user(user,"~CB[~FG Possible matches~CB: ]\n");
		sprintf(text2,"  ~FT%s\n",text);
		write_user(user,text2);
		return(NULL);
	}
	return(last->name);
}

/* return a level value based on a level name */
int get_level(char *name)
{
	int i;

	i = 0;
	while (level[i].name[0] != '*')
	{
		if (!strncasecmp(level[i].name,name,3)) return(i);
		else if (!strncasecmp(level[i].fname,name,3)) return(i);
		else if (!strncasecmp(level[i].mname,name,3)) return(i);
		++i;
	}
	return(-1);
}

/* Get a social value based on a name. */
Socials get_socname(char *name)
{
	Socials soc;

	for (soc=first_social;soc!=NULL;soc=soc->next)
	{
		if (!strncasecmp(soc->name,name,strlen(name))) return(soc);
	}
	return(NULL);
}

/* Get a command value based on a name. */
Commands get_command(User user,char *name)
{
	Commands com,last=NULL;
	char name2[21]; /* Should be enough? */
	char text2[ARR_SIZE*2];
	int found=0;

	strncpy(name2,name,20);
	text[0]  = '\0';
	text2[0] = '\0';
	for_comms(com)
	{
		if (!strcmp(com->name,name2)) return(com);
	}
	for_comms(com)
	{
		if (!strncmp(com->name,name2,strlen(name2))
		  && user->level >= com->min_lev)
		{
			if (found == 0) strcat(text,"    ~FT");
			else if (found%7 == 0) strcat(text,"\n    ~FT");
			strcat(text,com->name);
			strcat(text,"  ");
			found++;
			last = com;
		}
	}
	if (found%7) strcat(text,"\n");
	else strcat(text,"\n");
	if (found == 0) return(NULL);
	if (found > 1)
	{
		text2[0] = '\0';
		write_user(user,"~CB[~FG Ambiguous command~CB: ]\n");
		sprintf(text2,"~FT%s",text);
		write_user(user,text2);
		return(NULL);
	}
	return(last);
}

/* get a command by it's full name */
Commands get_command_full(char *name)
{
	Commands cmd;

	for_comms(cmd)
	{
		if (!strcasecmp(cmd->name,name)) return(cmd);
	}
	return(NULL);
}

/* Create ban object */
Bans new_ban(char *who,int type)
{
	Bans b;

	if ((b = (Bans)malloc(sizeof(struct ban_struct))) == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in new_ban()\n");
		return(NULL);
	}
	memset(b,0,sizeof(struct ban_struct));
	/* append into linked list. */
	if (first_ban == NULL)
	{
		first_ban = b;
		b->prev = NULL;
	}
	else
	{
		last_ban->next = b;
		b->prev	= last_ban;
	}
	b->next	= NULL;
	last_ban= b;
	strncpy(b->who,who,sizeof(b->who)-1);
	b->type = type;
	return(b);
}

/* get rid of ban */
void nuke_ban(Bans b)
{

	/* remove from linked list */
	if (b == first_ban)
	{
		first_ban = b->next;
		if (b == last_ban) last_ban = NULL;
		else first_ban->prev = NULL;
	}
	else
	{
		b->prev->next = b->next;
		if (b == last_ban)
		{
			last_ban = b->prev;
			last_ban->next = NULL;
		}
		else b->next->prev = b->prev;
	}
	memset(b,0,sizeof(struct ban_struct));
	free((Bans)b);
}

/* Get an exact ban match. */
Bans find_exact_ban(char *ban)
{
	Bans b;

	for_bans(b)
	{
		if (!strcasecmp(b->who,ban)) return(b);
	}
	return(NULL);
}

/* Get an exact ban match. */
Bans find_ban(char *ban,int type)
{
	Bans b;

	for_bans(b)
	{
		if (b->type != type) continue;
		if (!strcasecmp(b->who,ban)) return(b);
	}
	return(NULL);
}

/* Create an conversation buffer. */
Convbuff create_convbuff(void)
{
	Convbuff buff;
	int i;

	if ((buff=(Convbuff)malloc(sizeof(struct conv_buff)))==NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Memory allocation failure in create_convbuff()\n");
		return(NULL);
	}
	memset(buff,0,sizeof(struct conv_buff));
	/* append into linked list. */
	if (first_conv==NULL)
	{
		first_conv = buff;
		buff->prev = NULL;
	}
	else
	{
		last_conv->next = buff;
		buff->prev	= last_conv;
	}
	buff->next  = NULL;
	last_conv   = buff;
	/* Init the buff structure. */
	buff->count = 0;
	for (i=0;i<NUM_LINES;++i) buff->conv[i] = NULL;
	return(buff);
}

/* Gotta destruct it too i suppose. */
void delete_convbuff(Convbuff buff)
{

	if (buff==NULL) return;
	clear_convbuff(buff);
	/* remove from linked list */
	if (buff==first_conv)
	{
		first_conv=buff->next;
		if (buff==last_conv) last_conv=NULL;
		else first_conv->prev=NULL;
	}
	else
	{
		buff->prev->next=buff->next;
		if (buff==last_conv)
		{
			last_conv=buff->prev;
			last_conv->next=NULL;
		}
		else buff->next->prev=buff->prev;
	}
	memset(buff,0,sizeof(struct conv_buff));
	free(buff);
}

/*
   Resets a users variables. There's gotta be a better way for this...
   If you got one.. let me know.
*/
void reset_vars(User user)
{
	int i,x,y;

	user->recap[0]		= '\0';	user->desc[0]		= '\0';
	user->predesc[0]	= '\0';	user->lev_rank[0]	= '\0';
	user->pass[0]		= '\0';	user->email[0]		= '\0';
	user->webpage[0]	= '\0'; user->bday[0]		= '\0';
	user->married_to[0]	= '\0';	user->in_phr[0]		= '\0';
	user->out_phr[0]	= '\0';	user->last_site[0]	= '\0';
	user->afk_mesg[0]	= '\0';	user->autoexec[0]	= '\0';
	user->array[0]		= '\0'; user->talk_to[0]	= '\0';
	user->hangman_guessed[0]= '\0'; user->hangman_word[0]	= '\0';
	user->hangman_display[0]= '\0'; user->hangman_opp[0]	= '\0';
	user->check[0]		= '\0'; user->follow_by[0]	= '\0';
	user->following[0]	= '\0';	user->login_msg[0]	= '\0';
	user->logout_msg[0]	= '\0';	user->waiting[0]	= '\0';
	user->temp_recap[0]	= '\0';	user->bfafk[0]		= '\0';
	user->vows[0]		= '\0'; user->challenger[0]	= '\0';
	user->triv_check[0]	= '\0'; user->pager.page_file[0]= '\0';
	user->pet.name[0]	= '\0';

	user->invis		= 0;	user->cft		= 1;
	user->ignall		= 0;	user->prompt		= prompt_def;
	user->whitespace	= 0;	user->char_echo		= charecho_def;
	user->level		= 0;	user->page_finished	= 0;
	user->mode		= 0;	user->cloak_lev		= 0;
	user->profile		= 0;	user->bank_op		= 0;
	user->store_op		= 0;	user->misc_op		= 0;
	user->set_op		= 0;	user->warned		= 0;
	user->afk		= 0;	user->clone_hear	= Clone_All;
	user->ignall_store	= 0;	user->gshout		= 0;
	user->chkrev		= 0;	user->ignore		= 0;
	user->splits		= 0;	user->gsocs		= 0;
	user->first		= 0;	user->hangman_stage	= -2;
	user->monitor		= 0;	user->age		= 0;
	user->helpw_lev		= 0;	user->login_type	= 0;
	user->helpw_com		= 0;	user->hangman_rnds	= 0;
	user->turn		= 0;	user->verifynum		= 0;
	user->protection	= 0;	user->talk		= 0;
	user->c4_win		= 0;	user->c4_lose		= 0;
	user->rows		= 23;	user->cols		= 80;
	user->quit		= 0;	user->earning		= 0;
	user->no_follow		= 0;	user->has_room		= 0;
	user->tpromoted		= 0;	user->orig_lev		= 0;
	user->logons		= 0;	user->autofwd		= 0;
	user->game_op		= 0;	user->menus		= 0;
	user->win		= 0;	user->lose		= 0;
	user->draw		= 0;	user->no_fight		= 0;
	user->tic_win		= 0;	user->c4_turns		= 0;
	user->tic_lose		= 0;	user->tic_draw		= 0;
	user->money_time	= 0;	user->gpriv		= 0;
	user->hat		= 0;	user->carriage		= 0;
	user->condoms		= 0;	user->next_ping		= PingInterval;
	user->gender		= 0;	user->tchallenge	= 0;
	user->examined		= 0;	user->hide_email	= 0;
	user->ansi_on		= 0;	user->help		= 0;
	user->who		= 0;	user->helpw_same	= 0;
	user->helpw_type	= 0;	user->atmos		= 0;
	user->status		= 0;	user->delprotect	= 0;
	user->last_roll		= 0;	user->chamber		= 0;
	user->challenged	= 0;	user->has_macros	= 0;
	user->hang_wins		= 0;	user->hang_loses	= 0;
	user->playing		= 0;	user->chances		= 0;
	user->tries		= 0;	user->guess_num		= 0;
	user->alarm_time	= 0;	user->alarm_set		= 0;
	user->hangman_wins	= 0;	user->cchallenge	= 0;
	user->chksmail		= 0;	user->notify		= 0;
	user->tvar1		= 0;	user->tvar2		= 0;
	user->icqhide		= 0;	user->shot		= 0;
	user->hide_room		= 0;	user->steal_tries	= 0;
	user->beers		= 0;	user->nukes		= 0;
	user->keys		= 0;	user->bullets		= 0;
	user->drunk		= 0;	user->mail_op		= 0;
	user->bship_wins	= 0;	user->bship_lose	= 0;
	user->triv_mode		= 0;	user->triv_right	= 0;
	user->triv_wrong	= 0;	user->last_login_len	= 0;
	user->money		= 0;	user->dep_money		= 0;
	user->chips		= 0;	user->dep_chips		= 0;
	user->icq		= 0;	user->last_ping		= -1;
	user->total_login	= 0;	user->last_input	= time(0);
	user->pause_time	= -1;	user->last_login	= time(0);
	user->pager.page_num	= 1;	user->pager.last_page	= -1;
	user->pager.start_page	= 0;	user->pager.filepos	= 0;
	user->pager.lines	= 0;	user->pager.type	= 0;
	user->pager.pdone	= 0;	user->pet.type		= 0;
	user->editor.edit_op	= 0;	user->editor.editing	= 0;
	user->editor.charcnt	= 0;	user->editor.edit_line	= 0;
	user->editor.maxlines	= -1;	user->site_screened	= 0;
	user->autodetected	= 0;

	user->room		= NULL;	user->chess_opp		= NULL;
	user->invite_room	= NULL;	user->owner		= NULL;
	user->tic_opponent	= NULL; user->c4_opponent	= NULL;
	user->bjack		= NULL;	user->pop		= NULL;
	user->puzzle		= NULL;	user->editor.malloc_end	= NULL;
	user->checker_opp	= NULL; user->chess		= NULL;
	user->checkers		= NULL;
	user->editor.malloc_start	= NULL;
	if (user->away_buff == NULL) user->away_buff = newconv_buffer();
	if (user->tbuff == NULL) user->tbuff = newconv_buffer();
	for (i = 0; i < 20; ++i) user->clone_cnt[i] = 0;
	for (i = 0; i < MaxPages; ++i) user->pager.pages[i] = 0;
	for (i = 0; i < 20; ++i) user->pet.alias[i][0]	= '\0';
	for (i = 0; i < 20; ++i) user->pet.trigger[i][0]= '\0';
	for (i = 0; i < 17; ++i) user->fmacros[i][0]	= '\0';
	for (i = 0; i < 17; ++i) user->rmacros[i][0]	= '\0';
	for (i = 0; i < 50; ++i) user->friends[i][0]	= '\0';
	for (i = 0; i < 50; ++i) user->enemies[i][0]	= '\0';
	for (i = 0; i < 25; ++i) user->allowcom[i][0]	= '\0';
	for (i = 0; i < 25; ++i) user->denycom[i][0]	= '\0';
	for (i = 0; i < 5 ; ++i) user->ccopy[i][0]	= '\0';
	for (i = 0; i < 28; ++i) user->scut[i].scut	= 0;
	for (i = 0; i < 28; ++i) user->scut[i].scom[0]	= '\0';
	for (i = 0; i < 5 ; ++i) user->social[i][0]	= '\0';
	for (i = 0; i < 25; ++i) user->alias.find[i][0]	= '\0';
	for (i = 0; i < 25; ++i) user->alias.alias[i][0]= '\0';
	for (i = 0; i < 9 ; ++i) user->tnum[i]		= '\0';
	for (y = 0; y < 7 ; ++y)
		for (x = 0; x < 8 ; ++x)
			user->board[x][y] = 0;
	return;
}

/* END OF OBJECTS.C */
