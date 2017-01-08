/*
  connect.c
    The source within this file contains the functions that connect the
    user to the talker, and disconnects them.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Neil Robertson, and the original resolve() function
	which was written by tref and modified by Rob Melhuish

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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <ctype.h>
#include "include/blocking.h"
#define _CONNECT_SOURCE
  #include "include/needed.h"
#undef _CONNECT_SOURCE
#include <unistd.h>
extern char *crypt __P ((__const char *__key, __const char *__salt));
extern void send_ident_request(User u,struct sockaddr_in *sockadd);

char *resolve_site(User user,struct sockaddr_in rs)
{
	static char site[SiteNameLen];
	char ip_site[16]; /* xxx.xxx.xxx.xxx (only need 15.) */
	struct hostent *hp;

	strcpy(ip_site,(char *)inet_ntoa(rs.sin_addr)); /* numeric ip */
	if (resolve_sites == 2) strcpy(site,((char *)resolve(ip_site)));
	else
	{
		hp = gethostbyaddr((char *)&(rs.sin_addr.s_addr),sizeof(rs.sin_addr.s_addr),AF_INET);
		if (hp == NULL)
		{
			strcpy(site,ip_site);
		}
		else
		{
			strncpy(site,hp->h_name,sizeof(site));
		}
	}
	strtolower(site);
	return(site);
}

/*
  This code started out as the resolve fun function by tref for BSD
  systems, but as I've noticed, not only BSD systems can cause the
  severe hanging.. so I figured I'd make my own fix for linuxes
*/
char *resolve(char *site)
{
	FILE *pp;
	static char str[FSL],temp[FSL];
	char temp2[5][81];
	int i;

	if (!strcmp(site,"127.0.0.1")) return("localhost");
	sprintf(str,"/usr/bin/host %s",site);
	pp = popen(str,"r");
	*str = 0;  temp[0] = '\0';
	for (i = 0 ; i < 5 ; ++i) temp2[i][0] = '\0';
	fgets(str,255,pp);
	pclose(pp);
	if (!istrstr(str,"domain name pointer")) return(site);
	sscanf(str,"%s %s %s %s %s",temp2[0],temp2[1],temp2[2],temp2[3],temp2[4]);
	strcpy(temp,temp2[4]);
	strtolower(temp);
	return(temp);
}

/* Accept incoming connections on listen sockets */
void accept_connection(int lsock,int num)
{
	Bans b;
	User user;
	struct sockaddr_in conn;
	int as,pos,free;
	socklen_t size;
	char seq[4];

	pos = free = 0;
	size = sizeof(struct sockaddr_in);
	as = accept(lsock,(struct sockaddr *)&conn,&size);
	/* Create the user object */
	if ((user = create_user()) == NULL)
	{
		sprintf(text,"\n An error occurred in creating a user object for you.\n");
		write_socket(as,text,strlen(text));
		close(as);
	}
	no_prompt = 1;
	user->socket = as;
	user->last_input=time(0);
	sprintf(seq,"%c%c%c",IAC,DO,TELOPT_NAWS);
	write(user->socket,seq,strlen(seq));
	user->login = 1;
	if (user->autodetected == 0)
	{
		if (user->rows == 0) user->rows = 24;
		if (user->cols == 0) user->cols = 80;
	}
	getpeername(as,(struct sockaddr *)&conn,&size);
	user->site[0] = '\0';	user->ip_site[0] = '\0';
	strcpy(user->ip_site,(char *)inet_ntoa(conn.sin_addr));
	strcpy(user->site,resolve_site(user,conn));

	if (is_banned(user->site,Screen)
	  || is_banned(user->ip_site,Screen))
	{
		user->site_screened = 1;
	}
	else
	{
		user->site_screened = 0;
		logging_in = 1;
		WriteLevel(GOD,NULL," ~CB[~FTLOGIN~CB] ~FTSite~CB: [~FM%s~CB] ~FTIP~CB: [~FM%s~CB]\n",user->site,user->ip_site);
		logging_in = 0;
	}
	if (is_banned(user->site,SiteBan)
	  || is_banned(user->ip_site,SiteBan))
	{
		sprintf(text,"\r- Logins from your site/domain have been BANNED.\n");
		write_socket(as,text,strlen(text));
		sprintf(text,"\r- For more information, e-mail %s\n",TalkerEmail);
		write_socket(as,text,strlen(text));
		logging_in = 1;
		WriteLevel(WIZ,NULL," ~CB[~FTLOGIN~CB] ~FGBanned site~CB: [~FM%s~CB] ~FGwas disconnected.\n",is_banned(user->site,SiteBan)?user->site:user->ip_site);
		logging_in = 0;
		write_log(1,LOG1,"[SITE BAN] - Site: [%s]\n",is_banned(user->site,SiteBan)?user->site:user->ip_site);
		disconnect_user(user,1);
		return;
	}
	pos = InConnlist(user->ip_site);
	if (pos == -1)
	{
		/* Not in the list */
		free = FindFreeConnslot();
		strcpy(connlist[free].site,user->ip_site);
		connlist[free].connections = 1;
	}
	else
	{
		/* pos is where they are. */
		if (connlist[pos].connections > 20)
		{
			/* More then 15 connections in 60 seconds.. ban them. */
			b = find_exact_ban(user->ip_site);
			if (b == NULL)
			{
				b = new_ban(user->ip_site,SiteBan);
				if (b != NULL)
				{
					b->time_banned = time(0);
					strncpy(b->reason,"Auto-ban, login flood attempted.\n",sizeof(b->reason)-1);
					strncpy(b->by,"The_Talker",sizeof(b->by)-1);
				}
			}
			connlist[pos].connections = 0;
			connlist[pos].site[0] = '\0';
		}
		else connlist[pos].connections++;
	}
	fcntl(as,F_SETFL,NBLOCK_CMD);  /* Set socket to non-blocking */
	user->site_port = (int)ntohs(conn.sin_port);
	if (auto_auth)
	{
		send_ident_request(user,&conn);
	}
	echo_on(user);
	num_of_logins++;			user->colour	= 1;
	user->car_return = 1;			user->cols	= 80;
	write_user(user,"~CW- ~RSIf you would like ANSI colour enabled, please enter yes on the line below.\n");
	write_user(user,"~CW- ~RSANSI colour: ~CRYes~RS/no: ");
	if (num == 0) user->port = port[0];  else user->port = port[1];
	user->input = 0;
	no_prompt = 0;
	return;
}

/*
  Perform the logging in routine that a user has to do when connecting to
  the talker. I've been mucking around with so many different ways to do
  this, and have now finally come up with one.. it's now done the basic
  way that most of my menus are done.. the reason I did this... pretty
  much is for ease of typing.. instead of having to type full things out,
  you can just hit your choice of options or whatever, and carry on.
*/
void Login(User user,char *inpstr)
{
	User u,next;
	int i = 0,fsize1,fsize2,sock=user->socket,motd,rows = 0,err = 0;
	char line[ARR_SIZE],filename[FSL],site[SiteNameLen],text2[ARR_SIZE];
	struct stat fb;
	size_t x = 0;

	fsize1 = fsize2 = motd = 0;
	line[0] = filename[0] = text2[0] = '\0';
	if (user->site[0] && strcmp(user->site,"Unknown"))
	  strcpy(site,user->site);
	else strcpy(site,user->ip_site);
	switch (user->login)
	{
		/*
		  Prompting user for colour yes/no, if just ENTER is
		  hit, we assume that the user does want colour.
		*/
		case 1:
		  switch (toupper(inpstr[0]))
		  {
			case 'N':
			  user->colour = 0;
			  write_user(user,"Ansi colour is OFF.\n");
			  break;
			default :
			  user->colour = 1;
			  write_user(user,"Ansi colour is ~CRON\n");
			  break;
		  }
		  /* Now we do the motd */
		  motd = ran(5);
		  if (motd == 5) motd=0;
		  sprintf(filename,"%s/%s.%d",MiscDir,OpenScreen,motd);
		  rows = user->rows;		user->rows = 0;
		  more(user,sock,filename,0);
		  filename[0] = '\0';
		  sprintf(filename,"%s/%s",MiscDir,BirthDays);
		  if (file_exists(filename)) more(user,sock,filename,0);
		  user->rows = rows;
		  if (user->port == port[0])
		  {
			sprintf(text,"~FT-~CT=~CB> ~RSWelcome to %s~RS. Home of %d other user%s.~CB <~CT=~FT-\n\r",TalkerRecap,user_count,user_count == 1?"":"s");
			write_user(user,center(text,80));
		  }
		  else
		  {
			write_user(user,center("~FT-~CT=~CB> !Staff port login!~CB <~CT=~FT-\n\r",80));
			sprintf(text,"~FT-~CT=~CB> ~RSWelcome to %s~RS. Home of %d other user%s.~CB <~CT=~FT-\n\r",TalkerRecap,user_count,user_count == 1?"":"s");
			write_user(user,center(text,80));
		  }
		  write_user(user,login_prompt);
		  user->login = 2;
		  return;
		case 2:
		  line[0] = '\0';
		  sscanf(inpstr,"%s",line);
		  /* Now I've done this for greater efficiency in the login. */
		  if (strlen(line)<2 && user->input == 0)
		  {
			switch (toupper(line[0]))
			{
				case 'W': /* Who request */
				  if (is_banned(user->site,WhoSBan)
				    || is_banned(user->ip_site,WhoSBan))
				  {
					write_user(user," Your site has been banned from using this option at the login.\n");
					write_user(user," That means you gotta login to see who is on.\n\n");
					write_user(user,login_prompt);
					user->login = 2;
					return;
				  }
				  logging_in	= 1;
				  user->ansi_on = 0;
				  user->level	= 0;
				  who(user,NULL,0);
				  if (user->site_screened == 0 || user->port != port[1])
				  	WriteLevel(ADMIN,NULL,"~CB [~FTLOGIN~CB] ~CB[~CRWho request~CB] ~FTSite: ~CB[~FM%s~CB]\n",site);
				  logging_in = 0;
				  write_user(user,"\n\r");
				  write_user(user,login_prompt);
				  return;
				case 'N':
				  /*
				    New user is requesting an account, I decided to
				    do it this way, cause I'm wierd, give me a break
				    ok :P
				  */
				  if (is_banned(user->site,NewSBan)
				    || is_banned(user->ip_site,NewSBan))
				  {
					write_user(user,"~CW- ~RSNew accounts from your site/domain have been ~CRBANNED\n");
					writeus(user,"~CW- ~RSFor more information, e-mail ~FT%s\n",TalkerEmail);
					logging_in = 1;
					WriteLevel(WIZ,NULL,"~CB [~FTLOGIN~CB] ~CB[~FTNew user site ban~CB] ~FTSite:~CB [~CR%s~CB]\n",is_banned(user->site,NewSBan)?user->site:user->ip_site);
					logging_in = 0;
					write_log(1,LOG1,"[NEWSITE BAN] - User: [%s] Site: [%s]\n",user->name,is_banned(user->site,NewSBan)?user->site:user->ip_site);
					disconnect_user(user,1);
					return;
				  }
				  writeus(user,"~CW- ~RSWelcome to %s~RS please just fill out a few questions for us.\n",TalkerRecap);
				  write_user(user,"~CW- ~RSWhat would you like for a name?: ");
				  user->input = 2;
				  return;
				case 'C': /* User want's to check their s-mail. */
				  logging_in = 1;
				  if (user->site_screened == 0 || user->port != port[1])
					  WriteLevel(ADMIN,NULL," ~CB[~FTLOGIN~CB] [~CRs-mail request~CB]~FT Site: ~CB[~FM%s~CB]\n",site);
				  logging_in = 0;	user->input = 1;
				  write_user(user,"~CW- ~RSChecking for new s-mail...\n");
				  write_user(user,login_name);
				  return;
				case 'V': /* Version request */
				  logging_in = 1;
				  write_user(user,"\n\r");
				  what_version(user,NULL,0);
				  if (user->site_screened == 0 || user->port != port[1])
					  WriteLevel(ADMIN,NULL," ~CB[~FTLOGIN~CB] [~CRVersion request~CB]~FT Site: ~CB[~FM%s~CB]\n",site);
				  logging_in = 0;
				  write_user(user,"\n\r");
				  write_user(user,login_prompt);
				  return;
				case 'Q':
				  write_user(user,"\n");
				  write_user(user,center("~CB<~FT==-- ~FMFine, go ahead, just leave! ~FT--==~CB>\n\n",80));
				  logging_in = 1;
				  if (user->site_screened == 0 || user->port != port[1])
					  WriteLevel(ADMIN,NULL,"~CB [~FTLOGIN~CB] [~CRquit~CB] ~FTSite: ~CB[~FM%s~CB]\n",site);
				  logging_in=0;
				  disconnect_user(user,1);
				  return;
				case '?': /* User needs the menu re-drawn for them */
				  PrintMenu(user,10);
				  write_user(user,menuprompt);
				  return;
				default :
		  		  write_user(user,"Invalid option ~CW(~CR? ~RSfor help~CW)~RS: ");
				  return;
			}
		  }
		  else		/* User has actually entered a name */
		  {
			if (strlen(line) < 3)
			{
				write_user(user,"\n\r Wow, thats a pretty short name.. how about a longer one?\n\r");
				if (user->input != 2) write_user(user,login_prompt);
				else write_user(user,"~CW- ~RSWhat would you like for a name?: ");
				return;
			}
			if (strlen(line) > UserNameLen)
			{
				write_user(user,"\n\r Wow, that's one LONG name.. a bit shorter please...\n\r");
				if (user->input != 2) write_user(user,login_prompt);
				else write_user(user,"~CW- ~RSWhat would you like for a name?: ");
				return;
			}
			if (contains_swearing(line))
			{
				write_user(user,"\n\r~RS You can't have naughty words in your name..\n\r");
				if (user->input != 2) write_user(user,login_prompt);
				else write_user(user,"~CW- ~RSWhat would you like for a name?: ");
				return;
			}
			/* Make sure only letters in first part of name. */
			if (!isalpha(line[0]))
			{
				write_user(user,"\n\r Please use characters only in your name.\n\r");
				if (user->input != 2) write_user(user,login_prompt);
				else write_user(user,"~CW- ~RSWhat would you like for a name?: ");
				return;
			}
			for (x = 0; x < strlen(line); ++x)
			{
				if (!isalnum(line[x]))
				{
					write_user(user,"\n\r Please use characters only in your name.\n\r");
					if (user->input != 2) write_user(user,login_prompt);
					else write_user(user,"~CW- ~RSWhat would you like for a name?: ");
					return;
				}
			}
			strtolower(line);
			line[0] = toupper(line[0]);
			if (is_banned(line,UserBan))
			{
				logging_in = 1;
				WriteLevel(WIZ,NULL," ~CB[~FTLOGIN~CB] ~FTBanned user attempt:~CB [~FG%s~CB]\n",line);
				logging_in = 0;
				write_user(user," You have been banned from here. If you would like more information as to\n");
				writeus(user," why, e-mail ~CR%s~FG and ask why.\n\r",TalkerEmail);
				disconnect_user(user,1);
				write_log(1,LOG1,"[BAN-USER] - User: [%s] Site: [%s]\n",line,site);
				return;
			}
			strtolower(line);
			line[0] = toupper(line[0]);
			strcpy(user->name,line);
			/* If user has hung on another login, clear that session */
			u = user_first;
			while (u != NULL)
			{
				next = u->next;
				if (u->login && u != user
				  && !strcmp(u->name,user->name))
				{
					disconnect_user(u,1);
					break;
				}
				u = next;
			}
			strcpy(user->name,line);
			err = LoadUser(user,line);
			if (err == 0)
			{
				if (user->input == 1)
				{
					write_user(user,"\n\r~CW-~RS It appears as tho you don't even have an account on here.\n");
					write_user(user,"~CW-~RS So chances are you don't have any s-mail ;-)\n");
					user->input = 0;
					user->login = 2;
					write_user(user,login_prompt);
					return;
				}
				if (user->input == 2)
				{
					if (user->port == port[1])
					{
						sprintf(text,"~FT-~CT=~CB> Sorry bud, this is the staff-port, try port ~CR%d~CB <~CT=~FT-\n\r",port[0]);
						write_user(user,center(text,80));
						disconnect_user(user,1);
						return;
					}
					if (minlogin_level > -1)
					{
						write_user(user,"~CW-~RS Sorry, we currently aren't accepting new users right now.\n\r");
						disconnect_user(user,1);
						return;
					}
					for_users(u)
					{
						if (u == user) continue;
						if (!strcasecmp(u->name,user->name))
						{
							write_user(user,"~CW-~RS It appears as tho that name is already being used.\n");
							write_user(user,login_name);
							return;
						}
					}
					writeus(user,"~CW- ~RSWelcome to %s %s Hope you enjoy your stay :-)\n",TalkerRecap,user->name);
					write_user(user,"-~RS Please choose a password: ");
					user->login = 3;
					return;
				}
				write_user(user,"~RS New users must select the appropriate choice from the menu...\n");
				write_user(user,"~RS Hit 'N' to create an account.\n");
				user->login = 2;
				write_user(user,login_prompt);
				return;
			}
			else if (err == -1)
			{
				writeus(user,"~CW-~RS It appears as tho there is an error with your userfile.\n~CW-~RS Please e-mail %s and let them know about it.\n",TalkerEmail);
				write_log(1,LOG1,"[CORRUPT USER] - Username: [%s]\n",user->name);
				disconnect_user(user,1);
				return;
			}
			if (user->input == 2)
			{
				write_user(user,"~CW-~RS There is already an account with that name...\n");
				write_user(user,login_name);
				return;
			}
			if (user->port == port[1] && user->level < wizport_level)
			{
				writeus(user,"\n\r You must be of level~CR %s~RS and above to use this port.\n\r",level[wizport_level].name);
				writeus(user," Try port: ~CR%d\n\n\r",port[0]);
				disconnect_user(user,1);
				logging_in = 1;
				WriteLevel(WIZ,NULL," ~CB[~FTLOGIN~CB] ~FTUser~CB: [~RS%s~CB]~CW -~FGWizport attempt~CW- ~FTSite~CB: [~FM%s~CB]\n",line,site);
				logging_in = 0;
				return;
			}
			if (user->level < minlogin_level)
			{
				writeus(user,"\n\r Sorry, currently the talker is only letting people of ~CR%s\n level login, please try back later :)\n\r",level[minlogin_level].name);
				disconnect_user(user,1);
				logging_in = 1;
				WriteLevel(WIZ,NULL,"~CB [~FTLOGIN~CB] ~FTUser~CB: [~RS%s~CB] ~CW-~FGminlogin attempt~CW- ~FTLevel~CB: [~FM%s~CB]\n",user->recap,user->gender==Female?level[user->level].fname:level[user->level].mname);
				logging_in = 0;
				return;
			}
			logging_in = 1;
			if (user->input == 0)
			{
				if (user->site_screened == 0)
				{
					switch (user->login_type)
					{
						case 1 :
						  if (user->site_screened == 0 || user->port != port[1])
							WriteLevel(WIZ,NULL," ~CB[~FTPre-login~CB] ~FTUser~CB: [~RS%s~CB] ~FTSite~CB: [~FM%s~CB] ~CW(~CRInvis~CW)\n",user->recap,site);
						  break;
						case 2 :
						  if (user->site_screened == 0 || user->port != port[1])
							WriteLevel(ADMIN,NULL," ~CB[~FTPre-login~CB] ~FTUser~CB: [~RS%s~CB] ~FTSite~CB: [~FM%s~CB] ~CW(~CRHidden~CW)\n",user->recap,site);
						  break;
						case 3 :
						  if (user->site_screened == 0 || user->port != port[1])
							WriteLevel(GOD,NULL," ~CB[~FTPre-login~CB] ~FTUser~CB: [~RS%s~CB] ~FTSite~CB: [~FM%s~CB] ~CW(~CRFakelog~CW)\n",user->recap,site);
						  break;
						default:
						  if (user->site_screened == 0 || user->port != port[1])
							WriteLevel(WIZ,NULL," ~CB[~FTPre-login~CB] ~FTUser~CB: [~RS%s~CB] ~FTSite~CB: [~FM%s~CB]\n",user->recap,site);
						  break;
					}
				}
			}
			logging_in = 0;
			write_user(user,"\n\r");
			if (user->input == 0)
			{
				if (user->recap[0]) sprintf(text,"~FT-~CT=~CB> ~FYHiyas %s ~FG%s~FY hope you're having fun. ~CW:-)~CB <~CT=~FT-\n",user->gender==Female?level[user->level].fname:level[user->level].mname,user->recap);
				else sprintf(text,"~FT-~CT=~CB> ~FYHiyas %s ~FG%s~FY hope you're having fun. ~CW:-)~CB <~CT=~FT-\n",user->gender==Female?level[user->level].fname:level[user->level].mname,user->name);
				write_user(user,center(text,80));
			}
			write_user(user,login_pwd);
			echo_off(user);
			user->login = 3;
			return;
		  } /* End of else */
		  return;
		case 3: /* User enters their password. */
		  line[0] = '\0';
		  sscanf(inpstr,"%s",line);
		  if (strlen(line) < 3)
		  {
			write_user(user,"\n\r~CW- ~RSThat password is too short.. so it couldn't be your password.\n");
			attempts(user,0);
			return;
		  }
		  if (strlen(line) > PassLen)
		  {
			write_user(user,"\n\r~CW- ~RSThat password is too long.. so it couldn't be your password.\n");
			attempts(user,0);
			return;
		  }
		  if (!strcasecmp(line,user->name))
		  {
			write_user(user,"\n\r~CW-~RS Passwords can't be your login name.. so that couldn't be your password.\n");
			attempts(user,0);
			return;
		  }
		  /* See if it's a new user. */
		  if (user->pass[0] == '\0')
		  {
			strcpy(user->pass,(char *)crypt(line,salt));
			write_user(user,"~CW- ~RSOne more time: ");
			user->login = 4;
			return;
		  }
		  if (!strcasecmp(line,"imnew") && user->input == 0)
		  {
			if (user->port == port[1])
			{
				writeus(user,"\n~CW-~RS Sorry, this is the staff port.. please try port ~CR%d\n",port[0]);
				disconnect_user(user,1);
				return;
			}
			writeus(user,"\n~CW-~RS Welcome to %s please just fill out a few questions for us.\n",TalkerRecap);
			write_user(user,"~CW- ~RSWhat would you like for a name?:~RS ");
			user->input = 2;
			user->login = 2;
			echo_on(user);
			return;
		  }
		  /*
		     This user actually has an account.. first make sure that
		     the password that they gave us is correct.. otherwise do
		     the attempts, and make sure they aren't attempting to hack
		     into someone else's account.. if they are, let the user
		     know via s-mail.
		  */
		  else
		  {
			if (!strcmp(user->pass,(char *)crypt(line,salt)))
			{
				if (user->input)
				{
					if (!(has_unread_mail(user)))
					{
						write_user(user,"\n\r~CW-~RS Sorry, but it appears as though you have no new s-mail.\n~CW- ~RSBut feel free to logon anyway and say hi to us :)\n");
						write_user(user,login_prompt);
						user->login = 2;
						user->input = 0;
						echo_on(user);
						return;
					}
					else
					{
						rmail(user,NULL,1);
						write_user(user,login_prompt);
						user->login = 2;
						user->input = 0;
						echo_on(user);
						return;
					}
					return;
				}
				cls(user,NULL,user->socket);
				write_user(user,"\n\r");
				sprintf(text,"Welcome tp %s ~RS%s\n\r",TalkerRecap,user->recap);
				write_user(user,center(text,80));
				switch (user->login_type)
				{
					case 1 :
					  write_user(user,"     You are currently~CR invisible~RS to the users...\n");
					  break;
					case 2 :
					  write_user(user,"     You are currently~CR hidden~RS to the users...\n");
					  break;
					case 3 :
					  write_user(user,"     You are currently~CR fake logged off~RS...\n");
					  break;
					default:
					  write_user(user,"     You are currently~CR visible~RS to the users...\n");
					  break;
				}
				writeus(user,"     Your level is: ~CR%s\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
				writeus(user,"     You were last on from site: %s\n",user->last_site);
				writeus(user,"     Your current site: %s - IP: %s\n",user->site,user->ip_site);
				sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
				if (stat(filename,&fb) == -1) fsize1 = 0;
				else fsize1 = fb.st_size;  filename[0] = '\0';
				sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
				if (stat(filename,&fb) == -1) fsize2 = 0;
				else fsize2 = fb.st_size;  filename[0] = '\0';
				writeus(user,"     Your s-mail box is allocating ~CR%d~RS bytes out of a max ~CR%d~RS.\n",fsize1,MaxMbytes);
				writeus(user,"     Your sent s-mail box is allocating ~CR%d~RS bytes out of a max ~CR%d~RS.\n",fsize1,MaxSMbytes);
				if (session[0]) sprintf(text,"     The current session is: ~CR%s\n",session);
				else sprintf(text,"     There is no current session set.\n");
				write_user(user,text);
				sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
				if (file_exists(filename))
					writeus(user,"     You have ~CRnew~RS mail.\n");
				sprintf(file,"%s",Motd);
				show_screen(user);
				user->login = 10;
				return;
			}
			write_user(user,"\n");
			if (user->input == 0) write_user(user,wrong_pwd);
			else write_user(user,wrong_pwd2);
			if (user->attempts >= 2)
			{
				text[0] = '\0';
				sprintf(text2," Attempted login from site: ~CR%s ~RSattempted to use your\n name with the password: ~CR%s\n",site,line);
				send_oneline_mail(NULL,user->name,text2);
				logging_in = 1;
				WriteLevel(GOD,NULL,"~CB [~FTLOGIN~CB]~FT Failed password check: ~CB[~FM%s~CB] ~FTSite:~CB [~FM%s~CB]\n",user->recap,site);
				logging_in = 0;
			}
			attempts(user,0);
		  }
		  return;
		case 4: /* New user is re-entering their password. */
		  line[0] = '\0';
		  sscanf(inpstr,"%s",line);
		  if (strcmp(user->pass,(char *)crypt(line,salt)))
		  {
			write_user(user,"\n\r It usually helps if the two passwords match..\n");
			attempts(user,1);
			return;
		  }
		  write_user(user,"\n\r");
		  echo_on(user);
		  if (auto_promote)
		  {
			/* Ask the users if they agree with the rules or not. */
			sprintf(filename,"%s/%s",MiscDir,RulesFile);
			rows = user->rows;  user->rows = 0;
			more(user,user->socket,filename,0);
			write_user(user,"\n~CW- ~RSDo you agree with these rules?: ");
			user->rows = rows;
			user->login = 5;
		  }
		  else
		  {
			writeus(user,"     Your current site: ~CR%s~RS - IP: ~CR%s\n",user->site,user->ip_site);
			if (session[0]) sprintf(text,"     The current session is: %s\n",session);
			else sprintf(text,"     There is no current session set.\n");
			write_user(user,text);
			sprintf(file,"%s",Motd);
			show_screen(user);
			user->login = 11;
		  }
		  return;
		case 5: /* Answering yes or no for the rules. */
		  echo_on(user);
		  line[0] = '\0';
		  sscanf(inpstr,"%s",line);
		  switch (toupper(line[0]))
		  {
			case 'Y':
			  write_user(user,"~CW-~RS Perfect, you agree to the rules.. have a blast :)\n");
			  break;
			case 'N':
			  write_user(user,"~CW-~RS Alright, you don't agree.. c'ya later :)\n");
			  disconnect_user(user,1);
			  return;
			default :
			  write_user(user,"\n\r~CW- ~RSA simple yes or no please: ");
			  user->login = 5;
			  return;
		  }
		  write_user(user,"\n~CW- ~RSNow, if you would enter your ~CRREAL ~RSe-mail address, this is just for security\n");
		  write_user(user,"~CW- ~RSpurposes, and will remain hidden from users until you choose for it not to be.\n");
		  write_user(user,"~CB:->~RS ");
		  user->login = 6;
		  return;
		case 6: /* Entering their e-mail address. */
		  echo_on(user);
		  line[0] = '\0';
		  sscanf(inpstr,"%s",line);
		  if (!valid_addy(user,line,1))
		  {
			write_user(user,"~CW- ~RSA valid e-mail address please: ");
			return;
		  }
		  strncpy(user->email,line,sizeof(user->email)-1);
		  user->hide_email = 1;
		  writeus(user,"\n~CW- ~RSE-mail address set to: ~CR%s~RS and is hidden from other users.\n",line);
		  WriteLevel(WIZ,NULL,"~CB[~CRSYSTEM~CB] ~FGUser:~CB [~RS%s~CB] ~FGE-mail addy~CB: [~CR%s~CB]\n",user->name,line);
		  sprintf(text,"[NEW USER] - User: [%s] E-mail: [%s]\n",user->name,user->email);
		  write_log(1,LOG3,text);
		  write_record(user,1,text);
		  write_user(user,"\n\r~CW-~RS Your gender? ~CRM~RS/f: ");
		  user->login = 7;
		  return;
		case 7: /* user is submitting their gender. */
		  echo_on(user);
		  line[0] = '\0';
		  sscanf(inpstr,"%s",line);
		  switch (toupper(line[0]))
		  {
			case 'F':
			  user->gender = Female;
			  write_user(user,"\n~CW- ~RSYour gender has been set to ~CRFEMALE\n");
			  break;
			default :
			  user->gender = Male;
			  write_user(user,"\n~CW-~RS Your gender has been set to ~CRMALE\n");
			  break;
		  }
		  write_user(user,"~CW-~RS Finally, the last step is for you to provide us with a short description.\n\r");
		  write_user(user,"~CB:->~RS ");
		  user->login = 8;
		  return;
		case 8:
		  echo_on(user);
		  line[0] = '\0';
		  if (!strlen(inpstr))
		  {
			write_user(user,"\n~CW-~RS Sorry, a description is required here.\n");
			write_user(user,"~CB:->~RS ");
			return;
		  }
		  if ((strlen(ColourStrip(inpstr))>UserDescLen)
		    || strlen(inpstr) > 200)
		  {
			write_user(user,"~CW-~RS Sorry, that description would be a tad bit too long.\n");
			write_user(user,"~CB:->~RS ");
			return;
		  }
		  strcpy(user->desc,inpstr);
		  writeus(user,"~CW- ~RSYour description is now set to: %s\n\n\n",user->desc);
		  writeus(user," ~CW- ~RSYour current sites: %s~CW:~RS%s\n",user->site,user->ip_site);
		  if (session[0]) sprintf(text,"~CW- ~RSThe current session is: %s\n",session);
		  else sprintf(text,"~CW- ~RSThere is no current session set.\n");
		  write_user(user,text);
		  sprintf(file,"Newbiemotd");
		  show_screen(user);
		  user->login = 11;
		  return;
		case 10:  /* Perform the connection shit. */
		  for (i = 0 ; i < 4 ; ++i) ulogins[i] += 1;
		  convert_vars(user);
		  echo_on(user);
		  connect_user(user);
		  return;
		case 11:
		  /*
		    Virtually create the users account, setting up the users
		    vars with the default bullshit.
		  */
		  echo_on(user);
		  strcpy(user->in_phr,def_inphr);
		  strcpy(user->out_phr,def_outphr);
		  strcpy(user->recap,user->name);
		  strcpy(user->webpage,def_url);
		  strcpy(user->bday,def_bday);
		  user->input = 0;
		  strcpy(user->married_to,"Noone");
		  strcpy(user->sstyle,"~CB:");
		  strcpy(user->login_msg,def_lognmsg);
		  strcpy(user->logout_msg,def_logtmsg);
		  user->char_echo=charecho_def;
		  user->prompt = prompt_def;
		  if (free_rooms) user->has_room = 1;
		  else user->has_room = 0;
		  load_default_shortcuts(user);
		  SaveUser(user,1);
		  reset_junk(user);
		  for (i = 0 ; i < 4 ; ++i) ulogins[i] += 1;
		  i = 0;
		  for (i = 0 ; i < 3 ; ++i) newlog[i] += 1;
		  i = 0;
		  sprintf(text,"[NEW USER] - Name: [%s] Site: [%s]\n",user->name,site);
		  write_log(1,LOG1,text);	write_log(1,LOG2,text);
		  write_record(user,1,text);	connect_user(user);
		  add_userlist(user->name,auto_promote?USER:NEWBIE);
	}
}

/* Count up the attempts made by the user to login */
void attempts(User user,int which)
{

	++user->attempts;
	if (user->attempts >= 3)
	{
		write_user(user,"\n\r");
		write_user(user,center("~FT-~CT=~CB> ~FYSorry bud, it took ya too many tries~CB <~CT=~FT-\n\r",80));
		disconnect_user(user,1);
		return;
	}
	if (which == 0)
	{
		user->login = 3;
		write_user(user,login_pwd);
		echo_off(user);
		return;
	}
	else
	{
		user->login = 4;
		write_user(user,login_vpwd);
		echo_off(user);
		return;
	}
}

/* Check certain variables */
void convert_vars(User user)
{
	int i = 0;

	if (!user->sstyle[0] || strlen(user->sstyle)>sizeof(user->sstyle))
	{
		strcpy(user->sstyle,"~CB:");
	}
	if (free_rooms)
	{
		if (user->has_room == 0) user->has_room = 1;
	}
	if (strcasecmp(user->name,ColourStrip(user->recap)))
	{
		if (user->level<GOD) strcpy(user->recap,user->name);
	}
	for (i = 0 ; i < 17 ; ++i)
	{
		if (user->fmacros[i][0] != '\0')
		{
			user->has_macros = 1;
			break;
		}
	}
	return;
}

/* Connect the user to the talker properly */
void connect_user(User user)
{
	User u,u2;
	Room rm,rm2;
	Commands com;
	char levname[20],temp[81];
	int mcount = 0,bday = 0;

	user->login = 0;	num_of_logins--;
	/* See if already connected. */
	for_users(u)
	{
		if (user != u && u->type != CloneType
		  && !strcmp(user->name,u->name))
		{
			rm = u->room;
			write_log(1,LOG2,"[SESSION SWAP] - User: [%s] Site: [%s:%s]\n",user->name,user->site,user->ip_site);
			close(u->socket);
			u->socket = user->socket;
			fcntl(u->socket,F_SETFL,NBLOCK_CMD); /* non-block it. */
			strcpy(u->ip_site,user->ip_site);
			strcpy(u->site,user->site);
			u->site_port = user->site_port;
			if (u->splits)
			{
				splitscreen_off(u);
				splitscreen_on(u);
			}
			destruct_user(user);
			write_user(u,"\n\n~FY You are currently logged on... swapping sessions..\n");
			sprintf(text,"~CB[~FGSession Swap~CB] ~CW- ~FT%s~RS %s\n",u->recap,u->desc);
			if (u->invis == 0 && u->port != port[1]) write_room_except(NULL,u,text);
			look(u,NULL,0);  prompt(u);
			for_users(u2)		/* Reset sockets on clones */
			{
				if (u2->type == CloneType && u2->owner == u)
				  u2->socket = u->socket;
			}
			return;
		}
	}
	fcntl(user->socket,F_SETFL,NBLOCK_CMD); /* non-block the user socket. */
	/* Make sure that the user is actually in the staff list. */
	if (user->level >= POWER && !is_staff(user))
	{
		if (user->total_login >= 432000) user->level = POWER;
		else if (user->total_login >= 86400) user->level = SUPER;
		else user->level = USER;
	}
	/*
	   Check the user->login_type flag, assign the invis value to the right
	   one, and make sure that the user actually has the command so that
	   they can come back visible.. just in case.. ;)
	*/
	switch (user->login_type)
	{
		case 1 :
		  com = get_comname("vis");
		  if (com == NULL) break;
		  if (user->level < com->min_lev)
		  {
			user->invis = 0;  user->login_type = 0;
			break;
		  }
		  user->invis = 1;
		  user->login_type = 0;
		  break;
		case 2 :
		  com = get_comname("hide");
		  if (com == NULL) break;
		  if (user->level < com->min_lev)
		  {
			user->invis = 0;
			user->login_type = 0;
			break;
		  }
		  strcpy(user->temp_recap,user->recap);
		  strcpy(user->recap,"~FM[~FG?~FM]");
		  user->invis = 2;	user->login_type = 0;
		  break;
		case 3 :
		  com = get_comname("fakelog");
		  if (com == NULL) break;
		  if (user->level < com->min_lev)
		  {
			user->invis = 0;
			user->login_type = 0;
			break;
		  }
		  strcpy(user->temp_recap,user->recap);
		  strcpy(user->recap,"~CB[~CR?~CB]");
		  user->invis = 3;	user->login_type = 0;
		  break;
		default:
		  user->invis = 0;	user->login_type = 0;	break;
	}
	if (user->level >= WIZ && user->has_room)
	{
		user->has_room = 0;
	}
	/* User is jailed upon login.. throw them in the jail! */
	if (user->status & Jailed)
	{
		rm2 = get_room(jail_room);
		if (rm2 == NULL) write_user(user,"~FG Unable to find the jail.. but you're still under arrest.\n");
		else user->room = rm2;
	}
	else
	{
		user->room = get_room(main_room);
	}
	strcpy(levname,user->gender == Female?level[user->cloak_lev].fname:level[user->cloak_lev].mname);
	/*
	   Announce the users login, only sending it to certain levels if the
	   invis flag is actually set... otherwise, send it to everyone..
	*/
	if (user->port != port[1])
	{
		logging_in = 1;
		switch (user->invis)
		{
			case 1 : /* User is invis. */
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_invis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s~FM %s~CB <=-\n",login_invis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s~CB <=-\n",login_invis,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s~FM %s~CB <=-\n",login_invis,user->recap,user->desc);
			  }
			  write_level(WIZ,user,text);
			  write_room(user->room,"~FG A mystical being is amongst you now...\n");
			  WriteLevel(WIZ,user,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  write_log(1,LOG2,"[LOGIN-I] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->site,user->site_port,user->port);
			  break;
			case 2 : /* User is hidden. */
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_hide,user->predesc,user->temp_recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s~FM %s~CB <=-\n",login_hide,user->predesc,user->temp_recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s~CB <=-\n",login_hide,user->temp_recap,user->desc);
				else sprintf(text,"%s ~FY%s~FM %s~CB <=-\n",login_hide,user->temp_recap,user->desc);
			  }
			  write_level(ADMIN,user,text);
			  WriteLevel(ADMIN,user,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  write_log(1,LOG2,"[LOGIN-H] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->site,user->site_port,user->port);
			  break;
			case 3 : /* User is fake logged off. */
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_fake,user->predesc,user->temp_recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s~FM %s~CB <=-\n",login_fake,user->predesc,user->temp_recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s~CB <=-\n",login_fake,user->temp_recap,user->desc);
				else sprintf(text,"%s ~FY%s~FM %s~CB <=-\n",login_fake,user->temp_recap,user->desc);
			  }
			  write_level(GOD,user,text);
			  WriteLevel(GOD,user,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  break;
			default:
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s ~FY%s~FM%s~CB <=-\n",login_vis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s ~FY%s~FM %s~CB <=-\n",login_vis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FY%s~FM%s~CB <=-\n",login_vis,user->recap,user->desc);
				else sprintf(text,"%s ~FY%s~FM %s~CB <=-\n",login_vis,user->recap,user->desc);
			  }
			  write_room_except(NULL,user,text);
			  WriteLevel(WIZ,user,"~FT    Connecting from~CB: [~FM%s~CW:~FM%d~CB]\n",user->site,user->site_port);
			  if (user->desc[0] == '\'') sprintf(text,"~FT%s ~FY%s~RS%s\n",long_date(0),user->recap,user->desc);
			  else sprintf(text,"~FT%s ~FY%s ~RS%s\n",long_date(0),user->recap,user->desc);
			  RecordLogin(text);
			  write_log(1,LOG2,"[LOGIN] - User: [%s] Site: [%s:%d] Port: [%d]\n",user->name,user->site,user->site_port,user->port);
			  friend_check(user,1);		wait_check(user);
			  break;
		}
		logging_in=0;
		if (user->invis == 0)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s~FR %s\n",user->recap,user->login_msg);
		}
	}
	cls(user,NULL,user->socket);
	if (user->status & Jailed)
	{
		WriteRoomExcept(user->room,user,"~CW-~FM %s~RS gets swallowed by the gates of hell.\n",user->invis==Invis?invisname:user->recap);
		write_user(user,"~CW-~RS The gates of hell reach up and swallow you...\n\n");
	}
	writeus(user,"     ~FGWelcome to ~CR%s~FM %s~FG...\n",TalkerRecap,user->invis>Invis?user->temp_recap:user->recap);
	user->last_login = time(0);	/* Set to now */
	++user->logons;
	alpha_sort(user);
	look(user,NULL,0);
	if (user->level == NEWBIE && user->total_login<600)
	{
		if (auto_promote)
		{
			++user->level;
		        write_room(NULL,"~FG           >\\ <  > /<     ~CRsS* s S\n");
		        write_room(NULL,"~FG           >-<~FR0~FG  ~FR0~FG>-<   ~CRS*S*sS*s\n");
		        write_room(NULL,"~FG           >/ \\\\// \\<  ~CRs*Ss*s*S\n");
		        write_room(NULL,"~FG              <~CWoo~FG> ~CRs*Ss*s*SS*\n");
		        write_room(NULL,"~CW     _____~FGoOo~CW__~FG~~~CW__~FGoOo~CW__________     ~FG/~\\\n");
		        write_room(NULL,"~FY     Everyone please say hi to      ~FG<<^\\\\\n");
		        write_room(NULL,"~FY     the newest member to our~FG\\  \\____>>< >\n");
		        write_room(NULL,"~FY         cove:                ~FG\\______/  '\n");
		        sprintf(text,"~CR            %s                 \n",user->recap);
		        write_room(NULL,text);
		        write_room(NULL,"~CW     ~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	        }
		else
		{
			write_user(user,"~FG In order to maintain an account here, you must ask a staff member\n");
			write_user(user,"~FG for a promotion.\n");
		}
	}
	if (user->level == USER && user->total_login >= 86400)
	{
		++user->level;
		WriteRoomExcept(NULL,user,"~CW-~FG %s~FT has reached a total login of one day and has been\n~CW- ~FTpromoted to~CB: [~CR%s~CB]\n",user->invis==Invis?invisname:user->recap,user->gender==Female?level[user->level].fname:level[user->level].mname);
		writeus(user,"~FG You have been auto-promoted to level~CB: [~CR%s~CB]\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
	}
	if (user->level == SUPER && user->total_login >= 432000)
	{
		++user->level;
		WriteRoomExcept(NULL,user,"~CW-~FG %s~FT has reached a total login of five days and has been\n~CW- ~FTpromoted to~CB: [~CR%s~CB]\n",user->invis==Invis?invisname:user->recap,user->gender==Female?level[user->level].fname:level[user->level].mname);
		writeus(user,"~FG You have been auto-promoted to level~CB: [~CR%s~CB]\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
	}
	if (has_unread_mail(user))
	{
		sprintf(file,"newmail");
		if (!(show_screen(user)))
		  write_user(user,center("~FT-~CT=~CB> ~FGYou have ~RDN~RDE~RDW ~RDS~RD-~RDM~RDA~RDI~RDL~RD!~CB <~CT=~FT-\n",80));
	}
	mcount = count_mess(user,1);
	if (mcount)
	{
		writeus(user,"~CW- ~RSThere %s a total of ~CY%d~RS message%s found in your s-mail box.\n",mcount>1?"were":"was",mcount,mcount>1?"s":"");
	}
	if (user->level >= GOD && bug_num)
	{
		writeus(user,"~CW- ~RSThere %s ~CY%d~RS bug%s noted on the board.\n",bug_num>1?"are":"is",bug_num,bug_num>1?"s":"");
	}
	if (user->level >= WIZ && suggestion_num)
	{
		writeus(user,"~CW- ~RSThere %s ~CY%d~RS suggestion%s noted on the board.\n",suggestion_num>1?"are":"is",suggestion_num,suggestion_num>1?"s":"");
	}
	if (user->ignore & Quotd) { }
	else
	{
		write_user(user,qotdm);
		writeus(user," %s",qotd[ran(MaxQuotes)]);
	}
	prompt(user);
	user->cloak_lev = user->level;
	if (webpage_on) who_webpage();
	if (user->autoexec[0] != '\0')
	{
		sprintf(temp,"%s",user->autoexec);
		clear_words();
		word_count = wordfind(temp);
		exec_com(user,temp);
	}
	bday = is_bday_today(user->bday);
	if (bday == -1) write_user(user,"~CW-~RS Your birthday is in an invalid format.. please set it properly.. \n");
	else if (bday == 1)
	{
		create_bday_file(user);
		sprintf(text,"~FT-~CT=~CB> ~CRWould everyone please wish %s~CR a happy happy birthday!~CB <~CT=~FT-\n",user->invis == Invis?invisname:user->recap);
		write_room_except(NULL,user,center(text,80));
		sprintf(file,"bday");
		if (!(show_screen(user)))
		{
			write_user(user,"~CW - ~FGHappy Birthday to you.\n~CW - ~FGHappy Birthday to you.\n");
			writeus(user,"~CW - ~FGHappy Birthday dear %s.\n~CW - ~FGHappy Birthday to you.\n",user->recap);
		}
	}
	num_of_users++;
	user->editor.edit_op=0;
}

/* Disconnect the user from the talker. */
void disconnect_user(User user,int type)
{
	Telnet t;
	Editor e;
	Room   rm;
	int rows = 0;
	char   rmname[81];

	no_prompt = 1;
	if (user->login)
	{
		if (user->tbuff) delete_convbuff(user->tbuff);
		if (user->away_buff) delete_convbuff(user->away_buff);
		close(user->socket);
		destruct_user(user);
		num_of_logins--;
		return;
	}
	/* Restore the users level if they are tpromoted. */
	if (user->tpromoted)
	{
		user->level = user->orig_lev;
		user->orig_lev = 0;
		writeus(user,"~FT     Your level has been restored to~CB: ~CR%s\n",user->gender==Female?level[user->level].fname:level[user->level].mname);
		write_log(1,LOG1,"[RESTORE] - User: [%s] Level restored to: [%s]\n",user->name,level[user->level].name);
	}
	strcpy(rmname,user->room->name);
	if (user->pop != NULL) leave_poker(user,NULL,0);
	if (user->splits)
	{
		splitscreen_off(user);
	}
	/* Invisible flags. */
	if (user->invis == 1) user->login_type = 1;
	else if (user->invis == 2)
	{
		user->login_type = 2;
		strcpy(user->recap,user->temp_recap);
	}
	else if (user->invis == 3)
	{
		user->login_type = 3;
		strcpy(user->recap,user->temp_recap);
	}
	else
	{
		user->login_type=10;
	}
	if (user->temp_desc[0] != '\0')
	{
		strcpy(user->desc,user->temp_desc);
		user->temp_desc[0] = '\0';
	}
	SaveUser(user,1);
	destruct_bjgame(user);
	if (user->chess != NULL) kill_chess(user);
	if (user->checkers != NULL) checkers_kill(user);
	writeus(user,"\n\t  ~FGGoodbye, ~CR%s~FG. I hope you had a great time...c'ya later.\n",user->recap);
	sprintf(file,"%s/%s.ASC",ScreenDir,CloseScreen);
	rows = user->rows;	user->rows = 0;
	more(user,user->socket,file,0);
	user->rows = rows;
	destruct_alpha(user);
	if (user->tbuff) delete_convbuff(user->tbuff);
	if (user->away_buff) delete_convbuff(user->away_buff);
	logging_in = 1;
	if (user->port != port[1] && type == 1)
	{
		switch (user->login_type)
		{
			case 1 :
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s ~FM<=-\n",logout_inv,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s ~FR%s ~FM<=-\n",logout_inv,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s ~FM<=-\n",logout_inv,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FR%s ~FM<=-\n",logout_inv,user->recap,user->desc);
			  }
			  write_level(WIZ,user,text);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s] [INVIS]\n",user->name);
			  break;
			case 2 :
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s ~FM<=-\n",logout_hide,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s ~FR%s ~FM<=-\n",logout_hide,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s ~FM<=-\n",logout_hide,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FR%s ~FM<=-\n",logout_hide,user->recap,user->desc);
			  }
			  write_level(ADMIN,user,text);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s] [HIDDEN]\n",user->name);
			  break;
			case 3 :
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s ~FM<=-\n",logout_fake,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s ~FR%s ~FM<=-\n",logout_fake,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s ~FM<=-\n",logout_fake,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FR%s ~FM<=-\n",logout_fake,user->recap,user->desc);
			  }
			  write_level(GOD,user,text);
			  break;
			default:
			  WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FT%s\n",user->recap,user->logout_msg);
			  if (user->predesc[0])
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s ~FG%s~FR%s ~FM<=-\n",logout_vis,user->predesc,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FG%s ~FR%s ~FM<=-\n",logout_vis,user->predesc,user->recap,user->desc);
			  }
			  else
			  {
				if (user->desc[0] == '\'') sprintf(text,"%s ~FG%s~FR%s ~FM<=-\n",logout_vis,user->recap,user->desc);
				else sprintf(text,"%s ~FG%s ~FR%s ~FM<=-\n",logout_vis,user->recap,user->desc);
			  }
			  write_room_except(NULL,user,text);
			  friend_check(user,2);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s]\n",user->name);
			  break;
		}
	}
	/* User lost socket */
	else if (type == 2)
	{
		if (user->predesc[0] != '\0')
		{
			if (user->desc[0] == '\'') sprintf(text,"~CB[~FTLost-Socket~CB] %s %s%s\n",user->predesc,user->recap,user->desc);
			else sprintf(text,"~CB[~FTLost-Socket~CB] %s %s %s\n",user->predesc,user->recap,user->desc);
		}
		else
		{
			if (user->desc[0] == '\'') sprintf(text,"~CB[~FTLost-Socket~CB] %s%s\n",user->recap,user->desc);
			else sprintf(text,"~CB[~FTLost-Socket~CB] %s %s\n",user->recap,user->desc);
		}
		switch(user->invis)
		{
			case 1 :
			  write_level(WIZ,user,text);
			  break;
			case 2 :
			  write_level(ADMIN,user,text);
			  break;
			case 3 :
			  write_level(GOD,user,text);
			  break;
			default:
			  write_room_except(NULL,user,text);
			  friend_check(user,2);
			  write_log(1,LOG2,"[LOGOFF] - User: [%s]\n",user->name);
			  break;
		}
	}
	logging_in = 0;
	num_of_users--;
	if (user->editor.malloc_start != NULL) free(user->editor.malloc_start);
	if ((e = get_editor(user)) != NULL) destruct_editor(e);
	if ((t = get_telnet(user)) != NULL) disconnect_telnet(t);
	if (fight.first_user == user || fight.second_user == user)
	{
		fight.first_user = NULL;
		fight.second_user = NULL;
	}
	destroy_user_clones(user);
	close(user->socket);
	destruct_user(user);
	rm = get_room(rmname);
	if (rm != NULL) destruct_room(rm);
	if (webpage_on) who_webpage();
	return;
}

/* END OF CONNECT.C */
