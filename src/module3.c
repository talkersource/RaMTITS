/*
  module3.c
    The source code for commands and functions that are new in RaMTITS
    version 2.

    The source within this file is Copyright 1998 - 2001 by
 	Rob Melhuish, Arnaud Abelard, and Phypor.

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
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <arpa/telnet.h>
#define _MODULE3_C
  #include "include/needed.h"
#undef _MODULE3_C
#include "include/mod3lib.h"
#include <unistd.h>
extern char *crypt __P ((__const char *__key, __const char *__salt));

/* Lets a user shoot themself in the head.. AKA russian roulette */
void rroulette(User user,char *inpstr,int rt)
{
	char *name;

	if (user==NULL) return;
	if (user->invis==Invis) name = invisname;  else name = user->recap;
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
	if (user->bullets==0)
	{
		write_user(user,"~FG You need bullets from the store first...\n");
		return;
	}
	if (user->chamber == 0 || user->chamber>7) user->chamber = ran(7);
	++user->shot;
	write_user(user,"~CW-~FT You spin the chamber on your pistol........\n");
	write_user(user,"~CW-~FT You put the pistol to your head and pull the trigger...\n");
	sprintf(text,"~FT %s~FM puts %s pistol to %s head and pulls the trigger...\n",name,gender1[user->gender],gender1[user->gender]);
	write_room_except(user->room,user,text);
	if (user->shot >= user->chamber || user->chances >= 5)
	{
		write_user(user,"~CW- ~RDK~RDA~RDB~RDO~RDO~RDM~FT.. you were unlucky kid...you got that one bullet..\n");
		WriteRoomExcept(user->room,user," ~RDK~RDA~RDB~RDO~RDO~RDM~FM..~FT %s~FT tries %s luck and is no more..\n",name,gender1[user->gender]);
		user->bullets--;	disconnect_user(user,1);
		return;
	}
	write_user(user,"~CW-~RD C~RDL~RDI~RDC~RDK~FT is the sound the gun makes against your head...\n");
	write_user(user,"~CW- ~FTIt's your lucky day, you got the empty barrel..\n");
	WriteRoomExcept(user->room,user," ~RDC~RDL~RDI~RDC~RDK~FM is the sound the gun makes against %s's head..\n",name);
	user->chances++;
	return;
}

/* Allows a user to get drunk from the beers that they buy in the store. */
void drink(User user,char *inpstr,int rt)
{
	char *name;

	if (user==NULL) return;
	if (user->invis==Invis) name = invisname;  else name = user->recap;
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
	if (user->beers==0)
	{
		write_user(user,"~FG You have no beers to drink.\n");
		return;
	}
	write_user(user,"~CW- ~FGYou crack open a bottle of beer and start guzzling it down...\n");
	WriteRoomExcept(user->room,user,"~CW- ~FT%s~FM guzzles down a beer.\n",name);
	++user->drunk;  --user->beers;
	if (user->drunk<6) return;
	if (user->drunk>=6 && user->drunk<10)
	{
		write_user(user,"~CW- ~FGYou can begin to feel the effect this substance is having on you..\n");
	}
	if (user->drunk>10)
	{
		write_user(user,"~CW- ~FGYou start wobbling around.. now feeling the effects of this booze.\n");
		WriteRoomExcept(user->room,user,"~CW-~FT %s~FM is now drunk...\n",name);
	}
	return;
}

/*
   This is the .pet function, used if the user has a pet and wants to name
   their pet something, or train it to do something.. etc etc.
*/
void pet_stuff(User user,char *inpstr,int rt)
{
	char *name,alias[16],temp[256];
	int i,p,cnt=0;

	if (user==NULL) return;
	if (user->invis==Invis) name = invisname;  else name = user->recap;
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
	if (user->pet.type == 0)
	{
		write_user(user,"~CW- ~FGYou don't even have a pet.. sorry.\n");
		return;
	}
	if (word_count<2)
	{
		write_user(user,"~CW-~FG See ~CW'~CR.help pet~CW'~FG for info on this command.\n");
		return;
	}
	if (!strncasecmp(word[1],"-name",2))	/* Name the pet. */
	{
		inpstr = remove_first(inpstr);
		if (!strlen(inpstr))
		{
			write_user(user,"~CW- ~FGWhen naming your pet.. it's good to actually put a name ;-)\n");
			return;
		}
		if (strlen(ColourStrip(word[2]))>12)
		{
			write_user(user,"~CW-~FG That's an awful long name for a pet don't you think?\n");
			return;
		}
		strcpy(user->pet.name,ColourStrip(word[2]));
		writeus(user,"~CW-~FG You name your pet %s\n",user->pet.name);
		WriteRoomExcept(user->room,user,"~CW-~FT %s~FM names %s pet %s.\n",name,gender1[user->gender],user->pet.name);
		return;
	}
	if (!strncasecmp(word[1],"-list",2))	/* List the pet's alias' */
	{
		write_user(user,ascii_tline);
		cnt = 0;
		for (i = 0 ; i<20 ; ++i)
		{
			if (user->pet.alias[i][0] == '\0') continue;
			writeus(user,"~CW- ~FG%s~CB = ~RS%s\n",user->pet.alias[i],user->pet.trigger[i]);
			++cnt;
		}
		if (cnt == 0) write_user(user,"~CW-~FT You don't have any pet triggers set.\n");
		write_user(user,ascii_bline);
		cnt = 0;
		return;
	}
	if (!strncasecmp(word[1],"-trade",5))	/* Trade for a different pet. */
	{
		inpstr = remove_first(inpstr);
		if (!strlen(inpstr))
		{
			for (i = 1; i < 6; ++i)
				writeus(user,"~FGPet #~CR%d ~CB= ~FT %s\n",i,pet[i]);
			write_user(user,"~CW- ~FGYou've gotta specify what your trading it for.\n");
			write_user(user,"~CW-~FG To trade, type ~CR.pet #~FG replacing # with your choice.\n");
			return;
		}
		p = atoi(word[2]);
		if (p<1 || p>6)
		{
			write_user(user,"~CW-~FG There are only 6 different types of pets.. read the helpfile.\n");
			return;
		}
		for (i = 0 ; i<20 ; ++i)
		{
			user->pet.alias[i][0] = '\0';
			user->pet.trigger[i][0] = '\0';
		}
		user->pet.name[0] = '\0';
		user->pet.type = p;
		writeus(user,"~CW- ~FGYou trade your pet in for a %s.\n",pet[user->pet.type]);
		return;
	}
	if (!strncasecmp(word[1],"-train",5))
	{
		if (!strlen(word[2]) || word[3][0] != '='
		  || !strlen(word[4]))
		{
			write_user(user,"~CW-~FG For info on this command, type ~CW'~CR.help pet~CW'\n");
			return;
		}
		strcpy(alias,ColourStrip(word[2]));
		if (strlen(alias)>15)
		{
			write_user(user,"~CW- ~FG That alias name would be too long.\n");
			return;
		}
		inpstr = remove_first(inpstr);
		inpstr = remove_first(inpstr);
		inpstr = remove_first(inpstr);
		if (strlen(inpstr)>75)
		{
			write_user(user,"~CW-~FG That trigger would be too long.\n");
			return;
		}
		/* Find an empty slot */
		for (i = 0 ; i<20 ; ++i)
		{
			if (user->pet.alias[i][0] == '\0') continue;
			++cnt;
		}
		if (cnt>=20)
		{
			write_user(user,"~FG You already have the maximum amount of pet alias' (read the helpfile.)\n");
			return;
		}
		/* Make sure it's not a dupe alias. */
		for (i = 0 ; i<20 ; ++i)
		{
			if (!strcasecmp(alias,user->pet.alias[i]))
			{
				write_user(user,"~CW-~FG You already have an alias for your pet with that name.\n");
				return;
			}
		}
		for (i = 0; i < 20; ++i)
		{
			++cnt;
			if (user->pet.alias[cnt][0] == '\0') break;
		}
		strcpy(user->pet.alias[cnt],alias);
		strcpy(user->pet.trigger[cnt],inpstr);
		writeus(user,"~CW- ~FGWhen you use ~FT.pet %s~FG it will execute:\n",user->pet.alias[cnt]);
		writeus(user,"~FM%s ~FTwhispers ~CW'~CR%s~CW' ~FTin %s pet %s's ear...\n",user->recap,user->pet.alias[cnt],gender1[user->gender],pet[user->pet.type]);
		writeus(user,"~FY%s~FM %s\n",user->pet.name[0]?user->pet.name:pet[user->pet.type],user->pet.trigger[cnt]);
		return;
	}
	if (!strncasecmp(word[1],"-untrain",2))
	{
		inpstr = remove_first(inpstr);
		if (!strlen(inpstr))
		{
			write_user(user,"~CW-~FG You have to specify which alias you want to untrain your pet for.\n");
			return;
		}
		for (i = 0 ; i<20 ; ++i)
		{
			if (!strcasecmp(word[2],user->pet.alias[i]))
			{
				user->pet.alias[i][0] = '\0';
				user->pet.trigger[i][0] = '\0';
				write_user(user,"~CW- ~FGYour pet has been untrained.\n");
				return;
			}
		}
		write_user(user,"~CW-~FT I can't untrain something your pet hasn't been trained for.\n");
		return;
	}
	/* Now we go through and see if it was an actual alias that was -
	 - inputted.							*/
	for (i = 0 ; i<20 ; ++i)
	{
		if (strcasecmp(word[1],user->pet.alias[i])) continue;
		writeus(user,"~FTYou tell your pet ~CW'~CR%s~CW'~FT.\n",user->pet.alias[i]);
		WriteRoomExcept(user->room,user,"~FM%s ~FTwhispers ~CW'~CR%s~CW' ~FTin %s pet %s ear...\n",user->recap,user->pet.alias[i],gender1[user->gender],pet[user->pet.type]);
		inpstr = remove_first(inpstr);
		strcpy(temp,StringConvert(user->pet.trigger[i],inpstr));
		sprintf(text,"~FY%s~FM %s\n",user->pet.name[0]?user->pet.name:pet[user->pet.type],temp);
		write_user(user,text);
		write_room_except(user->room,user,text);
		return;
	}
	write_user(user,"~CW- ~FGYou specified an invalid alias.\n");
	return;
}

/* Process $1 $2's etc out of a pet trigger. */
char *StringConvert(char *trig,char *str)
{
	static char text2[256];
	char *t,*s;
	int i;

	if (!strchr(trig,'$')) return(trig);
	text2[0] = '\0';
	text[0]  = '\0';
	/* Clear the word[] array and fill it with str */
	clear_words();
	word_count = wordfind(str);
	t = text2;
	s = trig;
	if (word_count == 0) return(trig);
	/* Now we replace the $1's and stuff out of the pet trigger. */
	while (*s)
	{
		for (i = 0 ; i<word_count ; ++i)
		{
			sprintf(text,"$%d",(i+1));
			if (!strncasecmp(s,text,strlen(text)))
			{
				memcpy(t,word[i],strlen(word[i]));
				t += strlen(word[i])-1;
				s += strlen(text)-1;
				goto CONT;
			}
		}
		*t=*s;
		CONT:
		  ++s;	++t;
	}
	*t = '\0';
	return(text2);
}

/* Lets a user start a nuclear bomb count down. If no arguments are given  -
 - then it'll use the default time, which is ten seconds.... once the time -
 - is finished, then every user who hasn't made it to the bomb shelter in  -
 - time, will be disconnected from the talker.. not really all that useful -
 - just more of a fun thing :-)						   */
void nuclear_bomb(User user,char *inpstr,int rt)
{
	int cdown=0;

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
	if (user->nukes == 0)
	{
		write_user(user,"~FG You have no nuclear bombs to detonate.\n");
		return;
	}
	/* If word_count < 2, then use the default countdown, 20 seconds. */
	if (word_count<2)
	{
		write_user(user,"~CW-~FG Nuclear countdown set... nuclear bomb will be detonated in 20 seconds.\n");
		write_room_except(NULL,user,"~CW-~FT A nuclear bomb has been activated... you all have 20 seconds to retreat to the bomb shelter...\n");
		nuclear_countdown = 20;
		nuke_announce = time(0);
		--user->nukes;
		return;
	}
	cdown = atoi(word[1]);
	if (cdown<20 || cdown>600)	/* more then 10 minutes is too long. */
	{
		write_user(user,"~FG You specified an invalid time for the nuclear bomb to go off.\n");
		write_user(user,"~FG It has to be greater then 20 seconds, and less then 600 seconds.\n");
		return;
	}
	nuclear_countdown = cdown;
	nuke_announce = time(0);
	--user->nukes;
	writeus(user,"~CW- ~FGNuclear countdown set... nuclear bomb will be detonated in %d seconds.\n",cdown);
	WriteRoomExcept(NULL,user,"~CW- ~FTA nuclear bomb has been activated... everyone has %d seconds to get to the bomb shelter.\n",cdown);
	return;
}

/*
   This is an event function, called in do_events(), it checks the nuclear
   countdown variable, and checks to see if the coundown has gone off, if
   so, then we execute the bomb, if a user isn't in the bomb shelter, then
   they will be disconnected... if not, then we display the warning every
   so often..
*/
void nuclear_count(void)
{
	User user,next;
	int mins,secs;

	if (nuclear_countdown == -3) return;
	mins=secs=0;
	nuclear_countdown -= heartbeat;
	if (nuclear_countdown <= 0)
	{
		user = user_first;
		while (user != NULL)
		{
			next = user->next;
			write_room(NULL,"~CW- ~CRNuclear countdown is up.. executing nuclear bomb....\n");
			if (strcmp(user->room->name,"bomb shelter"))
			{
				write_user(user,"~CW-~FT Oh no, you got wasted by the nuclear bomb.\n");
				WriteRoomExcept(NULL,user,"~CW- ~FG%s~FM perished in the horrid blaze..\n",user->recap);
				disconnect_user(user,1);
			}
			user = next;
		}
		nuclear_countdown =- 3;
	}
	/* Print the message every minute or so. */
	secs = (int)(time(0)-nuke_announce);
	if (nuclear_countdown >= 60 && secs >= 60)
	{
		WriteRoom(NULL,"~CW- ~FMThe nuclear blast is going to be in.. %d minute%s and %d second%s....\n",nuclear_countdown/60,(nuclear_countdown/60)>1?"s":"",nuclear_countdown%60,(nuclear_countdown%60)>1?"s":"");
		nuke_announce = time(0);
	}
	if (nuclear_countdown<60 && secs >= 10)
	{
		WriteRoom(NULL,"~CW-~FM The nuclear blast is going to be in.. %d seconds..\n",nuclear_countdown);
		nuke_announce = time(0);
	}
}

/* Allows a user to toggle monitoring on/off. */
void toggle_monitor(User user,char *inpstr,int rt)
{

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
	if (user->monitor == 0)
	{
		user->monitor = 1;
		write_user(user,"~FT You will now monitor certain things...\n");
		return;
	}
	user->monitor = 0;
	write_user(user,"~FT You will no longer monitor certain things...\n");
	return;
}

/*
   Page certain files.. just call the file in a certain switch, then page
   it via more()... should be basic enough.
*/
void page_file(User user,char *inpstr,int which_file)
{
	char filename[FSL];

	if (user == NULL) return;
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	switch (which_file)
	{
		case 1 :
		  if (is_jailed(user))
		  {
			status_text(user,1,command[com_num].name);
			return;
		  }
		  sprintf(filename,"%s/%s",DataDir,NewsFile);
		  if (!file_exists(filename))
		  {
			write_user(user,"~CW-~FT Looks like there's nothing new going on..\n");
			return;
		  }
		  count_messages(user,4);
		  break;
		case 2 :
		  sprintf(filename,"%s/%s",MiscDir,StaffRules);
		  if (!file_exists(filename))
		  {
			write_user(user,"~CW-~FT It appears as tho there aren't any staff rules.\n");
			return;
		  }
		  break;
		case 3 :
		  sprintf(filename,"%s/%s",MiscDir,RulesFile);
		  if (!file_exists(filename))
		  {
			writeus(user,"~CW-~FT Looks like there aren't any rules for ~FG%s\n",TalkerRecap);
			return;
		  }
		  break;
		case 5 :
		  if (is_jailed(user))
		  {
			status_text(user,1,command[com_num].name);
			return;
		  }
		  sprintf(filename,"%s/%s",MiscDir,NewbieFile);
		  if (!file_exists(filename))
		  {
			writeus(user,"~CW-~FT It looks like there isn't a new user helpfile.. contact ~FG%s.\n",TalkerEmail);
			return;
		  }
		  break;
		case 6 :
		  sprintf(file,"%s",LevelList);
		  if (!(show_screen(user)))
		  {
			write_user(user,"~CW-~FG It appears as tho noone has made a list of levels.\n");
			return;
		  }
		  return;
		default:
		  write_user(user,"~FT Invalid page requested.\n");
		  return;
	}
	switch (more(user,user->socket,filename,0))
	{
		case 0:
		  write_user(user,"~CW-~FG The file you tried paging doesn't exist.\n");
		  return;
		case 1:
		  user->misc_op = 2;
	}
	return;
}

/* Quit the user. */
void quit_user(User user,char *inpstr,int rt)
{
	int rows=0;

	if (user == NULL) return;
	write_user(user,"\n\n\n");
	if (is_clone(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if ((user->mode && strlen(word[0])<2 && user->quit)
	  || (user->mode == 0 && strlen(word[0])<3 && user->quit))
	{
		sprintf(text,"~FGGoodbye ~CR%s~FG I hope you had a great time.. cya later...\n",user->recap);
		write_user(user,center(text,80));
		sprintf(file,"%s/%s.ASC",ScreenDir,CloseScreen);
		rows = user->rows;  user->rows = 0;
		more(user,user->socket,file,0);
		user->rows = rows;
		write_user(user,"\n\n~RSConnection closed by foreign hostess.\n");
		user->room = get_room(idle_room);
		if (user->room == NULL) user->room = room_first;
		return;
	}
	disconnect_user(user,1);
	return;
}

/* Save some talker details. */
int save_talker_stuff(void)
{
	FILE *fp;
	char filename[FSL];

	sprintf(filename,"%s/talker.dat",DataDir);
	if ((fp = fopen("tempfile","w")) != NULL)
	{
		/* New users. */
		fprintf(fp,"%ld %ld %ld\n",newlog[0],newlog[1],newlog[2]);
		/* Total logins */
		fprintf(fp,"%ld %ld %ld %ld\n",ulogins[0],ulogins[1],ulogins[2],ulogins[3]);
		/* Lottery jackpot */
		fprintf(fp,"%ld\n",jackpot);
		FCLOSE(fp);
		if (rename("tempfile",filename) != -1)
			return(1);
	}
	write_log(0,LOG1,"[ERROR] - Failed to save the talker stuff: %s.\n",strerror(errno));
	return(0);
}

/* Now load it. */
int load_talker_stuff(void)
{
FILE *fp;
char filename[FSL];

	sprintf(filename,"%s/talker.dat",DataDir);
	if ((fp = fopen(filename,"r")) != NULL)
	{
		/* New users. */
		fscanf(fp,"%ld %ld %ld\n",&newlog[0],&newlog[1],&newlog[2]);
		fscanf(fp,"%ld %ld %ld %ld\n",&ulogins[0],&ulogins[1],&ulogins[2],&ulogins[3]);
		fscanf(fp,"%ld\n",&jackpot);
		FCLOSE(fp);
		return(1);
	}
	write_log(0,LOG1,"[ERROR] - Failed to load the talker stuff.\n");
	return(0);
}

/*
   Check the amount of lag time between a user and and the talker.. it was
   originally based off of the PING system for EW-Too systems to which Arny
   converted to NUTS based systems, to which I converted to RaMTITS.
*/
void ping(User user)
{
	char seq[4];

	if (user->type == CloneType) return;
	sprintf(seq,"%c%c%c",IAC,DO,TELOPT_STATUS);
	write(user->socket,seq,strlen(seq));
	gettimeofday(&(user->ping_timer),(struct timezone *)NULL);
}

void ping_respond(User user)
{
	struct timeval endtv;
	long pt;

	if (user->type == CloneType) return;
	memset(&endtv,0,sizeof(struct timeval));
	gettimeofday(&endtv,(struct timezone *)NULL);
	pt = ((endtv.tv_sec-user->ping_timer.tv_sec)*1000000)+((endtv.tv_usec-user->ping_timer.tv_usec));
	if (user->last_ping == -1)
	{
		user->last_ping = pt;
		user->next_ping = PingInterval;
		return;
	}
	writeus(user,"~CW- ~FTYou have about %ld.%.2ld seconds of lag. ~CB(~CR%s~CB)\n",pt/1000000,(pt/10000)%1000000,ping_string(user));
}

void check_pings(void)
{
	User user,nxt;

	user = user_first;
	while (user != NULL)
	{
		nxt = user->next;
		if (user->type == CloneType)
		{
			user = nxt;
			continue;
		}
		if (user->next_ping == 0 || user->next_ping < -1)
		{
			/* wait til ping_respond resets us */
			user->next_ping = -1;
			ping_timed(user);
		}
		if (user->next_ping>0) user->next_ping -= heartbeat;
		else if (user->next_ping < -1) user->next_ping = PingInterval;
		user = nxt;
	}
}

void ping_timed(User user)
{

	if (user->type == CloneType) return;
	user->last_ping = -1;	/* Set the last ping to be pending. */
	ping(user);
}

void ping_user(User user,char *inpstr,int rt)
{
	User u;

	if (user == NULL) return;
	if (word_count<2)
	{
		write_user(user,center("~FT-~CT=~CB>~FG Ping Listing... ~CB<~CT=~FT-\n",80));
		for_ausers(u)
		{
			if (u->login || u->type == CloneType) continue;
			if (u->invis == Invis && user->level<u->level) continue;
			if (u->invis == Hidden && user->level<ADMIN) continue;
			if (u->invis == Fakelog && user->level<GOD) continue;
			if ((u->room->hidden && u->room != user->room)
			  && user->level<GOD) continue;
			writeus(user,"~CW - ~FT%s ~FMhas approx. %ld.%.2ld seconds of lag. ~CB[~CR%s~CB]\n",u->invis>Invis?u->temp_recap:u->recap,u->last_ping/1000000,(u->last_ping/10000)%1000000,ping_string(u));
		}
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	writeus(user,"~CW - ~FT%s ~FMhas approx. %ld.%.2ld seconds of lag. ~CB[~CR%s~CB]\n",u->recap,u->last_ping/1000000,(u->last_ping/10000)%1000000,ping_string(u));
	return;
}

char *ping_string(User user)
{
	int i;

	if (user->type == CloneType) return("Spanked!");
	for (i = 0 ; speeds[i].text[0] ; i++)
	  if ((user->last_ping/10000)<=(long)speeds[i].lag)
	    return(speeds[i].text);
	return("Spanked!");
}
/* End of Ping functions. */

/* New gag functions!! */
/* Disallow a user from shouting. */
void gag_shouts(User user)
{
	User u;
	char *uname;
	int err=0;

	if (user == NULL) return;
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u->level >= user->level && user->level<OWNER)
		{
			write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
			return;
		}
		if (u->gshout)
		{
			if (user->status & Silenced)
			{
				writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use shout commands again.\n",u->recap);
				return;
			}
			writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use shout commands again.\n",u->recap);
			write_user(u,"~CW-~FT You are now allowed to shout again..\n");
			sprintf(text,"~CB[~CRGAG~CB] - ~FT%s ~FGallowed ~FT%s~FG to shout.\n",user->recap,u->recap);
			write_level(WIZ,user,text);
			sprintf(text,"[UNGAG-SHOUT] - User: [%s] Recipient: [%s]\n",user->name,u->name);
			write_log(1,LOG4,text);	  write_record(u,1,text);
			u->gshout=0;			return;
		}
		if (user->status & Silenced)
		{
			writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using shout commands.\n",u->recap);
			return;
		}
		writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using shout commands.\n",u->recap);
		write_user(u,"~CW-~FT You have had shout commands taken away from you.\n");
		sprintf(text,"~CB[~CRGAG~CB] - ~FT%s~FG denied ~FT%s~FG from using shout commands.\n",user->recap,u->recap);
		write_level(WIZ,user,text);
		sprintf(text,"[GAG-SHOUT] - User: [%s] Recipient: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);		write_record(u,1,text);
		u->gshout = 1;			return;
	}
	if ((u=create_user()) == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred in creating a temp. user object.\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);  destructed = 0;  return;
	}
	else if (err == -1)
	{
		write_user(user,"~CW-~FT It appears as tho that users userfile is corrupted.\n");
		destruct_user(u);  destructed = 0;  return;
	}
	if (u->level >= user->level && user->level<OWNER)
	{
		write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
		destruct_user(u);	destructed = 0;
		return;
	}
	if (u->gshout)
	{
		if (user->status & Silenced)
		{
			writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use shout commands again.\n",u->recap);
			destruct_user(u);	destructed = 0;
			return;
		}
		u->socket = -2;		strcpy(u->site,u->last_site);
		u->gshout = 0;		SaveUser(u,0);
		writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use shout commands again.\n",u->recap);
		sprintf(text,"~CW- ~FTYou are now allowed to use shout commands.\n");
		send_oneline_mail(NULL,u->name,text);
		sprintf(text,"[UNGAG-SHOUT] - User: [%s] Recipient: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);		write_record(u,1,text);
		destruct_user(u);		destructed = 0;
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using shout commands.\n",u->recap);
		destruct_user(u);	destructed = 0;
		return;
	}
	u->socket = -2;		strcpy(u->site,u->last_site);
	u->gshout = 1;		SaveUser(u,0);
	writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using shout commands.\n",u->recap);
	sprintf(text,"~CW-~FT You have had shout commands taken away from you.\n");
	send_oneline_mail(NULL,u->name,text);
	sprintf(text,"[GAG-SHOUT] - User: [%s] Recipient: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);	write_record(u,1,text);
	destruct_user(u);	destructed = 0;
	return;
}

/* Gag a user from using socials. */
void gag_socials(User user)
{
	User u;
	char *uname;
	int err=0;

	if (user == NULL) return;
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u->level >= user->level && user->level<OWNER)
		{
			write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
			return;
		}
		if (u->gsocs)
		{
			if (user->status & Silenced)
			{
				writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use socials again.\n",u->recap);
				return;
			}
			writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use socials again.\n",u->recap);
			write_user(u,"~CW-~FT You are now allowed to use socials..\n");
			sprintf(text,"~CB[~CRGAG~CB] - ~FT%s ~FGallowed ~FT%s~FG to use socials.\n",user->recap,u->recap);
			write_level(WIZ,user,text);
			sprintf(text,"[UNGAG-SOCIALS] - User: [%s] Recipient: [%s]\n",user->name,u->name);
			write_log(1,LOG4,text);		write_record(u,1,text);
			u->gsocs = 0;			return;
		}
		if (user->status & Silenced)
		{
			writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using socials.\n",u->recap);
			return;
		}
		writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using socials.\n",u->recap);
		write_user(u,"~CW-~FT You have had socials taken away from you.\n");
		sprintf(text,"~CB[~CRGAG~CB] - ~FT%s~FG denied ~FT%s~FG from using socials.\n",user->recap,u->recap);
		write_level(WIZ,user,text);
		sprintf(text,"[GAG-SOCIALS] - User: [%s] Recipient: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);		write_record(u,1,text);
		u->gsocs = 1;			return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred in creating a temp. user object.\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);  destructed = 0;  return;
	}
	else if (err == -1)
	{
		write_user(user,"~CW-~FT It appears as tho that users userfile is corrupted.\n");
		destruct_user(u);  destructed = 0;  return;
	}
	if (u->level >= user->level && user->level<OWNER)
	{
		write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
		destruct_user(u);  destructed = 0;  return;
	}
	if (u->gsocs)
	{
		if (user->status & Silenced)
		{
			writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use socials again.\n",u->recap);
			destruct_user(u);	destructed = 0;
			return;
		}
		u->socket = -2;		strcpy(u->site,u->last_site);
		u->gsocs = 0;		SaveUser(u,0);
		writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use socials again.\n",u->recap);
		sprintf(text,"~CW- ~FTYou are now allowed to use socials again.\n");
		send_oneline_mail(NULL,u->name,text);
		sprintf(text,"[UNGAG-SOCIALS] - User: [%s] Recipient: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);		write_record(u,1,text);
		destruct_user(u);		destructed = 0;
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using socials\n",u->recap);
		destruct_user(u);	destructed = 0;
		return;
	}
	u->socket = -2;		strcpy(u->site,u->last_site);
	u->gsocs = 1;		SaveUser(u,0);
	writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using socials.\n",u->recap);
	sprintf(text,"~CW-~FT You have had socials taken away from you.\n");
	send_oneline_mail(NULL,u->name,text);
	sprintf(text,"[GAG-SOCIALS] - User: [%s] Recipient: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);	write_record(u,1,text);
	destruct_user(u);	destructed = 0;
	return;
}

void gag_private(User user)
{
	User u;
	char *uname;
	int err=0;

	if (user == NULL) return;
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) != NULL)
	{
		if (u->level >= user->level && user->level<OWNER)
		{
			write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
			return;
		}
		if (u->gshout)
		{
			if (user->status & Silenced)
			{
				writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use private message commands again.\n",u->recap);
				return;
			}
			writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use private message commands again.\n",u->recap);
			write_user(u,"~CW-~FT You are now allowed to use private message commands again..\n");
			sprintf(text,"~CB[~CRGAG~CB] - ~FT%s ~FGallowed ~FT%s~FG to use private message commands.\n",user->recap,u->recap);
			write_level(WIZ,user,text);
			sprintf(text,"[UNGAG-PRIVATE] - User: [%s] Recipient: [%s]\n",user->name,u->name);
			write_log(1,LOG4,text);		write_record(u,1,text);
			u->gpriv = 0;			return;
		}
		if (user->status & Silenced)
		{
			writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using private message commands.\n",u->recap);
			return;
		}
		writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using private message commands.\n",u->recap);
		write_user(u,"~CW-~FT You have had private message commands taken away from you.\n");
		sprintf(text,"~CB[~CRGAG~CB] - ~FT%s~FG denied ~FT%s~FG from using private message commands.\n",user->recap,u->recap);
		write_level(WIZ,user,text);
		sprintf(text,"[GAG-PRIVATE] - User: [%s] Recipient: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);		write_record(u,1,text);
		u->gpriv = 1;			return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred in creating a temp. user object.\n");
		return;
	}
	err = LoadUser(u,uname);
	if (err == 0)
	{
		write_user(user,center(nosuchuser,80));
		destruct_user(u);  destructed = 0;  return;
	}
	else if (err == -1)
	{
		write_user(user,"~CW-~FT It appears as tho that users userfile is corrupted.\n");
		destruct_user(u);  destructed = 0;  return;
	}
	if (u->level >= user->level && user->level<OWNER)
	{
		write_user(user,"~CW- ~FTSorry, you can't gag/ungag a user who's level is higher or equal then yours.\n");
		destruct_user(u);  destructed = 0;  return;
	}
	if (u->gshout)
	{
		if (user->status & Silenced)
		{
			writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use private message commands again.\n",u->recap);
			destruct_user(u);	destructed = 0;
			return;
		}
		u->socket = -2;		strcpy(u->site,u->last_site);
		u->gpriv = 0;		SaveUser(u,0);
		writeus(user,"~CW- ~FTYou have allowed ~FM%s~FT to use private message commands again.\n",u->recap);
		sprintf(text,"~CW- ~FTYou are now allowed to use private message commands.\n");
		send_oneline_mail(NULL,u->name,text);
		sprintf(text,"[UNGAG-PRIVATE] - User: [%s] Recipient: [%s]\n",user->name,u->name);
		write_log(1,LOG4,text);		write_record(u,1,text);
		destruct_user(u);		destructed = 0;
		return;
	}
	if (user->status & Silenced)
	{
		writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using private message commands.\n",u->recap);
		destruct_user(u);  destructed = 0;  return;
	}
	u->socket = -2;		strcpy(u->site,u->last_site);
	u->gpriv = 1;		SaveUser(u,0);
	writeus(user,"~CW-~FT You have disallowed ~FM%s ~FTfrom using private message commands.\n",u->recap);
	sprintf(text,"~CW-~FT You have had private message commands taken away from you.\n");
	send_oneline_mail(NULL,u->name,text);
	sprintf(text,"[GAG-PRIVATE] - User: [%s] Recipient: [%s]\n",user->name,u->name);
	write_log(1,LOG4,text);	write_record(u,1,text);
	destruct_user(u);	destructed = 0;
	return;
}

/* Tell a line something.. (Such as a user is at a login stage) and either -
 - just idling, or doing nothing or logging in.. you can send them a tell  -
 - saying hi, or help them if they need it, etc..			   */
void line_tell(User user,char *inpstr,int rt)
{
	User u;
	int sock,found=0;

	if (user == NULL) return;
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
	if (word_count<3 || !isnumber(word[1]))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRline~CB] [~CRmessage~CB]");
		return;
	}
	sock = atoi(word[1]);
	/* Find the line that we're telling. */
	for_users(u)
	{
		if (u->type != CloneType && u->socket==sock)
		{
			found = 1;
			break;
		}
	}
	if (found == 0)
	{
		write_user(user,"~CW- ~FTIt appears as tho that line isn't currently active.\n");
		return;
	}
	if (u->login == 0)
	{
		write_user(user,"~CW- ~FTWhy not just send the person a normal tell?\n");
		return;
	}
	inpstr = remove_first(inpstr);
	writeus(u,"\n~CW- ~FTMessage from~CB:~FG %s\n",user->recap);
	sprintf(text,"~CW- ~RS%s\n",inpstr);
	write_user(u,text);
	writeus(user,"~CW- ~FTMessage has been sent to line ~CR%d~FT...\n",sock);
}

/* Auto arrest function, called for in simple things that basically use the -
 - arrest_sweares, and for some other stuff which I've added.		    */
void auto_arrest(User user,int for_what)
{
	Room rm;

	if (user == NULL) return;
	switch(for_what)
	{
		case 1: /* Swear arrest ban. */
		  write_user(user,"~CW-~FT You have been arrested for swearing... You swore either in a\n");
		  write_user(user,"~CW-~FT public room, a shout, or in a channel... please see the ~CW'~CR.beg~CW' ~FTcommand.\n");
		  if (user->status & Jailed)
		  {
		  }
		  else user->status+=Jailed;
		  rm = get_room(jail_room);
		  if (rm == NULL)
		  {
			write_user(user,"~CW- ~FTUnable to find the jail.. but you're still arrested.\n");
			write_log(1,LOG1,"[Auto-Arrest] - Unable to find the jail.\n");
		  }
		  else user->room = rm;
		  WriteLevel(WIZ,user,"~CB[~CRAuto-Arrest~CB] -~FG Placing %s ~FGunder arrest for swearing.\n",user->recap);
		  write_log(1,LOG1,"[Auto-Arrest] - %s was arrested for swearing.\n",user->name);
		  break;
		case 2: /* Trigger guard. */
		  write_user(user,"~CW-~FT You have been paced under automatic arrest for spam/trigger protection.\n");
		  if (user->status & Jailed)
		  {
		  }
		  else user->status += Jailed;
		  rm = get_room(jail_room);
		  if (rm == NULL)
		  {
			write_user(user,"~CW- ~FTUnable to find the jail.. but you're still arrested.\n");
			write_log(1,LOG1,"[Auto-Arrest] - Unable to find the jail.\n");
		  }
		  else user->room = rm;
		  WriteLevel(WIZ,user,"~CB[~CRAuto-Arrest~CB] -~FG Placing %s ~FGunder arrest for spam/triggers.\n",user->recap);
		  write_log(1,LOG1,"[Auto-Arrest] - %s was arrested for spam/triggers.\n",user->name);
		  break;
		default:
		  return;
	}
}

/* Allows a WIZ member to cloak at any level they want. */
void cloak_level(User user,char *inpstr,int rt)
{
	int lev;

	if (user == NULL) return;
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRlevel~CW|~CR-reset~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-reset",2))
	{
		writeus(user,"~CW-~FG Your level has been restored to~CB: [~CR%s~CB]\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
		user->cloak_lev = user->level;
		return;
	}
	if ((lev = get_level(word[1])) == -1)
	{
		write_user(user,center(nosuchlev,80));
		return;
	}
	if (lev>user->level)
	{
		write_user(user,"~CW-~FG You can't cloak yourself at a level higher then your own.\n");
		return;
	}
	if (lev == user->level)
	{
		write_user(user,"~CW-~FG Why would you want to cloak yourself at your current level?\n");
		return;
	}
	user->cloak_lev = lev;
	writeus(user,"~CW-~FG Your level is now cloaked at level~CB: [~CR%s~CB]\n",user->gender==Female?level[user->cloak_lev].fname:level[user->cloak_lev].mname);
	write_log(1,LOG1,"[Cloak] - User: [%s] Level: [%s]\n",user->name,level[user->cloak_lev].name);
	return;
}

/* Allows a user to send an E-Mail message to someone else. */
void email_user(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *fp;
	char filename[FSL],*name,*c,uname[15];
	int line;

	if (user == NULL) return;
	edit = get_editor(user);
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
	if (user->tpromoted && user->orig_lev<POWER)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (done_editing == 0)
	{
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser@host~CB] [~CRsubject~CB]");
			return;
		}
		if (!valid_addy(user,word[1],0))
		{
			write_user(user,"~CW-~FG The e-mail addy you specified was invalid.\n");
			return;
		}
		if (strlen(word[1])>EmailLen)
		{
			write_user(user,"~CW-~FG The e-mail addy you specified is invalid.\n");
			return;
		}
		strcpy(user->emailing,word[1]);
		inpstr = remove_first(inpstr);
		strncpy(user->check,inpstr,sizeof(user->check)-1);
		user->editor.editing = 1;
		user->misc_op = 41;
		writeus(user,"~FT-~CT=~CB> ~FGSending e-mail to~CB: ~CR%s ~CB<~CT=~FT-\n",user->emailing);
		if (user->editor.editor==1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	if (user->status & Silenced)
	{
		write_user(user,"~CW-~FG Your e-mail has been sent...\n");
		return;
	}
	sprintf(filename,"%s/%s.email",TempFiles,user->name);
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW-~FT An error occurred in trying to e-mail.. sorry, try again in a bit.\n");
		write_log(0,LOG1,"[ERROR] - Failed to open '%s.email' for writing. Reason: [%s]\n",user->name,strerror(errno));
		return;
	}
	strcpy(uname,user->name);
	strtolower(uname);
	fprintf(fp,"From:    %s <%s>\n",ColourStrip(user->recap),TalkerEmail);
	fprintf(fp,"To:      %s\n",user->emailing);
	fprintf(fp,"Subject: %s.\n",user->check);
	fprintf(fp,"\n\n");
	if (user->editor.editor==1)
	{
		c = user->editor.malloc_start;
		while (c != user->editor.malloc_end) putc(*c++,fp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line<MaxLines)
		{
			if (edit->line[line][0] != '\0')
			{
				fprintf(fp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",fp);
			}
			line++;
		}
	}
	FCLOSE(fp);
	send_email(user->emailing,filename);
	write_user(user,"~CW-~FG Your e-mail has been sent...\n");
	write_log(1,LOG3,"[E-Mail] - User: [%s] E-mailed: [%s]\n",user->name,user->emailing);
	user->emailing[0] = '\0';
	user->check[0]	  = '\0';
	return;
}

/*
  This function just basically calls upon the misc_ops, and gives them loads
  of work to do.. for example, .make -helpfile.. you would enter the name of
  the command, and providing that it exists.. it will put you into the editor
  where you can enter the actual helpfile. .make -social will prompt you
  about 6 times.. See the .help make for more details reguarding this.
*/
void mymake(User user,char *inpstr,int rt)
{
	char *name,filename[FSL],*c;
	char *args="~CB[~CR-social~CW|~CR-helpfile~CB]";

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
	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (!strncasecmp(word[1],"-social",2))
	{
		if (!strncasecmp(word[3],"-delete",2))
		{
			/* Check for illegal crap. */
			c = word[2];
			while (*c)
			{
				if (*c == '.' || *c++ == '/')
				{
					write_user(user,"~CW-~FT That was an invalid social name..\n");
					write_log(1,LOG1,"[Illegal FileName] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[2],command[com_num].name);
					return;
				}
			}
			sprintf(filename,"%s/%s.dat",SocialDir,word[2]);
			if (file_exists(filename))
			{
				unlink(filename);
				writeus(user,"~CW-~FY Social ~FT%s~FY has been deleted.\n",word[2]);
				destruct_social(word[2]);
				WriteRoomExcept(NULL,user,"~CB[~CRRaMTITS~CB] - ~FTSocial ~FY%s~FT has been removed.\n",word[2]);
				write_log(1,LOG1,"[Socials] - User: [%s] Deleted: [%s]\n",user->name,word[2]);
				return;
			}
			write_user(user,"~CW-~FG There isn't even a social with that name.\n");
			return;
		}
		write_user(user,"~CW-~FT Creating a new social...\n");
		WriteRoomExcept(user->room,user,"~CW-~FT %s~FT starts writing up a new social...\n",name);
		user->ignall_store = user->ignall;
		user->ignall = 1;
		user->misc_op = 38;
		write_user(user,"~CW-~FT You are going to be given 6 lines.. please follow the directions for each line.\n");
		write_user(user,"~CW-~FT Line1~CB:[~FGEnter a name for the social~CB]\n");
		write_user(user,"~CW-~FT Line1~CB:~RS");
		return;
	}
	else if (!strncasecmp(word[1],"-helpfile",2))
	{
		if (!strncasecmp(word[3],"-delete",2))
		{
			c = word[2];
			while (*c)
			{
				if (*c == '.' || *c++ == '/')
				{
					write_user(user,"~CW-~FT That was an invalid helpfile name..\n");
					write_log(1,LOG1,"[Illegal FileName] - User: [%s] File: [%s] Command: [%s]\n",user->name,word[2],command[com_num].name);
					return;
				}
			}
			sprintf(filename,"%s/%s",HelpDir,word[2]);
			if (file_exists(filename))
			{
				unlink(filename);
				writeus(user,"~CW-~FY Helpfile ~FT%s~FY has been deleted.\n",word[2]);
				WriteRoomExcept(NULL,user,"~CB[~CRRaMTITS~CB] - ~FTHelpfile ~FY%s~FT has been removed.\n",word[2]);
				write_log(1,LOG1,"[Helpfile] - User: [%s] Deleted: [%s]\n",user->name,word[2]);
				return;
			}
			write_user(user,"~CW-~FG There isn't even a helpfile with that name.\n");
			return;
		}
		write_user(user,"~CW-~FY Creating a helpfile...\n");
		WriteRoomExcept(user->room,user,"~CW-~FT %s~FT starts writing up a helpfile...\n",name);
		user->ignall_store = user->ignall;
		user->ignall = 1;
		user->misc_op = 47;
		write_user(user,"~CW-~FG Once you enter the name of the helpfile, then you will be brought into\n");
		write_user(user,"~CW-~FG the editor, so you can enter the body of the helpfile..\n");
		write_user(user,"~CW-~FT Command Name~CB:~RS ");
		return;
	}
	else
	{
		writeus(user,usage,command[com_num].name,args);
		return;
	}
}

/* save the helpfile into a file. */
void make_helpfile(User user,int done_edit)
{
	Editor edit;
	FILE *fp;
	char filename[FSL],*c;
	int line;

	if (user == NULL || done_edit == 0) return;
	if (user->check[0] == '\0') return;
	edit = get_editor(user);
	strtolower(user->check);
	sprintf(filename,"%s/%s",HelpDir,user->check);
	if ((fp=fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW-~FT Unable to save the helpfile.\n");
		write_log(0,LOG1,"[ERROR] - Unable to open '%s' in make_helpfile(). Reason: [%s]\n",user->check,strerror(errno));
		return;
	}
	fputs(ascii_line,fp);
	fprintf(fp,"~FYCommand Name~CB:~RS %s\n",user->check);
	fputs(ascii_line,fp);
	if (user->editor.editor==1)
	{
		c = user->editor.malloc_start;
		while (c != user->editor.malloc_end) putc(*c++,fp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while (line<MaxLines)
		{
			if (edit->line[line][0] != '\0')
			{
				fprintf(fp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1] != '\n') fputs("\n",fp);
			}
			line++;
		}
	}
	fputs(ascii_line,fp);
	fputs("\n",fp);
	FCLOSE(fp);
	return;
}

/* resets a users .dat file in case somehow it gets corrupted. */
void reset_user(User user,char *inpstr,int rt)
{
	User u;
	int err;

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
	if (user->tpromoted && user->orig_lev<GOD)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if (word_count<3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRnew password~CB]");
		return;
	}
	if ((get_user(word[1])) != NULL)
	{
		write_user(user,"~CW - ~FTYou can't reset a user who is currently logged on.\n");
		return;
	}
	if (strlen(word[2])<3)
	{
		write_user(user,"~CW- ~FT You need to provide a longer password.\n");
		return;
	}
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FT An error occurred in creating a temp user.. aborting.\n");
		write_log(1,LOG1,"[ERROR] - Unable to create temp user in reset_user().\n");
		return;
	}
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	err = LoadUser(u,word[1]);
	/* This user has an actual user w/ their .dat file */
	if (err == -1)
	{
		reset_vars(u);
		load_default_shortcuts(u);
		strcpy(u->name,word[1]);
		strcpy(u->pass,(char *)crypt(word[2],salt));
		strcpy(u->desc,def_desc);	strcpy(u->in_phr,def_inphr);
		strcpy(u->out_phr,def_outphr);	strcpy(u->recap,u->name);
		strcpy(u->webpage,def_url);	strcpy(u->bday,def_bday);
		strcpy(u->married_to,"Noone.");
		if (free_rooms) u->has_room = 1;  else u->has_room = 0;
		strcpy(u->login_msg,def_lognmsg);
		strcpy(u->logout_msg,def_logtmsg);
		strcpy(u->last_site,"not.known");
		sprintf(text,"%s@%s",u->name,u->last_site);
		strcpy(u->email,text);		u->char_echo=charecho_def;
		u->prompt = prompt_def;
		SaveUser(u,0);
		write_log(1,LOG1,"[RESET USER] - User: [%s] Resetted: [%s]\n",user->name,u->name);
		/* Go back, check once more to make sure that the user was resetted. */
		reset_vars(u);
		err = LoadUser(u,u->name);
		if (err == 0 || err == -1)
		{
			write_user(user,"~CW-~FT The reset failed.. try again.\n");
			destruct_user(u);  destructed = 0;
			return;
		}
		write_user(user,"~CW- ~FTUser ~FG%s~FT has successfully been resetted.\n");
		destruct_user(u);  destructed = 0;
		return;
	}
	else
	{
		write_user(user,"~CW-~FT That user doesn't need to be reset.\n");
		destruct_user(u);  destructed = 0;
		return;
	}
	if (u != NULL)
	{
		destruct_user(u);
		destructed = 0;
	}
	return;
}

/* allows a user to buy a lottery ticket. */
void lottery(User user,char *inpstr,int rt)
{
	Lotto l;
	int ball[6], i = 0,cnt = 0, tot = 0;

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
	if (!strncasecmp(word[1],"-jackpot",2))
	{
		writeus(user,"~CW-~FT The jackpot is currently up to ~FG%ld~FT dollars.\n",jackpot);
		return;
	}
	else if (!strncasecmp(word[1],"-buy",2))
	{
		if (user->money < 20)
		{
			write_user(user,"~CW-~FT Lottery tickets are ~FG$~FY20~FT a piece.\n");
			return;
		}
		for (i = 0 ; i < 6 ; ++i)
		{
			ball[i] = 0;
		}
		/*
		   Now go through and pick a random number for each, making
		   sure that it isn't identical to another. There is a better
		   way to do this, but I'll do that later.
		*/
		ball[0] = ran(75);
		do
		{
			ball[1] = ran(75);
		} while (ball[0] == ball[1]);
		do
		{
			ball[2] = ran(75);
		} while (ball[0] == ball[2] || ball[1] == ball[2]);
		do
		{
			ball[3] = ran(75);
		} while ((ball[0] == ball[3] || ball[1] == ball[3]
		    || ball[2] == ball[3]));
		do
		{
			ball[4] = ran(75);
		} while (ball[0] == ball[4] || ball[1] == ball[4]
		    || ball[2] == ball[4] || ball[3] == ball[4]);
		do
		{
			ball[5] = ran(75);
		} while (ball[0] == ball[5] || ball[1] == ball[5]
		    || ball[2] == ball[5] || ball[3] == ball[5]
		    || ball[4] == ball[5]);
		qsort(ball,6,sizeof(ball[0]),intcmp);
		if (already_has_numbers(user,ball[0],ball[1],ball[2],ball[3],ball[4],ball[5]))
		{
			write_user(user,"~CW-~FT An error occurred, can you try again in a second.\n");
			return;
		}
		if (!(add_ticket(user->name,ball[0],ball[1],ball[2],ball[3],ball[4],ball[5])))
		{
			write_user(user,"~CW-~FT An error occurred, can you try again in a second.\n");
			return;
		}
		writeus(user,"~CW-~FT And your lucky numbers are...~FY%d, %d, %d, %d, %d, and %d\n",ball[0],ball[1],ball[2],ball[3],ball[4],ball[5]);
		user->money -= 20;
		write_user(user,"~CW-~FT Your lottery tickets have been saved. The draw is every Thursday at 6:49 pm EST.\n");
		return;
	}
	else if (!strncasecmp(word[1],"-my",2))
	{
		for_lotto(l)
		{
			++tot;
			if (!strcmp(l->name,user->name))
			{
				if (++cnt == 1)
				{
					write_user(user,"~CB--~FT You have the following lucky numbers:\n");
				}
				writeus(user,"~CB-- ~FY%2d %2d %2d %2d %2d %2d\n",l->ball[0],l->ball[1],l->ball[2],l->ball[3],l->ball[4],l->ball[5]);
			}
		}
		writeus(user,"~CB-- ~FTYou have a total of %d ticket%s out of %d bought.\n",cnt,cnt>1?"s":"",tot);
		return;
	}
	else
	{
		writeus(user,usage,command[com_num].name,"~CB[~CR-buy~CW|~CR-jackpot~CW|~CR-mytickets~CB]");
		return;
	}
}

/*
  Allows a user to steal items from another user, if the user has a bodyguard
  the user has a 1 in 35 chance of success.. if not, then they have a 1 in 3
  to rob the victim.. Thanks to Ziffnab on the ideas for this command.
*/
void steal_stuff(User user,char *inpstr,int rt)
{
	User	u = NULL;
	int	success,bg_chance,item,i;

	success = bg_chance = item = i = 0;
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRitem~CB]");
		write_user(user,"~CW - ~FGWhere the item can be~CB: [~CRhat~CW|~CRroom~CW|~CRcarriage~CB]\n");
		write_user(user,"~CW - ~FG                     ~CB  [~CRpet~CW|~CRbeer~CW|~CRbullets ~CW|~CRnuke~CB]\n");
		return;
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return;
	}
	if (u == user)
	{
		write_user(user,"~CW-~FT Sorry.. we don't do fake insurance scams here.\n");
		return;
	}
	if (!strcasecmp(word[2],"hat")) item = 1;
	else if (!strcasecmp(word[2],"room"))
	{
		if (free_rooms)
		{
			write_user(user,"~CW-~FT You don't need to steal a room.. rooms are free.\n");
			return;
		}
		else item = 2;
	}
	else if (!strcasecmp(word[2],"carriage")) item = 3;
	else if (!strcasecmp(word[2],"pet")) item = 4;
	else if (!strcasecmp(word[2],"beer")) item = 5;
	else if (!strcasecmp(word[2],"bullets")) item = 6;
	else if (!strcasecmp(word[3],"nuke")) item = 7;
	else
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRitem~CB]");
		write_user(user,"~CW - ~FGWhere the item can be~CB: [~CRhat~CW|~CRroom~CW|~CRcarriage~CB]\n");
		write_user(user,"~CW - ~FG                     ~CB  [~CRpet~CW|~CRbeer~CW|~CRbullets ~CW|~CRnuke~CB]\n");
		return;
	}
	if (u->protection) success = ran(35);	/* body guard..  1 in 50 */
	else success = ran(3);			/* otherwise 1 in 3 */
	if ((success != 1) || (u->protection && user->steal_tries != 35)
	  || (user->steal_tries != 3))
	{
		++user->steal_tries;
		if (u->protection)
		{
			writeus(user,"~FG  You step up to rob %s%s place while %s away... "
				     "~FGBut %s body guard meets you as you come up... beating"
				     "~FGyou senseless",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s",gender3[u->gender],gender1[u->gender]);
			bg_chance = ran(15);
			if (bg_chance == 1)
			{
				writeus(user,"~FG, when %s takes your wallet from you, taking all your money.\n",gender3[u->gender]);
				if (user->money != 0) write_user(u,"~CW-~FT Your wallet feels a little heavier.\n");
				u->money += user->money;
				user->money = 0;
			}
			else write_user(user,"~FG, leaving you there.. absolutely humiliated.\n");
			return;
		}
		else
		{
			writeus(user,"~FG  You step up to rob %s%s place while %s is away... "
				     "~FGwhen you hear a noise.. and rather than risk it..."
				     "~FGyou just cut out.. not getting a single thing.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s",gender3[u->gender]);
			return;
		}
	}
	else	/* Success... we've robbed them! */
	{

		if (u->protection)
		{
			writeus(user,"~FG  You step up to rob %s%s place while %s away... "
				     "~FGBut %s body guard meets you as you come up..."
				     "~FGNot backing down, you meet the body guard head on and the body guard"
				     "~FGends up just running off.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s",gender3[u->gender],gender1[u->gender]);
		}

		else
		{
			writeus(user,"~FG  You step up to rob %s%s place while %s is away... "
				     "~FGwhen you hear a noise.. but you decide to stay to claim your"
				     "~FGloot anyway.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s",gender3[u->gender]);
		}
		user->steal_tries = 0;
		switch (item)
		{
			case 1 :
			  if (u->hat == 0)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  if (u->hat)
			  {
				write_user(user,"~CW-~FT You already have a hat.. you don't need two.\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully grab %s%s hat.. and quickly run out again..\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
				user->hat = u->hat;
				u->hat = 0;
				write_user(user,"~CW-~FT Your head is starting to feel colder..\n");
				return;
			  }
			case 2 :
			  if (u->has_room == 0)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  if (user->has_room)
			  {
				write_user(user,"~CW-~FT You already have a room.. you don't need another.\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully take possession of %s%s whole house.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
				user->has_room = u->has_room;
				u->has_room = 0;
				write_user(user,"~CW-~FT You no longer have a place to live...\n");
				return;
			  }
			case 3 :
			  if (u->carriage == 0)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  if (user->carriage)
			  {
				write_user(user,"~CW-~FT You already have a carriage.. you don't need another.\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully take possession of %s%s horse carriage.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
				user->carriage = u->carriage;
				u->carriage = 0;
				write_user(user,"~CW-~FT You no longer own your horse drawn carriage...\n");
				return;
			  }
			case 4 :
			  if (u->pet.type == 0)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  if (user->pet.type != 0)
			  {
				write_user(user,"~CW-~FT You can't have two pets... I don't run a zoo..\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully take possession of %s%s pet %s.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s",pet[u->pet.type]);
				user->pet.type = u->pet.type;
				u->pet.type = 0;
				for (i = 0; i < 20; ++i)
				{
					strcpy(user->pet.alias[i],u->pet.alias[i]);
					u->pet.alias[i][0] = '\0';
					strcpy(user->pet.trigger[i],u->pet.trigger[i]);
					u->pet.trigger[i][0] = '\0';
				}
				write_user(user,"~CW-~FT You no longer own your pet :(\n");
				return;
			  }
			case 5 :
			  if (u->beers < 1)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully take possession of a couple of %s%s beer..\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
				if (u->beers >= 2) user->beers += 2;
				u->beers -= 2;
				write_user(u,"~CW-~FT It looks like a couple of your beer are gone...\n");
				return;
			  }
			case 6 :
			  if (u->bullets < 5)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully take possession of some of %s%s bullets..\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
				if (u->bullets >= 5) user->bullets += 5;
				u->bullets -= 5;
				write_user(u,"~CW-~FT It looks like your ammunition got a bit lighter...\n");
				return;
			  }
			case 7 :
			  if (u->nukes == 0)
			  {
				write_user(user,"~CW-~FT But you didn't get what you came for :-(\n");
				return;
			  }
			  else
			  {
				writeus(user,"~FG  You successfully take possession %s%s nuke.\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
				++user->nukes;
				--u->nukes;
				write_user(u,"~CW-~FT It looks like your losing your nuclear arsenal.\n");
				return;
			  }
			default:
			  write_user(user,"~CW-~FT Failed to steal anything..\n");
			  return;
		}
		return;
	} /* End of else */
	return;
}

/*
   Another new addition to RaMTITS 2 is that when a site/domain/user is banned
   the user must specify a reason why it's being banned. This command will then
   check to see if the ban specified is actually a ban.. and if so.. display
   information about it.
*/
void ban_info(User user,char *inpstr,int rt)
{
	Bans	b;
	int	hours,mins,secs,days,hours2,mins2,secs2,days2,i;
	long	ban_length;
	char	time_buff[81],temp[41];

	hours = mins = secs = days = ban_length = 0;
	hours2 = mins2 = secs2 = days2 = i = 0;
	for (i = 0; i < sizeof(time_buff)-1; ++i) time_buff[i] = '\0';
	for (i = 0; i < sizeof(temp)-1; ++i) temp[i] = '\0';
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (!strncasecmp(word[1],"-new",2))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-new ~CB[~CRnew site that was banned~CB]");
			return;
		}
		b = find_ban(word[2],NewSBan);
		if (b == NULL)
		{
			write_user(user,"~CW-~FT It doesn't appear as tho that site/domain is banned for new user logins.\n");
			return;
		}
	}
	else if (!strncasecmp(word[1],"-site",2))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-site ~CB[~CRsite that was banned~CB]");
			return;
		}
		b = find_ban(word[2],SiteBan);
		if (b == NULL)
		{
			write_user(user,"~CW-~FT It doesn't appear as tho that site/domain is banned.\n");
			return;
		}
	}
	else if (!strncasecmp(word[1],"-user",2))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-user ~CB[~CRusername that was banned~CB]");
			return;
		}
		b = find_ban(word[2],UserBan);
		if (b == NULL)
		{
			write_user(user,"~CW-~FT It doesn't appear as tho that user is banned.\n");
			return;
		}
	}
	else
	{
		writeus(user,usage,command[com_num].name,"-new  ~CB[~CRIP/Site of domain banned for new logons~CB]");
		writeus(user,usage,command[com_num].name,"-site ~CB[~CRIP/Site of domain banned~CB]");
		writeus(user,usage,command[com_num].name,"-user ~CB[~CRuser name~CB]");
		write_user(user,"~CW-~FT For a list of who is banned, use the ~CR.listbans~FT command.\n");
		return;
	}
	ban_length = (time(0) - b->time_banned);
	days	= ban_length / 86400;
	hours	= (ban_length%86400) / 3600;
	mins	= (ban_length%3600) / 60;
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
	writeus(user,"~FM Banned By~CB    : ~FT%s\n",b->by);
	writeus(user,"~FM Banned For~CB   : ~FT%s\n",time_buff);
	if (b->ban_length)
	{
		for (i = 0; i < sizeof(time_buff)-1; ++i) time_buff[i] = '\0';
		for (i = 0; i < sizeof(temp)-1; ++i) temp[i] = '\0';
		days2	= b->ban_length / 86400;
		hours2	= (b->ban_length%86400) / 3600;
		mins2	= (b->ban_length%3600) / 60;
		secs2	= (b->ban_length%60);
		if (days2)
		{
			sprintf(temp,"%d day%s",days2,days2 == 1?"":"s");
			strcat(time_buff,temp);
		}
		if (hours2)
		{
			if (strstr(time_buff,"day")) strcat(time_buff,", ");
			sprintf(temp,"%d hour%s",hours2,hours2 == 1?"":"s");
			strcat(time_buff,temp);
		}
		if (mins2)
		{
			if (strstr(time_buff,"hour") || strstr(time_buff,"day"))
			strcat(time_buff,", ");
			sprintf(temp,"%d minute%s",mins2,mins2 == 1?"":"s");
			strcat(time_buff,temp);
		}
		if (strstr(time_buff,"minute") || strstr(time_buff,"hour")  
		  || strstr(time_buff,"day")) strcat(time_buff,", and ");
		sprintf(temp,"%d second%s",secs2,secs2 == 1?"":"s");
		strcat(time_buff,temp);
		writeus(user,"~FMTime Remaining~CB: ~FT%s\n",time_buff);
	}
	writeus(user,"~FM Reason~CB       : ~FT%s\n",b->reason);
}

/*
   Due to a bug when saving a user while they're logged in.. this is
   just something that will re-nullify certain user values instead of
   them being set to #NOTSET#
*/
void reset_junk(User u)
{
	int i = 0;

	if (u == NULL) return;
	for (i = 0; i < 50; ++i)
	{
		if (!strcasecmp(u->friends[i],"#NOTSET#"))
			u->friends[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		if (!strcasecmp(u->enemies[i],"#NOTSET#"))
			u->enemies[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (!strcasecmp(u->rmacros[i],"#NOTSET#"))
			u->rmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 16; ++i)
	{
		if (u->rmacros[i][0] == '\0')
		{
			sprintf(text,"Unset%d",i);
			strncpy(u->rmacros[i],text,sizeof(u->rmacros[i])-1);
		}
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (!strcasecmp(u->fmacros[i],"#NOTSET#"))
			u->fmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 16; ++i)
	{
		if (u->fmacros[i][0] == '\0')
		{
			sprintf(text,"Unset%d",i);
			strncpy(u->fmacros[i],text,sizeof(u->fmacros[i])-1);
		}
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->allowcom[i],"#NOTSET#"))
			u->allowcom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->denycom[i],"#NOTSET#"))
			u->denycom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->alias.find[i],"#NOTSET#"))
			u->alias.find[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->alias.alias[i],"#NOTSET#"))
			u->alias.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (!strcasecmp(u->pet.alias[i],"#NOTSET#"))
			u->pet.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (!strcasecmp(u->pet.trigger[i],"#NOTSET#"))
			u->pet.trigger[i][0] = '\0';
	} i = 0;
	return;
}

/* END OF MODULE3.C */
