/*
  server.c
    The code for which contains server calls, such as finger a user on 
    a remote system, perform a whois server, etc.

    The source within this file is Copyright 1998 - 2001 by
 	Rob Melhuish, Mike Irving, and Andrew Collington.  Patches
	submitted by Cygnus (maintainer of ncohafmuta)

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
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#define _SERVER_C
  #include "include/needed.h"
#undef _SERVER_C

/* forks a system call into the process, (helps get rid of zombie processes) */
int double_fork(void)
{
	pid_t pid;
	int status;

	if (!(pid = fork()))
	{
		switch(fork())
		{
			case  0:
			  return(0);
			case -1:
			  _exit(-1);
			default:
			  _exit(0);
		}
	}
	if (pid<0 || waitpid(pid,&status,0)<0) return(-1);
	if (WIFEXITED(status))
	{
		if (!WEXITSTATUS(status)) return(-1);
		else errno = WEXITSTATUS(status);
	}
	else errno = EINTR;
	return(-1);
}

/* Sends the email */
int send_email(char *addy,char *mail_file)
{
	char *home;

	home = getenv("HOME");
	if (home == NULL) return(0);
	switch(double_fork())
	{
		case -1:
		  return(0);        /* Didn't work */
		case 0 :
		  sprintf(text,"%s %s < %s",SendMailExec,addy,mail_file);
		  system(text);
		  unlink(mail_file);
		  _exit(1);
		  return(1);
		default:
		  break;
	}
	return(1);
}

/*
   This is the function that creates the talker back up... It used to be a
   zip file, but I've found zip is inefficient and tar and gzip work much
   better and don't take as long to compress. The talker backup is done
   every 24 hours..
*/
int backup_files(void)
{
	char *home;
	char temp[256];

	home = getenv("HOME");
	switch(double_fork())
	{
		case -1:
		  return(0);	/* Failed to fork */
		case 0 :
		  write_log(1,LOG1,"[RaMTITS] - Talker files backed up.\n");
		  write_log(1,LOG1,"[RaMTITS] - For more backup info see ~CW'~CR.view -backuplog~CW'\n");
		  sprintf(text,"/bin/bash ./backup");
		  system(text);
		  sprintf(temp,"~CW[~CRRaMTITS~CW]~RS General maintenance completed.\n");
		  write_room(NULL,temp);
		  _exit(1);
		  return(1); /* We shouldn't even get this far */
	}
	return(0);
}

/* Uses the system's finger command */
void uni_inger(User user,char *inpstr,int rt)
{
	char filename[FSL];

	if (rt == 0)
	{
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
		if (word_count<2 || strlen(inpstr)<3)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CW<~CR@host~CW>~CB]");
			return;
		}
		if (word[1][0] == '-')
		{
			write_user(user,"~FG That didn't work..\n");
			return;
		}
		if (strpbrk(inpstr,"<>%^&*()$#!;") && user->level<OWNER)
		{
			write_user(user,"~FG That didn't work..\n");
			return;
		}
		if (strlen(inpstr)>SiteNameLen)
		{
			write_user(user,"~CB - ~~FMThat addy's a tad bit too long.\n");
			return;
		}
		page_sysfile(user,inpstr,1);
		user->sys_com	 = 1;
		user->pause_time = (time(0)+15);
		write_user(user,"~CW- ~FTPaging file... this may cause you some lag..\n");
		return;
	}
	else
	{
		sprintf(filename,"%s/%s.F",TempFiles,user->name);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,"~CW- ~FGFingering failed.\n");
			  break;
			case 1:
			  user->misc_op = 2;
			  break;
		}
		return;
	}
}

/* Uses the system's finger command */
void uni_ois(User user,char *inpstr,int rt)
{
	char filename[FSL];

	if (rt == 0)
	{
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
		if (word_count<2 || strlen(inpstr)<3)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRsite~CB]");
			return;
		}
		if (word[1][0] == '-')
		{
			write_user(user,"~FG That was an invalid address you specified.\n");
			return;
		}
		if (strpbrk(inpstr,"<>%^&*()$#!;"))
		{
			write_user(user,"~FG That was an invalid address you specified.\n");
			return;
		}
		if (strlen(inpstr)>SiteNameLen)
		{
			write_user(user,"~FM Address too long.\n");
			return;
		}
		page_sysfile(user,inpstr,2);
		user->sys_com	 = 2;
		user->pause_time = (time(0)+15);
		write_user(user,"~CW- ~FTPaging file... this may cause you some lag..\n");
		return;
	}
	else
	{
		sprintf(filename,"%s/%s.W",TempFiles,user->name);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,"~CW- ~FGWhois search failed.\n");
			  break;
			case 1:
			  user->misc_op = 2;
			  break;
		}
		return;
	}
}

/* Uses the system's finger command */
void ns_lookup(User user,char *inpstr,int rt)
{
	char filename[FSL];

	if (rt == 0)
	{
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
		if (word_count<2 || strlen(inpstr)<3)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRsite~CB] [~CW<~CRserver~CW>~CB]");
			return;
		}
		if (word[1][0] == '-')
		{
			write_user(user,"~FG That was an invalid address you specified.\n");
			return;
		}
		if (strpbrk(inpstr,"<>%^&*()$#!;"))
		{
			write_user(user,"~FG That was an invalid address you specified.\n");
			return;
		}
		if (strlen(inpstr)>SiteNameLen)
		{
			write_user(user,"~FM Address too long!\n");
			return;
		}
		page_sysfile(user,inpstr,3);
		user->sys_com	 = 3;
		user->pause_time = (time(0)+15);
		write_user(user,"~CW-~FT Paging file.. this may cause you some lag..\n");
		return;
	}
	else
	{
		sprintf(filename,"%s/%s.NS",TempFiles,user->name);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,"~CW- ~FGName server search failed.\n");
			  break;
			case 1:
			  user->misc_op = 2;
			  break;
		}
		return;
	}
}

/* Get the webpage.. and dump it to a file. */
int page_sysfile(User user,char *getit,int com)
{
	char exec[ARR_SIZE],filename[FSL];

	switch (com)
	{
		case 1 :
		  sprintf(filename,"%s/%s.F",TempFiles,user->name);
		  sprintf(exec,"finger %s > %s",getit,filename);
		  break;
		case 2 :
		  sprintf(filename,"%s/%s.W",TempFiles,user->name);
		  sprintf(exec,"whois %s > %s",getit,filename);
		  break;
		case 3 :
		  sprintf(filename,"%s/%s.NS",TempFiles,user->name);
		  sprintf(exec,"nslookup %s > %s",getit,filename);
		  break;
		case 4 :
		  sprintf(filename,"%s/%s.URL",TempFiles,user->name);
		  sprintf(exec,"%s %s > %s",WWWExec,getit,filename);
		  break;
		default:
		  return(0);
	}
	switch(double_fork())
	{
		case -1:
		  return(0);        /* Didn't work */
		case 0 :
		  system(exec);
		  _exit(1);
		  write_user(user,"Paging finished..\n");
		  return(1);
	}
	return(0);
}

/* Jump to a webpage */
void goto_webpage(User user,char *inpstr,int rt)
{
	char text2[ARR_SIZE];
	char filename[FSL];

	if (rt == 0)
	{
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
		if (word_count<2)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRwww site~CW/~CRhome~CB]");
			return;
		}
		if (word[1][0] == '-')
		{
			write_user(user,"~FG That was an invalid addy yous specified.\n");
			return;
		}
		if (strpbrk(inpstr,"<>%^&*()$#!;"))
		{
			write_user(user,"~FG That was an invalid addy you specified.\n");
			return;
		}
		if (!strcasecmp(word[1],"home")) sprintf(text2,"%s",user->webpage);
		else if (!istrstr(inpstr,"http://"))  sprintf(text2,"http://%s",inpstr);
		else sprintf(text2,"%s",inpstr);
		strtolower(text2);
		writeus(user,"~FG Looking for~CB: [~CR%s~CB]\n",text2);
		user->sys_com	 = 4;
		user->pause_time = (time(0)+15);
		page_sysfile(user,text2,4);
		return;
	}
	else
	{
		sprintf(filename,"%s/%s.URL",TempFiles,user->name);
		switch(more(user,user->socket,filename,0))
		{
			case 0:
			  write_user(user,"~CB - ~FGI wasn't able to find the webpage.\n");
			  break;
			case 1:
			  user->misc_op = 2;
			  break;
		}
		return;
	}
}

/* END OF SERVER.C */
