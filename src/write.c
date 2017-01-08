/*
  write.c
    Source code for writing data to either a user, a room, or to a
    level. Also contains the majority of the code for handling of the
    split screen. (Thanks to Wookey for this). This file is one of the
    essential files for the RaMTITS source.

    The source within this file is Copyright 1998 - 2001 by
 	Rob Melhuish, Tim van der Leeuw, Andrew Collington, and	Neil
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
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/telnet.h>
#define _WRITE_SOURCE
  #include "include/needed.h"
#undef _WRITE_SOURCE
#include "include/writelib.h"

/*
  Write data over a socket, splitting screens when wanted by the user. There
  are 3 options for identifying when to write the data: 0-default, 1-input
  echo 2-prompt, ignored when not in split screen mode. When in split-screen
  mode, at the end of writing data the cursor should always be in the bottom
  panel, the cursor is assumed to be in the bottom pane always. For input
  echo, the cursor is therefore not moved. Default writes are cached. The
  cache is flushed when the total size would overflow the cache size, or
  when a write for another user is done. Also when NOT in split-screen mode
  the cache is flushed when an input echo is written.
  NOTE: Flushing the cache can be achieved by passing NULL as the user.
    Original functions written by Wookey for IFORMS (converted to RaMTITS /
    NUTS by me Rob Melhuish (Squirt)) I kept his original comments in so
    those looking at this code won't get lost too easily.. including me ;)
*/
int write_window(User user,char *str,int len,int where)
{
	/* static variables for cache writing */
	User lastu = NULL;
	static char cachedata[CACHE_SIZE+500]; /* one full screen.. more would be useless */
	static int cachedatalen,lastuid;
	int bytessent=0;  /* # of bytes written/cached */
	int flush=0,dowrite=0,promptlen;
	int ptop,pbottom,pcurrent=WriteInputEcho;
	char prompt[500];

	/* If writing the prompt, write it, and return, don't touch the cache */
	if (where == WritePrompt)
	{
		if (user->splitseffect && len)
		{
			int sock = user->socket; /* ease of writing */
			save_cursor(sock);
			get_pane_size(user,where,&ptop,&pbottom);
			set_pane(sock,ptop,pbottom);
			set_cursor(sock,pbottom,1);
			pcurrent = where;
			promptlen = sprintf(prompt,"%s",str);
			write_socket(sock,prompt,promptlen);
			write_socket(sock,"\033[0m",4);
			get_pane_size(user,WriteInputEcho,&ptop,&pbottom);
			set_pane(sock,ptop,pbottom);
			restore_cursor(sock);
			pcurrent = WriteInputEcho;
		}
		else promptlen = 0;
		write_socket(-1,NULL,0);
		return(promptlen);
	}
	if ((lastu = get_userid(lastuid)) == NULL) lastu = NULL;
	/* Determine flags what to do */
	if (user != lastu || cachedatalen+len >= CACHE_SIZE
	  || where == WriteDirect || where == WriteInputEcho)
	{
		flush = (cachedatalen>0 && lastu != NULL)?1:0;
		dowrite=(len>0 && user != NULL && str != NULL)?1:0;
	}
	/*
	  WriteDirect is a variation on WriteDefault. For the rest of the
	  code it is easier to change the value at this point.
	*/
	if (where == WriteDirect) where = WriteDefault;
	/* Do the stuff.. print cache if needed */
	if (flush)
	{
		if (lastu->splitseffect)
		{
			save_cursor(lastu->socket);
			get_pane_size(lastu,WriteDefault,&ptop,&pbottom);
			set_pane(lastu->socket,ptop,pbottom);
			pcurrent = WriteDefault;
			set_cursor(lastu->socket,pbottom,1);
		}
		write_socket(lastu->socket,cachedata,cachedatalen);
		cachedatalen = 0;
		/*
		  Restore input pane IF:
		   -- Cached data were not for this user, OR
		   -- No data is to be written.
		*/
		if ((lastu != user || !dowrite) && lastu->splitseffect)
		{
			get_pane_size(lastu,WriteInputEcho,&ptop,&pbottom);
			set_pane(lastu->socket,ptop,pbottom);
			pcurrent = WriteInputEcho;
			restore_cursor(lastu->socket);
		}
	}
	if (dowrite)
	{
		/* Set up the screen panes */
		if (where != pcurrent && user->splitseffect)
		{
			if (pcurrent == WriteInputEcho) save_cursor(user->socket);
			get_pane_size(user,where,&ptop,&pbottom);
			set_pane(user->socket,ptop,pbottom);
			if (where == WriteInputEcho) restore_cursor(user->socket);
			else set_cursor(user->socket,pbottom,1);
			pcurrent = where;
		}
		bytessent = write_socket(user->socket,str,len);
		/* If we're not in the input pane, restore it */
		if (user->splitseffect && pcurrent != WriteInputEcho)
		{
			get_pane_size(user,WriteInputEcho,&ptop,&pbottom);
			set_pane(user->socket,ptop,pbottom);
			restore_cursor(user->socket);
			pcurrent = WriteInputEcho;
		}
	}
	else if (str && len>0)  /* Don't write this data, so cache it! */
	{
		memcpy(cachedata+cachedatalen,str,len);
		cachedatalen += len;
		bytessent = len;
	}
	if (user != NULL) lastuid = user->id;
	if (flush || dowrite)
	{
		write_socket(-1,NULL,0); /* Flush the cache */
	}
	return(bytessent);
}

/*
  I thought that with write_window I had done enough, but it turns out I
  hadn't. The cursor was jumpy. It needed to be smoothened. I realized
  the cursor was jumpy because of all the writes were done to set up the
  panes, save the cursor, and restore it later. In the time it took for
  all those packets to arrive, I could type several characters.
  The solution then was to send everything in one single write() including
  all the escape codes and everything. The cleanest way to do that,
  was to cache everything yet another time! In all the code, this
  function write_socket() is now the only function allowed to send data
  to a socket. Call it just like you would call write() on a socket.
  Better of course is to call write_window() because that is at a slight
  higher level and knows about splitscreens. And if you don't wish to
  know about split screens and just want an easier write to a user, call
  write_user.  NOTE: Flush the cache by calling the function with a -1
 		value for the socket or a NULL pointer for the data.
*/
int write_socket(int sock,char *str,int len)
{
	static int lastsock=-1,cachedatalen;
	static char cachedata[9700*2]; /* This need to be BIG */
	/* Internal flags */
	int flush=0,docache=0;

	if (len+cachedatalen>9700*2 || lastsock != sock || str == NULL)
	{
		flush = (lastsock != -1 && cachedatalen>0)?1:0;
	}
	if (sock != -1 && str != NULL && len>0)
	{
		docache = 1;
	}
	if (flush)
	{
		write(lastsock,cachedata,cachedatalen);
		cachedatalen = 0;
	}
	if (docache)
	{
		/* One final check to see if data will not overflow */
		if (len+cachedatalen>9700*2)
		{
			write(sock,str,len);
		}
		else
		{
			memcpy(cachedata+cachedatalen,str,len);
			cachedatalen += len;
		}
	}
	lastsock = sock;
	return(len);
}

/* Support functions for write_window() */
void get_pane_size(User user,int pane,int *top,int *bottom)
{
	int rows;

	if (user->rows<15 || user->rows>150) rows = 23;
	else rows = user->rows;
	switch (pane)
	{
		case WriteDefault:
		  *top = 1;
		  *bottom = rows-4;
		  break;
		case WriteInputEcho:
		  *top = user->rows-2;
		  *bottom = rows;
		  break;
		case WritePrompt:
		  *top = user->rows-3;
		  *bottom = rows-3;
		  break;
	}
}

/* sets the scrolling region */
void set_pane(int sock,int top,int bottom)
{
	char buff[20];
	int len;

	len = sprintf(buff,VT100_SET_REGION,top,bottom);
	write_socket(sock,buff,len);
}

/* Save/restore VT100 cursor position */
void save_cursor(int sock)
{
	write_socket(sock,VT100_CURSOR_SAVE,VT100_CURSOR_SAVE_LEN);
}

void restore_cursor(int sock)
{
	write_socket(sock,VT100_CURSOR_RESTORE,VT100_CURSOR_RESTORE_LEN);
}

/* Set the cursor to somewhere on the screen */
void set_cursor(int sock,int line,int col)
{
	char buff[20];
	int len;

	len = sprintf(buff,VT100_CURSOR_SET,line,col);
	write_socket(sock,buff,len);
}

/* More split-screen stuff */
void splitscreen_on(User user)
{
	int sock=user->socket;
	int ptop,pbottom;

	if (user == NULL) return;
	if (user->login) return;
	user->splitseffect = 1;
	show_prompt(user);
	get_pane_size(user,WriteInputEcho,&ptop,&pbottom);
	set_pane(sock,ptop,pbottom);
	set_cursor(sock,ptop,1);
	write_socket(sock,"\033[J",4);
	write_socket(-1,NULL,0);
}

void splitscreen_off(User user)
{
	int sock = user->socket,ptop,pbottom,rows;

	if (user == NULL) return;
	if (user->login) return;
	if (user->rows < 15 || user->rows > 150) rows = 25;
	else rows = user->rows;
	user->splitseffect = 0;
	get_pane_size(user,WriteDefault,&ptop,&pbottom);
	set_pane(sock,1,rows);
	set_cursor(sock,pbottom+1,1);
	write_socket(sock,"\033[J",4);
	write_socket(-1,NULL,0);
}

void show_prompt(User user)
{
	char prompt[500],name[UserNameLen+2];
	char editprompt[15];
	int idle,mins,len;

	if (!user->splitseffect || user->login) return;
	editprompt[0] = '\0';
	mins = (int)(time(0)-user->last_login)/60;
	idle = (int)(time(0)-user->last_input)/60;
	if (user->invis>Invis) strcpy(name,ColourStrip(user->temp_recap));
	else strcpy(name,ColourStrip(user->recap));
	if (user->editor.editing)
	{
		switch(user->misc_op)
		{
			case 3 :
			  strcpy(editprompt,"message board ");
			  break;
			case 4 :
			  strcpy(editprompt,"s-mail        ");
			  break;
			case 5 :
			  strcpy(editprompt,"profile       ");
			  break;
			case 7 :
			  strcpy(editprompt,"news          ");
			  break;
			case 8 :
			  strcpy(editprompt,"room desc.    ");
			  break;
			case 9 :
			  strcpy(editprompt,"suggestion    ");
			  break;
			case 10:
			  strcpy(editprompt,"friend s-mail ");
			  break;
			case 11:
			  strcpy(editprompt,"staff s-mail  ");
			  break;
			case 12:
			  strcpy(editprompt,"bug           ");
			  break;
			case 21:
			  strcpy(editprompt,"user s-mail   ");
			  break;
			case 22:
			  strcpy(editprompt,"vote topic    ");
			  break;
			default:
			  strcpy(editprompt,"editor        ");
			  break;
		}
		if (user->colour) sprintf(prompt,CEDDIVIDOR,name,editprompt);
		else sprintf(prompt,EDDIVIDOR,name,editprompt);
		len = strlen(prompt);
		write_window(user,prompt,len,WritePrompt);
		return;
	}
	if (user->colour) sprintf(prompt,CDIVIDOR,name,mins,idle);
	else sprintf(prompt,DIVIDOR,name,mins,idle);
	len = strlen(prompt);
	write_window(user,prompt,len,WritePrompt);
	return;
}

/* User prompt */
void prompt(User user)
{
	Room rm;
	char vis[30];

	if (user == NULL || destructed) return;
	rm = user->room;
	if (user->login) return;
	if (no_prompt) return;
	if (user->splits)
	{
		show_prompt(user);
		return;
	}
	vis[0] = '\0';
/*
	if (user->mode && !is_busy(user))
	{
		if (user->invis>Invis) writeus(user,"~CB[~FT%s~CB]:~RS ",user->temp_recap);
		else writeus(user,"~CB[~FT%s~CB]:~RS ",user->recap);
		return;
	}
*/
	if (!user->prompt || user->misc_op || user->set_op || user->bank_op 
	  || user->store_op || user->game_op || user->editor.edit_op
	  || user->mail_op) return;
	switch (user->invis)
	{
		case 1 :
		  strcpy(vis,"~CB[~FTInvis~CB]");
		  break;
		case 2 :
		  strcpy(vis,"~CB[~FTHidden~CB]");
		  break;
		case 3 :
		  strcpy(vis,"~CB[~FTFake logged off~CB]");
		  break;
		default:
		  vis[0] = '\0';
		  break;
	}
	writeus(user,"~CB[~FM%s %s %d ~FT%d:%2.2d~CB] %s\n",day[twday],month[tmonth],tmday,thour,tmin,vis);
	return;
}

/* Tell telnet not to echo characters - for password entry. */
void echo_off(User user)
{
	char seq[4];

	if (password_echo) return;
	sprintf(seq,"%c%c%c",255,251,1);
	write_user(user,seq);
}

/* Now we tell it to echo characters. */
void echo_on(User user)
{
	char seq[4];

	if (password_echo) return;
	sprintf(seq,"%c%c%c",255,252,1);
	write_user(user,seq);
}

/* write something out to the user.. */
void write_user(User user,char *str)
{
	int buffpos,i,cnt,mrc,cols,sock,space,tcnt,hs,wrapped;
	char *start,buff[OutBuffSize+5],*s=NULL;

	if (user == NULL) return;
	if (strchr(str,'$') || contains_swearing(str))
	{
		str = parse_string(user,str);
	}
	start = str;
	buffpos = cnt = tcnt = space = cols = hs = wrapped = 0;
	sock = user->socket;
	*buff = '\0';
	if (user->cols == -1) cols = -1;
	else if (user->cols<15 || user->cols>512) cols = 80;
	else cols = user->cols;
	while (*str)
	{
		if (*str == '\n')
		{
			if (buffpos>OutBuffSize-6)
			{
				if (user->login || user->login_type)
				{
					write_window(user,buff,buffpos,WriteDirect);
				}
				else write_window(user,buff,buffpos,WriteDefault);
				buffpos=space=cnt=tcnt=wrapped=0;
			}
			if (user->colour)
			{
//				memcpy(buff+buffpos,"\033[0m",4);  buffpos += 4;
				memmove(buff+buffpos,"\033[0m",4);  buffpos += 4;
			}
			*(buff+buffpos) = '\n';
			if (user->car_return == 1) *(buff+buffpos+1) = '\r';
			buffpos += 2;  ++str;  cnt=wrapped=space=0;
		}
		else
		{
			if (cols != -1)
			{
				if (*str == ' ')
				{
					s=str;  tcnt=cnt;  space = buffpos;  hs = 1;
					while (!isspace(*s))
					{
						++tcnt;
						if (tcnt >= cols)
						{
							break;
						}
						else continue;
					}
				}
				if (cnt >= cols)
				{
					if (hs == 1)
					{
						str = s+1;
						buffpos = space;
					}
					*(buff+buffpos) = '\n';
					if (user->car_return == 1)
					  *(buff+buffpos+1) = '\r';
					buffpos += 2;  space=tcnt=cnt=hs=0;
				}
			}
			if (*str == '^' && *(str+1) == '~')
			{
				++str;
				continue;
			}
			if (str != start && *str == '~' && *(str-1) == '^')
			{
				*(buff+buffpos) = *str;  goto CONT;
			}
			/* Process colour commands. */
			if (*str == '~')
			{
				if (buffpos>OutBuffSize-6)
				{
					if (user->login || user->login_type)
					  write_window(user,buff,buffpos,WriteDirect);
					else write_window(user,buff,buffpos,WriteDefault);
					buffpos=cnt=tcnt=space=0;
				}
				++str;
				if (!strncmp(str,"LI",2))
				{
					if (user->ignore & Blinks)
					{
						++str;  ++str;  ++str;  --cnt;
					}
				}
				for (i=0 ; i<NumCols ; ++i)
				{
					if (!strncmp(str,colcom[i],2))
					{
						if (user->colour)
						{
//							memcpy(buff+buffpos,colcode[i],strlen(colcode[i]));
							memmove(buff+buffpos,colcode[i],strlen(colcode[i]));
							buffpos += strlen(colcode[i])-1;
						}
						else buffpos--;
						++str;  --cnt;
						goto CONT;
					}
				}
				if (!strncmp(str,"RD",2))
				{
					mrc = ran(NumRand);
					if (user->colour)
					{
//						memcpy(buff+buffpos,rcolour[mrc],strlen(rcolour[mrc]));
						memmove(buff+buffpos,rcolour[mrc],strlen(rcolour[mrc]));
						buffpos += strlen(rcolour[mrc])-1;
					}
					else buffpos--;
					++str;  --cnt;
					goto CONT;
				}
				*(buff+buffpos) = *(--str);
			}
			else *(buff+buffpos) = *str;
			CONT:
			  ++buffpos;  ++str;  ++cnt;
		}
		if (buffpos == OutBuffSize)
		{
			if (user->login || user->login_type) write_window(user,buff,buffpos,WriteDirect);
			else write_window(user,buff,buffpos,WriteDefault);
			buffpos=space=cnt=tcnt=wrapped=0; 
		}
	}
	if (buffpos)
	{
		if (user->login || user->login_type) write_window(user,buff,buffpos,WriteDirect);
		else write_window(user,buff,buffpos,WriteDefault);
	}
	if (user->colour) write_sock(sock,"\033[0m");
}

/* Write a NULL terminated string to a socket */
void write_sock(int sock,char *str)
{
	write_socket(sock,str,strlen(str));
}

/*
   A different kind of write_() so you can write to a user with arguments
   such as writeus(user,"%s says hi\n",user->name); do this to get rid of
   a shitload of unneeded sprintf()'s
*/
void writeus(User user,char *str, ...)
{
	va_list opts;

	vtext[0] = '\0';		va_start(opts,str);
	vsprintf(vtext,str,opts);	va_end(opts);
	if (user->type == CloneType) write_user(user->owner,vtext);
	else write_user(user,vtext);
}

/* Does the same as about.. except does it for writing to a room. */
void WriteRoom(Room rm,char *str, ...)
{
	va_list opts;

	vtext[0] = '\0';		va_start(opts,str);
	vsprintf(vtext,str,opts);	va_end(opts);
	write_room_except2(rm,NULL,NULL,vtext);
}

/* And also one for write_room_except.. */
void WriteRoomExcept(Room rm,User user,char *str, ...)
{
	va_list opts;

	vtext[0] = '\0';		va_start(opts,str);
	vsprintf(vtext,str,opts);	va_end(opts);
	write_room_except2(rm,user,NULL,vtext);
}

/* Finally.. one for write_level :-) */
void WriteLevel(int lev,User user,char *str, ...)
{
	va_list opts;

	vtext[0] = '\0';		va_start(opts,str);
	vsprintf(vtext,str,opts);	va_end(opts);
	write_level(lev,user,vtext);
}

/* writes to level of lev and above */
void write_level(int lev,User user,char *str)
{
	User u;

	for_users(u)
	{
		if (lev >= WIZ && (u->ignore & Wizard)) continue;
		if ((u->ignore & Logons) && logging_in) continue;
		if (is_enemy(u,user)) continue;
		if ((u->ignore & Channels) && (com_num == get_comnum("channel")
		  || com_num == get_comnum("chemote")
		  || com_num == get_comnum("session")
		  || com_num == get_comnum("comment"))) continue;
		if (u != user && u->level >= lev && u->login == 0
		  && u->type != CloneType)
		{
			if (is_busy(u))
			{
				record_away(u,str);
				continue;
			}
		}
		if (u->ignall && force_listen == 0) continue;
		if (u != user && u->level >= lev && u->login == 0
		  && u->type != CloneType)
		{
			write_user(u,str);
		}
	}
}

/* Subsid function to write_room_except2, except this one's used more. */
void write_room(Room rm,char *str)
{
	write_room_except2(rm,NULL,NULL,str);
}

/* Subsid function to below.. */
void write_room_except(Room rm,User u,char *str)
{
	write_room_except2(rm,u,NULL,str);
}

/*
   Writes to everyone in rm except for u2, and u.
   If rm is NULL write to ALL rooms.
*/
void write_room_except2(Room rm,User u,User u2,char *str)
{
	User us;
	char text2[ARR_SIZE*2];

	text2[0] = '\0';
	for_users(us)
	{
		if (us->login || us->room == NULL) continue;
		if (us->room != rm && rm != NULL) continue;
		if (((us->ignore & Shouts) && rm == NULL)
		  && (com_num == get_comnum("shout")
		  || com_num == get_comnum("semote")
		  || com_num == get_comnum("shecho")
		  || com_num == get_comnum("sthink")
		  || com_num == get_comnum("ssing")
		  || com_num == get_comnum("welcome")
		  || com_num == get_comnum("yell"))) continue;
		if (us == u || us == u2) continue;
		if ((us->ignore & Logons) && logging_in) continue;
		if (is_enemy(us,u) || is_enemy(us,u2)) continue;
		if (is_busy(us) && (us != u || us != u2)
		  && us->type != CloneType)
		{
			record_away(us,str);
			continue;
		}
		if (us->ignall && force_listen == 0) continue;
		if (us->type == CloneType)
		{
			if (is_busy(us->owner)
			  && (us != u || us != u2)) continue;
			if (us->clone_hear == Clone_Nothing
			  || us->owner->ignall) continue;
			if (rm != us->room) continue;
			if (us->clone_hear == Clone_Swears)
			{
				if (!contains_swearing(str)) continue;
			}
			sprintf(text2,"~CB[~FM%s~CB]:~RS %s",us->room->recap,str);
			write_user(us->owner,text2);
		}
		else write_user(us,str);
	}
}

/* Writes the users name to the monitor. */
void write_monitor(User user,int rec,Room rm)
{
	User u;
	Commands com = get_comname("monitor");

	if (com == NULL)
	{
		write_user(user,"~CW- ~FTThere was an error writing to the monitor.\n");
		return;
	}
	sprintf(text,"~CB[~FT%s~CB]:~RS ",user->recap);
	for_users(u)
	{
		if (u == user || u->login || u->type == CloneType) continue;
		if (u->level<com->min_lev || u->monitor == 0) continue;
		if (is_enemy(u,user)) continue;
		if (is_busy(u) && (u->room == rm || rm == NULL))
		{
			if (rec) record_tell(u,text);
			record_away(u,text);
			continue;
		}
		if (u->ignall && force_listen == 0) continue;
		if (u->room == rm || rm == NULL)
		{
			if (rec) record_tell(u,text);
			write_user(u,text);
		}
	}
}

/* Write a string to the specified log. */
void write_log(int write_time,int logfile,const char *str, ...)
{
	FILE *fp;
	va_list opts;
	char buff[ARR_SIZE],filename[FSL];

	va_start(opts,str);
	vsprintf(buff,str,opts);
	va_end(opts);
	switch (logfile)
	{
		case LOG1:
		  sprintf(filename,"%s/%s",SysLogDir,SysLog);
		  break;
		case LOG2:
		  sprintf(filename,"%s/%s",SysLogDir,LoginLog);
		  break;
		case LOG3:
		  sprintf(filename,"%s/%s",SysLogDir,EmailLog);
		  break;
		case LOG4:
		  sprintf(filename,"%s/%s",SysLogDir,StaffLog);
		  break;
		case LOG5:
		  sprintf(filename,"%s/%s",SysLogDir,WWWLog);
		  break;
		case LOG6:
		  sprintf(filename,"%s/%s",SysLogDir,TelnetLog);
		  break;
		default  :
		  sprintf(filename,"%s/%s",SysLogDir,SysLog);
		  break;
	}
	if ((fp = fopen(filename,"a")) == NULL) return;
	if (write_time == 0) fputs(buff,fp);
	else fprintf(fp,"%s %s",long_date(2),buff);
	FCLOSE(fp);
}

/* writes a string to a users record */
void write_record(User user,int write_time,const char *str, ...)
{
	FILE *fp;
	va_list opts;
	char buff[ARR_SIZE],filename[FSL];

	va_start(opts,str);
	vsprintf(buff,str,opts);
	va_end(opts);
	sprintf(filename,"%s/%s/%s.R",UserDir,UserMisc,user->name);
	if ((fp = fopen(filename,"a")) == NULL) return;
	if (write_time == 0) fputs(buff,fp);
	else fprintf(fp,"%s %s",long_date(2),buff);
	FCLOSE(fp);
}

/*
   This will write to the users who are on the user who is calling the
   function correspondings friend list. The idea was taken from amnuts
   as well as alot of the functions relating to the friends to which
   I give thanks to Andy for.
*/
void write_friends(User user,char *str)
{
	User u;

	for_users(u)
	{
		if (u->login || u == NULL || u->ignall || u->room == NULL
		  || u->type == CloneType || is_enemy(u,user)
		  || (u->ignore & Friends)
		  || (is_friend(user,u) == 0)) continue;
		if (is_busy(u))
		{
			record_away(u,str);
			continue;
		}
		write_user(u,str);
		record_tell(u,str);
	}
}

/*
   Page a file out to a user. We'll only put the colour coms into the file
   if user==NULL and colour_def is set to ON. This started out as the
   basic NUTS more() but I've made extensive mods to it, to which I'd like
   to thank Arny, and Andy for helping me. This function returns three
   types of values: 0 - can't find file 1 - found, 2 - found and finished
*/
int more(User user,int sock,char *filename,int type)
{
	FILE	*fp;
	int	i, lines, retval, len, maxlines, cnt, temp, fsize, cols;
	char	filename2[FSL], text2[ARR_SIZE];
	struct stat fb, mfb;

	if (user == NULL) return(0);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		user->pager.filepos = 0;	user->pager.page_file[0] = '\0';
		user->pager.last_page = -1;	user->pager.page_num = 1;
		user->pager.start_page = 0;	user->pager.lines = 0;
		user->pager.type = 0;		user->pager.pdone = 0;
		for (i = 0 ; i<MaxPages ; ++i) user->pager.pages[i] = 0;
		return(0);
	}
	if (stat(filename,&fb) == -1) fsize = 0;
	else fsize = fb.st_size;
	if (fsize == 0) return(0);
	/* Jump to reading position in file */
	if (user->rows)
	{
		if (user->pager.start_page == 0) fill_pages(user,filename);
		user->pager.filepos = user->pager.pages[user->pager.page_num];
		fseek(fp,user->pager.filepos,0);
	}
	maxlines = 0;
	if (user->pager.start_page == 0)
	{
		if (user->rows == 0) maxlines = 24;
		else maxlines = user->rows;
		if (maxlines<15 || maxlines>99) maxlines = 24;
		if (user->splits) maxlines -= 3;
	}
	else maxlines = user->pager.lines;
	text[0] = '\0';	retval = 1;	len = 0;
	cnt = 0;	lines = 0;	temp = 0; cols = user->cols;
	fgets(text,sizeof(text)-1,fp);
	while (!feof(fp) && (lines<maxlines || user->rows == 0))
	{
		write_user(user,text);
		lines++;
		fgets(text,sizeof(text)-1,fp);
	}
	/* If user is logging in, don't page file */
	if (user->rows == 0)
	{
		FCLOSE(fp);
		if (user->login == 0)
		{
			write_socket(user->socket,"\n",1);
			return(2);
		}
		RemoveSysfiles(user);
		if (type == 1)
		{
			sprintf(filename2,"%s/%s/%s.M",UserDir,UserMesg,user->name);
			if (stat(filename2,&mfb) == -1) fsize = 0;
			else fsize = mfb.st_size;
			writeus(user,"~FG In total your s-mail box is allocating ~CY%d ~FGbytes, out of a max. of ~CT%d~FG.\n",fsize,MaxMbytes);
			sprintf(filename2,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
			unlink(filename2);
		}
		write_socket(user->socket,"\n",1);
		return(2);
	};
	user->pager.type = type;
	strcpy(user->pager.page_file,filename);
	temp = user->pager.page_num+1;
	user->pager.pdone = (float)((int)user->pager.pages[temp]*100)/(int)fsize;
	if (user->pager.page_num != user->pager.last_page)
	{
		sprintf(text2,PagerPrompt1,user->pager.pdone,user->pager.page_num,user->pager.last_page);
	}
	else sprintf(text2,PagerPrompt2,user->pager.page_num,user->pager.last_page);
	write_user(user,text2);
	no_prompt = 1;
	FCLOSE(fp);
	return(retval);
}

/* This we go through and fill up the pager pages so we know where to jump to */
void fill_pages(User user,char *filename)
{
	FILE *fp;
	int lines,i;

	if ((fp = fopen(filename,"r")) == NULL) return;
	text[0] = '\0';
	lines = 0;
	user->pager.last_page = 1;
	if (user->mail_op == 0)
	{
		user->ignall_store = user->ignall;
		user->ignall = 1;
	}
	for (i = 1 ; i<MaxPages ; ++i) user->pager.pages[i] = 0;
	user->pager.start_page = 1;
	/* Go through and fill it up :) */
	fgets(text,sizeof(text)-1,fp);
	while (!feof(fp))
	{
		lines++;
		if (lines >= user->rows)
		{
			lines = 0;
			user->pager.pages[++user->pager.last_page] = ftell(fp);
		}
		fgets(text,sizeof(text)-1,fp);
	}
	user->pager.lines = user->rows;
	FCLOSE(fp);
}

/* View an ansi screen. */
int show_screen(User user)
{
	FILE *fp;
	char filename[FSL],line[ARR_SIZE];
	int retcode,ansi,view;

	retcode = ansi = view = 0;
	sprintf(filename,"%s/%s.%s",ScreenDir,file,file_ext[1]);
	ansi = file_exists(filename);
	if (user->ansi_on && ansi)
	{
		sprintf(filename,"%s/%s.%s",ScreenDir,file,file_ext[user->ansi_on]);
		view = 1;
	}
	else sprintf(filename,"%s/%s.%s",ScreenDir,file,file_ext[0]);
	if (view)
	{
		retcode = view_file(user,filename);
		return(retcode);
	}
	if ((fp = fopen(filename,"r")) == NULL) return(0);
	fgets(line,ARR_SIZE-1,fp);
	while (!feof(fp))
	{
		write_user(user,line);
		fgets(line,ARR_SIZE-1,fp);
	}
	FCLOSE(fp);
	return(1);
}

/* prints out a file one char at a time. */
int view_file(User user,char *filename)
{
	FILE *fp;
	char fname[FSL],line[ARR_SIZE];

	strcpy(fname,filename);
	if ((fp = fopen(fname,"r")) == NULL) return(0);
	fgets(line,ARR_SIZE-1,fp);
	while (!(feof(fp)))
	{
		write_window(user,line,strlen(line),WriteDefault);
		fgets(line,ARR_SIZE-1,fp);
	}
	FCLOSE(fp);
	write_window(user,"\13\033[0m",strlen("\13\033[0m"),WriteDefault);
	return(1);
}

/* Fix for telnet ctrl-c and ctrl-d *arctic9* */
void will_time_mark(User user)
{
	char seq[4];

	sprintf(seq,"%c%c%c",255,251,6);
	write_user(user,seq);
}

void init_inputbuffer(User user)
{

	if ((user->buffer = (Input)malloc(sizeof(struct input_buff))) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Failed to allocate memory for input buffer.\n");
		return;
	}
	user->buffer->telnet_mode = 0;
	user->buffer->telnet_opt = 0;
	user->buffer->telnet_cmd = 0;
	user->buffer->buff = user->buffer->buffer;
	user->buffer->start_buff = user->buffer->buffer;
	user->buffer->buff_max = user->buffer->buffer+ARR_SIZE;
	user->buffer->buffer[0] = 0;
}

/*
   Internal function that adds a character to an input buffer, does the
   real work of addcharto_inputbuff and addstringto_inputbuff
*/
int add_char_internal(User user,char ch)
{
	static int skiplf=0;

	if (user->buffer == NULL && (destructed == 0 || disconnecting == 0))
	{
		init_inputbuffer(user);
	}
	if (skiplf)
	{
		skiplf = 0;
		if (ch == '\n') return(0);
	}
	/* simple emulations for char-mode input */
	if (ch == 8 || ch == 127)		/* Back space */
	{
		remove_from_inputbuff(user,1);
		return(0);
	}
	else if (ch == 21)			/* ctrl-u kill line */
	{
		clear_inputbuff(user);
		write_user(user,"");
		return(0);
	}
	else if (ch == 12 || ch == 18)	/* ctrl-r or ctrl-l redraw line */
	{
		write_user(user,"");
		write_sock(user->socket,user->buffer->start_buff);
		return(0);
	}
	else if (ch == '\r')		/* rebuild car. return to lf */
	{
		ch = '\n';
		skiplf = 1;
	}
	else if ((unsigned char)ch == IAC)	/* Telnet commands */
	{
		user->buffer->telnet_mode = 1;
		user->buffer->telnet_opt = 0;
		user->buffer->telnet_cmd = 0;
		return(0);
	}
	else if (user->buffer->telnet_mode)
	{
		if (!user->buffer->telnet_cmd)
		{
			user->buffer->telnet_cmd = (unsigned char)ch;
		}
		else
		{
			user->buffer->telnet_opt = (unsigned char)ch;
			log_telnetseq(user);
			user->buffer->telnet_mode = 0;
			user->buffer->telnet_opt = 0;
			user->buffer->telnet_cmd = 0;
		}
		return(0);
	}
	if (user->buffer->buff >= user->buffer->buff_max)
	{
		write_user(user,"~FG Buffer overload.. careful what you do next time eh.\n");
		write_log(1,LOG1,"[BUFFER OVERLOAD] - User: [%s] Site: [%s:%s]\n",user->name,user->site,user->ip_site);
		*(user->buffer->buff) = '\n';
		return(1);
	}
	*(user->buffer->buff)++ = ch;
	*(user->buffer->buff) = '\0';
	return(0);
}

int addstring_tobuff(User user,char *str,int len)
{

	while (len)
	{
		if (add_char_internal(user,*str++)) return(1);
		--len;
	}
	return(0);
}

/* Remove last num characters from the input buffer */
void remove_from_inputbuff(User user,int num)
{

	if (num<0)
	{
		num = -num;
	}
	if (user->buffer != NULL)
	{
		user->buffer->buff -= num;
		if (user->buffer->buff<user->buffer->buffer)
		{
			user->buffer->buff = user->buffer->buffer;
		}
		if (user->buffer->start_buff>user->buffer->buff)
		{
			user->buffer->start_buff = user->buffer->buff;
		}
		*(user->buffer->buff) = 0;
	}
}

int getline_from_inputbuff(User user,char *dest)
{
	register char *in,*out=dest,*buff_end=user->buffer->buff;

	if (user == NULL)
	{
		return(1);
	}
	if (destructed)
	{
		return(1);
	}
	if (user->buffer == NULL)
	{
		return(1);
	}
	in = user->buffer->start_buff;
	while (in<buff_end && *in != '\n')
	{
		*out++ = *in++;
	}
	*out++	= *in++;
	*out++	= 0;
	*out	= 0;
	user->buffer->start_buff = in;
	if (user->buffer->start_buff >= user->buffer->buff)
	{
		return(1);
	}
	return(0);
}

int newline_inbuff(int user_id)
{
	User user;
	char *in;

	if ((user = get_userid(user_id)) == NULL) return(0);
	if (user == NULL) return(0);
	if (destructed == 1) return(0);
	if (user->buffer == NULL) return(0);
	in = user->buffer->start_buff;
	while (in<user->buffer->buff && *in != '\n')
	{
		++in;
	}
	if (*in == '\n')
	{
		return(1);
	}
	return(0);
}

void release_inputbuff(User user)
{

	if (user->buffer != NULL)
	{
		free(user->buffer);
		user->buffer = NULL;
	}
}

void clear_inputbuff(User user)
{

	if (user->buffer != NULL)
	{
		user->buffer->buff = user->buffer->buffer;
		*(user->buffer->buff) = 0;
	}
}

void log_telnetseq(User user)
{

	if (user->buffer == NULL) return;
	write_log(1,LOG1,"[TELNET CMD]: %u %u %u\n",(unsigned)IAC,(unsigned)user->buffer->telnet_cmd,(unsigned)user->buffer->telnet_opt);
	return;
}

/* Review buffer routines. */
Convbuff newconv_buffer(void)
{
	Convbuff tmp;

	if ((tmp = create_convbuff()) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Failed to allocate memory in newconv_buffer()\n");
		return(NULL);
	}
	return(tmp);
}

void clear_convbuff(Convbuff buff)
{
int i;

	if (buff == NULL)
	{
		return;
	}
	for (i = 0 ; i<NUM_LINES ; ++i)
	{
		if (buff->conv[i] != NULL)
		{
			free(buff->conv[i]);
			buff->conv[i] = NULL;
		}
	}
	buff->count = 0;
}

void addto_convbuff(Convbuff buff,char *str)
{

	if (buff == NULL) return;
	if (buff->conv[buff->count] != NULL)
	{
		free(buff->conv[buff->count]);
		buff->conv[buff->count] = NULL;
	}
	buff->conv[buff->count] = strdup(str);
	buff->count = (++(buff->count))%NUM_LINES;
}

void write_convbuff(User user,Convbuff buff)
{
	int pos,f;

	if (buff == NULL)
	{
		write_user(user,"~FG Failed to review buffer.\n");
		return;
	}
	pos = (buff->count)%NUM_LINES;
	for (f = 0 ; f<NUM_LINES ; ++f)
	{
		if (buff->conv[pos] != NULL)
		{
			write_user(user,buff->conv[pos]);
		}
		pos = ++pos%NUM_LINES;
	}
}

void filewrite_convbuff(FILE *fp,Convbuff buff)
{
	int pos,f;

	if (buff == NULL || fp == NULL) return;
	pos = (buff->count)%NUM_LINES;
	for (f = 0 ; f<NUM_LINES ; ++f)
	{
		if (buff->conv[pos] != NULL)
		{
			fprintf(fp,buff->conv[pos]);
		}
		pos = ++pos%NUM_LINES;
	}
}

/* Count the contents of a directory. */
int count_dir(char *dirname)
{
	struct dirent *dp;
	DIR *dir;
	int cnt;

	cnt = 0;
	dir = opendir((char *)dirname);
	if (dir == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Failed to open dir '%s' in write_dir()\n",dirname);
		return(0);
	}
	/* Go through the dir and count our contents. */
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")) continue;
		cnt++;
	}
	(void)closedir(dir);
	return(cnt);
}

/*
   Print the contents of a Directory. I got this idea from looking at a buddy
   of mines code, and realized that going through directories would be faster
   as well as more efficient then having a bunch of file i/o.
*/
void write_dir(User user,char *dirname)
{
	struct dirent *dp;
	DIR *dir;
	char small_buff[64];
	int cnt;

	cnt = 0;
	dir = opendir((char *)dirname);
	if (dir == NULL)
	{
		write_user(user,"~CW- ~FTFailed to open directory.\n");
		write_log(0,LOG1,"[ERROR] - Failed to open dir '%s' in write_dir()\n",dirname);
		return;
	}
	/* Go through the dir and print our contents. */
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")) continue;
		sprintf(small_buff,"%-35s",dp->d_name);
		write_user(user,small_buff);
		cnt++;
		if (cnt%2 == 0) write_user(user,"\n");
	}
	if (cnt%2 != 0) write_user(user,"\n");
	(void)closedir(dir);
	return;
}

/* END OF WRITE.C */
