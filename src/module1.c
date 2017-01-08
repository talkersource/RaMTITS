/*
   module1.c
     The source within this source file contains the functions for various
     commands that were new for RaMTITS version 1.1.x	

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Mike Irving, and Reddawg

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
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#define _MODULE1_C
  #include "include/needed.h"
#undef _MODULE1_C

/*
   Here we write the contents of the debug buffer out to debug_file.. I've
   updated this quite a bit so that I'll have more information on if the
   talker crashes I'll have a better understanding of why the fuck it did
   that.
*/
void WriteDebugFile(void)
{
	User u,nxt;
	FILE *fp;
	char filename[FSL],name[UserNameLen+2],site[SiteNameLen+1],ip_site[16];
	int i = 0,cnt;

	cnt = 0;
	sprintf(filename,"%s/%s/%s.%u",SysLogDir,DebugLogs,DebugLog,(unsigned int)getpid());
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Can't open '%s.%u' in WriteDebugFile()\n"
				 "        - Reason: [%s]\n",DebugLog,(unsigned int)getpid(),strerror(errno));
		return;
	}
	u = user_first;
	while (u != NULL)
	{
		nxt = u->next;
		if (u->type != UserType) continue;
		/* First see if the user is in the login stage */
		if (u->login > 0)
		{
			++cnt;
			if (u->name[0]!='\0') strcpy(name,u->name);
			else strcpy(name,"Not Known");
			if (u->site[0]!='\0') strcpy(site,u->site);
			else strcpy(site,"Not Known");
			if (u->ip_site[0]!='\0') strcpy(ip_site,u->ip_site);
			else strcpy(ip_site,"Not Known");
			fprintf(fp,"[Login - Stage: (%2d)] - Name: [%s] Site: [%s:%s]\n",u->login,name,site,ip_site);
			if (cnt != 0) fprintf(fp,"- Total of %d logins.\n",cnt);
		}
		if (u->set_op != 0)
	          fprintf(fp,"[SetOp   (%2d)] - User: [%s]\n",u->set_op,u->name);
		if (u->bank_op != 0)
		  fprintf(fp,"[BankOp  (%2d)] - User: [%s]\n",u->bank_op,u->name);
		if (u->store_op != 0)
		  fprintf(fp,"[StoreOp (%2d)] - User: [%s]\n",u->store_op,u->name);
		if (u->editor.editing != 0)
		  fprintf(fp,"[Editor  (%2d)] - User: [%s]\n",u->editor.editor,u->name);
		if (u->game_op != 0)
		  fprintf(fp,"[GameOp  (%2d)] - User: [%s]\n",u->game_op,u->name);
		if (u->mail_op != 0)
		  fprintf(fp,"[MailOp  (%2d)] - User: [%s]\n",u->mail_op,u->name);
		u = nxt;
	}
	cnt = 0;
	for(i = 0; i < 15; ++i)
	{
		if (debugbuff->conv[i] != NULL) ++cnt;
	}
	if (cnt >= 1)
	{
		fprintf(fp,"- Start of commands that were used...\n");
		filewrite_convbuff(fp,debugbuff);
		fprintf(fp,"- End of commands that were used...\n");
	}
	else if (cnt == 0) fprintf(fp,"- No commands in memory...\n");
	FCLOSE(fp);
	write_log(1,LOG1,"[RaMTITS] - Crash file saved to: [%s.%u]\n",DebugLog,(unsigned int)getpid());
	return;
}

/* Lets a user page another user via the ICQ pager.mirabilis.com server. */
void icqpage(User user,char *inpstr,int rt)
{
	User u;
	FILE *fp;
	char filename[FSL],receiver[UserNameLen+2],*uname;
	int icq_num = 0,err = 0;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRmessage~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u=get_user(word[1])))	/* Online user */
	{
		if (u->icq == 0)
		{
			write_user(user,"~FG Sorry, that user must set their icq number!\n");
			return;
		}
		if (u->icqhide)
		{
			write_user(user,"~FG That user has their ICQ number hidden, you can't send them an icqpage.\n");
			return;
		}
		sprintf(filename,"%s/%s.icq",TempFiles,user->name);
		icq_num = u->icq;
		strcpy(receiver,u->name);
		if ((fp = fopen(filename,"w")) == NULL)
		{
			write_user(user,"~CW-~FT I wasn't able to send your icqpage.. sorry.\n");
			write_log(0,LOG1,"[ERROR] - Cannot open file '%s.icq' to write in icqpage()\n   - Reason: [%s]\n",user->name,strerror(errno));
			return;
		}
		fprintf(fp,"This page is being sent to you from %s@%s!\n",user->name,TalkerName);
		fprintf(fp,"To connect to this place telnet to '%s'\n",TalkerAddy);
		fprintf(fp,"RaMSoft ICQ pager (c)1999 Mike Irving & Rob Melhuish All Rights Reserved\n");
		fprintf(fp,"Below is the message sent:\n");
		inpstr=remove_first(inpstr);
		fprintf(fp,"%s\n",inpstr);
		FCLOSE(fp);
		send_icq_page(user,icq_num,filename,receiver);
		return;
	}
	if ((u = create_user()) == NULL)	/* Offline user */
	{
		write_user(user,"~CW-~FT I wasn't able to send your icqpage.. sorry.\n");
		write_log(0,LOG1,"[ERROR] - Can't create temp user object in icqpage().\n");
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
		write_user(user,"~CW-~FT It appears as tho that users userfile is corrupted..\n");
		write_log(1,LOG1,"[ERROR] - Corrupt userfile found: [%s]\n",u->name);
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	strcpy(u->site,u->last_site);
	if (u->icq == 0)
	{
		write_user(user,"~FG Sorry, that user must set their icq number.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	if (u->icqhide)
	{
		write_user(user,"~FG Sorry, that user has their ICQ number hidden.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	sprintf(filename,"%s/%s.icq",TempFiles,user->name);
	icq_num = u->icq;
	strcpy(receiver,u->name);
	destruct_user(u);
	destructed = 0;
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW-~FT I wasn't able to send your icqpage.. sorry.\n");
		write_log(0,LOG1,"[ERROR] - Cannot open file '%s.icq' to write in icqpage()\n   - Reason: [%s]\n",user->name,strerror(errno));
		return;
	}
	fprintf(fp,"This page is being sent to you from %s@%s\n",user->name,TalkerName);
	fprintf(fp,"To connect to this place telnet to '%s'\n",TalkerAddy);
	fprintf(fp,"RaMSoft ICQ pager (c)1999 Mike Irving & Rob Melhuish All Rights Reserved\n");
	fprintf(fp,"Below is the message sent:\n");
	inpstr = remove_first(inpstr);
	fprintf(fp,"%s\n",inpstr);
	FCLOSE(fp);
	send_icq_page(user,icq_num,filename,receiver);
	write_user(user,"~FGYour ICQ page has been sent.\n");
	return;
}

/* Now send the page */
int send_icq_page(User user,int icq_num,char *fname,char *receive)
{

	switch(double_fork())
	{
		case -1:
		  return(0);      /* failed to fork */
		case 0 :
		  writeus(user,"~FG Sending an ICQ page to ~CT%s: ~FT%d...\n",receive,icq_num);
		  write_user(user,"~FG If they are currently online, they will receive your message shortly.\n");
		  write_user(user,"~FG Otherwise, they will receive it when they come online next.\n");
		  write_log(1,LOG1,"[ICQ Page]: User: [%s] Recipient: [%s]\n",user->name,receive);
		  sprintf(text,"mail -s \"ICQ page from %s\" %d@pager.mirabilis.com < %s",user->name,icq_num,fname);
		  system(text);
		  unlink(fname);
		  _exit(1);
		  /* Really.. if it gets this far.. something wrong happened */
		  return(1);
	}
	return(0);
}

/*
   Deletes any users who haven't been on in DeleteSecs and aren't protected
   from expiration. Much like Amnuts purge.
   NOTE: 1 day = 86400 seconds
*/
void delete_old(User user,char *inpstr,int rt)
{
	User u;
	Ulobj ul,ulnext;
	int total = 0,num_del = 0,timelen = 0,err = 0;

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
	if ((u = create_user()) == NULL)
	{
		write_user(user,"~CW- ~FTFailed to create a temp. user object.\n");
		return;
	}
	write_log(1,LOG1,"[RaMTITS (%s)] - User wipe starting...\n",command[com_num].name);
	ul = ulist_first;
	while (ul != NULL)
	{
		ulnext = ul->next;
		total++;
		if (get_user(ul->name) != NULL)
		{
			ul = ulnext;
			continue;
		}
		reset_vars(u);
		u->socket = -2;
		err = LoadUser(u,ul->name);
		if (err == 0)
		{
			clean_userlist(u->name);
			delete_files(u->name);
			ul = ulnext;
			reset_vars(u);
			continue;
		}
		else if (err==-1)
		{
			write_log(1,LOG1,"[ERROR] - Corrupt userfile found: [%s]\n",ul->name);
			ul = ulnext;
			reset_vars(u);
			continue;
		}
		timelen = (int)(time(0)-u->last_login);
		if (timelen >= DeleteSecs)
		{
			if (u->delprotect)
			{
				write_log(1,LOG1,"[RaMTITS (delold)]  - User: [%s] Protected....Skipping\n",u->name);
				ul = ulnext;
				reset_vars(u);
				continue;
			}
			writeus(user,"~CB -- ~FMOld User~CB: [~CR%s~CB] ~FMfound, deleting...\n",u->name);
			write_log(1,LOG1,"[RaMTITS (delold)]  - Deleting: [%s]\n",u->name);
			clean_userlist(u->name);
			delete_files(u->name);
			num_del++;
			reset_vars(u);
			ul = ulnext;
			continue;
		}
		reset_vars(u);
		ul = ulnext;
	}
	destruct_user(u);
	destructed = 0;
	writeus(user,"~CW- ~FGTotal of ~CY%d ~FGuser%s checked. ~CR%d~FG were deleted.\n",total,total>1?"s":"",num_del);
	write_log(1,LOG1,"[RaMTITS (delold)] - Checked %d users. Deleted %d\n",total,num_del);
}

/* Deletes files of the given name */
void delete_files(char *name)
{
	char filename[FSL];

	strtolower(name);
	name[0] = toupper(name[0]);
	sprintf(filename,"%s/%s.dat",UserDir,name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.R",UserDir,UserMisc,name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.html",WWWDir,"users",name);
	unlink(filename);
	sprintf(filename,"%s/%s/%s.jpg",WWWDir,UserPics,name);
	unlink(filename);
	sprintf(filename,"%s/%s.dat",RoomDir,name);
	unlink(filename);
}

/* Let the user set their room name if they have a room */
void set_roomname(User user,char *inpstr,int rt)
{
	Room rm;
	char roomname[RoomNameLen+2];
	size_t i = 0;

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
		writeus(user,usage,command[com_num].name,"~CB[~CRa name for your room~CB]");
		return;
	}
	if (strcmp(rm->owner,user->name))
	{
		write_user(user,"~FG You have to be in your room in order to do this.\n");
		return;
	}
	strcpy(roomname,ColourStrip(inpstr));
	strtolower(roomname);
	if (strlen(inpstr) > 80)
	{
		write_user(user,"~FG That room name is too long.\n");
		return;
	}
	if (strlen(roomname)>RoomNameLen)
	{
		write_user(user,"~FG That room name is too long.\n");
		return;
	}
	for (i = 0; i < strlen(roomname); ++i)
	{
		if (!isalpha(roomname[i]) && !isspace(roomname[i]))
		{
			write_user(user,"~FG You have an illegal character in that room name.\n");
			return;
		}
	}
	if (room_exists(roomname))
	{
		write_user(user,"~FG That room name is already being used.\n");
		return;
	}
	strncpy(rm->recap,inpstr,sizeof(rm->recap)-1);
	writeus(user,"~CW - ~FGYour room name has been set to ~CW'~CR%s~CW'\n",rm->recap);
	SaveRoom(rm);
}

/* Gives a user the called upon obect */
void give_stuff(User user,char *inpstr,int rt)
{
	User u;
	int object = 0,on = 0,object2 = 0,err = 0;
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
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR room");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR atmos ~CB[~CR# of atmos to give~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR carriage");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR hat");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR condoms~CB [~CR# of condoms to give~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR bguard");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR money~CB [~CRamount of money to give~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR chips~CB [~CRamount of chips to give~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) == NULL)	/* Not logged on */
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred.. try again in a couple of minutes.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in give_stuff()!\n");
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
			write_user(user,"~CW-~FT It appears as tho that user's userfile is corrupted.\n");
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
		u->socket = -2;
		strcpy(u->site,u->last_site);
	}
	else on = 1;
	if (u==user)
	{
		write_user(user,"~FG You cannot give yourself stuff..\n");
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	if (!strcasecmp(word[2],"room"))
	{
		if (u->level >= WIZ)
		{
			writeus(user,"~FM %s~FG doesn't need a personal room.\n",u->recap);
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		if (u->has_room)
		{
			writeus(user,"~FM %s~FG already has a personal room.\n",u->recap);
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->has_room = 1;
		writeus(user,"~FG You give ~FM%s~FG a personal room.\n",u->recap);
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [room]\n",user->name,u->name);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)	write_user(u,"~FG You have been given a personal room.  Use ~CW'~CR.rmname~CW'~FG to enter a name for it.\n");
		return;
	}
	if (!strcasecmp(word[2],"bguard"))
	{
		if (u->protection)
		{
			writeus(user,"~CW-~FT %s~FG already has a bodyguard.\n",u->recap);
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->protection = 1;
		writeus(user,"~FG You hire ~FM%s~FG a body guard.\n",u->recap);
		write_log(1,LOG1,"[GIVE] - User: [%s] Recipient: [%s] Item: [bodyguard]\n",user->name,u->name);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed=0;
		}
		if (on)	write_user(u,"~FG You have been given a body guard.\n");
		return;
	}
	if (!strcasecmp(word[2],"hat"))
	{
		if (u->hat)
		{
			writeus(user,"~FM %s~FG already has a coloured hat.\n",u->recap);
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->hat = 1;
		writeus(user,"~FG You give ~FM%s~FG a coloured hat.\n",u->recap);
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [hat]\n",user->name,u->name);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)	write_user(u,"~FG You have been given a coloured hat.  Use ~CW'~CR.hatcolour~CW'~FG to change the colour.\n");
		return;
	}
	if (!strcasecmp(word[2],"carriage"))
	{
		if (u->carriage == 1)
		{
			writeus(user,"~CW-~FT %s~FG already has a horse carriage.\n",u->recap);
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->carriage = 1;
		writeus(user,"~FG You give ~FM%s~FG a carriage.\n",u->recap);
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [carriage]\n",user->name,u->name);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)	write_user(u,"~FG You have been given a horse carriage.\n");
		return;
	}
	if (!strcasecmp(word[2],"atmos"))
	{
		if (word_count < 4)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR atmos~CB [~CR# of atmos to give~CB]");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}	
		object = atoi(word[3]);
		if (object < 1)
		{
			write_user(user,"~FG The atmos. amount has to be greater then zero.\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		if (object >= MaxAtmos)
		{
			write_user(user,"~FG You cannot give the user that many atmospherics.\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed=0;
			}
			return;
		}
		object2 = object;
		object2 += u->atmos;
		if (object2>=MaxAtmos)
		{
			write_user(user,"~FG You cannot give the user that many atmospherics.\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed=0;
			}
			return;
		}
		u->atmos += object;
		writeus(user,"~FG You have given ~FM%s ~CT%d~FG atmospheric%s for %s room.\n",u->recap,object,object>1?"s":"",gender1[u->gender]);
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [atmos (%d)]\n",user->name,u->name,object);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)
		{
			writeus(u,"~FG You have been given ~CT%d~FG atmospheric%s for your room.\n",object,object>1?"s":"");
		}
		return;
	}
	if (!strcasecmp(word[2],"condoms"))
	{
		if (word_count < 4)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR condoms ~CB[~CR# of condoms to give~CB]");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}	
		object = atoi(word[3]);
		if (object < 1)
		{
			write_user(user,"~FG The condom amount has to be greater then zero.\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		object2 = object;
		object2 += u->condoms;
		if (object2 >= 65535)
		{
			write_user(user,"~CW-~FT You can't give out that many condoms..\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->condoms += object;
		writeus(user,"~FG You have given ~FM%s ~CT%d~FG condom%s.\n",u->recap,object,object>1?"s":"");
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [condoms (%d)]\n",user->name,u->name,object);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)
		{
			writeus(u,"~FG You have been given ~CT%d~FG condom%s.\n",object,object>1?"s":"");
		}
		return;
	}
	if (!strcasecmp(word[2],"money"))
	{
		if (word_count < 4)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR money ~CB[~CRamount of money to give~CB]");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		object = atoi(word[3]);
		if (object < 1 || object > 5000)
		{
			write_user(user,"~CW-~FT You can't give away that kinda money..\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->money += object;
		writeus(user,"~FG You have given ~FM%s ~CT%d~FG dollar%s.\n",u->recap,object,object>1?"s":"");
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [money (%d)]\n",user->name,u->name,object);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)
		{
			writeus(u,"~FG You have been given ~CT%d~FG dollar%s.\n",object,object>1?"s":"");
		}
		return;
	}
	if (!strcasecmp(word[2],"chips"))
	{
		if (word_count < 4)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]~CR chips ~CB[~CRamount of chips to give~CB]");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		object = atoi(word[3]);
		if (object < 1 || object > 5000)
		{
			write_user(user,"~CW-~FT You can't give away that amount of chips..\n");
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}
			return;
		}
		u->money += object;
		writeus(user,"~FG You have given ~FM%s ~CT%d~FG chip%s.\n",u->recap,object,object>1?"s":"");
		write_log(1,LOG4,"[GIVE] - User: [%s] Recipient: [%s] Item: [chips (%d)]\n",user->name,u->name,object);
		if (on == 0)
		{
			SaveUser(u,0);
			destruct_user(u);
			destructed = 0;
		}
		if (on)
		{
			writeus(u,"~FG You have been given ~CT%d~FG chip%s.\n",object,object>1?"s":"");
		}
		return;
	}
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
	write_user(user,"~FG You have entered an invalid option.\n");
	return;
}

/*
   Do the users webpage. Gets updated everytime a user .sets an object,
   Leaves the set menu, or enters a profile.
*/
void do_user_webpage(User user)
{
	FILE *fp;
	char html_file[FSL],pic_file[FSL],tempfile[FSL],name[UserNameLen+1];
	char ranking[20],temp1[21],temp2[21],temp3[81],line[81];

	if (webpage_on == 0) return; /* webpage generator ain't on.. fuck it */
	strcpy(ranking,user->gender==Female?level[user->level].fname:level[user->level].mname);
	if (user->lev_rank[0] != '\0') strcpy(ranking,user->lev_rank);
	strncpy(name,user->name,sizeof(name)-1);
	strtolower(name);
	temp1[0] = temp2[0] = temp3[0] = line[0] = '\0';
	strncpy(temp1,ColourStrip(user->predesc),sizeof(temp1)-1);
	strncpy(temp2,ColourStrip(user->recap),sizeof(temp2)-1);
	strncpy(temp3,ColourStrip(user->desc),sizeof(temp3)-1);
	sprintf(line,"%s%s%s%s%s",user->predesc[0]?temp1:"",user->predesc[0]?" ":"",temp2,user->desc[0]=='\''?"":" ",temp3);

	sprintf(html_file,"%s/%s/%s.html",WWWDir,"users",name);
	sprintf(pic_file,"%s/%s/%s.jpg",WWWDir,UserPics,user->name);
	if ((fp = fopen(html_file,"w")) == NULL)
	{
		writeus(user,"~CW-~FT An error occurred in creating your webpage file.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open file '%s.html' to write in do_user_webpage()\n"
				 "        - Reason: [%s]\n",name,strerror(errno));
		return;
	}
	fputs(html_header,fp); /* Put in the header.. */
	/*
           Now below this point is the actual HTML tags, I've decided to go
	   about it in a table manner because it looks much cleaner and nicer.
	   If you want to check out a sample, check out:
	     http://dragons.ramtits.ca/users/index.php (or squirt.html)
	   So I would advise you touch nothing beyond this point, unless you
	   know exactly what you're doing. For more info about the generated
	   webpages read 'README.webpage' in the documents directory.
	*/
	fprintf(fp,
"
<center>
<table cellpadding=\"4\" cellspacing=\"1\" width=\"80%%\" border=\"3\">
<tr>
<td colspan=\"4\" align=\"center\"><font size=\"+2\">Information About %s</font></td>
</tr>
<tr>
<td align=\"left\" colspan=\"4\">
<font size=\"+1\"><u>Pre-desc, name, description:</u></font>
<br>
<font size=\"+1\">%s</font>
</td>
</tr>
<tr>
<td align=\"left\"><font size=\"+1\">Age:</font></td>
<td align=\"left\"><font size=\"+1\"><u>%d</u></b></font></td>
<td align=\"left\"><font size=\"+1\">Level:</font></td>
<td align=\"left\"><font size=\"+1\"><u>%s</u></font></td>
</tr>
<tr>
<td align=\"left\"><font size=\"+1\">E-mail Addy:</font></td>
",user->name,line,user->age,ranking);

	if (user->hide_email==1) fprintf(fp,"<td align=\"left\"><font size=\"+1\"><u>Hidden</u></font></td>\n");
	else fprintf(fp,"<td align=\"left\"><font size=\"+1\"><a href=\"mailto:%s\"><u>%s</u></a></font></td>\n",user->email,user->email);

	fprintf(fp,"<td align=\"left\"><font size=\"+1\"><u>ICQ UIN:</u></font></td>\n");
	if (user->icqhide == 1) fprintf(fp,"<td align=\"left\"><font size=\"+1\"><u>Hidden</u></font></td>\n");
	else fprintf(fp,"<td align=\"left\"><font size=\"+1\"><u>%ld</u></font></td>\n",user->icq);
	fprintf(fp,
"
</tr>
<tr>
<td align=\"left\"><font size=\"+1\">Homepage:</font></td>
<td align=\"left\"><font size=\"+1\"><a href=\"%s\"><u>%s</u></a></font></td>
<td align=\"left\"><font size=\"+1\">Gender:</font></td>
<td align=\"left\"><font size=\"+1\"><u>%s</u></b></font></td>
</tr>
<tr>
<td align=\"left\"><font size=\"+1\">Married To:</font></td>
",user->webpage,user->webpage,gendert[user->gender]);

	sprintf(tempfile,"%s/%s/%s.html",WWWDir,"users",user->married_to);
	strtolower(tempfile);
	if (file_exists(tempfile))
	{
		sprintf(tempfile,"%s.html",user->married_to);
		strtolower(tempfile);
		fprintf(fp,"<td align=\"left\"><font size=\"+1\"><a href=\"%s\"><u>%s</u></a></font></td>\n",tempfile,user->married_to);
	}
	else fprintf(fp,"<td align=\"left\"><font size=\"+1\"><u>%s</u></font></td>\n",user->married_to);
	fprintf(fp,
"
<td align=\"left\"><font size=\"+1\">Birthday [MM/DD]:</font></td>
<td align=\"left\"><font size=\"+1\"><u>%s</u></font></td>
</tr>
<tr>
<td colspan=\"4\" align=\"left\"><font size=\"+2\">%s%s Profile:</font></td>
</tr>
<tr>
<td colspan=\"4\" align=\"left\"><font size=\"+1\">
",user->bday,user->name,user->name[strlen(user->name)-1] == 's'?"'":"'s");
	process_profile(user,fp);
	fputs("</font></td>\n</tr>\n</table>\n<hr width=\"80%%\" noshade color=\"#FFFFFF\">\n",fp);
	if (file_exists(pic_file))
	{
		fprintf(fp,
"
<font size=\"+2\">%s%s Picture:</font>
<br>
<hr widht=\"80%%\" noshade color=\"#FFFFFF\">
<br>
<img src=\"%s/%s/%s.jpg\" alt=\"Picture of %s\" border=\"0\">
</center>
",user->name,user->name[strlen(user->name)-1] == 's'?"'":"'s",WWWDir,UserPics,user->name,user->name);
	}
	fputs(html_footer,fp);
	FCLOSE(fp);
	write_log(1,LOG5,"[WWW-Generator] - User: [%s]\n",user->name);
	write_user(user,"~CW-~FT Your user webpage has been modified.\n");
	return;
}

/* Process' a users profile.. (Need something to remove $vars, and stuff) */
void process_profile(User user,FILE *fp)
{
	FILE *pfp;
	char pro_file[FSL],line[ARR_SIZE];
	int in_html = 0;

	if (user == NULL) return;
	if (fp == NULL) return;
	sprintf(pro_file,"%s/%s/%s.P",UserDir,UserMesg,user->name);
	if ((pfp = fopen(pro_file,"r")) == NULL)
	fprintf(fp,"<b><font size=\"+1\">%s doesn't have a profile to view.</font></b>\n",user->name);
	else
	{
		fgets(line,ARR_SIZE-1,pfp);
		while (!feof(pfp))
		{
			if (!strncasecmp(line,"[HTML]",6))
			{
				in_html = 1;
				fgets(line,ARR_SIZE-1,pfp);
				continue;
			}
			if (!strncasecmp(line,"[/HTML]",7))
			{
				in_html = 0;
				fgets(line,ARR_SIZE-1,pfp);
				continue;
			}
			strcpy(line,ColourStrip(line));
			fputs(htmlize(line),fp);
			if (in_html == 0) fputs("<br>\n",fp);
			fgets(line,ARR_SIZE-1,pfp);
		}
		FCLOSE(pfp);
	}
	return;
}

/* This process' the $vars and replaces them accordingly. */
char *htmlize(char *str)
{
	static char text2[ARR_SIZE*2];
	char *s,*t;
	int x;

	x = 0; t = text2; s = str;
	str = ColourStrip(str);
	if (!strchr(str,'$')) return(str);
	if (!strlen(str)) return(str);
	while (*s)
	{
		for (x = 0; x < NumVars; ++x)
		{
			if (!strcasecmp(s,"$HTML"))
			{
				strcpy(text2,remove_first(str));
				return(text2);
			}
			if (!strncasecmp(s,user_vars[x],strlen(user_vars[x])))
			{
				memcpy(t,html_var(user_vars[x]),strlen(html_var(user_vars[x])));
				t += strlen(html_var(user_vars[x]))-1;
				s += strlen(user_vars[x])-1;
				goto CONT;
			}
		}
		*t = *s;
		CONT:
		  ++s; ++t;
	}
	*t = '\0';
	return(text2);
}

char *html_var(char *which_one)
{
	static char temp[ARR_SIZE];

	temp[0] = '\0';
	if (!strcasecmp(which_one,"$USER")) strcpy(temp,"WebSurfer");
	else if (!strcasecmp(which_one,"$SITE")) strcpy(temp,"ramtits.ca");
	else if (!strcasecmp(which_one,"$HANGCH")) strcpy(temp,"N/A");
	else if (!strcasecmp(which_one,"$GENDER")) strcpy(temp,"Not known");
	else if (!strcasecmp(which_one,"$EMAIL")) strcpy(temp,"anaddy@somewhere.com");
	else if (!strcasecmp(which_one,"$URL"))	strcpy(temp,"http://www.ramtits.ca");
	else if (!strcasecmp(which_one,"$LALIAS")) strcpy(temp,"SuperSurfer");
	else if (!strcasecmp(which_one,"$BDAY")) strcpy(temp,"Not known");
	else if (!strcasecmp(which_one,"$RLEVEL")) strcpy(temp,"Surfer");
	else if (!strcasecmp(which_one,"$RNAME")) strcpy(temp,"Webuser");
	else if (!strcasecmp(which_one,"$TIME")) strcpy(temp,"Now");
	else if (!strcasecmp(which_one,"$ICQ"))	strcpy(temp,"Not known");
	else if (!strcasecmp(which_one,"$MONEY")) strcpy(temp,"None");
	else if (!strcasecmp(which_one,"$DMONEY")) strcpy(temp,"None");
	else if (!strcasecmp(which_one,"$CHIPS")) strcpy(temp,"None");
	else if (!strcasecmp(which_one,"$DCHIPS")) strcpy(temp,"None");
	else if (!strcasecmp(which_one,"$AGE")) strcpy(temp,"Not known");
	else if (!strcasecmp(which_one,"$PDESC")) strcpy(temp,"Super");
	else if (!strcasecmp(which_one,"$DESC")) strcpy(temp,"a lonely surfer");
	else strcpy(temp,"Not known");
	return(temp);
}

/*
   Creates a who.html file so users may check via the web who is on the
   talker without having to actually log into it.  Gets updated whenever
   someone logs on/off.
*/
void who_webpage(void)
{
	User u;
	Room rm;
	FILE *fp;
	int total,mins,idle,in_room;
	char filename[FSL],levelname[10],status[10],line[500],
	     name[UserNameLen+1];

	total = 0;
	sprintf(filename,"%s/%s/%s.html",WWWDir,"users",WhoWebpage);
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Unable to open '%s.html' to write in who_webpage()!\n",WhoWebpage);
		return;
	}
	fputs(html_header,fp);
	fprintf(fp,
"
<center>
<table border=\"3\" cellpadding=\"3\" cellspacing=\"1\" width=\"80%%\">
<tr>
<td align=\"center\" colspan=\"5\"><font size=\"+2\">Users currently logged on %s</font></td>
</tr>
<tr>
<td align=\"left\" width=\"10%%\"><font size=\"+1\" color=\"Green\">Ranking</font></td>
<td align=\"center\" width=\"5%%\"><font size=\"+1\" color=\"Purple\">Min</font></td>
<td align=\"center\" width=\"5%%\"><font size=\"+1\" color=\"Purple\">Id</font></td>
<td align=\"center\" width=\"10%%\"><font size=\"+1\" color=\"Red\">Status</font></td>
<td align=\"left\"><font size=\"+1\">&nbsp;&nbsp;&nbsp;Pre-desc, Name, and description</font></td>
</tr>
",TalkerName);
	for_rooms(rm)
	{
		in_room = 0;
		for_ausers(u)
		{
			if (u->room != rm || u->type == CloneType
			  || u->invis>=Invis || u->room->hidden) continue;
			mins = (int)(time(0)-u->last_login)/60;
			idle = (int)(time(0)-u->last_input)/60;
			if (u->lev_rank[0] != '\0') strcpy(levelname,u->lev_rank);
			else strcpy(levelname,u->gender==Female?level[u->level].fname:level[u->level].mname);
			if (u->status & Jailed) strcpy(levelname,"JAILED");
			++total;
			strncpy(name,u->name,sizeof(name)-1);
			strtolower(name);
			if (u->predesc[0] != '\0') sprintf(line,"%s <a href=\"%s.html\">%s</a> %s",u->predesc,name,u->recap,u->desc);
			else sprintf(line,"<a href=\"%s.html\">%s</a> %s",name,u->recap,u->desc);
			if (u->afk) strcpy(status," AFK ");
			else if (u->set_op) strcpy(status,"SETUP");
			else if (u->bank_op) strcpy(status,"BANK");
			else if (u->store_op) strcpy(status,"STORE");
			else if (u->editor.editing) strcpy(status,"EDIT");
			else if (idle) strcpy(status,"sleep");
			else if (!idle) strcpy(status,"awake");
			if (u->room == rm)
			  if (in_room == 0)
			  {
				sprintf(text,"<tr>\n<td align=\"left\" colspan=\"5\"><b><font size=\"+1\" color=\"Blue\">[</font><font size=\"+1\" color=\"Purple\">%s</font><font size=\"+1\" color=\"Blue\">]</font></b></td>\n</tr>\n",ColourStrip(u->room->recap));
				in_room = 1;
				fputs(text,fp);
			  }
			fprintf(fp,
"
<tr>
<td align=\"left\" width=\"10%%\"><font size=\"+1\" color=\"Green\">%s</font></td>
<td align=\"center\" width=\"5%%\"><font size=\"+1\" color=\"Purple\">%-4.4d</font></b></td>
<td align=\"center\" width=\"5%%\"><font size=\"+1\" color=\"Purple\">%-4.4d</font></td>
<td align=\"center\" width=\"10%%\"><font size=\"+1\" color=\"Red\">%s</font></td>
<td align=\"left\"><font size=\"+1\">&nbsp;&nbsp;&nbsp;%s</font></td>
</tr>
",levelname,mins,idle,status,htmlize(line));
		}
	}
	if (total == 0)
	{
		fprintf(fp,"<tr>\n<td colspan=\"5\"><font size=\"+1\">There are currently no users online.</font></td>\n</tr>\n");
	}
	else
	{
		fprintf(fp,"<tr>\n<td colspan=\"5\"><font size=\"+1\">There %s a total of <font color=red>%d</font> user%s online.</font></td>\n</tr>\n",total>1?"are":"is",total,total>1?"s":"");
	}
	fprintf(fp,"</table>\n</center>\n");
	fputs(html_footer,fp);
	FCLOSE(fp);
}

/*
   Allows a user to capture their tell buffer, or review buffer of the room
   they are in, or the specified s-mail message to the capture log. This
   will help prevent harassive users from getting away with whatever they
   are doing.
*/
void capture(User user,char *inpstr,int rt)
{
	Room rm;
	FILE *fp,*mfp;
	char filename[FSL],mailfile[FSL],line2[ARR_SIZE],check[ARR_SIZE];
	int i,cnt,msg_num,amount,valid;

	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CR-tells~CW|~CR-revbuff~CW|~CR-shouts~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CR-smail #~CB]");
		write_user(user,"~CW - ~FGUpon capturing something it is sent to a capture log, where the staff can see\n");
		write_user(user,"~CW - ~FGwhat has happened. For more info ~CW'~CR.help capture~CW'\n");
		return;
	}
	sprintf(filename,"%s/%s",SysLogDir,CapLog);
	if ((fp = fopen(filename,"a")) == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred while trying to right to the log..\n");
		write_log(0,LOG1,"[ERROR] - Unable to open capture log to append in capture()!\n");
		return;
	}
	cnt = 0;
	if (!strcasecmp(word[1],"-tells"))
	{
		for (i = 0; i < NUM_LINES; ++i)
			if (user->tbuff->conv[i]!=NULL) ++cnt;
		if (cnt)
		{
			fprintf(fp,center("-=[ Tell buffer submitted by %s. ]=-\n",80),user->name);
			filewrite_convbuff(fp,user->tbuff);
			write_user(user,"~FG Submitting your tell buffer to the capture log. Upon completion\n");
			write_user(user,"~FG your tell buffer will be cleared, in case the harrassive user\n");
			write_user(user,"~FG continues. If which they do, submit your tell buffer to the capture log again.\n");
			fprintf(fp,"\n");
			fprintf(fp,mesg_line);
			write_log(1,LOG1,"[CAPTURE] - User: [%s] Type: [tell buffer]\n",user->name);
			clear_convbuff(user->tbuff);
		}
		else write_user(user,"~FG Your tell buffer was empty.\n");
		FCLOSE(fp);
		return;
	}
	if (!strcasecmp(word[1],"-revbuff"))
	{
		rm = user->room;
		for (i = 0; i < NUM_LINES; ++i)
			if (rm->revbuff->conv[i] != NULL) ++cnt;
		if (cnt)
		{
			fprintf(fp,center("-=[ Room review buffer submitted by %s for room %s. ]=-\n",80),user->name,rm->name);
			filewrite_convbuff(fp,rm->revbuff);
			write_user(user,"~FG Submitting the room review buffer to the capture log. Upon completion\n");
			write_user(user,"~FG this buffer will be cleared, in case the harrassive user\n");
			write_user(user,"~FG continues. If which they do, submit the room buffer to the capture log again.\n");
			fprintf(fp,"\n");
			fprintf(fp,mesg_line);
			clear_convbuff(rm->revbuff);
			write_log(1,LOG1,"[CAPTURE] - User: [%s] Type: [revbuff] Room: [%s]\n",user->name,rm->name);
		}
		else write_user(user,"~FG The review buffer was empty!\n");
		FCLOSE(fp);
		return;
	}
	if (!strcasecmp(word[1],"-shouts"))
	{
		for (i = 0; i < NUM_LINES; ++i)
			if (shoutbuff->conv[i] != NULL) ++cnt;
		if (cnt)
		{
			fprintf(fp,center("-=[ Shout buffer submitted by %s. ]=-\n",80),user->name);
			filewrite_convbuff(fp,shoutbuff);
			write_user(user,"~FG Submitting the shout buffer to the capture log. Upon completion\n");
			write_user(user,"~FG the shout buffer will be cleared, in case the harrassive user\n");
			write_user(user,"~FG continues. If which they do, submit the shout buffer to the capture log again.\n");
			fprintf(fp,"\n");
			fprintf(fp,mesg_line);
			write_log(1,LOG1,"[CAPTURE] - User: [%s] Type: [shout buffer]\n",user->name);
			clear_convbuff(shoutbuff);
		}
		else write_user(user,"~FG The shout buffer was empty.\n");
		FCLOSE(fp);
		return;
	}
	if (!strcasecmp(word[1],"-smail"))
	{
		if (word_count < 3)
		{
			write_user(user,"~FG You must specify a message number!\n");
			FCLOSE(fp);
			return;
		}
		if (!(amount = count_mess(user,1)))
		{
			write_user(user,"~FG You don't even have any s-mail.\n");
			FCLOSE(fp);
			return;
		}
		msg_num = atoi(word[2]);
		if (msg_num == 0)
		{
			write_user(user,"~FG You must specify a message number.\n");
			FCLOSE(fp);
			return;
		}
		if (amount < msg_num)
		{
			write_user(user,"~FG You don't even have that many s-mail messages!\n");
			FCLOSE(fp);
			return;
		}
		sprintf(mailfile,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		if ((mfp = fopen(mailfile,"r")) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred, please try again in a couple minutes.\n");
			write_log(0,LOG1,"[ERROR] Unable to open '%s's' s-mail box in capture()!\n",user->name);
			FCLOSE(fp);
			return;
		}
		write_user(user,"~FG Submitting the s-mail message that you specified to the capture log.\n");
		fprintf(fp,center("-=[ S-mail message submitted by %s. ]=-\n",80),user->name);
		valid = cnt = 1;
		fgets(line2,ARR_SIZE-1,mfp);
		while(!feof(mfp))
		{
			if (*line2 == '\n') valid = 1;
			sscanf(line2,"%s",check);
			if (valid && (!strcmp(check,"~FYWriter~CB:")
			  || !strcmp(check,"Writer:")
			  || !strcmp(check," ~FYWriter~CB:")))
			{
				if (msg_num == cnt)
				{
					while(*line2 != '\n')
					{
						fputs(ColourStrip(line2),fp);
						fgets(line2,ARR_SIZE-1,mfp);
					}
				}
				valid = 0; cnt++;
			}
			fgets(line2,ARR_SIZE-1,mfp);
		}
		FCLOSE(mfp);
		fprintf(fp,"\n");
		fprintf(fp,mesg_line);
		FCLOSE(fp);
		write_log(1,LOG1,"[CAPTURE] - User: [%s] Type: [s-mail message]\n",user->name);
		return;
	}
	writeus(user,usage,command[com_num].name,"~CB[~CR-tells~CW|~CR-revbuff~CW|~CR-shouts~CB]");
	writeus(user,usage,command[com_num].name,"~CB[~CR-smail #~CB]");
	write_user(user,"~CW - ~FGUpon capturing something it is sent to a capture log, where the staff can see\n");
	write_user(user,"~CW - ~FGwhat has happened. For more info ~CW'~CR.help capture~CW'\n");
	return;
	FCLOSE(fp);
}

/*
   This function renames the system logs and puts them in their appropriate
   directories. Did it this way, because I'm starting to collect up to
   many logfiles, and it looks awful in the check_messages() function.
*/
void rename_logfiles(void)
{
	char oldfile[FSL],newfile[FSL];
	sprintf(oldfile,"%s/%s",SysLogDir,SysLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,SysLogs,SysLog,tmonth+1,tmday);
	rename(oldfile,newfile);
	write_log(1,LOG1,"SysLog renamed to: [%s.%02d%02d]\n",SysLog,tmonth+1,tmday);
	sprintf(oldfile,"%s/%s",SysLogDir,LoginLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,LoginLogs,LoginLog,tmonth+1,tmday);
	rename(oldfile,newfile);
	write_log(1,LOG2,"LoginLog renamed to: [%s.%02d%02d]\n",LoginLog,tmonth+1,tmday);
	sprintf(oldfile,"%s/%s",SysLogDir,EmailLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,EmailLogs,EmailLog,tmonth+1,tmday);
	rename(oldfile,newfile);
	write_log(1,LOG3,"EmailLog renamed to: [%s.%02d%02d]\n",EmailLog,tmonth+1,tmday);
	sprintf(oldfile,"%s/%s",SysLogDir,TelnetLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,TelnetLogs,TelnetLog,tmonth+1,tmday);
	rename(oldfile,newfile);
	write_log(1,LOG4,"TelnetLog renamed to: [%s.%02d%02d]\n",TelnetLog,tmonth+1,tmday);
	sprintf(oldfile,"%s/%s",SysLogDir,StaffLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,StaffLogs,StaffLog,tmonth+1,tmday);
	rename(oldfile,newfile);
	write_log(1,LOG4,"StaffLog renamed to: [%s.%02d%02d]\n",StaffLog,tmonth+1,tmday);
	sprintf(oldfile,"%s/%s",SysLogDir,WWWLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,WWWLogs,WWWLog,tmonth+1,tmday);
	rename(oldfile,newfile);
	write_log(1,LOG5,"WWWLog renamed to: [%s.%02d%02d]\n",WWWLog,tmonth+1,tmday);
	sprintf(oldfile,"%s/%s",SysLogDir,CapLog);
	sprintf(newfile,"%s/%s/%s.%02d%02d",SysLogDir,CapLogs,CapLog,tmonth+1,tmday);
	if (file_exists(oldfile))
	{
		write_log(1,LOG1,"CaptureLog renamed to: [%s.%02d%02d]\n",CapLog,tmonth+1,tmday);
		rename(oldfile,newfile);
	}
	sprintf(oldfile,"%s/%s",MiscDir,BirthDays);
	unlink(oldfile); /* Birthday's are finished.. */
	return;
}

/*
   This allows a user to be protected from being deleted if the .delold
   command is used.
*/
void protect(User user,char *inpstr,int rt)
{
	User u;
	int on = 0,err = 0;
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB]");
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) == NULL)	/* Not logged on */
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW-~FT An error occurred, I was unable to perform your request.\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in give_stuff()!\n");
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
			write_user(user,"~CW-~FT It appears as tho that user's userfile is corrupted..\n");
			write_log(1,LOG1,"[ERROR] - Corrupt userfile found: [%s]\n",u->name);
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
	}
	else on = 1;
	if (u == user)
	{
		if (user->delprotect == 0)
		{
			write_user(user,"~FG Alrighty, you are now protected from deletion!\n");
			user->delprotect = 1;
			return;
		}
		write_user(user,"~FG Alright, you are no longer protected from deletion!\n");
		user->delprotect = 0;
		return;
	}
	if (u->delprotect)
	{
		write_log(1,LOG1,"[Un-Protect] - User: [%s] Unprotected: [%s]\n",user->name,u->name);
		u->delprotect = 0;
		writeus(user,"~FG You unprotect ~FT%s ~FGfrom deletion.\n",u->recap);
		sprintf(text,"~FG You have been unprotected from deletion.\n");
		if (on == 0)
		{
			send_oneline_mail(NULL,u->name,text);
			SaveUser(u,0);
		}
		else write_user(u,text);
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	write_log(1,LOG1,"[Protect] - User: [%s] Protected: [%s]\n",user->name,u->name);
	u->delprotect = 1;
	writeus(user,"~FM You protect ~FT%s ~FMfrom deletion.\n",u->recap);
	sprintf(text,"~FM You have been protected from deletion.\n");
	if (on == 0)
	{
		send_oneline_mail(NULL,u->name,text);
		SaveUser(u,0);
	}
	else write_user(u,text);
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
	return;
}

/*
   Lets a user view the called upon log file. Decided to lump it all into one
   command, for easier access, and all that sort of fun stuff.
*/
void view_logs(User user,char *inpstr,int rt)
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
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CR-debug   ~CB[~CRPID #~CB]");
		writeus(user,usage,command[com_num].name,"~CR-sys     ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-staff   ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-login   ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-email   ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-www     ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-capture ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-telnet  ~CB[<~CRdate~CB>]");
		writeus(user,usage,command[com_num].name,"~CR-backup");
		write_user(user,"~CW - ~FT The ~CB[<~CRdate~CB>]~FT is in the format MMDD\n");
		write_user(user,"~CW - ~FT For example~CB: ~CR.view -syslog ~CT0223\n");
		return;
	}
	if (!strncasecmp(word[1],"-debug",2) && word_count<3)
	{
		writeus(user,usage,command[com_num].name,"~CR-debug ~CB[~CRPID #~CB]");
		write_user(user,"~CW - ~FTWhere PID is the process number when the talker last crashed.\n");
		return;
	}
	if (word_count<3)		/* date not specifed */
	{
		if (!strncasecmp(word[1],"-sys",3))
			sprintf(filename,"%s/%s",SysLogDir,SysLog);
		else if (!strncasecmp(word[1],"-staff",3))
			sprintf(filename,"%s/%s",SysLogDir,StaffLog);
		else if (!strncasecmp(word[1],"-login",2))
			sprintf(filename,"%s/%s",SysLogDir,LoginLog);
		else if (!strncasecmp(word[1],"-email",2))
			sprintf(filename,"%s/%s",SysLogDir,EmailLog);
		else if (!strncasecmp(word[1],"-telnet",2))
			sprintf(filename,"%s/%s",SysLogDir,TelnetLog);
		else if (!strncasecmp(word[1],"-www",2)) 
			sprintf(filename,"%s/%s",SysLogDir,WWWLog);
		else if (!strncasecmp(word[1],"-capture",2))
			sprintf(filename,"%s/%s",SysLogDir,CapLog);
		else if (!strncasecmp(word[1],"-backup,",2))
			sprintf(filename,"%s/Backuplog",SysLogDir);
		else
		{
			write_user(user,"~FG You have entered an invalid log file.\n");
			return;
		}
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_line);
		sprintf(text,"~FT-~CT=~CB> ~FYViewing the %slog file... ~CB<~CT=~FT-\n",word[1]);
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_line);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,center(emp,80));
			  return;
			case 1:
			  user->misc_op=2;
		}
		return;
	}
	if (!strncasecmp(word[1],"-sys",3))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,SysLogs,SysLog,word[2]);
	else if (!strncasecmp(word[1],"-staff",3))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,StaffLogs,StaffLog,word[2]);
	else if (!strncasecmp(word[1],"-login",2))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,LoginLogs,LoginLog,word[2]);
	else if (!strncasecmp(word[1],"-email",2))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,EmailLogs,EmailLog,word[2]);
	else if (!strncasecmp(word[1],"-telnet",2))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,TelnetLogs,TelnetLog,word[2]);
	else if (!strncasecmp(word[1],"-www",2))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,WWWLogs,WWWLog,word[2]);
	else if (!strncasecmp(word[1],"-capture",2))
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,CapLogs,CapLog,word[2]);
	else if (!strncasecmp(word[1],"-debug",2))
	{
		sprintf(filename,"%s/%s/%s.%s",SysLogDir,DebugLogs,DebugLog,word[2]);
	}
	else
	{
		write_user(user,"~FG You have entered an invalid log file.\n");
		return;
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_line);
	if (!strncasecmp(word[1],"-debug",2))
	{
		sprintf(text,"~FT-~CT=~CB> ~FYViewing the debuglog for PID %s.. ~CB<~CT=~FT-\n",word[2]);
		write_user(user,center(text,80));
	}
	else
	{
		sprintf(text,"~FT-~CT=~CB> ~FYViewing the %slog file for date %s.. ~CB<~CT=~FT-\n",word[1],word[2]);
		write_user(user,center(text,80));
	}
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_line);
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  write_user(user,center(emp,80));
		  return;
		case 1:
		  user->misc_op=2;
	}
	return;
}

/* Gets a random social name. */
char *get_social(void)
{
	Socials soc;
	int tot,cnt,i;
	static char socexec[21];

	tot = cnt = i = 0;	socexec[0] = '\0';
	soc = first_social;
	while(soc != NULL)
	{
		++tot;
		soc = soc->next;
	}
	cnt = ran(tot);
	sprintf(text,"hug");
	soc = first_social;
	while (soc != NULL)
	{
		if (i == cnt)
		{
			strcpy(socexec,soc->name);
			return(socexec);
		}
		++i;
		soc = soc->next;
	}
	return(text);
}

/*
   Executes a random social. If a user is given, executes it on that given
   user. Might be interesting to try a .execall .randsoc  ;-)
*/
void random_social(User user,char *str,int rt)
{
	char inpstr[ARR_SIZE],*socword;

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
	socword = get_social();
	if (word_count < 2)
	{
		sprintf(inpstr,"%s",socword);
		clear_words();
		word_count=wordfind(inpstr);
		exec_com(user,inpstr);
		return;
	}
	sprintf(inpstr,"%s %s",socword,word[1]);
	clear_words();
	word_count = wordfind(inpstr);
	exec_com(user,inpstr);
	return;
}

/*
   Makes a clone execute something, I took out the clone_say, and clone_emote
   functions because well, this command will make the clone execute ANYTHING
   so you can make it emote with .cexec [room clone is in] ;hrms.
*/
void clone_exec(User user,char *inpstr,int rt)
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
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRroom clone is in~CB] [~CRcommand to execute~CB] [~CRarguments..~CB]");
		return;
	}
	if ((rm = get_room(word[1])) == NULL)
	{
		write_user(user,center(nosuchroom,80));
		return;
	}
	for_users(u)
	{
		if (u->type == CloneType && u->room == rm && u->owner == user)
		{
			inpstr = remove_first(inpstr);
			clear_words();
			word_count = wordfind(inpstr);
			exec_com(u,inpstr);
			return;
		}
	}
	write_user(user,"~FT You don't have a clone in that room.\n");
}

/*
   This function lists out the users macros if word_count<2 otherwise, it
   searches to see if it is an actual macro being used.
*/
void macros(User user,char *inpstr,int rt)
{
	int macnum = 0,i = 0;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (is_clone(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (user->tpromoted && user->orig_lev<POWER)
	{
		write_user(user,"~FG Sorry, temp promoted users can't set macros.\n");
		return;
	}
	if (word_count<2)
	{
		write_user(user,"~FG You have the following macros~CB: \n");
		for (i = 0; i < 16; ++i)
		{
			writeus(user,"~CB[~FT%2d~CB]~CW -~RS %s ~CW= ~RS%s\n",i,user->fmacros[i],user->rmacros[i]);
		}
		return;
	}
	macnum = atoi(word[1]);
	if (macnum > 15)
	{
		write_user(user,"~FG You can't have that many macros.\n");
		return;
	}
	if (macnum < 0)
	{
		write_user(user,"~FG The macro number has to be between 0 and 15.\n");
		return;
	}
	if (!strcasecmp(word[2],"-delete"))
	{
		*user->fmacros[macnum] = '\0';
		*user->rmacros[macnum] = '\0';
		sprintf(text,"Unset%d",macnum);
		strncpy(user->fmacros[macnum],text,sizeof(user->fmacros[macnum])-1);
		strncpy(user->rmacros[macnum],text,sizeof(user->fmacros[macnum])-1);
		writeus(user,"~FG Macro ~FT%d~FG has been deleted.\n",macnum);
		for (i = 0; i < 16; ++i)
		{
			if (user->fmacros[i][0] != '\0') return;
		}
		user->has_macros = 0;
		return;
	}
	if (word_count < 5)
	{
		write_user(user,"~FG Missing something, see ~CW'~CR.help macros~CW'\n");
		return;
	}
	if (strlen(word[2]) > FindLen)
	{
		write_user(user,"~FG The pattern you want to search for is too long.\n");
		return;
	}
	for (i = 0; i < 16; ++i)
	{
		if (!strcasecmp(word[2],user->fmacros[i]))
		{
			writeus(user,"~FG Unable to save macro ~CW'~CR%s~CW'~FG conflicts with another macro.\n",word[2]);
			return;
		}
	}
	if (strcmp(word[3],"="))
	{
		write_user(user,"~FG Missing a character, see ~CW'~CR.help macros~CW'\n");
		return;
	}
	inpstr = remove_first(inpstr);
	inpstr = remove_first(inpstr);
	inpstr = remove_first(inpstr);
	if (strlen(inpstr)>ReplaceLen)
	{
		write_user(user,"~FG The pattern you want to replace is to long.\n");
		return;
	}
	*user->fmacros[macnum] = '\0';
	*user->rmacros[macnum] = '\0';
	user->has_macros	 = 1;
	strcpy(user->fmacros[macnum],word[2]);
	strcpy(user->rmacros[macnum],inpstr);
	write_user(user,"~CW-~FT Alright, your macro has been set.\n");
	return;
}

/* Lets a user set their alarm, so they will be alerted when it goes off! */
void user_alarm(User user,char *inpstr,int rt)
{
	int stime,htime,mtime;

	stime = htime = mtime = 0;
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
		if (user->alarm_time <= 0)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRhours minutes seconds~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CR-reset~CB]");
			return;
		}
		if (user->ansi_on)
		{
			write_user(user,"~FMÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»\n");
			write_user(user,"~FMº                                      º\n");
			write_user(user,"~FMº                                      º\n");
			write_user(user,"~FMº~FY   °±²ÛÛ²±°                °±²ÛÛ²±°   ~FMº\n");
			write_user(user,"~FMº~CBÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿~FMº\n");
			write_user(user,"~FMº~CB³~CR              Your                  ~CB³~FMº\n");
			write_user(user,"~FMº~CB³~CR     ÖÄ¿  ·    ÖÄ¿  ÒÄ¿  ÖÄÂÄ¿      ~CB³~FMº\n");
			write_user(user,"~FMº~CB³~CR     ÇÄ´  º    ÇÄ´  ÇÂÙ  º ³ ³      ~CB³~FMº\n");
			write_user(user,"~FMº~CB³~CR     ½ À  ÓÄÙ  ½ À  Ð \\  Ð   Á      ~CB³~FMº\n");
			write_user(user,"~FMº~CB³~CR       is set to go off in:         ~CB³~FMº\n");
			write_user(user,"~FMº~CBÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ~FMº\n");
			write_user(user,"~FMÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼\n");
			word_time(user,user->alarm_time);
			write_user(user,"~FG To reset it type, ~CW'~CR.alarm -reset~CW'\n");
			return;
		}
		else
		{
			write_user(user,"~FM.--------------------------------------.\n");
			write_user(user,"~FM|                                      |\n");
			write_user(user,"~FM|                                      |\n");
			write_user(user,"~FM|~FY   /------\\                /------\\   ~FM|\n");
			write_user(user,"~FM|~CB.------------------------------------.~FM|\n");
			write_user(user,"~FM|~CB|~CR              Your                  ~CB|~FM|\n");
			write_user(user,"~FM|~CB|~CR     .-.  .    .-.  .-.  .-.-.      ~CB|~FM|\n");
			write_user(user,"~FM|~CB|~CR     |-|  |    |-|  |_|  | | |      ~CB|~FM|\n");
			write_user(user,"~FM|~CB|~CR     | |  |_.  | |  | \\  | ` |      ~CB|~FM|\n");
			write_user(user,"~FM|~CB|~CR        is set to go off in:        ~CB|~FM|\n");
			write_user(user,"~FM|~CB`------------------------------------'~FM|\n");
			write_user(user,"~FM`--------------------------------------'\n");
			word_time(user,user->alarm_time);
			write_user(user,"~FG To reset it type, ~CW'~CR.alarm -reset~CW'\n");
			return;
		}
	}
	if (!strcasecmp(word[1],"-reset"))
	{
		if (user->alarm_time <= 0)
		{
			write_user(user,"~FG Your alarm isn't even set.\n");
			return;
		}
		user->alarm_time = 0;
		user->alarm_set = 0;
		write_user(user,"~FG Your alarm has been reset!\n");
		return;
	}
	if (user->alarm_time)
	{
		write_user(user,"~FG Your alarm is already set.. use ~CW'~CR.alarm~CW' ~FGto see!\n");
		return;
	}
	htime = atoi(word[1]);
	mtime = atoi(word[2]);
	stime = atoi(word[3]);
	if (htime == 0 && mtime == 0 && stime == 0)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRhours minutes seconds~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CR-reset~CB]");
		return;
	}
	if ((htime<0) || (htime == 0 && mtime<0)
	  || (htime == 0 && mtime == 0 && stime<10))
	{
		write_user(user,"~FG Illegal time specification! See ~CW'~CR.help alarm~CW'\n");
		return;
	}
	if (stime == 0 && mtime == 0)
	{
		user->alarm_time = (htime*3600);
		write_user(user,"~FG You set your alarm to go off in~CB:");
		word_time(user,user->alarm_time);
		user->alarm_set = 1;
		return;
	}
	else if (stime == 0)
	{
		user->alarm_time = (htime*3600);
		user->alarm_time += (mtime*60);
		write_user(user,"~FG You set your alarm to go off in~CB:");
		word_time(user,user->alarm_time);
		user->alarm_set = 1;
		return;
	}
	else
	{
		user->alarm_time = (htime*3600);
		user->alarm_time += (mtime*60);
		user->alarm_time += stime;
		user->alarm_set = 1;
		write_user(user,"~FG You set your alarm to go off in~CB:");
		word_time(user,user->alarm_time);
		return;
	}
	return;
}

/* When user->alarm_time==0 this will sound it for them ;-) */
void sound_alarm(User user)
{

	if (user->ansi_on)
	{
		write_user(user,"\07");
		write_user(user,"~FMÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»\n");
		write_user(user,"~FMº                                      º\n");
		write_user(user,"~FMº                                      º\n");
		write_user(user,"~FMº~FY   °±²ÛÛ²±°                °±²ÛÛ²±°   ~FMº\n");
		write_user(user,"~FMº~CBÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿~FMº\n");
		write_user(user,"~FMº~CB³~CR              Your                  ~CB³~FMº\n");
		write_user(user,"~FMº~CB³~CR     ÖÄ¿  ·    ÖÄ¿  ÒÄ¿  ÖÄÂÄ¿      ~CB³~FMº\n");
		write_user(user,"~FMº~CB³~CR     ÇÄ´  º    ÇÄ´  ÇÂÙ  º ³ ³      ~CB³~FMº\n");
		write_user(user,"~FMº~CB³~CR     ½ À  ÓÄÙ  ½ À  Ð \\  Ð   Á      ~CB³~FMº\n");
		write_user(user,"~FMº~CB³~CR           has gone off             ~CB³~FMº\n");
		write_user(user,"~FMº~CBÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ~FMº\n");
		write_user(user,"~FMÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼\n");
		write_user(user,"\07");
		user->alarm_set = 0;
		return;
	}
	else
	{
		write_user(user,"\07");
		write_user(user,"~FM.--------------------------------------.\n");
		write_user(user,"~FM|                                      |\n");
		write_user(user,"~FM|                                      |\n");
		write_user(user,"~FM|~FY   /------\\                /------\\   ~FM|\n");
		write_user(user,"~FM|~CB.------------------------------------.~FM|\n");
		write_user(user,"~FM|~CB|~CR              Your                  ~CB|~FM|\n");
		write_user(user,"~FM|~CB|~CR     .-.  .    .-.  .-.  .-.-.      ~CB|~FM|\n");
		write_user(user,"~FM|~CB|~CR     |-|  |    |-|  |_|  | | |      ~CB|~FM|\n");
		write_user(user,"~FM|~CB|~CR     | |  |_.  | |  | \\  | ` |      ~CB|~FM|\n");
		write_user(user,"~FM|~CB|~CR           has gone off             ~CB|~FM|\n");
		write_user(user,"~FM|~CB`------------------------------------'~FM|\n");
		write_user(user,"~FM`--------------------------------------'\n");
		user->alarm_set = 0;
		write_user(user,"\07");
		return;
	}
}

/* Takes time_conv, and converts it to how many days/hours/minutes/seconds. */
void word_time(User user,long time_conv)
{
	int secs,mins,hours,days;

	secs = mins = hours = days = 0;
	days = time_conv/86400;
	hours = (time_conv%86400)/3600;
	mins = (time_conv%3600)/60;
	secs = time_conv%60;
	if (days == 0 && hours == 0 && mins == 0) sprintf(text," ~CY%d ~FGsecond%s.\n",secs,secs>1?"s":"");
	else if (days == 0 && hours == 0) sprintf(text," ~CY%d ~FGminute%s, and ~CY%d~FG second%s.\n",mins,mins>1?"s":"",secs,secs>1?"s":"");
	else if (days == 0) sprintf(text," ~CY%d ~FGhour%s, ~CY%d ~FGminute%s, and ~CY%d~FG second%s.\n",hours,hours>1?"s":"",mins,mins>1?"s":"",secs,secs>1?"s":"");
	else sprintf(text," ~CY%d ~FGday%s,~CY %d ~FGhour%s, ~CY%d~FG minute%s, and ~CY%d ~FGsecond%s.\n",days,days>1?"s":"",hours,hours>1?"s":"",mins,mins>1?"s":"",secs,secs>1?"s":"");
	write_user(user,text);
}

/*
   Shows a users gaming statistics.
*/
void game_stats(User user,char *inpstr,int rt)
{
	User u;
	char *name,*uname;
	int on = 0,err = 0,i,tgames[7];
	float percent[7];

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2)
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
				write_user(user,"~CW-~FT An error occurred, try again in a little while.\n");
				write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in game_stats()\n");
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
				write_log(1,LOG1,"[ERROR] - Corrupt userfile found: [%s]\n",u->name);
				write_user(user,"~CW-~FT It appears as tho that user's userfile is currupted.\n");
				destruct_user(u);
				destructed = 0;
				return;
			}
			on = 0;
		}
		else on = 1;
	}
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
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_line);
	sprintf(text,"~FT-~CT=~CB> ~FGGaming statistics for~CB: ~CT%s ~CB<~CT=~FT-\n",name);
	write_user(user,center(text,80));
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_line);
	for (i = 0; i < 7; ++i)
	{
		percent[i] = 0;
		tgames[i] = 0;
	}
	tgames[0] = (u->bship_wins+u->bship_lose);
	tgames[1] = (u->chess_wins+u->chess_lose+u->chess_draws);
	tgames[2] = (u->c4_win+u->c4_lose+u->myint3);
	tgames[3] = (u->tic_win+u->tic_lose+u->tic_draw);
	tgames[4] = (u->win+u->lose+u->draw);
	tgames[5] = (u->hang_wins+u->hang_loses);
	tgames[6] = (u->myint1+u->myint2);

	if (tgames[0] != 0)
		percent[0] = (float)((int)u->bship_wins*100)/(int)tgames[0];
	else percent[0] = 0;

	if (tgames[1] != 0)
		percent[1] = (float)((int)u->chess_wins*100)/(int)tgames[1];
	else percent[1] = 0;

	if (tgames[2] != 0)
		percent[2] = (float)((int)u->c4_win*100)/(int)tgames[2];
	else percent[2] = 0;

	if (tgames[3] != 0)
		percent[3] = (float)((int)u->tic_win*100)/(int)tgames[3];
	else percent[3] = 0;

	if (tgames[4] != 0)
		percent[4] = (float)((int)u->win*100)/(int)tgames[4];
	else percent[4] = 0;

	if (tgames[5] != 0)
		percent[5] = (float)((int)u->hang_wins*100)/(int)tgames[5];
	else percent[5] = 0;

	if (tgames[6] != 0)
		percent[6] = (float)((int)u->myint1*100)/(int)tgames[6];
	else percent[6] = 0;

	write_user(user,"\n~FY   TicTacToe Stats~CB:~FY   Chess Stats~CB:~FY       Connect 4 Stats~CB:~FY   Fighting Stats~CB:\n");
	write_user(user,"~FM   ----------------   ----------------   ----------------   ----------------\n");
	writeus(user,"~FG   Has won~CB:~CT    %4d~FT   Has won~CB:~CT    %4d~FG   Has won~CB:~CT    %4d~FT   Has won~CB:~CT    %4d\n",u->tic_win,u->chess_wins,u->c4_win,u->win);
	writeus(user,"~FG   Has lost~CB:~CT   %4d~FT   Has lost~CB:~CT   %4d~FG   Has lost~CB:~CT   %4d~FT   Has lost~CB:~CT   %4d\n",u->tic_lose,u->chess_lose,u->c4_lose,u->lose);
	writeus(user,"~FG   Has tied~CB:~CT   %4d~FT   Has tied~CB:~CT   %4d~FG   Has tied~CB:~CT   %4d~FT   Has tied~CB:~CT   %4d\n",u->tic_draw,u->chess_draws,u->myint3,u->draw);
	write_user(user,"\n");
	writeus(user,"~FG     Wins ~CT%3.0f~FR%~FT            Wins ~CT%3.0f~FR%~FG           Wins ~CT%3.0f~FR%~FT          Wins ~CT%3.0f~FR%\n",percent[3],percent[1],percent[2],percent[4]);
	write_user(user,"\n");
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	write_user(user,"\n~FY         Battleship Stats~CB:     ~FYCheckers Stats~CB:~FY      Hangman Stats~CB:\n");
	write_user(user,"~FM         -----------------     ----------------     ----------------\n");
	writeus(user,"~FG         Has won~CB:     ~CT%4d     ~FTHas won~CB:    ~CT%4d     ~FGHas won~CB:    ~CT%4d\n",u->bship_wins,u->myint1,u->hang_wins);
	writeus(user,"~FG         Has lost~CB:    ~CT%4d     ~FTHas lost~CB:   ~CT%4d     ~FGHas lost~CB:   ~CT%4d\n",u->bship_lose,u->myint2,u->hang_loses);
	write_user(user,"\n");
	writeus(user,"~FG             Wins ~CT%3.0f~FR%~FT             Wins ~CT%3.0f~FR%~FG            Wins ~CT%3.0f~FR%\n",percent[0],percent[6],percent[5]);
	write_user(user,"\n");
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
}

/*
   Users can use this by calling the function 1 of three ways:
   .guess -play (initialises the game)
   .guess -quit (lets the user abort the game)
   .guess #     (lets them guess the number.)
*/
void guess_it(User user,char *inpstr,int rt)
{
	int guess=0;
	if (word_count < 2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CR-play~CW|~CR-quit~CB]");
		writeus(user,usage,command[com_num].name,"~CB[~CRnumber to guess~CB]");
		return;
	} 
	if (!strcasecmp(word[1],"-play"))
	{
		if (user->playing)
		{
			write_user(user,"~FG Silly, you're already playing the game.\n");
			write_user(user,"~FG Use ~CW'~CR.guess ~CB[~CRnumber~CB]'~FG to guess a number.\n");
			return;
		}
		user->playing = 1;
		user->chances = 3;
		user->tries = 0;
		user->guess_num = ran(10);
		writeus(user,"~CB - ~FTWelcome to GuessIT ~FT%s.\n",user->recap);
		writeus(user,"~CB - ~FTYou have ~FG%d~FT chances to guess the number.\n",user->chances);
		write_user(user,"~CB - ~FTTo guess the number, use ~CW'~CR.guess ~CB[~CRnumber~CB]~CW'\n");
		return;
	}
	if (!strcasecmp(word[1],"-quit"))
	{
		write_user(user,"~FG Ok, fine, we don't want you to play this game anymore.\n");
		user->tries = user->playing = user->guess_num = 0;
		user->chances = 0;
		return;
	}
	guess = atoi(word[1]);
	if (guess == 0)
	{
		write_user(user,"~FG You actually need to take a guess..\n");
		return;
	}
	if (guess < 1 || guess > 10)
	{
		write_user(user,"~FG The number is between 1 and 10.\n");
		return;
	}
	user->tries++;
	if (guess < user->guess_num)
	{
		write_user(user,"~CW -~FT Nope, that ain't it, it's higher then that.\n");
		user->chances--;
		if (user->chances == 0)
		{
			write_user(user,"~CW - ~FTCrappy deal, you didn't guess the number.\n");
			user->tries = user->playing = 0;
			user->guess_num = user->chances = 0;
		}
		else
		{
			writeus(user,"~CW -~FT You have ~FG%d~FT chances left.\n",user->chances);
		}
		return;
	}
	else if (guess > user->guess_num)
	{
		write_user(user,"~CW - ~FTNope, that ain't it, it's lower then that.\n");
		user->chances--;
		if (user->chances == 0)
		{
			write_user(user,"~CW -~FT Crappy deal, you didn't guess the number.\n");
			user->tries = user->playing = 0;
			user->guess_num = user->chances = 0;
		}
		else
		{
			writeus(user,"~CW -~FT You have ~FG%d~FT chances left.\n",user->chances);
		}
		return;
	}
	else if (guess == user->guess_num)
	{
		if (user->tries == 1)
		{
			write_user(user,"~CW -~FT Wicked deal, it only took you one try to guess the number.\n");
		}
		else
		{
			writeus(user,"~CW -~FT It took you ~FG%d ~FTtries to guess the number.\n",user->tries);
			write_user(user,"~CW -~FT But hey, at least you guessed it right. :-)\n");
		}
		user->tries = user->playing = 0;
		user->guess_num = user->chances = 0;
		return;
	}
	writeus(user,usage,command[com_num].name,"~CB[~CR-play~CW|~CR-quit~CB]");
	writeus(user,usage,command[com_num].name,"~CB[~CRnumber to guess~CB]");
	return;
} 

/* This function directs an emote at another user. */
void to_emote(User user,char *inpstr,int rt)
{
	Room rm;
	User u;
	char *rname,*name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
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
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CRmessage~CB]");
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
		write_user(user,"~FG Why would you want to direct and emote to yourself?\n");
		return;
	}
	switch(user->speech)
	{
		case 1:
		  alter_speech(inpstr,1);
		  break;
		case 2:
		  alter_speech(inpstr,2);
		  break;
		case 3:
		  alter_speech(inpstr,3);
		  break;
		case 4:
		  alter_speech(inpstr,4);
		  break;
        }
	if (u->invis == Invis) rname = invisname;  else rname = u->recap;
	if (u->afk)
	{
		if (u->afk_mesg[0]) writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard~CB:~FT %s\n",rname,u->afk_mesg);
		else writeus(user,"~FT%s~FG is ~CW[~CRA~CW]~FGway ~CW[~CRF~CW]~FGrom ~CW[~CRK~CW]~FGeyboard at the moment!\n",rname);
		write_user(user,"~FGIf you would like to leave them a message ~CW'~FR.tell~CW'~FG them!\n");
		return;
	}
	if (u->editor.editing)
	{
		writeus(user,"~FT%s~FG is using the editor at the moment...\n",u->recap);
		write_user(user,"~FGIf you would like to leave them a message ~CW'~FR.tell~CW'~FG them!\n");
		return;
	}
	rm = user->room;
	if (rm != u->room)
	{
		writeus(user,"~FT %s ~FGisn't in the same room as you.\n",u->recap);
		return;
	}
	if (rm->access == FIXED_PUBLIC && ban_swearing
	  && contains_swearing(inpstr))
	{
		write_user(user,center(noswearing,80));
		return;
	}
	if (rm->access == FIXED_PUBLIC && arrest_swearers
	  && contains_swearing(inpstr) && user->level < GOD)
	{
		auto_arrest(user,1);
	        return;
	}
	inpstr = remove_first(inpstr);
	inpstr = replace_macros(user,inpstr);
	if (user->status & Silenced)
	{
		if (strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
		{
			if (!user->hat) sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s~FY%s\n",rname,name,inpstr);
			else sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s%s%s\n",rname,name,hatcolor[user->hat],inpstr);
		}
		else
		{
			if (!user->hat) sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s ~FY%s\n",rname,name,inpstr);
			else sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s %s%s\n",rname,name,hatcolor[user->hat],inpstr);
		}
		if (user->type == CloneType)
			writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
		else
			write_user(user,text);
		return;
	}
	if (strchr("\'",inpstr[0]) || strchr("\'",inpstr[1]))
	{
		if (!user->hat) sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s~FY%s\n",rname,name,inpstr);
		else sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s%s%s\n",rname,name,hatcolor[user->hat],inpstr);
	}
	else
	{
		if (!user->hat) sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s ~FY%s\n",rname,name,inpstr);
		else sprintf(text,"~CB[~FRTo ~FM%s~CB] ~FM%s %s%s\n",rname,name,hatcolor[user->hat],inpstr);
	}
	if (user->type == CloneType)
		writeus(user->owner,"~CB[~FM%s~CB]: %s\n",user->room->recap,text);
	else
		write_user(user,text);
	write_room_except(user->room,user,text);
	record(rm,text);
	if (user->speech == 1)
	{
		alter_speech(inpstr,1);
		WriteRoomExcept(user->room,user,"   ~CB[~FMTranslation~CB]~RS %s\n",inpstr);
	}
}

/*
   Allows a user to view the current system time or check the time by using
   a + or - switch followed by a number.
*/
void what_time(User user,char *inpstr,int rt)
{
	char ttype[5],tstr[80],pm,tchange[10];
	int hour=0,tch,i,hour2;

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (user->tzone[0] == '+' || user->tzone[0] == '-')
	{
		pm = user->tzone[0];
		strcpy(tchange,user->tzone);
		i = 0;
		do
		{
			tchange[i] = tchange[i+1]; ++i;
		} while(tchange[i+1] != '\0');
		tchange[i] = '\0';
		tch = atoi(tchange);
		if (pm == '+') hour2 = (thour+tch);
		else hour2 = (thour-tch);
		if (hour2 > 24) hour2 -= 24;
		if ((int)hour2 >= 12)
		{
	        	if ((int)hour2>12) hour = (int)hour2-12;
			if (hour==0) hour = 12;
			strcpy(ttype,"p.m.");
		}
		else
		{
	        	if ((int)hour2 == 0) hour = 12;
			else hour = (int)hour2;
		        strcpy(ttype,"a.m.");
		}
		writeus(user,"~CW-~FT According to your timezone, the time is~CB: ~FY%d:%02d:%02d %s~FT.\n",hour,tmin,tsec,ttype);
		return;
	}
	else
	{
		if ((int)thour >= 12)
		{
			if ((int)thour > 12) hour = (int)thour-12;
			strcpy(ttype,"p.m.");
		}
		else
		{
			if (!(int)thour) hour = 12;
			else hour = (int)thour; 
			strcpy(ttype,"a.m.");
		}
		sprintf(tstr,"~FG%s %s %d, %d ~CB[~FY%d:%02d:%02d %s~CB]",day[twday],month[tmonth],tmday,tyear,hour,tmin,tsec,ttype);
		writeus(user,"~CW-~FT The system time is~CB: %s\n",tstr);
		return;
	}
}

/* Allows a staff member to change a users total login time. */
void redo_time(User user,char *inpstr,int rt)
{
	User u;
	int temp,days,hours,days2,hours2,mins2,total,err=0;
	char *name,text2[ARR_SIZE],*uname;

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count<3 || !isnumber(word[2]))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CR# of days~CB] [~CR# of hours~CB]");
		write_user(user,"~CW-~CR NOTE~CB: ~FTYou must supply a 0 (zero) for days if less then one day.\n");
		return;
	}
	if (word_count == 4 && (!isnumber(word[2]) || !isnumber(word[3])))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CR# of days~CB] [~CR# of hours~CB]");
		write_user(user,"~CW-~CR NOTE~CB: ~FTYou must supply a 0 (zero) for days if less then one day.\n");
		return;
	}
	if (user->invis == Invis) name = invisname; else name = user->recap;
	days = atoi(word[2]);
	hours = atoi(word[3]);
	total = (days*86400)+(hours*3600);
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)))
	{
		if (u == user)
		{
			write_user(user,"~FTTrying to give yourself time this way is just crazy.\n");
			return;
		}
		temp = u->total_login;
		days2 = u->total_login/86400;
		hours2 = (u->total_login%86400)/3600;
		mins2 = (u->total_login%3600)/60;
		u->total_login = total;
		writeus(user,"\n~CG%s's ~CGtotal log on time has been changed:\n~CMFrom :~CY %d ~CMday%s, ~CY%d ~CMhour%s, & ~CY%d ~CMminute%s.\n~CMTo   :  ~CY%d ~CMday%s, ~CY%d~CM hour%s, & ~CY0 ~CMminutes.\n\n",u->recap,days2,days2>1?"s":"",
		  hours2,hours2>1?"s":"",mins2,mins2>1?"s":"",days,days>1?"s":"",hours,hours>1?"s":"");
		writeus(u,"\n~CG%s ~CGhas changed your total log on time:\n~CMFrom :~CY %d ~CMday%s, ~CY%d ~CMhour%s, & ~CY%d ~CMminute%s.\n~CMTo   :  ~CY%d ~CMday%s, ~CY%d~CM hour%s, & ~CY0 ~CMminutes.\n\n",name,days2,days2>1?"s":"",
		  hours2,hours2>1?"s":"",mins2,mins2>1?"s":"",days,days>1?"s":"",hours,hours>1?"s":"");
		sprintf(text,"%s changed %s's total log on time:\nFrom : %d day%s, %d hour%s, & %d minute%s.\nTo   :  %d day%s, %d hour%s, & 0 minutes.\n\n",user->name,u->name,days2,days2>1?"s":"",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"",
		  days,days>1?"s":"",hours,hours>1?"s":"");
		write_log(1,LOG1,text);
		write_record(u,1,text);
		return;
	}
	/* Users not logged on. */
	if ((u = create_user()) == NULL)
	{
		writeus(user,"~CW-~FT An error occurred, please try again in a little while.\n");
		write_log(0,LOG1,"[ERROR] - Cannot create temporary user object in redo_time()\n");
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
		write_log(1,LOG1,"[ERROR] - Corrupt userfile found: [%s]\n",u->name);
		write_user(user,"~CW-~FT It appears as tho that user's userfile is currupted.\n");
		destruct_user(u);
		destructed = 0;
		return;
	}
	u->socket = -2;
	temp = u->total_login;
	days2 = u->total_login/86400;
	hours2 = (u->total_login%86400)/3600;
	mins2 = (u->total_login%3600)/60;
	u->total_login = total;
	strcpy(u->site,u->last_site);
	SaveUser(u,0);
	sprintf(text,"\n~CG%s's ~CGtotal log on time has been changed:\n~CMFrom :~CY %d ~CMday%s, ~CY%d ~CMhour%s, & ~CY%d ~CMminute%s.\n~CMTo   :  ~CY%d ~CMday%s, ~CY%d~CM hour%s, & ~CY0 ~CMminutes.\n\n",u->recap,days2,days2>1?"s":"",
	  hours2,hours2>1?"s":"",mins2,mins2>1?"s":"",days,days>1?"s":"",hours,hours>1?"s":"");
	write_user(user,text);
	sprintf(text2,"\n~CG%s ~CGhas changed your total log on time:\n~CMFrom :~CY %d ~CMday%s, ~CY%d ~CMhour%s, & ~CY%d ~CMminute%s.\n~CMTo   :  ~CY%d ~CMday%s, ~CY%d~CM hour%s, & ~CY0 ~CMminutes.\n\n",name,days2,days2>1?"s":"",
	  hours2,hours2>1?"s":"",mins2,mins2>1?"s":"",days,days>1?"s":"",hours,hours>1?"s":"");
	send_oneline_mail(NULL,u->name,text2);
	sprintf(text,"%s changed %s's total log on time:\nFrom : %d day%s, %d hour%s, & %d minute%s.\nTo   :  %d day%s, %d hour%s, & 0 minutes.\n\n",user->name,u->name,days2,days2>1?"s":"",hours2,hours2>1?"s":"",mins2,mins2>1?"s":"",
	  days,days>1?"s":"",hours,hours>1?"s":"");
	write_log(1,LOG1,text);
	write_record(u,1,text);
	destruct_user(u);
	destructed = 0;
}

/* Toggle the shouting ban on/off */
void toggle_shoutban(User user,char *inpstr,int rt)
{

	if (is_jailed(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (shout_ban)
	{
		write_user(user,"~CRShouting ban has been turned: ~CTOFF.\n");
		write_log(1,LOG4,"[ShoutBan] - User: [%s] Disabled the shoutban.\n",user->name);
		shout_ban = 0;
		return;
	}
	shout_ban = 1;
	write_user(user,"~CTShouting ban has been turned: ~CRON.\n");
	write_log(1,LOG4,"[ShoutBan] - User: [%s] Enabled the shoutban.\n",user->name);
}

/* END OF MODULE1.C */
