/*
  bootup.c
    Source for parsing the config file, and other functions that are
    performed when the talker server first boots up.

    The source within this file is Copyright 1999 - 2001 by
 	Rob Melhuish, and Neil Robertson.

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
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "include/blocking.h"
#define _BOOTUP_C
  #include "include/needed.h"
#undef _BOOTUP_C
#include <signal.h>

/* First load up the config file. */
void load_and_parse_config(void)
{
	FILE *fp;
	char line[82]; /* Should be long enough */
	char filename[FSL];
	int i,section_in,got_init;
	Room rm;

	section_in = got_init = 0;
	sprintf(filename,"%s/%s",DataDir,confile);
	printf("    Parsing config file \"%s\"...\n",filename);
	if ((fp=fopen(filename,"r")) == NULL)
	{
		perror("    [RaMTITS] - Can't open config file");
		boot_exit(1);
	}
	/* Main reading loop */
	config_line = 0;
	fgets(line,81,fp);
	while (!feof(fp))
	{
		config_line++;
		for (i=0;i<8;++i) wrd[i][0]='\0';
		sscanf(line,"%s %s %s %s %s %s %s %s",wrd[0],wrd[1],wrd[2],wrd[3],wrd[4],wrd[5],wrd[6],wrd[7]);
		if (wrd[0][0] == '#' || wrd[0][0] == '\0')
		{
			fgets(line,81,fp);
			continue;
		}
		parse_init_section();
		got_init = 1;
		fgets(line,81,fp);
	}
	FCLOSE(fp);
	/*
	   See if required sections were present and if required parameters -
	   were set.
	*/
	if (got_init == 0)
	{
		fprintf(stderr,"    [RaMTITS] - INIT section missing from config file.\n");
		boot_exit(1);
	}
	if (!port[0])
	{
		fprintf(stderr,"    [RaMTITS] - Main port number not set in config file.\n");
		boot_exit(1);
	}
	if (!port[1])
	{
		fprintf(stderr,"    [RaMTITS] - Wiz port number not set in config file.\n");
		boot_exit(1);
	}
	if (port[0] == port[1])
	{
		fprintf(stderr,"    [RaMTITS] - Port numbers must be unique.\n");
		boot_exit(1);
	}
	if (room_first == NULL)
	{
		fprintf(stderr,"    [RaMTITS] - No main_room found.\n");
		boot_exit(1);
	}
	/*
	   Everything has parsed successfully, we should have 3 rooms loaded
	   the main_room (which is your talkers main room, the idle_room,
	   and the jail_room. reason these three rooms are automatically
	   loaded is because they are the rooms with the most traffic in and
	   out of them.. so now we go and make sure we have some atmospherics
	*/
	for_rooms(rm)
	{
		if (!(load_atmos(rm))) atmos_assign(rm);
	}
}

/* Now parse all our data. */
void parse_init_section(void)
{
	Room room;
	int op,val=0,err=0;
	size_t i=0;
	char temp[81];
	char *options[]={
		"mainport",	  "wizport",	    "minlogin_level",
		"mesg_life",	  "wizport_level",  "prompt_def",
		"gatecrash_level","min_private",    "ignore_mp_level",
		"mesg_check_time","max_users",	    "heartbeat",
		"login_idle_time","user_idle_time", "password_echo",
		"ignore_sigterm", "max_clones",	    "ban_swearing",
		"crash_action",   "time_out_afks",  "charecho_def",
		"time_out_maxlevel","backup_talker","debug_talker",
		"webpage_on",	  "arrest_swearers","free_rooms",
		"auto_promote",	  "auto_auth",      "main_room",
		"idle_room",	  "jail_room",
		"*" /* Stopping clause */
	};
	op = 0;

	while (strcmp(options[op],wrd[0]))
	{
		if (options[op][0]=='*')
		{
			fprintf(stderr,"    [RaMTITS] - Unknown initialization option on line %d.\n",config_line);
			boot_exit(1);
		}
		++op;
	}
	if (wrd[1][0] == '\0')
	{
		fprintf(stderr,"    [RaMTITS] - Required parameter missing on line %d.\n",config_line);
		boot_exit(1);
	}
	if (wrd[2][0] != '\0' && wrd[2][0] != '#')
	{
		fprintf(stderr,"    [RaMTITS] - Unexpected word following init parameter on line %d.\n",config_line);
		boot_exit(1);
	}
	strncpy(temp,wrd[1],sizeof(temp)-1);
	if (!isdigit(temp[0])) {
		for (i=0;i<strlen(temp);++i)
		{
			if (temp[i] == '_') temp[i] = ' ';
		}
	}
	else val = atoi(wrd[1]);
	switch (op)
	{
		case 0 : /* Main talker port.	*/
		case 1 : /* Staff port.		*/
		  if ((port[op] = val)<100 || val>65535)
		  {
			fprintf(stderr,"    [RaMTITS] - Illegal port number on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 2 : /* Min login level.	*/
		  if ((minlogin_level=get_level(wrd[1])) == -1)
		  {
			if (strcasecmp(wrd[1],"NONE"))
			{
				fprintf(stderr,"    [RaMTITS] - Unknown level specifier for minlogin_level on line %d.\n",config_line);
				boot_exit(1);
			}
			minlogin_level = -1;
		  }
		  return;
		case 3 : /* Message lifetime	*/
		  if ((mesg_life=val)<1)
		  {
			fprintf(stderr,"    [RaMTITS] - Illegal message lifetime on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 4 : /* Wizport Level.	*/
		  if ((wizport_level = get_level(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - Unknown level specifier for wizport_level on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 5 : /* Prompt default	*/
		  if ((prompt_def = onoff_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - prompt_def must be ON or OFF on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 6 : /* Gatecrash level.	*/
		  if ((gatecrash_level = get_level(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - Unknown level specifier for gatecrash_level on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 7 : /* Min private users.	*/
		  if (val<1)
		  {
			fprintf(stderr,"    [RaMTITS] - Number too low for min_private_users on line %d.\n",config_line);
			boot_exit(1);
		  }
		  min_private_users = val;
		  return;
		case 8 : /* Ignore min private level.	*/
		  if ((ignore_mp_level = get_level(wrd[1]))==-1)
		  {
			fprintf(stderr,"    [RaMTITS] - Unknown level specifier for ignore_mp_level on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 9 : /* mesg_check_time	*/
		  if (wrd[1][2]!=':' || strlen(wrd[1])>5 || !isdigit(wrd[1][0])
		    || !isdigit(wrd[1][1]) || !isdigit(wrd[1][3])
		    || !isdigit(wrd[1][4]))
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid mesg_check_time on line %d.\n",config_line);
			boot_exit(1);
		  }
		  sscanf(wrd[1],"%d:%d",&mesg_check_hour,&mesg_check_min);
		  if (mesg_check_hour>23 || mesg_check_min>59)
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid mesg_check_time on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 10: /* Max users allowed online.	*/
		  if ((max_users = val)<1)
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid value for max_users on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 11: /* Talker heartbeat.		*/
		  if ((heartbeat = val)<1)
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid value for heartbeat on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 12: /* Login idle timeout.		*/
		  if ((login_idle_time = val)<10)
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid value for login_idle_time on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 13: /* User idle timeout.		*/
		  if ((user_idle_time = val)<10)
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid value for user_idle_time on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 14: /* Password echoing.		*/
		  if ((password_echo = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - Password_echo must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 15: /* Ignore sigterm.		*/
		  if ((ignore_sigterm = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - ignore_sigterm must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 16: /* Max. amount of clones.	*/
		  if ((max_clones = val)<1)
		  {
			fprintf(stderr,"    [RaMTITS] - Invalid value for max_clones on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 17: /* Swearing ban.		*/
		  if ((ban_swearing = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - ban_swearing must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 18: /* Crash action.		*/
		  if (!strcasecmp(wrd[1],"NONE")) crash_action = 0;
		  else if (!strcasecmp(wrd[1],"IGNORE"))  crash_action = 1;
		  else if (!strcasecmp(wrd[1],"REBOOT"))  crash_action = 2;
		  else if (!strcmp(wrd[1],"SREBOOT")) crash_action = 3;
		  else
		  {
			fprintf(stderr,"    [RaMTITS] - crash_action must be NONE, IGNORE, REBOOT, or SREBOOT on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 19: /* Time out afks.		*/
		  if ((time_out_afks = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - time_out_afks must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 20: /* Character Echo default.	*/
		  if ((charecho_def = onoff_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - charecho_def must be ON or OFF on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 21: /* Time out maxlevel.		*/
		  if ((time_out_maxlevel = get_level(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - Unknown level specifier for time_out_maxlevel on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 22: /* Talker backup.		*/
		  if ((backup_talker = onoff_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - Backup_talker must be ON or OFF on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 23: /* Talker debugging.		*/
		  if ((debug_talker = onoff_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - debug_talker must be ON or OFF on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 24: /* Webpage generator.		*/
		  if ((webpage_on = onoff_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - webpage_on must be ON or OFF on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 25: /* Arrest swearers.		*/
		  if ((arrest_swearers = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - arrest_swearers must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  if (arrest_swearers && ban_swearing)
		  {
			fprintf(stderr,"    [RaMTITS] - arrest_swearers AND ban_swearing are toggled on (only 1 allowed)\n");
			boot_exit(1);
		  }
		  return;
		case 26: /* Free user rooms.		*/
		  if ((free_rooms = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - free_rooms must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 27: /* Auto promote.		*/
		  if ((auto_promote = yn_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - auto_promote must be YES or NO on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 28: /* Auto auth lookup.		*/
		  if ((auto_auth = onoff_check(wrd[1])) == -1)
		  {
			fprintf(stderr,"    [RaMTITS] - auto_auth must be ON or OFF on line %d.\n",config_line);
			boot_exit(1);
		  }
		  return;
		case 29: /* The main room.		*/
		  if ((room = create_room()) == NULL)
		  {
			fprintf(stderr,"    [RaMTITS] - Failed to allocate memory to create the main room.\n");
			boot_exit(1);
		  }
		  strncpy(room->name,temp,sizeof(room->name)-1);
	  	  for (i=0;i<strlen(room->name);++i)
		  {
		  	if (room->name[i]=='_') room->name[i]=' ';
		  }
		  main_room[0] = '\0';
		  strncpy(main_room,room->name,sizeof(main_room)-1);
		  err = LoadRoom(room,main_room);
		  if (err == 0 || err == -1)
		  {
			reset_room(room);
			strcpy(room->name,main_room);
			fprintf(stderr,"    [RaMTITS] - Unable to find room details for room %s (main_room).\n",room->name);
			fprintf(stderr,"    [RaMTITS] - Will attempt to create one.\n");
			strcpy(room->recap,room->name);
			strcpy(room->topic,def_topic);
			strcpy(room->owner,"NULL");
			strcpy(room->map,"main");
			room->access=FIXED_PUBLIC;  room->no_buff=0;
			room->hidden = 0;
			for (i=0;i<MaxAtmos;++i) room->atmos[i][0] = '\0';
			for (i=0;i<MaxKeys;++i) room->key[i][0] = '\0';
			SaveRoom(room);
		  }
		  return;
		case 30: /* The idle room.	*/
		  if ((room = create_room()) == NULL)
		  {
			fprintf(stderr,"    [RaMTITS] - Failed to allocate memory to create the idle room.\n");
			boot_exit(1);
		  }
		  strncpy(room->name,temp,sizeof(room->name)-1);
		  for (i=0;i<strlen(room->name);++i)
		  {
		  	if (room->name[i]=='_') room->name[i]=' ';
		  }
		  idle_room[0] = '\0';
		  strncpy(idle_room,room->name,sizeof(idle_room)-1);
		  err = LoadRoom(room,idle_room);
		  if (err < 1)
		  {
			reset_room(room);
			strcpy(room->name,idle_room);
			fprintf(stderr,"    [RaMTITS] - Unable to find room details for room %s (idle_room).\n",room->name);
			fprintf(stderr,"    [RaMTITS] - Will attempt to create one.\n");
			strcpy(room->recap,room->name);
			strncpy(room->link[0],main_room,sizeof(room->link[0])-1);
			strcpy(room->topic,def_topic);
			strcpy(room->owner,"NULL");
			strcpy(room->map,"main");
			room->access = FIXED_PRIVATE;
			room->no_buff = 0;
			room->hidden = 0;
			for (i=0;i<MaxAtmos;++i) room->atmos[i][0] = '\0';
			for (i=0;i<MaxKeys;++i) room->key[i][0] = '\0';
			SaveRoom(room);
		  }
		  return;
		case 31: /* The jail room.	*/
		  if ((room = create_room()) == NULL)
		  {
			fprintf(stderr,"    [RaMTITS] - Failed to allocate memory to create the jail room.\n");
			boot_exit(1);
		  }
		  strncpy(room->name,temp,sizeof(room->name)-1);
		  for (i=0;i<strlen(room->name);++i)
		  {
		  	if (room->name[i] == '_') room->name[i] = ' ';
		  }
		  jail_room[0] = '\0';
		  strncpy(jail_room,room->name,sizeof(jail_room));
		  err = LoadRoom(room,jail_room);
		  if (err < 1)
		  {
			reset_room(room);
			strcpy(room->name,jail_room);
			fprintf(stderr,"    [RaMTITS] - Unable to find room details for room %s (jail_room).\n",room->name);
			fprintf(stderr,"    [RaMTITS] - Will attempt to create one.\n");
			strcpy(room->recap,room->name);
			strncpy(room->link[0],main_room,sizeof(room->link[0])-1);
			strncpy(room->link[1],idle_room,sizeof(room->link[1])-1);
			strncpy(room->topic,def_topic,sizeof(room->topic)-1);
			strcpy(room->owner,"NULL");
			strcpy(room->map,"main");
			room->access = FIXED_PRIVATE;
			room->no_buff = 0;
			room->hidden = 0;
			for (i=0;i<MaxAtmos;++i) room->atmos[i][0] = '\0';
			for (i=0;i<MaxKeys;++i) room->key[i][0] = '\0';
			SaveRoom(room);
		  }
		  return;
	}
	return;
}

/* put the commands in the ordered linked list. Based off of the Amnuts  -
 - command ordering. 							 */
void parse_cmnds(void)
{
	int cnt;

	cnt = 0;
	while (command[cnt].name[0] != '*')
	{
		if (!(add_command(cnt)))
		{
			fprintf(stderr,"    [RaMTITS] - Memory allocation failure in parse_cmds().\n");
			boot_exit(9);
		}
		++cnt;
	}
	return;
}

/*
  This will make the userlist.. reading from the UserFiles dir all of the
  .dat files. I thank Andy for parts of this function (his process_users())
  I know now why he did this.. and to me it makes a helluva lotta sence
*/
void make_userlist(void)
{
	DIR *dir;
	User u;
	char dirname[FSL],name[UserNameLen+5];
	struct dirent *dp;
	int err=0;

	sprintf(dirname,"%s",UserDir);
	dir = opendir(dirname);
	if (dir == NULL)
	{
		fprintf(stderr,"    [RaMTITS] - Unable to open userfiles directory in make_userlist().\n");
		boot_exit(10);
	}
	if ((u = create_user()) == NULL)
	{
		fprintf(stderr,"    [RaMTITS] - Unable to create temp, user object in make_userlist()\n");
		(void)closedir(dir);
		boot_exit(10);
	}
	/* Go through and read each file in the dir, including . and .. */
	while ((dp = readdir(dir)))
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
		  continue;
		if (strstr(dp->d_name,".dat"))
		{
			strcpy(name,dp->d_name);
			name[strlen(name)-4] = '\0';
			if (strlen(name)<3 || strlen(name)>UserNameLen)
			  continue;
			err = LoadUser(u,name);
			if (err == 0)
			{
				fprintf(stderr,"    [RaMTITS] - Unable to load userfile for [%s] in make_userlist()\n",name);
				write_log(1,LOG1,"[RaMTITS] [ERROR] - Couldn't load userfile: [%s]\n",name);
				continue;
			}
			else if (err == -1)
			{
				fprintf(stderr,"    [RaMTITS] - Userfile '%s' is corrupted.\n",name);
				write_log(0,LOG1,"[RaMTITS] [ERROR] - Corrupt userfile: [%s]\n",name);
				continue;
			}
			else add_userlist(u->name,u->level);
		}
		reset_vars(u);
	}
	destruct_user(u);
	(void)closedir(dir);
	destructed = 0;
}

/* Does pretty much the same thing as the above.. but does it for rooms. */
void make_roomlist(void)
{
	DIR *dir;
	Room rm,next;
	char dirname[FSL],name[RoomNameLen+6];
	struct dirent *dp;
	int err=0;

	sprintf(dirname,"%s",RoomDir);
	dir=opendir(dirname);
	if (dir == NULL)
	{
		fprintf(stderr,"    [RaMTITS] - Unable to open roomfiles directory in make_roomlist().\n");
		boot_exit(11);
	}
	if ((rm = create_room()) == NULL)
	{
		fprintf(stderr,"    [RaMTITS] - Unable to create temp, room object in make_roomlist()\n");
		(void)closedir(dir);
		boot_exit(11);
	}
	while ((dp = readdir(dir)))
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")
		  || !strcmp(dp->d_name,".dat")) continue;
		if (strstr(dp->d_name,".dat"))
		{
			strcpy(name,dp->d_name);
			name[strlen(name)-4]='\0';
			if (strlen(name)<2 || strlen(name)>RoomNameLen) continue;
			strcpy(rm->name,name);
			err = LoadRoom(rm,name);
			if (err < 1)
			{
				fprintf(stderr,"    [RaMTITS] - Unable to load roomfile for [%s] in make_roomlist()\n",rm->name);
				write_log(0,LOG1,"[RaMTITS] [ERROR] - Unable to load roomfile: [%s]\n",name);
				continue;
			}
			else add_roomlist(rm);
		}
		reset_room(rm);
	}
	destruct_room(rm);
	(void)closedir(dir);
	rm=room_first;
	/* go through.. make sure they are all gone. */
	while (rm != NULL)
	{
		next = rm->next;
		destruct_room(rm);
		rm = next;
	}

}

/* Initialize and reset the talker globals. */
void init_globals(void)
{
	int i;

	port[0] 	= 0;		port[1]		= 0;
	max_users 	= 0;		max_clones	= 0;
	ban_swearing 	= 0;		arrest_swearers = 0;
	heartbeat	= 2;		login_idle_time = 0;
	user_idle_time  = 0;		time_out_afks 	= 0;
	wizport_level	= 0;		minlogin_level	= -1;
	mesg_life	= 0;		no_prompt	= 0;
	num_of_users	= 0;		num_of_logins	= 0;
	password_echo	= 0;		ignore_sigterm	= 0;
	crash_action	= 0;		backup_talker	= 0;
	debug_talker	= 0;		webpage_on	= 0;
	free_rooms	= 0;		auto_promote	= 0;
	auto_auth	= 0;		prompt_def	= 0;
	mesg_check_hour	= 0;		charecho_def	= 0;
	time_out_maxlevel=0;		mesg_check_min	= 0;
	num_of_telnets	= 0;		max_telnets	= 5;
	wiznote_num	= 0;		num_editors	= 0;
	shout_ban	= 0;		rs_countdown	= 0;
	rs_announce	= 0;		rs_which	= -1;
	gatecrash_level	= 0;		sclosetime	= 0;
	seed		= 0;		min_private_users=0;
	ignore_mp_level = 0;		user_count	= 0;
	new_users	= 0;		logging_in	= 0;
	suggestion_num	= 0;		bug_num		= 0;
	destructed	= 0;		nuclear_countdown=-3;
	nuke_announce	= 0;		check_time	= 0;
	resolve_sites	= 1;		id_count	= 0;
	last_uid	= 0;		sreboot_time	= 0;

	rs_user		= NULL;		user_first	= NULL;
	user_last	= NULL;		user_afirst	= NULL;
	user_alast	= NULL;		room_first	= NULL;
	room_last	= NULL;		pgame_first	= NULL;
	pgame_last	= NULL;		telnet_first	= NULL;
	telnet_last	= NULL;		edit_first	= NULL;
	edit_last	= NULL;		first_command	= NULL;
	last_command	= NULL;		pplayer_first	= NULL;
	pplayer_last	= NULL;		first_social	= NULL;
	last_social	= NULL;		rlist_first	= NULL;
	rlist_last	= NULL;		ulist_first	= NULL;
	ulist_last	= NULL;		lotto_first	= NULL;
	lotto_last	= NULL;		first_ban	= NULL;
	last_ban	= NULL;
#ifdef HAVE_BSHIP
	bships_first	= NULL;
#endif
	memset(main_room,0,sizeof(main_room));
	memset(idle_room,0,sizeof(idle_room));
	memset(jail_room,0,sizeof(jail_room));
	memset(text,0,sizeof(text));
	memset(sessionwho,0,sizeof(sessionwho));
	memset(file,0,sizeof(file));
	memset(session,0,sizeof(session));
	memset(vtext,0,sizeof(vtext));

	if (jackpot == 0) jackpot = 500;
	for (i = 0 ; i < MaxStaff ; ++i)
		memset(staffmember[i],0,sizeof(staffmember[0]));
	for (i = 0 ; i < 20 ; ++i)
		memset(disabled[i],0,sizeof(disabled[0]));
	for (i = 0 ; i < 50 ; ++i)
		memset(member[i],0,sizeof(member[0]));
	i=0;
	clear_words();
	time(&boot_time);
}

/* Initialise the signal traps etc */
void init_signals(void)
{

	SIGNAL(SIGTERM,sig_handler);
	SIGNAL(SIGHUP,sig_handler);
	SIGNAL(SIGSEGV,sig_handler);
	SIGNAL(SIGBUS,sig_handler);
	SIGNAL(SIGILL,SIG_IGN);
	SIGNAL(SIGTRAP,SIG_IGN);
	SIGNAL(SIGIOT,SIG_IGN);
	SIGNAL(SIGTSTP,SIG_IGN);
	SIGNAL(SIGCONT,SIG_IGN);
	SIGNAL(SIGINT,SIG_IGN);
	SIGNAL(SIGQUIT,SIG_IGN);
	SIGNAL(SIGABRT,SIG_IGN);
	SIGNAL(SIGFPE,SIG_IGN);
	SIGNAL(SIGPIPE,SIG_IGN);
	SIGNAL(SIGTTIN,SIG_IGN);
	SIGNAL(SIGTTOU,SIG_IGN);
}

/* Exit because of error during bootup */
void boot_exit(int code)
{

	switch (code)
	{
		case 1 :
		  write_log(0,LOG1,"[BOOT FAILURE] - Error while parsing configuration file.\n");
		  exit(1);
		case 2 :
		  perror("    [RaMTITS] - Can't open main port listen socket.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Can't open main port listen socket.\n");
		  exit(2);
		case 3 :
		  perror("    [RaMTITS] - Can't open wiz port listen socket.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Can't open wiz port listen socket.\n");
		  exit(3);
		case 4 :
		  perror("    [RaMTITS] - Can't open ident port listen socket.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Can't open ident port listen socket.\n");
		  exit(4);
		case 5 :
		  perror("    [RaMTITS] - Can't bind to main port.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Can't bind to main port.\n");
		  exit(5);
		case 6 :
		  perror("    [RaMTITS] - Can't bind to wiz port.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Can't bind to wiz port.\n");
		  exit(6);
		case 7 :
		  perror("    [RaMTITS] - Can't bind to ident port.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Can't bind to ident port.\n");
		  exit(7);
		case 8 :
		  perror("    [RaMTITS] - Listen error on main port.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Listen error on main port.\n");
		  exit(8);
		case 9 :
		  perror("    [RaMTITS] - Listen error on wiz port.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Listen error on wiz port.\n");
		  exit(9);
		case 10:
		  perror("    [RaMTITS] - Listen error on ident port.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Listen error on ident port.\n");
		  exit(10);
		case 11:
		  perror("    [RaMTITS] - Failed to fork.");
		  write_log(0,LOG1,"[BOOT FAILURE] - Failed to fork.\n");
		  exit(11);
		case 12:
		  perror("    [RaMTITS] - Failed to parse the commands.\n");
		  write_log(0,LOG1,"[BOOT FAILURE] - Failed to parse the commands.\n");
		  exit(12);
		case 13:
		  perror("    [RaMTITS] - Failed to process the user list.\n");
		  write_log(0,LOG1,"[BOOT FAILURE] - Failed to process user list.\n");
		  exit(13);
		case 14:
		  perror("    [RaMTITS] - Failed to process the room list.\n");
		  write_log(0,LOG1,"[BOOT FAILURE] - Failed to process room list.\n");
		  exit(14);
		case 15:
		  perror("    [RaMTITS] - Failed to process the social list.\n");
		  write_log(0,LOG1,"[BOOT FAILURE] - Failed to process social list.\n");
		  exit(15);
	}
}

/* Initialise sockets on ports */
void init_sockets(void)
{
	struct sockaddr_in sock_addr;
	int i,on,size;

	printf("    Initialising %s talker sockets.\n",TalkerName);
	printf("    User port: [%d] Staff port: [%d]\n",port[0],port[1]);
	on = 1;
	size = sizeof(struct sockaddr_in);
	sock_addr.sin_family            = AF_INET;
	sock_addr.sin_addr.s_addr       = htonl(INADDR_ANY);
	for (i = 0 ; i < 2 ; ++i)
	{
		/* Create the sockets. */
		if ((listen_sock[i] = socket(AF_INET,SOCK_STREAM,0)) == -1)
		  boot_exit(i+2);
		/* allow reboots on port even with TIME_WAITS */
		setsockopt(listen_sock[i],SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
		/* bind sockets and set up listen queues */
		sock_addr.sin_port=htons(port[i]);
		if (bind(listen_sock[i],(struct sockaddr *)&sock_addr,size) == -1)
		  boot_exit(i+5);
		if (listen(listen_sock[i],5) == -1) boot_exit(i+8);
		fcntl(listen_sock[i],F_SETFL,NBLOCK_CMD);
	}
}

/*
   Load up the socials into the social structure.. socials won't really take
   up much mem, cause all that's really being stored is the social name. but
   I suppose if you have like 50,000 socials.. then it could take up lots of
   memory... hehe.. well.. maybe not.. just multiply 50,000 time 21 bytes..
*/
void make_sociallist(void)
{
	DIR *dir;
	char dirname[FSL],name[21+5];
	struct dirent *dp;

	sprintf(dirname,"%s",SocialDir);
	dir = opendir(dirname);
	if (dir == NULL)
	{
		fprintf(stderr,"    [RaMTITS] - Unable to open socials directory.\n");
		boot_exit(12);
	}
	/* Go through and read each file in the dir, including . and .. */
	while ((dp = readdir(dir)))
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
		  continue;
		if (strstr(dp->d_name,".dat"))
		{
			strcpy(name,dp->d_name);
			name[strlen(name)-4] = '\0';
			if (strlen(name)<2 || strlen(name)>20) continue;
			if (!(add_social(name)))
			{
				fprintf(stderr,"    [RaMTITS] - Failed to add a social..\n");
				(void)closedir(dir);
				boot_exit(12);
			}
		}
	}
	(void)closedir(dir);
}

/* Generates a picture listing... */
int GeneratePiclist(void)
{
	FILE *fp;
	DIR *dir;
	struct dirent *dp;
	char picfile[FSL],dirname[FSL],line[16];
	int cnt, header = 0;

	sprintf(dirname,"%s",PicDir);
	dir = opendir(dirname);
	if (dir == NULL) return(-1);
	sprintf(picfile,"%s/%s",MiscDir,PicList);
	if ((fp=fopen(picfile,"w")) == NULL) return(-1);
	cnt = 0;
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
		  continue;
		line[0] = '\0';
		++cnt;
		if (cnt == 1 && header == 0)
		{
			header = 1;
			sprintf(text,"~FT-~CT=~CB> ~FG%s ~FRPicture listing... ~CB<~CT=~FT-\n",TalkerRecap);
			fprintf(fp,center(text,80));
			fputs(ascii_tline,fp);
			fputs(" ",fp);
			cnt = 0;
		}
		if (cnt%5 == 0 && cnt != 0)
		{
			fputs("\n",fp);
			fputs(" ",fp);
		}
		sprintf(line,"%-15.15s",dp->d_name);
		fputs(line,fp);
	}
	(void)closedir(dir);
	if (cnt && cnt%5 != 0) fputs("\n",fp);
	if (cnt) fputs(ascii_bline,fp);
	else fputs("~CW- ~FGNo pictures found.\n",fp);
	FCLOSE(fp);
	return(0);
}

/* Now Generate the text list.. */
int GenerateTextList(void)
{
	DIR *dir;
	FILE *fp;
	char textfile[FSL],dirname[FSL],line[16];
	int cnt, header = 0;
	struct dirent *dp;

	sprintf(dirname,"%s",TextDir);
	dir = opendir(dirname);
	if (dir == NULL) return(-1);
	sprintf(textfile,"%s/%s",MiscDir,TextList);
	if ((fp = fopen(textfile,"w")) == NULL) return(-1);
	cnt = 0;
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
		  continue;
		line[0] = '\0';
		++cnt;
		if (cnt == 1 && header == 0)
		{
			header = 1;
			sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYText file listing... ~CB<~CT=~FT-\n",TalkerRecap);
			fprintf(fp,center(text,80));
			fputs(ascii_tline,fp);
			fputs(" ",fp);
			cnt = 0;
		}
		if (cnt%5 == 0 && cnt != 0)
		{
			fputs("\n",fp);
			fputs(" ",fp);
		}
		sprintf(line,"%-15.15s",dp->d_name);
		fputs(line,fp);
	}
	(void)closedir(dir);
	if (cnt && cnt%5 != 0) fputs("\n",fp);
	if (cnt) fputs(ascii_bline,fp);
	else fputs("~CW- ~FGNo textfiles found.\n",fp);
	FCLOSE(fp);
	return(0);
}

/* Now Generate the text list.. */
int GenerateFigfontList(void)
{
	DIR *dir;
	FILE *fp;
	char textfile[FSL], dirname[FSL], line[16], name[16];
	int cnt, header = 0;
	struct dirent *dp;

	sprintf(dirname,"../fonts");
	dir = opendir(dirname);
	if (dir == NULL) return(-1);
	sprintf(textfile,"%s/figfonts",TextDir);
	if ((fp = fopen(textfile,"w")) == NULL) return(-1);
	cnt = 0;
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
		  continue;
		if (!strstr(dp->d_name,".flf")) continue;
		strncpy(name,dp->d_name,sizeof(name)-1);
		name[strlen(name) - 4] = '\0';
		line[0] = '\0';
		++cnt;
		if (cnt == 1 && header == 0)
		{
			header = 1;
			sprintf(text,"~FT-~CT=~CB> ~FG%s ~FYFiglet Font listing... ~CB<~CT=~FT-\n",TalkerRecap);
			fprintf(fp,center(text,80));
			fputs(ascii_tline,fp);
			fputs(" ",fp);
			cnt = 0;
		}
		if (cnt%5 == 0 && cnt != 0)
		{
			fputs("\n",fp);
			fputs(" ",fp);
		}
		sprintf(line,"%-15.15s",name);
		fputs(line,fp);
	}
	(void)closedir(dir);
	if (cnt && cnt%5 != 0) fputs("\n",fp);
	if (cnt) fputs(ascii_bline,fp);
	else fputs("~CB-- ~RSNo Figlet fonts found.\n",fp);
	FCLOSE(fp);
	return(0);
}

/*
   Remove any expired messages from boards unless force == 2 in which case
   we just do a recount. This function also does the maintenance of the
   talker, due to the ease of it.
*/
void check_messages(User user,char *inpstr,int force)
{
	Room rm;
	FILE *infp,*outfp=NULL;
	DIR *dir;
	struct dirent *dp;
	char	id[ARR_SIZE],line[ARR_SIZE],dash[20],from[81],name[81],
		filename[FSL],rmname[RoomNameLen+6],dirname[FSL];
	int	valid,pt,write_rest,pth,shit,ccnt,board_cnt,old_cnt,bad_cnt,
		tmp,msg_num,loaded;
	static int done=0,user_rooms=0;

	switch (force)
	{
		case 0:
		  if (mesg_check_hour == thour && mesg_check_min == tmin)
		  {
			user_rooms = 1;
			if (done) return;
		  }
		  else
		  {
			user_rooms = 0;
			done = 0;
			return;
		  }
		  break;
		case 1:
		  shit = 1;
	}
	done = 1;
	board_cnt = old_cnt = bad_cnt = ccnt = 0;
	sprintf(dirname,"%s",RoomDir);
	dir = opendir((char *)dirname);
	if (dir == NULL)
	{
		if (force) fprintf(stderr,"     [RaMTITS] - Failed to open RoomDir: [%s]\n",strerror(errno));
		write_log(0,LOG1,"[ERROR] - Failed to open RoomDir in check_messages().\n");
		return;
	}
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strstr(dp->d_name,".dat")) continue;
		strcpy(rmname,dp->d_name);
		rmname[strlen(rmname)-4] = '\0';
		if (rmname[0] == '\0') continue;
		if ((rm = get_room(rmname)) == NULL)
		{
			if ((rm = create_room()) == NULL)
			{
				if (force) fprintf(stderr,"     [RaMTITS] - Failed to create temp room object.\n");
				write_log(0,LOG1,"[ERROR] - Failed to create a temp room object.\n");
				(void)closedir(dir);
				return;
			}
			reset_room(rm);
			strcpy(rm->name,rmname);
			if ((LoadRoom(rm,rmname)) == -1)
			{
				destruct_room(rm);
				continue;
			}
			loaded = 0;
		}
		else loaded = 1;
		if (user_rooms == 1)
		{
			if ((get_user_full(NULL,rmname)) != NULL)
			{
				/* 
				   User's personal room.. don't bother
				   deleteing msgs
				*/
				destruct_room(rm);
				continue;
			}
		}
		tmp = rm->mesg_cnt;
		rm->mesg_cnt = 0;
		sprintf(filename,"%s/%s/%s.B",RoomDir,MesgBoard,rm->name);
		if ((infp = fopen(filename,"r")) == NULL)
		{
			destruct_room(rm);
			continue;
		}
		if (force < 2)
		{
			if ((outfp = fopen("tempfile","w")) == NULL)
			{
				if (force) fprintf(stderr,"     [RaMTITS] - Couldn't open tempfile to write: [%s]\n",strerror(errno));
				write_log(0,LOG1,"[ERROR] - Failed to open tempfile in check_messages().\n");
				FCLOSE(infp);
				(void)closedir(dir);
				if (loaded == 0) destruct_room(rm);
				return;
			}
		}
		board_cnt++;
		valid = 1;
		write_rest = 1;
		fgets(line,ARR_SIZE-1,infp);
		while (!(feof(infp)))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s %d %s %s %s %d",id,&pt,from,name,dash,&msg_num);
			pth = pt;
			if (write_rest == 1)
			{
				if (valid && !strcmp(id,"PT:"))
				{
					if (force == 2 || force == 3)
					  rm->mesg_cnt++;
					else
					{
						/* 86400 = secs in a day */
						if ((int)time(0)-pt < mesg_life*86400)
						{
							ccnt=colour_com_count(name);
							rm->mesg_cnt++;
							msg_num=rm->mesg_cnt;
							sprintf(line,"%s %d\r %s %-*.*s ~CW-~CR %d\n",id,pth,from,20+ccnt*3,20+ccnt*3,name,msg_num);
							fputs(line,outfp);
							write_rest = 1;
						}
						else
						{
							write_rest = 0;
							old_cnt++;
						}
					}
					valid = 0;
				}
				else fputs(line,outfp);
			}
/*
			else
			{
				if (valid && !strcmp(id,"PT:"))
				{
					ccnt = colour_com_count(name);
					rm->mesg_cnt++;
					msg_num = rm->mesg_cnt;
					sprintf(line,"%s %d\r %s %-*.*s ~CW-~CR %d\n",id,pth,from,20+ccnt*3,20+ccnt*3,name,msg_num);
					fputs(line,outfp);
					valid = 0;
				}
				else fputs(line,outfp);
			}
*/
			fgets(line,ARR_SIZE-1,infp);
		}
		FCLOSE(infp);
		if (force < 2)
		{
			FCLOSE(outfp);
			unlink(filename);
			if (write_rest == 0) unlink("tempfile");
			else rename("tempfile",filename);
		}
		if (rm->mesg_cnt != tmp) bad_cnt++;
		if (loaded == 0) destruct_room(rm);
	}
	(void)closedir(dir);
	if (com_num == get_comnum("wipe") || com_num == get_comnum("read")
	  || com_num == get_comnum("bfrom")) return;
	switch (force)
	{
		case 0:
		  write_room(NULL,"~CW[~CRRaMTITS~CW]~RS General maintenance is being performed...\n");
		  if (bad_cnt) sprintf(text,"[Check-Messages] - Files checked: [%d] Incorrect: [%d] Deleted: [%d]\n",board_cnt,bad_cnt,old_cnt);
		  else sprintf(text,"[Check-Messages] - Files checked: [%d] Deleted: [%d]\n",board_cnt,old_cnt);
		  write_log(1,LOG1,text);
		  do_maintenance();
		  break;
		case 1:
		  printf("    [RaMTITS] - Files Checked: [%d] Outdated: [%d]\n",board_cnt,old_cnt);
		  break;
		case 2:
		  writeus(user,"~CW- ~FYChecked ~CR%d ~FYboard%s.. ~CR%d~FY produced an incorrect message count.\n",board_cnt,board_cnt>1?"s":"",bad_cnt);
		  if (bad_cnt) sprintf(text,"[Check-Messages-Forced] - Files checked: [%d] Incorrect: [%d] Deleted: [%d] User: [%s]\n",board_cnt,bad_cnt,old_cnt,user->name);
		  sprintf(text,"[Check-Messages-Forced] - Files checked: [%d] Deleted: [%d] User: [%s]\n",board_cnt,old_cnt,user->name);
		  write_log(1,LOG1,text);
	}
}

void load_bans(int type)
{
	Bans	b;
	FILE	*fp;
	char	filename[FSL],banned_by[15],sitemask[81],line[256];
	int	ban_length, i;
	time_t	ban_time;

	ban_length = ban_time = i = 0;
	switch(type)
	{
		case SiteBan:
		  sprintf(filename,"%s/%s",DataDir,BanSites);
		  break;
		case NewSBan:
		  sprintf(filename,"%s/%s",DataDir,BanNew);
		  break;
		case WhoSBan:
		  sprintf(filename,"%s/%s",DataDir,BanWho);
		  break;
		case UserBan:
		  sprintf(filename,"%s/%s",DataDir,BanUsers);
		  break;
		case Screen :
		  sprintf(filename,"%s/%s",DataDir,SiteScreen);
		  break;
	}
	fp = fopen(filename,"r");
	if (fp == NULL) return;
	for (i = 0; i < sizeof(line)-1; ++i) line[i] = '\0';
	fgets(line,255,fp);
	while (!feof(fp))
	{
		sscanf(line,"%s %ld %d %s",sitemask,&ban_time,&ban_length,banned_by);
		strcpy(line,remove_first(line));
		strcpy(line,remove_first(line));
		strcpy(line,remove_first(line));
		strcpy(line,remove_first(line));
		
		b = new_ban(sitemask,type);
		if (b == NULL)
		{
			printf("    -- ERROR: Unable to load bans (%d).\n",type);
			FCLOSE(fp);
			return;
		}
		b->time_banned = ban_time;
		b->ban_length  = ban_length;
		line[strlen(line)-1] = '\0';
		strncpy(b->reason,line,sizeof(b->reason)-1);
		strncpy(b->by,banned_by,sizeof(b->by)-1);
		for (i = 0; i < sizeof(line)-1; ++i) line[i] = '\0';
		fgets(line,255,fp);
	}
	FCLOSE(fp);
	return;
}

/* END OF BOOTUP.C */
