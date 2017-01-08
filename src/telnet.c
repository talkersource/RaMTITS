/*
  telnet.c
    Contains the source code for the commands such as .topen, .tclose
    and all the inner workings for the telnet code.

    The source within this file is Copyright 1998 - 2001 by
 	Rob Melhuish, Frank Chavez, and Neil Robertson.

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
#include <arpa/telnet.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#define _TELNET_C
  #include "include/needed.h"
#undef _TELNET_C

/* Creates a telnet object */
Telnet create_telnet(void)
{
	Telnet t;

	if ((t = (Telnet)malloc(sizeof(struct telnet_struct))) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Memory allocation failure in create_telnet().\n");
		return(NULL);
	}
	memset(t,0,sizeof(struct telnet_struct));
	if (telnet_first == NULL)
	{
		telnet_first = t;
		t->prev = NULL;
	}
	else
	{
		telnet_last->next = t;
		t->prev = telnet_last;
	}
	t->next = NULL;
	telnet_last = t;

	t->site[0]	 = '\0';
	t->socket	 = -1;
	t->port		 = 0;
	t->phone	 = 0;
	t->opened	 = time(0);
	t->connected	 = 0;
	t->owner	 = NULL;
	t->room		 = NULL;
	t->last_received = time(0);
	t->buff		 = NULL;
	return(t);
}

/* Destructs the telnet object */
void destruct_telnet(Telnet t)
{
	if (t == telnet_first)
	{
		telnet_first = t->next;
		if (t == telnet_last) telnet_last = NULL;
		else telnet_first->prev = NULL;
	}
	else
	{
		t->prev->next = t->next;
		if (t == telnet_last)
		{
			telnet_last = t->prev;
			telnet_last->next = NULL;
		}
	}
	memset(t,0,sizeof(struct telnet_struct));
	free(t);
}


/* Get a telnet object */
Telnet get_telnet(User user)
{
	Telnet t;

	for(t = telnet_first; t != NULL; t = t->next)
	{
		if (t->owner == user) return(t);
	}
	return(NULL);
}

/* writes a string to the room where the telnet is located. */
void display_telnet(Telnet t,char *str,int len)
{

	if (len == 0) return;
	if (t->owner == NULL) return;
	write(t->owner->socket,str,len);
}

/* Makes a connection to a site */
int connect_to_site(Telnet t)
{
	struct sockaddr_in ca;
	struct hostent *he;
	struct timeval tv;
	int inetnum,ret=0;
	char *sn;
	fd_set trm;

	sn=t->site;
	while (*sn && (*sn == '.' || isdigit(*sn))) sn++;
	if (*sn)
	{
		if (!(he = gethostbyname(t->site))) return(2);
		memcpy((char *)&ca.sin_addr,he->h_addr,(size_t)he->h_length);
	}
	else
	{
		if ((inetnum = inet_addr(t->site)) == -1) return(1);
		memcpy((char *)&ca.sin_addr,(char *)&inetnum,(size_t)sizeof(inetnum));
	}
	if (-1 == (t->socket = socket(PF_INET,SOCK_STREAM,0))) return(1);
	ca.sin_family = PF_INET;
	ca.sin_port   = htons(t->port);
	ret = connect(t->socket,(struct sockaddr *)&ca,sizeof(ca));
	if (ret != 0)
	{
		reset_alarm();
		return(1);
	}
	FD_ZERO(&trm);
	FD_SET(t->socket,&trm);
	tv.tv_sec  = TelnetWait;
	tv.tv_usec = 0;
	if (select(FD_SETSIZE,0,&trm,0,&tv) == 0
	  || !FD_ISSET(t->socket,&trm))
	{
		close(t->socket);
		reset_alarm();
		return(1);
	}
	reset_alarm();
	return(0);
}

/* Disconnects the telnet connection */
void disconnect_telnet(Telnet t)
{
	if (t == NULL) return;
	if (t->buff != NULL) delete_convbuff(t->buff);
	if (t->socket != -1) close(t->socket);
	destruct_telnet(t);
}

/* Connects the telnet connection */
void connect_telnet(Telnet t)
{
	int ret;

	writeus(t->owner," - Trying %s:%d...\n",t->site,t->port);
	t->connected = 0;
	errno = 0;
	fflush(stdout);
	if ((ret=connect_to_site(t)))
	{
		sprintf(text," - telnet: Unable to connect to remote host: ");
		if (ret == 1) strcat(text,strerror(errno));
		else strcat(text,"Unknown hostname");
		strcat(text,"\n");
		write_user(t->owner,text);
		disconnect_telnet(t);
	}
	else
	{
		writeus(t->owner," - Connected to %s:%d.\n",t->site,t->port);
		t->opened = time(0);
		t->connected = 1;
		write_log(1,LOG6,"[TELNET] - User: [%s] - Site/Port: [%s:%d]\n",t->owner->name,t->site,t->port);
	}
}

/* The actual telnet command */
void telnet_com(User user,char *inpstr,int rt)
{
	Room rm;
	Telnet t;
	int port;

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
	rm = user->room;
	if ((t = get_telnet(user)) != NULL)
	{
		write_user(user,"~CB - ~FGYou already have a telnet connection opened.\n");
		return;
	}
	if (word_count<3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CRsite~CB] [~CRport~CB]");
		return;
	}
	port = atoi(word[2]);
	if (port<1024 || port>65535)
	{
		write_user(user,"~CB - ~FGYou specified an invalid port number.\n");
		return;
	}
	if ((t = create_telnet()) == NULL)
	{
		write_user(user,"~CB - ~FGUnable to create a telnet session right now.\n");
		return;
	}
	strncpy(t->site,word[1],sizeof(t->site)-1);
	t->port = port;
	t->owner = user;
	t->room = user->room;
	if (t->buff == NULL) t->buff = newconv_buffer();
	connect_telnet(t);
}

/* Closes the telnet connection */
void telnet_close(User user,char *inpstr,int rt)
{
	Telnet t;

	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if ((t = get_telnet(user)) == NULL)
	{
		write_user(user,"~CB - ~FTYou need a telnet connection open first.\n");
		return;
	}
	write_user(user," - Connection closed by foreign host.\n");
	disconnect_telnet(t);
}

/* Send a string through to execute commands and what not through the telnet */
void telnet_say(User user,char *inpstr,int rt)
{
	Telnet t;

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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if ((t = get_telnet(user)) == NULL)
	{
		write_user(user,"~CB - ~FTYou don't even have a telnet connection open.\n");
		return;
	}
	if (t->socket == -1 || t->connected == 0)
	{
		write_user(user,"~CB - ~FTYou're not fully connected yet.\n");
		return;
	}
	strcat(inpstr,"\n");
	write(t->socket,inpstr,strlen(inpstr));
}

/* Lets you make it so whatever you type gets sent right to your telnet -
 - connection providing that you have one opened.			*/
void telnet_phone(User user,char *inpstr,int rt)
{
	Telnet t;

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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if ((t = get_telnet(user)) == NULL)
	{
		write_user(user,"~CB - ~FTYou need a telnet connection opened first.\n");
		return;
	}
	if (t->phone == 1) t->phone = 0;  else t->phone = 1;
	writeus(user,"~CB - ~FTTelnet phone is now currently~CB: [~CR%s~CB]\n",offon[t->phone]);
}

/* list all the telnet connections that are open. */
void telnet_list(User user,char *inpstr,int rt)
{
	Telnet t;
	int cnt = 0;
	char sockstr[4];

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	for_telnet(t)
	{
		if (t->owner->invis && t->owner->level>user->level) continue;
		if (++cnt == 1)
		{
			write_user(user,"~CB -------- ~FTCurrent telnet connections open~CB:\n");
		}
		if (t->connected == 0 || t->socket == -1) strcpy(sockstr,"-");
		else sprintf(sockstr,"%2d",t->socket);
		writeus(user,"~CB -- ~FY%s ~CB-- ~FT%15s %5d~CB - ~CR%s\n",t->owner->recap,t->site,t->port,sockstr);
	}
	if (cnt == 0) write_user(user,"~CB - ~FTNo telnet connections are open.\n");
	else writeus(user,"~CB --------~FT Currently %d telnet connection%s are open.\n",cnt,cnt>1?"s":"");
}

int check_telnet(Telnet t,char *inpstr)
{
	char temp[ARR_SIZE];

	if (t == NULL || t->phone == 0) return(0);
	if (inpstr[0] != '\0')
	{
		sscanf(inpstr,"%s",temp);
		if (!strcasecmp(temp,".local")) return(2);
		if (!strcasecmp(temp,".tphone")) return(3);
	}
	return(1);
}

/* END OF TELNET.C */
