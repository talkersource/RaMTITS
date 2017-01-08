/*
  main.c
    The main loop, as well as many needed functions that contain the
    inner workings of the talker.

    The source within this file is Copyright 1998 - 2001 by
	Rob Melhuish, Neil Robertson, Tim van der Leeuw, Andrew Collington,
	Phypor, and Arnaud Abelard.

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

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#define MAIN_SOURCE
  #include "include/ident.h"	/* Ident exerpt */
  #include "include/levels.h"
  #include "include/menus.h"
  #include "include/custom1.h"	/* the custom libraries. */
#undef MAIN_SOURCE
#include <unistd.h>
extern char *crypt __P ((__const char *__key, __const char *__salt));
extern int ident_toclient_fds[2];
extern int ident_toserver_fds[2];
extern void read_ident_reply(void);
extern int init_ident_server(void);
extern void setup_itimer();

/*
  This function calls all the setup routines and also contains the main
  program loop
*/
int main(int argc,char *argv[])
{
	User user,next;
	Telnet t,tnxt;
	fd_set readmask;
	int i,len,stat = 0,curr_id,next_id;
	char inpstr[ARR_SIZE],inpchar[2];

	strcpy(progname,argv[0]);
	if (argc<2)
	{
		sprintf(text,"%s/config.talker",DataDir);
		if (file_exists(text)) strcpy(confile,"config.talker");
		else strcpy(confile,"config");
		text[0]='\0';
	}
	else strcpy(confile,argv[1]);

	/* Startup */
	printf("\n-=[ %s talker running %s now booting. ]=-\n",TalkerName,VERSION);
	write_log(0,LOG1,"[RaMTITS] - %s talker running %s now booting.\n",TalkerName,VERSION);
	init_globals();
	set_date_time();
	init_signals();
	reset_alarm();
	printf("    Processing rooms.. this may take a while..\n");
	boot_up = 1;
	make_roomlist();
	boot_up = 0;
	load_and_parse_config();
	if (!possibly_reboot())
	{
		init_sockets();
	}
	printf("    Checking message boards...\n");
	check_messages(NULL,NULL,1);
	printf("    Parsing the command structure...\n");
	parse_cmnds();
	printf("    Processing users.. this may take a while..\n");
	make_userlist();
	printf("    Processing socials.. this may take a while..\n");
	make_sociallist();
	printf("    Loading ban lists...\n");
	load_bans(SiteBan);
	load_bans(NewSBan);
	load_bans(UserBan);
	load_bans(WhoSBan);
	load_bans(Screen);
	sprintf(text,"%s/lotto.dat",MiscDir);
	if (file_exists(text))
	{
		printf("    Loading lottery tickets...\n");
		load_lotto();
	}
	sprintf(text,"%s/talker.dat",DataDir);
	if (file_exists(text))
	{
		printf("    Loading total logins...\n");
		load_talker_stuff();
	}
	if (!(GenerateFigfontList())) printf("    Generating Figlet Font list...\n");
	if (!(GeneratePiclist())) printf("    Generating Picture list...\n");
	if (!(GenerateTextList())) printf("    Generating Textfile list...\n");
	text[0] = '\0';
	suggestion_num = count_mess(NULL,3);
	bug_num = count_mess(NULL,4);
	wiznote_num = count_mess(NULL,5);
	load_tvars(NULL,NULL,0);
	if (shoutbuff == NULL) shoutbuff = newconv_buffer();
	if (wizbuff == NULL) wizbuff = newconv_buffer();
	if (channelbuff == NULL) channelbuff = newconv_buffer();
	if (debugbuff == NULL) debugbuff = newconv_buffer();
	if (loginbuff == NULL) loginbuff = newconv_buffer();
	/*
	   We have this because if we want to run the program for debugging,
	   we can't have it go off and fork.. otherwise.. well..it'll render
	   gdb useless
	*/
#ifndef TDEBUG
	switch (fork())
	{
		case -1:
		  boot_exit(8);		/* fork failure	   */
		case  0:
		  break;                /* child continues */
		default:
		  sleep(1);
		  exit(0);		/* parent dies	   */
	}
#endif
	printf("-=[ %s talker booted properly with PID %u.. ]=-\n",TalkerName,(unsigned int)getpid());
	write_log(0,LOG1,"[RaMTITS] - %s talker booted properly with PID %u\n",TalkerName,(unsigned int)getpid());
	write_pid_file();
	reset_alarm();
	init_ident_server();
	/* Main program loop. */
	while (1)
	{
		setup_readmask(&readmask);  /* set up mask then wait */
		if (select(FD_SETSIZE,&readmask,0,0,0) == -1) continue;
		/* check for connection to listen sockets */
		for(i = 0 ; i < 2 ; ++i)
		{
			if (FD_ISSET(listen_sock[i],&readmask)) 
			   accept_connection(listen_sock[i],i);
		}
		if (IDENT_CLIENT_READ != -1)
		{
			if (FD_ISSET(IDENT_CLIENT_READ,&readmask))
			{
				/* A reply from the ident server */
				read_ident_reply();
			}
		}
		/* Cycle through the telnet connections */
		t = telnet_first;
		while (t != NULL)
		{
			tnxt = t->next;
			if (t->socket == -1 || !FD_ISSET(t->socket,&readmask))
			{
				t = tnxt;
				continue;
			}
			inpstr[0] = '\0';
			len = read(t->socket,inpstr,sizeof(inpstr)-3);
			if (len == 0)
			{
				write_user(t->owner," - Connection closed by foreign host.\n");
				disconnect_telnet(t);
				t = tnxt;
				continue;
			}
		 	t->last_received = time(0);
			display_telnet(t,inpstr,len);
			t = tnxt;
		}
		/*
		  Cycle through users. Use a while loop instead of a for
		  because we may lose the user structure during the loop
		  in which case user->next might be NULL, and the talker
		  will go buh bye.
		*/
		user = user_first;
		curr_id = user->id;
		while (user != NULL)
		{
			if (get_userid(curr_id) == NULL) user = user_first;
			if (user == NULL) break;
			next = user->next;			
			curr_id = user->id;
			if (next != NULL) next_id = next->id;
			else next_id = curr_id;
			/* store in case user goes bye */
			if (user == NULL)
			{
				curr_id = next_id;
				user = next;
				continue;
			}
			/* If clone ignore */
			if (user->type != UserType)
			{
				curr_id = next_id;
				user = next;
				continue;
			}
			destructed = 0;
			/* see if any data on socket else continue */
			if (user->socket == -1 || user->socket == -2
			  || !FD_ISSET(user->socket,&readmask))
			{
				curr_id = next_id;
				user = next;
				continue;
			}
			/* see if client (eg telnet) has closed socket */
			inpstr[0]='\0';
			len = read(user->socket,inpstr,sizeof(inpstr)-1);
			if (len == 0)
			{
				disconnect_user(user,2);
				curr_id = next_id;
				user = next;
				continue;
			}
			if ((unsigned char)inpstr[0] == IAC)
			{
				if ((unsigned char)inpstr[1] == IP)
				{
					will_time_mark(user);
				}
				/*
				  Get terminal information.. thanks to Arny
				  for this.. it's quite handy :-)
				*/
				if ((unsigned char)inpstr[1] == WILL
				  && (unsigned char)inpstr[2] == TELOPT_NAWS)
				{
					user->cols = (int)inpstr[7];
					user->rows = (int)inpstr[9];
				}

				if ((unsigned char)inpstr[1] == SB
				  && (unsigned char)inpstr[2] == TELOPT_NAWS)
				{
					writeus(user,"\n~CB- ~FTWindow resize detected: ~FTColumns: ~CB[~CR%d~CB]~FT Rows: ~CB[~CR%d~CB]\n",(int)inpstr[4],(int)inpstr[6]);
					user->cols = (int)inpstr[4];
					user->rows = (int)inpstr[6];
					user->autodetected = 1;
				}
				if ((unsigned char)inpstr[1] == WONT
				  && (unsigned char)inpstr[2] == TELOPT_STATUS)
				{
					ping_respond(user);
				}
				curr_id = next_id;
				user = next;
				continue;
			}

			if (len != EOF && len>0)
			{
				addstring_tobuff(user,inpstr,len);
				inpchar[0] = inpstr[len-1];
			}
			/* shouldn't happen. */
			else
			{
				curr_id = next_id;
				user = next;
				continue;
			}
			if (user->char_echo)
			{
				inpstr[len] = 0;
				write_window(user,inpstr,len,WriteInputEcho);
			}
			else
			{
				terminate(inpstr);
			}
			if (inpchar[0] > 0 && newline_inbuff(curr_id) == 0 )
			{
				curr_id = next_id;
				user = next;
				continue;
			}
			while (newline_inbuff(curr_id))
			{
				/*
				  Copy into inpstr and recycle buffer for
				  greater efficiency.
				*/
				if (getline_from_inputbuff(user,inpstr))
				{
					release_inputbuff(user);
				}
				inpchar[0] = inpstr[strlen(inpstr)-1];
				/* See if there's anything else in the string.*/
				if (!strlen(inpstr) && (inpchar[0] != 10
				  && inpchar[0] != '\r'))
				{
					continue;
				}
				if (inpstr[0] == 0)
				{
					continue;
				}
				/* misc stuff. */
				remove_ctrl_chars(user,inpstr);
				user->last_input = time(0);
				no_prompt = 0;
				com_num	= -1;
				force_listen = 0;
				destructed = 0;
				if (user->login)
				{
					Login(user,inpstr);
					continue;
				}
				clear_words();
				word_count = wordfind(inpstr);
				if (user->triv_mode)
				{
					trivia(user,inpstr,0);
					continue;
				}
				if (user->afk)
				{
					if (user->afk == 2)
					{
						if (strcmp((char *)crypt(word[0],salt),user->pass))
						{
							write_user(user," ~FR You have entered an invalid password.\n"); 
							prompt(user);
							continue;
						}
						cls(user,NULL,0);
						write_user(user,"~CB[~FGSession unlocked, you are no longer afk.~CB]\n");
					}
					user->afk_mesg[0] = '\0';
					if (user->chkrev)
					{
						writeus(user,new_tells1,user->recap);
						write_user(user,new_tells2);
						user->chkrev = 0;
					}
					if (user->invis == 0)
					{
						if (user->bfafk[0] == '\0')
						{
							i = ran(NumBfafkMesgs);
							writeus(user,BfafkText[i],user->recap);
							WriteRoomExcept(user->room,user,BfafkText[i],user->recap);
							if (user->temp_desc[0] != '\0')
							  strncpy(user->desc,user->temp_desc,sizeof(user->desc)-1);
						}
						else
						{
							sprintf(text,"~CB -- ~FT%s ~FM%s ~CB[~FYR~CW.~FYT~CW.~FYK~CB]\n",user->recap,user->bfafk);
							WriteRoomExcept(user->room,user,text);
							write_user(user,text);
							if (user->temp_desc[0] != '\0')
							  strncpy(user->desc,user->temp_desc,sizeof(user->desc)-1);
						}
					}
					else
					{
						if (user->temp_desc[0] != '\0')
						  strncpy(user->desc,user->temp_desc,sizeof(user->desc)-1);
					}
					rev_away(user);
					user->temp_desc[0] = '\0';
					prompt(user);
					if (user->afk == 2)
					{
						user->afk = 0;
						continue;
					}
					else user->afk = 0;
				}
				t = get_telnet(user);
				if (t != NULL
				  && (stat = check_telnet(t,inpstr)))
				{
					if (stat == 1)
					{
						telnet_say(user,inpstr,0);
						continue;
					}
					else if (stat == 2)
					{
						strcpy(inpstr,remove_first(inpstr));
						clear_words();
						word_count=wordfind(inpstr);
						if (!inpstr[0] && !word_count)
						write_user(user,"~CW - ~FGCommand usage~CB: ~CR.local~CB [~CRlocal command~CW/~CRtext~CB]\n");
					}
					else if (stat == 3)
					{
						telnet_phone(user,NULL,0);
						continue;
					}
				}
				if (word_count == 0)
				{
					if (user->bank_op)
					{
						BankOps(user,inpstr,1);
						continue;
					}
					if (user->set_op)
					{
						SetOps(user,inpstr,1);
						continue;
					}
					if (user->store_op)
					{
						StoreOps(user,inpstr,1);
						continue;
					}
					if (misc_ops(user,inpstr))
					{
						continue;
					}
					if (game_ops(user,inpstr))
					{
						continue;
					}
					if (user->mail_op)
					{
						MailOps(user,inpstr);
						continue;
					}
					if (user->mode) prompt(user);
					continue;
				}
				if (user->bank_op)
				{
					BankOps(user,inpstr,1);
					continue;
				}
				if (user->set_op)
				{
					SetOps(user,inpstr,1);
					continue;
				}
				if (user->store_op)
				{
					StoreOps(user,inpstr,1);
					continue;
				}
				if (misc_ops(user,inpstr))
				{
					continue;
				}
				if (game_ops(user,inpstr))
				{
					continue;
				}
				if (user->mail_op)
				{
					MailOps(user,inpstr);
					continue;
				}
				com_num = -1;
				if (user->whitespace) white_space(inpstr);

				if (user->mode == EWTOO || inpstr[0] == '.'
				  || inpstr[0] == ',' || (user->mode == IRC
				  && inpstr[0] == '/') || is_scut(user,inpstr))
				{
					exec_com(user,inpstr);
				}
				else
				{
					com_num=get_comnum("say");
					say(user,inpstr,0);
				}
				if (destructed != 1) prompt(user);
			} /* End of while (newline) */
			if (destructed) next = user_first;
			destructed = 0;
			curr_id = next_id;
			user = next;
		} /* end while (user) */
		/* Flush any cached data */
		write_window(NULL,NULL,0,0);
	} /* end while (1) */
	return(0);
} /* End of main() */

/* Set up readmask for select */
void setup_readmask(fd_set *mask)
{
	User u;
	Telnet t;
	int i;

	FD_ZERO(mask);
	for(i=0;i<2;++i)
	{
		FD_SET(listen_sock[i],mask);
	}
	/* user sockets */
	for_users(u)
	{
		if (u->socket == -1 || u->socket == -2) continue;
		if (u->type == UserType) FD_SET(u->socket,mask);
	}
	/* telnet sockets */
	for_telnet(t)
	{
		if (t->socket != -1) FD_SET(t->socket,mask);
	}
	/* Ident client socket */
	if (IDENT_CLIENT_READ != -1)
	{
		FD_SET(IDENT_CLIENT_READ,mask);
	}
}

/* Talker signal handler function. Can either shutdown,ignore or reboot */
void sig_handler(int sig)
{

	force_listen = 1;
	switch(sig)
	{
		case SIGHUP:
		  write_room(NULL,"\n~CB[~CRSYSTEM~CB] - ~FRSIGHUP received. ~FTInitiating seamless reboot.\n\n");
		  do_reboot(NULL);
		  return;
		case SIGTERM:
		  if (ignore_sigterm)
		  {
			write_log(1,LOG1,"SIGTERM signal received - ignoring.\n");
			return;
		  }
		  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRSIGTERM ~FTreceived, shutting down now.\n");
		  save_talker_stuff();
		  save_all_bans();
		  save_lotto();
		  talker_shutdown(NULL,"a termination signal - [SIGTERM]",0); 
		case SIGSEGV:
		  if (debug_talker)
		  {
			WriteDebugFile();
		  }
		  switch(crash_action)
		  {
			case 0:	
			case 1:
			  save_talker_stuff();
			  save_all_bans();
			  save_lotto();
			  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRSegmentation fault. ~FTShutting down NOW.\n");
			  talker_shutdown(NULL,"a segmentation fault - [SIGSEGV]",0);
			case 2:
			  save_talker_stuff();
			  save_all_bans();
			  save_lotto();
			  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRSegmentation fault. ~FTInitiating reboot NOW.\n");
			  talker_shutdown(NULL,"a segmentation fault - [SIGSEGV]",1); 
			case 3:
			  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRSegmentation fault. ~FTInitiating seamless reboot.\n");
			  do_reboot(NULL);
			  return;
		  }
		case SIGBUS:
		  if (debug_talker)
		  {
			WriteDebugFile();
		  }
		  switch(crash_action)
		  {
			case 0:
			case 1:
			  save_talker_stuff();
			  save_all_bans();
			  save_lotto();
			  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRBus fault. ~FTShutting down NOW.\n");
			  talker_shutdown(NULL,"a bus error - [SIGBUS]",0);
			case 2:
			  save_talker_stuff();
			  save_all_bans();
			  save_lotto();
			  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRBus fault. ~FTInitiating reboot NOW.\n");
			  talker_shutdown(NULL,"a bus error - [SIGBUS]",1);
			case 3:
			  write_room(NULL,"\n\07~CB[~CRSYSTEM~CB] - ~FRBus fault. ~FTInitiating seamless reboot.\n");
			  do_reboot(NULL);
			  return;
		  }
	}
}

/*
   Stuff that is neither speech nor a command is dealt with here
*/
int misc_ops(User user,char *inpstr)
{
	FILE *fp,*fp2;
	User u;
	int i,temp,page_num,fsize = 0,cnt = 0;
	char *name,filename[FSL],worh[10],check[UserNameLen+2];
	struct stat fb;
	size_t x=0;

	temp = page_num = 0;
	check[0] = '\0';
	if (user == NULL) return(0);
	if (user->invis == Invis) name = invisname; else name = user->recap;
	switch (user->misc_op)
	{
		case 1: 
		  switch(toupper(inpstr[0]))
		  {
			case 'Y':
			  if (rs_countdown && rs_which == 0)
			  {
				if (rs_countdown>60) sprintf(text,"\n  ~CT[~CRRaMTITS~CT] - ~FGSHUTDOWN initiated. Shutting down in ~CR%d ~FGminute%s, ~CR%d~FG second%s\n\n",rs_countdown/60,rs_countdown/60>1?"s":"",rs_countdown%60,rs_countdown%60>1?"s":"");
				else sprintf(text,"\n  ~CT[~CRRaMTITS~CT] ~FGSHUTDOWN initiated. Shutting down in ~CR%d~FG second%s\n\n",rs_countdown,rs_countdown>1?"s":"");
				force_listen = 1;	write_room(NULL,text);
				force_listen = 0;	rs_user = user;
				write_log(1,LOG1,"[SHUTDOWN] - User: [%s] Timer: [%d second%s]\n",user->name,rs_countdown,rs_countdown>1?"s":"");
				rs_announce = time(0);	user->misc_op = 0;  
				rev_away(user);
				prompt(user);
				return(1);
			  }
			  else
			  {
				talker_shutdown(user,NULL,0);
				return(1);
			  }
			case 'N':
			  /* This will reset any reboot countdown that was -
			   - started 					   */
			  write_user(user,"~FG You think again before shutting down.\n");
			  if (!rs_which && rs_countdown && rs_user == NULL)
			  {
				rs_countdown = 0;	rs_announce = 0;
				rs_which = -1;
			  }
			  rev_away(user);
			  user->misc_op = 0;
			  prompt(user);
			  return(1);
			default :
			  write_user(user,"~FG Please specify either YES or NO~CB:~RS ");
		  }
		  return(1);
		case 2:
		  /* The RaMTITS pager misc_op */
		  switch(toupper(inpstr[0]))
		  {
			case 'Q':
			  user->misc_op			= 0;
			  user->pager.page_file[0] 	= '\0';
			  user->pager.filepos		= 0;
			  user->pager.pdone		= 0;
			  for (i=0;i<MaxPages;++i) user->pager.pages[i] = 0;
			  user->pager.type		= 0;
			  user->pager.page_num		= 1;
			  user->pager.last_page 	= -1;
			  user->pager.lines		= 0;
			  user->pager.start_page 	= 0;
			  RemoveSysfiles(user);
			  if (user->mail_op == 0)
			  {
				rev_away(user);
			  }
			  if (user->mail_op)
			  {
				write_user(user,continue1);
				user->mail_op = 7;
			  }
			  return(1);
			case 'H':
			  write_user(user,center("~FT-~CT=~CB> ~FGRaMTITS Pager help.~CB <~CT=~FT-\n",80));
			  write_user(user," ~CW'~CRQ~CW' = ~FTQuit the pager.\n");
			  write_user(user," ~CW'~CRH~CW' = ~FTWhat you're reading.\n");
			  write_user(user," ~CW'~CRP~CW' = ~FTGo back a page.\n");
			  write_user(user," ~CW'~CRR~CW' = ~FTRedraw your current page.\n");
			  write_user(user," ~CW'~CRB~CW' = ~FTGo to the last page.\n");
			  write_user(user," ~CW'~CRT~CW' = ~FTGo to the first page.\n");
			  write_user(user," ~CW'~CR#~CW' = ~FTJump to a certain page.\n");
			  write_user(user," ~CW[~CRENTER~CW] = ~FTGo to the next page.\n");
		  
			  if (user->pager.page_num!=user->pager.last_page)
			    sprintf(text,PagerPrompt1,user->pager.pdone,user->pager.page_num,user->pager.last_page);
			  else sprintf(text,PagerPrompt2,user->pager.page_num,user->pager.last_page);
			  write_user(user,text);
			  return(1);
			case 'P':
			  if (user->pager.page_num == 1)
			  {
				if (user->pager.page_num!=user->pager.last_page)
				    sprintf(text,PagerPrompt1,user->pager.pdone,user->pager.page_num,user->pager.last_page);
				else sprintf(text,PagerPrompt2,user->pager.page_num,user->pager.last_page);
				write_user(user,text);
				return(1);
			  }
			  --user->pager.page_num;
			  more(user,user->socket,user->pager.page_file,user->pager.type);
			  return(1);
			case 'R':
			  more(user,user->socket,user->pager.page_file,user->pager.type);
			  return(1);
			case 'T':
			  user->pager.page_num = 1;
			  more(user,user->socket,user->pager.page_file,user->pager.type);
			  return(1);
			case 'B':
			  user->pager.page_num = user->pager.last_page;
			  more(user,user->socket,user->pager.page_file,user->pager.type);
			  return(1);
			default :
			  if (!isdigit(inpstr[0]))
			  {
				if (++user->pager.page_num>user->pager.last_page)
				{
					user->misc_op		 = 0;
					user->pager.page_file[0] = '\0';
					user->pager.filepos	 = 0;
					user->pager.page_num	 = 1;
					user->pager.lines	 = 0;
					for (i=0;i<MaxPages;++i)
					  user->pager.pages[i]	 = 0;
					user->pager.last_page	 = -1;
					user->pager.start_page	 = 0;
					no_prompt		 = 0;
					write_user(user,"\n");
					RemoveSysfiles(user);
					if (user->pager.type == 1)
					{
						sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
						if (stat(filename,&fb) == -1)
						  fsize = 0;
						else fsize = fb.st_size;
						writeus(user,"~FG In total your s-mail box is allocating ~CY%d ~FGbytes, out of a max. of ~CT%d~FG.\n",fsize,MaxMbytes);
						sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
						unlink(filename);
					}
					if (user->pager.type == 2)
					{
						sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
						if (stat(filename,&fb) == -1)
						  fsize = 0;
						else fsize=fb.st_size;
					 	writeus(user,"~FG Your sent s-mail box is using ~CY%d ~FGbytes, out of a max. of ~CT%d~FG.\n",fsize,MaxSMbytes);
					}
					user->pager.type = 0;
					if (user->mail_op == 0)
					{
						rev_away(user);
					}
					if (user->mail_op)
					{
						write_user(user,continue1);
						user->mail_op = 7;
					}
					return(1);
				}
				more(user,user->socket,user->pager.page_file,user->pager.type);
				return(1);
			  }
			  page_num = atoi(inpstr);
			  if (page_num<1 || page_num>user->pager.last_page)
			  {
				if (user->pager.page_num!=user->pager.last_page)
				  sprintf(text,PagerPrompt1,user->pager.pdone,user->pager.page_num,user->pager.last_page);
				else sprintf(text,PagerPrompt2,user->pager.page_num,user->pager.last_page);
				write_user(user,text);
				return(1);
			  }
			  user->pager.page_num = page_num;
			  more(user,user->socket,user->pager.page_file,user->pager.type);
			  return(1);
		  }
		  return(1);
		/* The message boards. */
		case 3:  	/* writing on board	*/
		case 4:  	/* Writing mail		*/
		case 5:  	/* doing profile	*/
		case 6:  	/* users comments	*/
		case 7:  	/* add some news	*/
		case 8:  	/* enter room desc	*/
		case 9:  	/* suggestions		*/
		case 10: 	/* friend s-mail	*/
	        case 11: 	/* staff s-mail		*/
		case 12: 	/* jot down some bugs	*/
		case 21:	/* mass user s-mail	*/
		case 22:	/* enter vote topic	*/
		case 40:	/* Wiznote		*/
		case 41:	/* E-mail.		*/
		case 48:	/* Helpfile.		*/
		case 50:	/* enter riddle		*/
		  if (user->editor.editor == 1) nuts_editor(user,inpstr);
		  else ramtits_editor(user,inpstr);
		  return(1);
		case 13:
		  switch(toupper(inpstr[0]))
		  {
			case 'Y':
			  writeus(user,"~FG Alright %s..~FG it was nice having you around :-)\n",user->recap);
			  delete_user(user,NULL,1); /* They just .suicided. */
			  return(1);
			case 'N':
			  write_user(user,"~FG Alright.... scared us for a second there :)\n");
			  rev_away(user);
			  user->misc_op = 0;
			  prompt(user);
			  return(1);
			default :
			  write_user(user,"~FG You have to specify YES or NO~CB:~RS ");
		  }
		  return(1);
		case 14:
		  switch(toupper(inpstr[0]))
		  {
			case 'Y':
			  if (rs_countdown && rs_which==1)
			  {
				if (rs_countdown>60) sprintf(text,"\n    ~CB[~CRRaMTITS~CB] - ~FGREBOOT initiated. Rebooting in ~CR%d ~FGminute%s, ~CR%d~FG second%s\n\n",rs_countdown/60,rs_countdown/60>1?"s":"",rs_countdown%60,rs_countdown%60>1?"s":"");
				else sprintf(text,"\n    ~CB[~CRRaMTITS~CB] - ~FGREBOOT initiated. Rebooting in ~CR%d~FG second%s\n\n",rs_countdown,rs_countdown>1?"s":"");
				force_listen = 1;
				write_room(NULL,text);
				force_listen = 0;
				write_log(1,LOG1,"[REBOOT] - User: [%s] Time: [%d second%s]\n",user->name,rs_countdown,rs_countdown>1?"s":"");
				rs_user = user;
				rs_announce = time(0);
				user->misc_op = 0;
				rev_away(user);
				prompt(user);
				return(1);
			  }
			  else
			  {
				talker_shutdown(user,NULL,1);
				return(1);
			  }
			case 'N':
		  	  if (rs_which == 1 && rs_countdown && rs_user == NULL)
			  {
				rs_countdown = 0;
				rs_announce = 0;
				rs_which = -1;
			  }
		  	  user->misc_op = 0;
			  rev_away(user);
			  prompt(user);
			  return(1);
			default :
			  write_user(user,"~FG You have to specify YES or NO~CB:~RS ");
		  }
		  return(1);
		case 15:
		  if (toupper(inpstr[0]) == 'Q')
		  {
			user->misc_op	 = 0;
			user->helpw_com	 = 0;
			user->helpw_lev	 = 0;
			user->helpw_type = 0;
			user->helpw_same = 0;
			rev_away(user);
			prompt(user);
		  }
		  else normal_help(user,1);
		  return(1);
		case 18:
		  if (toupper(inpstr[0]) == 'Q')
		  {
			user->misc_op	= 0;
			user->check[0]	= '\0';
			rev_away(user);
			prompt(user);
		  }
		  else
		  {
			examine(user,NULL,1);
			user->misc_op = 0;
		  }
		  return(1);
		case 23:
		  if (toupper(inpstr[0]) == 'Q')
		  {
			user->misc_op	 = 0;
			user->helpw_com  = 0;
			user->helpw_lev	 = 0;
			user->helpw_type = 0;
			user->helpw_same = 0;
			rev_away(user);
			prompt(user);
		  }
		  else command_help(user,1);
		  return(1);
		case 24:
		  if (toupper(inpstr[0]) == 'Q')
		  {
			user->misc_op	 = 0;
			user->helpw_com	 = 0;
			user->helpw_lev	 = 0;
			user->helpw_type = 0;
			user->helpw_same = 0;
			rev_away(user);
			prompt(user);
		  }
		  else type_help(user,1);
		  return(1);
		case 25:
		  if (!strlen(inpstr))
		  {
			write_user(user,"~CW - ~CRAbandoning your talker addition!\n");
			user->misc_op = 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  strcpy(user->check,inpstr);
		  writeus(user,"~FR Ok, the talker name is set to~CB: [~RS%s~CB]\n",user->check);
		  write_user(user,"~FG Now enter the address to the talker (i.e. ramtits.ca 6980)\n");
		  write_user(user,"~CB:~RS ");
		  user->misc_op = 26;
		  return(1);
		case 26:
		  if (!strlen(inpstr))
		  {
			write_user(user,"~FR Abandoning your talker addition!\n");
			user->check[0] = '\0';
			user->misc_op = 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  sprintf(filename,"%s/%s",DataDir,TalkerFile);
		  if ((fp = fopen(filename,"a")) == NULL)
		  {
			write_user(user,"~CW - ~FGUnable to open talkerfile at the moment.\n");
			write_log(0,LOG1,"[ERROR] - Unable to open talker file to append!\n");
			user->check[0] = '\0';
			user->misc_op = 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  sprintf(text," ~CB[~FT%s~CB] ~CW- ~FGName: ~CB[~CR%s~CB] ~CW--- ~FGAddy: ~CB[~RS%s~CB]\n",user->recap,user->check,inpstr);
		  fputs(text,fp);
		  FCLOSE(fp);
		  write_user(user,center("~FT-~CT=~CB>~FG The talker has been added successfully.~CB <~CT=~FT-\n",80));
		  user->misc_op = 0;  user->check[0] = '\0';
		  rev_away(user);
		  prompt(user);
		  return(1);
		case 28:
		  if (toupper(inpstr[0]) == 'Y')
		  {
			if ((u = get_user(user->check)) == NULL)
			{
				write_user(user,"~FG Uh oh...looks like your spouse left... abandoning the marriage!\n");
				user->check[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			}
			writeus(user,"~FT You accept ~CT%s%s~FT marriage proposal...\n",u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
			WriteRoomExcept(user->room,user,"~FM%s ~FGagrees to marry %s~FG.\n",user->recap,u->recap);
			write_user(user,"~FG Would you like to enter your own vows?\n");
			write_user(user,"~CB[~CRYes or No~CB]:~RS ");
			user->misc_op = 29;
			return(1);
		  }
		  else if (toupper(inpstr[0]) == 'N')
		  {
			WriteRoomExcept(user->room,user,"~FG %s~FM does not agree to the wedding.\n",user->recap);
			if ((u = get_user(user->check)) == NULL)
				user->check[0] = '\0';
			u->check[0]	= '\0';
			user->check[0]	= '\0';
			user->misc_op	= 0;
			write_user(user,"~CW-~FT Alright, you don't agree to the wedding.\n");
			rev_away(user);
			prompt(user);
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYes or no please~CB:~RS");
			user->misc_op=28;
			return(1);
		  }
		  return(1);
		case 29:
		  if (toupper(inpstr[0]) == 'Y')
		  {
			if ((u = get_user(user->check)) == NULL)
			{
				write_user(user,"~FG Uh oh...looks like your spouse left... abandoning the marriage!\n");
				user->check[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			}
			write_user(user,"~FT Ok, you want to enter your own vows... After your finished your vows, just hit ~CW'~CRenter~CW'\n");
			write_user(user,"~CB:~RS ");
			user->misc_op = 30;
			return(1);
		  }
		  else
		  {
			user->misc_op = 0;
			rev_away(user);
			net_marry(user,NULL,1);
			prompt(user);
		  }
		  return(1);
		case 30:
		  if (!strlen(inpstr))
		  {
			write_user(user,"~FG Would help if you entered something ;)\n");
			write_user(user,"~FT Re-enter your vows~CB:~RS ");
			user->misc_op = 30;
			return(1);
		  }
		  if (strlen(inpstr)>OutBuffSize)
		  {
			write_user(user,"~FG You exceded the vows buffer....\n");
			write_user(user,"~FT Re-enter your vows~CB:~RS ");
			user->misc_op = 30;
			return(1);
		  }
		  write_user(user,"~FG Your vows are now set.\n");
		  strncpy(user->vows,inpstr,sizeof(user->vows)-1);
		  user->misc_op = 0;
		  rev_away(user);
		  net_marry(user,NULL,1);
		  prompt(user);
		  return(1);
		case 31:
		  if (toupper(inpstr[0]) == 'Y')
		  {
			if ((u = get_user(user->check)) == NULL)
			{
				write_user(user,"~FG Uh oh...looks like your spouse left... abandoning the marriage!\n");
				user->check[0]	= '\0';
				user->vows[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			}
			sprintf(text,"~FT %s~FG looks into ~FT%s%s~FG eyes and says ~CW'~CM I do~CW '\n",user->recap,u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
			write_room_except(user->room,user,text);
			write_user(user,text);
			strcpy(worh,"thing");
			if (u->gender == Female) strcpy(worh,"wife");
			if (u->gender == Male) strcpy(worh,"husband");
			if (user->vows[0] != '\0')
			{
				sprintf(text,"~FT  %s~FG makes the following vows to ~FT%s~FG...\n",user->recap,u->recap);
				write_room_except(user->room,user,text);
				write_user(user,text);
				sprintf(text," %s\n\n",user->vows);
				write_room_except(user->room,user,text);
				write_user(user,text);
			}
			sprintf(text,"~FG  Do you~FM %s~FG take~FM %s ~FG to be your lawfully wedded %s?\n"
				     "~FG To have and to hold and to cherish for all time?\n",u->recap,user->recap,worh);
			write_room_except(user->room,user,text);
			write_user(user,text);
			write_user(u,"~CW[<~CR Yes or No~CW >]~CB:~RS ");
			u->misc_op	= 32;
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  else if (toupper(inpstr[0]) == 'N')
		  {
			write_user(user,"~CW-~FT Alrighty.. you don't accept then...\n");
			WriteRoomExcept(user->room,user,"~FT  %s~FG didn't accept... looks like this wedding is over...\n",user->recap);
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  else
		  {
			write_user(user,"~CW-~FT Please specify yes or no.~CB:~RS ");
			user->misc_op	= 31;
			return(1);
		  }
		  return(1);
		case 32:
		  if (toupper(inpstr[0]) == 'Y')
		  {
			if ((u = get_user(user->check)) == NULL)
			{
				write_user(user,"~FG Uh oh...looks like your spouse left... abandoning the marriage.\n");
				user->check[0]	= '\0';
				user->vows[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			}
			sprintf(text,"~FT %s~FG looks into ~FT%s%s~FG eyes and says ~CW'~CM I do~CW '\n",user->recap,u->recap,u->name[strlen(u->name)-1] == 's'?"'":"'s");
			write_room_except(user->room,user,text);
			write_user(user,text);
			user->misc_op	= 0;
			rev_away(user);
			net_marry(user,NULL,2);
			prompt(user);
			return(1);
		  }
		  else if (toupper(inpstr[0]) == 'N')
		  {
			write_user(user,"~CW-~FT Alrighty.. you don't accept then...\n");
			WriteRoomExcept(user->room,user,"~FT  %s~FG didn't accept... looks like this wedding is over..!\n",user->recap);
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
		  }
		  return(1);
		case 33:
		  switch(toupper(inpstr[0]))
		  {
			case 'Y':
			  if (rs_countdown && rs_which == 2)
			  {
				if (rs_countdown>60) sprintf(text,"\n    ~CT[~CRRaMTITS~CT] ~FGSeamless reboot initiated. Rebooting in ~CR%d ~FGminute%s, ~CR%d~FG second%s\n\n",rs_countdown/60,rs_countdown/60>1?"s":"",rs_countdown%60,rs_countdown%60>1?"s":"");
				else sprintf(text,"\n    ~CT[~CRRaMTITS~CT] ~FGSeamless reboot initiated. Rebooting in ~CR%d~FG second%s\n\n",rs_countdown,rs_countdown>1?"s":"");
				write_room(NULL,text);
				write_log(1,LOG1,"[SREBOOT] - User: [%s] Time: [%d second%s]\n",user->name,rs_countdown,rs_countdown>1?"s":"");
				rs_user		= user;
				rs_announce	= time(0);
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			  }
			  else
			  {
				user->misc_op	= 0;
				rev_away(user);
				do_reboot(user);
				return(1);
			  }
			case 'N':
			  if (rs_which == 2 && rs_countdown && rs_user == NULL)
			  {
				rs_countdown	= 0;
				rs_announce	= 0;
				rs_which	= -1;
			  }
			  user->misc_op	= 0;
			  rev_away(user);
			  prompt(user);
			  return(1);
			default :
			  write_user(user,"~FG You have to specify YES or NO~CB:~RS ");
		  }
		  return(1);
		case 34:
		  if (toupper(inpstr[0]) == 'Q')
		  {
			user->misc_op	= 0;
			user->check[0]	= '\0';
			rev_away(user);
			prompt(user);
		  }
		  else
		  {
			socials(user,NULL,1);
		  }
		  return(1);
		case 35:
		  switch (toupper(inpstr[0]))
		  {
			case 'Y':
			  sprintf(filename,"%s/%s",DataDir,StaffFile);
			  cnt = count_lines(filename);
			  if (cnt >= MaxStaff)
			  {
				writeus(user,"~CW- ~FTUnable to add ~CW'~FY%s~CW' ~FTto the staff file. Too many staff members.\n",user->check);
				user->check[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			  }
			  if ((fp = fopen(filename,"a")))
			  {
				fprintf(fp,"%s\n",user->check);
				FCLOSE(fp);
			  }
			  load_tvars(user,NULL,0);
			  writeus(user,"~CR %s~FG has been added to the staff file and variables have been reloaded.\n",user->check);
			  user->check[0]	= '\0';
			  user->misc_op		= 0;
			  rev_away(user);
			  prompt(user);
			  return(1);
			case 'N':
			  writeus(user,"~CR %s~FG won't be added to the staff_file.\n",user->check);
			  user->check[0]	= '\0';
			  user->misc_op		= 0;
			  rev_away(user);
			  prompt(user);
			  return(1);
			default :
			  write_user(user,"~FG Please answer with either YES or NO.\n");
			  writeus(user," ~FG Would you like to add '%s' to the staff_file?~CB:~RS ",user->check);
			  user->misc_op	= 35;
		  }
		  return(1);
		case 36:
		  switch (toupper(inpstr[0]))
		  {
			case 'Y':
			  sprintf(filename,"%s/%s",DataDir,StaffFile);
			  fp = fopen(filename,"r");
			  if (fp == NULL)
			  {
				write_user(user,"~CW-~FT Failed to save the staff file.\n");
				user->check[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				prompt(user);
				return(1);
			  }
			  fp2 = fopen("templist","w");
			  if (fp2 == NULL)
			  {
				write_user(user,"~CW-~FT Failed to save the staff file.\n");
				user->check[0]	= '\0';
				user->misc_op	= 0;
				rev_away(user);
				FCLOSE(fp);
				prompt(user);
				return(1);
			  }
			  fscanf(fp,"%s",check);
			  while (!(feof(fp)))
			  {
				if (strcasecmp(user->check,check))
					fprintf(fp2,"%s\n",check);
				fscanf(fp,"%s",check);
			  }
			  FCLOSE(fp);
			  FCLOSE(fp2);
			  unlink(filename);
			  rename("templist",filename);
			  load_tvars(user,NULL,0);
			  writeus(user,"~CR %s~FG has been removed from the staff file and variables have been reloaded.\n",user->check);
			  user->check[0]	= '\0';
			  user->misc_op		= 0;
			  rev_away(user);
			  prompt(user);
			  return(1);
			case 'N':
			  writeus(user,"~CR %s~FG won't be removed from the staff_file.\n",user->check);
			  user->check[0]	= '\0';
			  user->misc_op		= 0;
			  rev_away(user);
			  prompt(user);
			  return(1);
			default :
			  write_user(user,"~FG Please answer with either YES or NO.\n");
			  writeus(user," ~FG Would you like to remove '%s' to the staff_file?~CB:~RS ",user->check);
			  user->misc_op = 35;
		  }
		  return(1);
		case 37:
		  if (toupper(inpstr[0]) == 'Q')
		  {
			user->misc_op	= 0;
			user->check[0]	= '\0';
			rev_away(user);
			prompt(user);
		  }
		  else find_user(user,NULL,1);
		  return(1);
		case 38: /* Making a social */
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FT Aborting social creation...\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  if ((get_command_full(word[0])) != NULL)
		  {
			write_user(user,"~CW-~FG There is already a command with that name.\n");
			write_user(user,"~CW-~FT Line 1~CB: [~FGEnter a name for the social~CB]\n");
			write_user(user,"~CW-~FT Line 1~CB:~RS ");
			user->misc_op	= 38;
			return(1);
		  }
		  sprintf(filename,"%s/%s.dat",SocialDir,word[0]);
		  if (file_exists(filename))
		  {
			write_user(user,"~CW-~FG There is already a social with that name.\n");
			write_user(user,"~CW-~FT Line1~CB: [~FGEnter a name for the social~CB]\n");
			write_user(user,"~CW-~FT Line1~CB:~RS ");
			user->misc_op	= 38;
			return(1);
		  }
		  if (strlen(word[0]) > 13 || !strlen(inpstr))
		  {
			write_user(user,"~CW-~FT That social name is invalid.\n");
			write_user(user,"~CW-~FT Line 1~CB: [~FGEnter a name for the social~CB]\n");
			write_user(user,"~CW-~FT Line 1~CB:~RS ");
			user->misc_op	= 38;
			return(1);
		  }
		  strcpy(user->social[0],word[0]);
		  user->misc_op	= 39;
		  write_user(user,"\n~FRSample~CB:\n~CT%s ~FTtakes out a smoke and starts puffing away!\n");
		  write_user(user,"~CW-~FT Line 2~CB: [~FGEnter what you want the social to execute if used by itself.~CB]\n");
		  writeus(user,"~CW-~FT Line 2~CB: [~FGFor example typing ~CW'~CR.%s~CW'~FG and hitting enter.~CB]\n",user->social[0]);
		  write_user(user,"~CW-~FT Line2~CB:~RS ");
		  return(1);
		case 39:
		  if (!strlen(inpstr) || strlen(inpstr) > 250)
		  {
			write_user(user,"~CW-~FG That line would be either too long or too short.\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s ~FTtakes out a smoke and starts puffing away!\n");
			write_user(user,"~CW-~FT Line 2~CB: [~FGEnter what you want the social to execute if used by itself.~CB]\n");
			write_user(user,"~CW-~FT Line 2~CB:~RS ");
			user->misc_op	= 39;
			return(1);
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FT Aborting social creation...\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  for (x = 0; x < strlen(inpstr); ++x)
		  {
			if (inpstr[x] == '%' && inpstr[x+1] == 's') ++cnt;
		  }
		  if (cnt > 1)
		  {
			write_user(user,"~CW-~FT You have too many %s' in that line.. which could crash the talker..\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s ~FTtakes out a smoke and starts puffing away!\n");
			write_user(user,"~CW-~FT Line 2~CB: [~FGEnter what you want the social to execute if used by itself.~CB]\n");
			write_user(user,"~CW-~FT Line 2~CB:~RS ");
			user->misc_op	= 39;
			return(1);
		  }
		  if (cnt == 0)
		  {
			write_user(user,"~CW-~FT Your missing something.. did you read the helpfile?\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s ~FTtakes out a smoke and starts puffing away!\n");
			write_user(user,"~CW-~FT Line 2~CB: [~FGEnter what you want the social to execute if used by itself.~CB]\n");
			write_user(user,"~CW-~FT Line 2~CB:~RS ");
			user->misc_op	= 39;
			return(1);
		  }
		  strncpy(user->social[1],inpstr,sizeof(user->social[1])-1);
		  write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses $G2self a smoke and puffs away happily!\n");
		  write_user(user,"~CW- ~FTLine 3~CB: [~FGEnter what you want the social to execute if used on the same user.~CB]\n");
		  writeus(user,"~CW-~FT Line3~CB:[~FGFor example you type ~CW'~CR.%s %s~CW'~FG and hit enter.~CB]\n",user->social[0],user->name);
		  write_user(user,"~CW-~FT Line 3~CB:~RS ");
		  user->misc_op	= 42;
		  return(1);
		case 42:
		  if (!strlen(inpstr) || strlen(inpstr) > 250)
		  {
			write_user(user,"~CW-~FG That line would be either too long or too short.\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses $G2self a smoke and puffs away happily!\n");
			write_user(user,"~CW- ~FTLine 3~CB: [~FGEnter what you want the social to execute if used on the same user.~CB]\n");
			write_user(user,"~CW-~FT Line 3~CB:~RS ");
			user->misc_op = 42;
			return(1);
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FT Aborting social creation...\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  for (x = 0; x < strlen(inpstr); ++x)
		  {
			if (inpstr[x] == '%' && inpstr[x+1] == 's') ++cnt;
		  }
		  if (cnt > 1)
		  {
			write_user(user,"~CW-~FT You have too many %s' in that line.. which could crash the talker..\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses $G2self a smoke and puffs away happily!\n");
			write_user(user,"~CW- ~FTLine 3~CB: [~FGEnter what you want the social to execute if used on the same user.~CB]\n");
			write_user(user,"~CW-~FT Line 3~CB:~RS ");
			user->misc_op = 42;
			return(1);
		  }
		  if (cnt == 0)
		  {
			write_user(user,"~CW-~FT Your missing something.. did you read the helpfile?\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses $G2self a smoke and puffs away happily!\n");
			write_user(user,"~CW-~FT Line 3~CB: [~FGEnter what you want the social to execute if used on the same user.~CB]\n");
			write_user(user,"~CW-~FT Line 3~CB:~RS ");
			user->misc_op	= 42;
			return(1);
		  }
		  strncpy(user->social[2],inpstr,sizeof(user->social[2])-1);
		  write_user(user,"\n~FRSample~CB:\n~CB ::~CT %s ~FTtosses you a smoke and a lighter.\n");
		  write_user(user,"~CW- ~FTLine 4~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
		  write_user(user,"~CW- ~FTLine 4~CB: [~FGThis gets displayed to the given user~CB]\n");
		  writeus(user,"~CW-~FT Line 4~CB: [~FGFor example you type ~CW'~CR.%s squirt~CW'~FG and hit enter.~CB]\n",user->social[0]);
		  write_user(user,"~CW-~FT Line4~CB:~RS ");
		  user->misc_op	= 43;
		  return(1);
		case 43:
		  if (!strlen(inpstr) || strlen(inpstr) > 250)
		  {
			write_user(user,"~CW-~FG That line would be either too long or too short.\n");
			write_user(user,"\n~FRSample~CB:\n~CB::~CT %s ~FTtosses you a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 4~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
			write_user(user,"~CW- ~FTLine 4~CB: [~FGThis gets displayed to the given user~CB]\n");
			write_user(user,"~CW-~FT Line 4~CB:~RS ");
			user->misc_op	= 43;
			return(1);
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FT Aborting social creation...\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  for (x = 0; x < strlen(inpstr); ++x)
		  {
			if (inpstr[x] == '%' && inpstr[x+1] == 's') ++cnt;
		  }
		  if (cnt > 1)
		  {
			write_user(user,"~CW-~FT You have too many %s' in that line.. which could crash the talker..\n");
			write_user(user,"\n~FRSample~CB:\n~CB::~CT %s ~FTtosses you a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 4~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
			write_user(user,"~CW- ~FTLine 4~CB: [~FGThis gets displayed to the given user~CB]\n");
			write_user(user,"~CW-~FT Line 4~CB:~RS ");
			user->misc_op	= 43;
			return(1);
		  }
		  if (cnt == 0)
		  {
			write_user(user,"~CW-~FT Your missing something.. did you read the helpfile?\n");
			write_user(user,"\n~FRSample~CB:\n~CB::~CT %s ~FTtosses you a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 4~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
			write_user(user,"~CW- ~FTLine 4~CB: [~FGThis gets displayed to the given user~CB]\n");
			write_user(user,"~CW-~FT Line 4~CB:~RS ");
			user->misc_op	= 43;
			return(1);
		  }
		  strncpy(user->social[3],inpstr,sizeof(user->social[3])-1);
		  write_user(user,"\n~FRSample~CB:\n~CB::~FT You toss ~CT%s~FT a smoke and a lighter!\n");
		  write_user(user,"~CW- ~FTLine 5~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
		  write_user(user,"~CW- ~FTLine 5~CB: [~FGThis gets displayed to the user executing it.~CB]\n");
		  writeus(user,"~CW-~FT Line 5~CB: [~FGFor example you type ~CW'~CR.%s squirt~CW'~FG and hit enter.~CB]\n",user->social[0]);
		  write_user(user,"~CW-~FT Line 5~CB:~RS ");
		  user->misc_op	= 44;
		  return(1);
		case 44:
		  if (!strlen(inpstr) || strlen(inpstr) > 250)
		  {
			write_user(user,"~CW-~FG That line would be either too long or too short.\n");
			write_user(user,"\n~FRSample~CB:\n~CB::~FT You toss ~CT%s~FT a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 5~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
			write_user(user,"~CW- ~FTLine 5~CB: [~FGThis gets displayed to the user executing it.~CB]\n");
			write_user(user,"~CW-~FT Line 5~CB:~RS ");
			user->misc_op	= 44;
			return(1);
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FT Aborting social creation...\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  for (x = 0; x < strlen(inpstr); ++x)
		  {
			if (inpstr[x] == '%' && inpstr[x+1] == 's') ++cnt;
		  }
		  if (cnt > 1)
		  {
			write_user(user,"~CW-~FT You have too many %s' in that line.. which could crash the talker..\n");
			write_user(user,"\n~FRSample~CB:\n~CB::~FT You toss ~CT%s~FT a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 5~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
			write_user(user,"~CW- ~FTLine 5~CB: [~FGThis gets displayed to the user executing it.~CB]\n");
			write_user(user,"~CW-~FT Line 5~CB:~RS ");
			user->misc_op	= 44;
			return(1);
		  }
		  if (cnt == 0)
		  {
			write_user(user,"~CW-~FT Your missing something.. did you read the helpfile?\n");
			write_user(user,"\n~FRSample~CB:\n~CB::~FT You toss ~CT%s~FT a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 5~CB: [~FGWhat to execute if used on another user in a different room.~CB]\n");
			write_user(user,"~CW- ~FTLine 5~CB: [~FGThis gets displayed to the user executing it.~CB]\n");
			write_user(user,"~CW-~FT Line 5~CB:~RS ");
			user->misc_op	= 44;
			return(1);
		  }
		  strncpy(user->social[4],inpstr,sizeof(user->social[4])-1);
		  write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses ~CT%s~FT a smoke and a lighter!\n");
		  write_user(user,"~CW- ~FTLine 6~CB: [~FGWhat to execute if used on another user in the same room.~CB]\n");
		  writeus(user,"~CW-~FT Line 6~CB: [~FGFor example you type ~CW'~CR.%s squirt~CW'~FG and hit enter.~CB]\n",user->social[0]);
		  write_user(user,"~CW-~FT Line 6~CB:~RS ");
		  user->misc_op	= 45;
		  return(1);
		case 45:
		  if (!strlen(inpstr) || strlen(inpstr) > 250)
		  {
			write_user(user,"~CW-~FG That line would be either too long or too short.\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses ~CT%s~FT a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 6~CB: [~FGWhat to execute if used on another user in the same room.~CB]\n");
			write_user(user,"~CW-~FT Line 6~CB:~RS ");
			user->misc_op	= 45;
			return(1);
		  }
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FT Aborting social creation...\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  for (x = 0; x < strlen(inpstr); ++x)
		  {
			if (inpstr[x] == '%' && inpstr[x+1] == 's') ++cnt;
		  }
		  if (cnt > 2)
		  {
			write_user(user,"~CW-~FT You have too many %s' in that line.. which could crash the talker..\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses ~CT%s~FT a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 6~CB: [~FGWhat to execute if used on another user in the same room.~CB]\n");
			write_user(user,"~CW-~FT Line 6~CB:~RS ");
			user->misc_op	= 45;
			return(1);
		  }
		  if (cnt < 2)
		  {
			write_user(user,"~CW-~FT Your missing something.. did you read the helpfile?\n");
			write_user(user,"\n~FRSample~CB:\n~CT%s~FT tosses ~CT%s~FT a smoke and a lighter!\n");
			write_user(user,"~CW- ~FTLine 6~CB: [~FGWhat to execute if used on another user in the same room.~CB]\n");
			write_user(user,"~CW-~FT Line 6~CB:~RS ");
			user->misc_op	= 45;
			return(1);
		  }
		  strncpy(user->social[5],inpstr,sizeof(user->social[5])-1);
		  write_user(user,"~CW-~FG To actually save the social, just hit enter, or type ~CW'~CRsave~CW'\n");
		  write_user(user,"~CW-~FG If you want to bail out, type ~CW'~CRquit~CW'\n");
		  write_user(user,"~CW-~FG Your choice~CB:~RS ");
		  user->misc_op	= 46;
		  return(1);
		case 46: /* Save the social. */
		  if (!strcasecmp(word[0],"quit"))
		  {
			write_user(user,"~CW-~FG Your social creation has been aborted.\n");
			for (i = 0; i < 5; ++i) user->social[i][0] = '\0';
			user->misc_op = 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  else if (!strlen(inpstr) || !strcasecmp(word[0],"save"))
		  {
			strtolower(user->social[0]);
			sprintf(filename,"%s/%s.dat",SocialDir,user->social[0]);
			fp = fopen(filename,"w");
			if (fp == NULL)
			{
				write_user(user,"~CW-~FG Failed to save your social..\n");
				write_log(0,LOG1,"[ERROR] - Unable to open social file to save. Reason: [%s]\n",strerror(errno));
				user->misc_op	= 0;
				for (i = 0; i < 5; ++i)
					user->social[i][0] = '\0';
				rev_away(user);
				prompt(user);
				return(1);
			}
			add_social(user->social[0]);
			fputs(user->social[1],fp);	fputs("\n",fp);
			fputs(user->social[2],fp);	fputs("\n",fp);
			fputs(user->social[3],fp);	fputs("\n",fp);
			fputs(user->social[4],fp);	fputs("\n",fp);
			fputs(user->social[5],fp);	fputs("\n",fp);
			FCLOSE(fp);
			WriteRoomExcept(NULL,user,"~CB[~CRRaMTITS~CB] - ~FGA new social called ~CW'~FR%s~CW' ~FGhas been created.\n",user->social[0]);
			write_log(1,LOG1,"[Socials] - User: [%s] Created: [%s]\n",user->name,user->social[0]);
			for (i = 0; i < 5; ++i) user->social[i][0]='\0';
			write_user(user,"~CW-~FG Your social has been saved.\n");
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  else
		  {
			write_user(user,"~CW-~FG Choices are save or quit~CB:~RS ");
			user->misc_op	= 46;
		  }
		  return(1);
		case 47:
		  if ((get_command_full(word[0])) == NULL)
		  {
			write_user(user,"~CW-~FG There isn't even a command with that name.\n");
			write_user(user,"~CW-~FG Aborting helpfile writing..\n");
			user->misc_op	= 0;
			rev_away(user);
			prompt(user);
			return(1);
		  }
		  strcpy(user->check,word[0]);
		  sprintf(text,"~FT-~CT=~CB>~FG Writing a helpfile for~CB: [~FR%s~CB]~CB <~CT=~FT-\n",user->check);
		  write_user(user,center(text,80));
		  user->editor.editing	= 1;
		  user->misc_op		= 48;
		  user->tvar1		= 1;
		  user->ignall		= user->ignall_store;
		  user->ignall_store	= 0;
		  if (user->editor.editor == 1) nuts_editor(user,NULL);
		  else ramtits_editor(user,NULL);
		  return(1);
		case 49:
		  if (user->tvar2 == 0)
		  {
			writeus(user,"~CW-~FG Your helpfile ~CW'~FR%s~CW'~FG has been saved.. check it out with .h %s.\n",user->check,user->check);
			write_log(1,LOG1,"[HelpFiles] - User: [%s] Created: [%s]\n",user->name,user->check);
			WriteRoomExcept(NULL,user,"~CB[~CRRaMTITS~CB] - ~FGA helpfile called ~FR%s ~FGhas been created.\n",user->check);
		  }
		  else
		  {
			write_user(user,"~CW- ~FGYour helpfile has been aborted.\n");
		  }
		  user->check[0]	= '\0';
		  user->misc_op		= 0;
		  rev_away(user);
		  prompt(user);
		  user->tvar2		= 0;
		  user->tvar1		= 0;
		  return(1);
	}
	return(0);
}

/* Set global vars. hours,minutes,seconds,date,day,month,year */
void set_date_time(void)
{
	struct tm *tm_struct; /* structure is defined in time.h */
	time_t tm_num;

	/* Set up the structure */
	time(&tm_num);
	tm_struct=localtime(&tm_num);

	/* Get the values */
	tday=tm_struct->tm_yday;
	tyear=1900+tm_struct->tm_year;
	tmonth=tm_struct->tm_mon;
	tmday=tm_struct->tm_mday;
	twday=tm_struct->tm_wday;
	thour=tm_struct->tm_hour;
	tmin=tm_struct->tm_min;
	tsec=tm_struct->tm_sec; 
}

/* Shutdown the talker */
void talker_shutdown(User user,char *str,int reboot)
{
	User u,next;
	Room rm;
	int i;
	char *ptr;
	char *args[]={ progname,confile,NULL };

	if (user != NULL) ptr = user->name;  else ptr = str;
	save_talker_stuff();
	save_lotto();
	if (reboot)
	{
		write_room(NULL,"\07\n~CT[~CWSYSTEM~CT] ~CW- ~CR~LI Rebooting now...\n\n");
		sprintf(text,"-=> REBOOT initiated by %s <=-\n",ptr);
	}
	else
	{
		write_room(NULL,"\07\n~CT[~CWSYSTEM~CT]~CW - ~CR~LI Shutting down now...\n\n");
		sprintf(text,"-=> SHUTDOWN initiated by %s <=-\n",ptr);
	}
	write_log(1,LOG1,text);
	for_rooms(rm) SaveRoom(rm);
	u = user_first;
	while (u != NULL)
	{
		next=u->next;
		disconnect_user(u,1);
		u=next;
	}
	for(i=0;i<2;++i) close(listen_sock[i]);
	if (reboot)
	{
		/*
		   If someone has changed the binary or the config filename
		   while this prog has been running this won't work
		*/
		execvp(progname,args);
		/* If we get this far it hasn't worked */
		write_log(0,LOG1,"-=> REBOOT FAILED %s: %s <=-\n",long_date(1),strerror(errno));
		exit(12);
	}
	unlink("runtime/ramtits.pid");
	write_log(0,LOG1,"-=> SHUTDOWN complete %s <=-\n",long_date(1));
	exit(0);
}

void do_events(int sig)
{

	set_date_time();
	check_pings();
	check_reboot_shutdown();
	check_idle_and_timeout();
	check_atmos();
	check_sockets();
	check_messages(NULL,NULL,0);
	check_logins();
	nuclear_count();
	CheckSysCommands();
	ClearConnlist();
	LotteryDraw();
	reset_alarm();
}

void reset_alarm(void)
{

	SIGNAL(SIGALRM,do_events);
	alarm(heartbeat);
}

/* See if timed reboot or shutdown is underway */
void check_reboot_shutdown(void)
{
	int secs;
	char *w[]={ "Shutdown","Rebooting","Seamless reboot" };

	if (rs_user == NULL) return;
	rs_countdown -= heartbeat;
	if (rs_countdown <= 0)
	{
		if (rs_which == 1 || !rs_which)
		  talker_shutdown(rs_user,NULL,rs_which);
		else
		{
			rs_countdown = 0;
			rs_which = -1;
			do_reboot(rs_user);
			return;
		}
	}
	/*
	   Print countdown message every minute unless we have less than 1
	   minute to go when we print every 10 secs.
	*/
	secs = (int)(time(0)-rs_announce);
	if (rs_countdown>=60 && secs >= 60)
	{
		WriteRoom(NULL,"    ~CT[~CRRaMTITS~CT] ~FG%s in ~CR%d ~FGminute%s, ~CR%d ~FGsecond%s.\n",w[rs_which],rs_countdown/60,rs_countdown/60>1?"s":"",rs_countdown%60,rs_countdown%60>1?"s":"");
		rs_announce = time(0);
	}
	if (rs_countdown<60 && secs >= 10)
	{
		WriteRoom(NULL,"    ~CT[~CRRaMTITS~CT] ~FG%s in ~CR%d ~FGseconds.\n",w[rs_which],rs_countdown);
		rs_announce = time(0);
	}
}

/*
   login_time_out is the length of time someone can idle at login,
   user_idle_time is the length of time they can idle once logged in
   Also ups users total login time.
*/
void check_idle_and_timeout(void)
{
	User user,next;
	Room rm,rm2;
	int min,has_printed=0;

	/*
	   Use while loop here instead of for loop for when user structure
	   gets destructed, we may lose ->next link and crash the program,
	   this function also does more things, such as increasing the users
	   money, and checks their alarm clock. If you would like to change
	   how often a user earns 1 dollar, change the line if
	   (user->money_time>=65, to how often you want a user to earn money.
	   the time is in seconds.and seeing as how I noticed the heartbeat
	   goes every 2 seconds, the user makes about a buck every 66
	   seconds.
	*/
	user = user_first;
	while (user != NULL)
	{
		next = user->next;
		if (user->type == CloneType)
		{
			user = next;
			continue;
		}
		min = (int)(time(0)-user->last_input);
		if (user->login && min >= login_idle_time)
		{
			has_printed = 1;
			write_user(user,"\n\r");
			write_user(user,center("~FB-~CB==~FT[ ~FGYou've had plenty of time to login...buhbye. ~FT]~CB==~FB-\n\r",80));
			disconnect_user(user,1);
			user=next;
			continue;
		}
		if (user->money_time >= 66)
		{
			if (user->earning) user->money++;
			user->money_time = 0;
		}
		user->money_time += heartbeat;
		user->total_login += heartbeat; 
		if (user->alarm_time) user->alarm_time -= heartbeat;
		if (user->alarm_time <= 0 && user->alarm_set) sound_alarm(user);
		if (user->warned)
		{
			if (user->level>time_out_maxlevel)
			{
				user = next;
				continue;
			}
			if (min<user_idle_time-3060)
			{
				if (user->room == get_room(idle_room))
				{
					rm2 = get_room(main_room);
					if (rm2 == NULL)
					{
						user->warned = 0;
						has_printed  = 1;
						write_user(user,"~CW- ~CR Unable to find the main room at the moment..so you stay here.\n");
						user = next;
						continue;
					}
					else
					{
						user->room = rm2;
						user->warned = 0;
						write_user(user,"~CW- ~FT Welcome back to the land of the living.\n");
						has_printed = 1;
						user = next;
						continue;
					}
				}
				else
				{
					user->warned = 0;
					has_printed  = 1;
					write_user(user,"~FT Welcome back to the land of the living.\n");
					user = next;
					continue;
				}
			}
			if (min >= user_idle_time)
			{
				has_printed = 1;
				write_user(user,center("~FT-~CT==~CB> ~CRYou have been disconnected for idling to long. ~CB<~CT==~FT-\n",80));
				disconnect_user(user,1);
				user = next;
				continue;
			}
		}
		if ((!user->afk || (user->afk && time_out_afks))
		  && user->login == 0 && user->warned == 2
		  && min >= user_idle_time-3000)
		{
			rm = get_room(idle_room);
			if (user->level>time_out_maxlevel)
			{
				show_prompt(user);
			}
			else if (rm == NULL
			  || user->room != get_room(main_room))
			{
				has_printed = 1;
				user->warned = 1;
				write_user(user,"~CB\07--~CRIDLE ALERT~CB-- ~FMYou have been idling over ten minutes.\n");
			}
			else
			{
				has_printed = 1;
				user->room = rm;
				user->warned = 1;
				write_user(user,center("~FT-~CT==~CB>~FY You have been moved to the idle room for idling to long. ~CB<~CT==~FT-\n",80));
			}
		}
		/* 9 min alert, let them know they're gonna be moved */
		if ((!user->afk || (user->afk && time_out_afks))
		  && user->login == 0 && user->warned == 0
		  && min >= user_idle_time-3060)
		{
			if (user->level > time_out_maxlevel)
			{
				show_prompt(user);
			}
			else if (user->room == get_room(main_room))
			{
				has_printed = 1;
				user->warned = 2;
				write_user(user,"~CB\07--~CRIDLE ALERT~CB-- ~FMInput within 1 minute or you'll be moved.\n");
			}
			else user->warned = 2;
		}
		show_prompt(user);
		user = next;
	}
	if (has_printed)
	{
		write_window(NULL,NULL,0,0);
	}
}
	
/* check the atmospherics */
void check_atmos(void)
{
	User u;
	int at=0,hp=0;

	++atmos_trigger;
	if (atmos_trigger >= AtmosFreq)
	{
		for_users(u)
		{
			if (u->login || u->type == CloneType || u->ignall
			  || (u->ignore & Atmos)) continue;
			at = ran(MaxAtmos);
			if (u->room->atmos[at][0] != '\0')
			{
				writeus(u,AtmosString,u->room->atmos[at]);
				hp = 1;
			}
		}
		atmos_trigger = 0;
	}
	if (hp == 1)
	{
		write_window(NULL,NULL,0,0);
	}
}

/* Check the login vars, and reset certain ones. */
void check_logins(void)
{

	if (thour == 0 && tmin == 0)	/* New day? */
	{
		if (tmday == 1)
		{
			newlog[2] = 0;
			ulogins[2] = 0;
		}
		newlog[1] = 0;
		ulogins[1] = 0;
	}
}

/* Go through and close any open dead sockets */
void check_sockets(void)
{

	sclosetime += heartbeat;
	if (sclosetime >= 3600)	/* check every 30 minutes */
	{
		close_sockets();
		sclosetime = 0;
	}
}

/*
  This is function goes through all sockets that are open on listen_sock[]
  and closes them.
*/
void close_sockets(void)
{
	User u;
	Telnet t;
	int i,d = 0;

	for (i = 3; i < (i<<12); i++)  /* start at 3 so we don't kill stdout, etc */
	{
		if (i == listen_sock[0] || i == listen_sock[1]) continue;
		if (i == IDENT_CLIENT_READ) continue;
		if (i == IDENT_CLIENT_WRITE) continue;
		for_users(u)
		{
			if (u->socket == i) break;
		}
		if (u != NULL) continue;
		/* Bug fix for v1.1.3 fuck I'm dumb */
		for_telnet(t)
		{
			if (t->socket == i) break;
		}
		if (t != NULL) continue;
		if (close(i) == 0) d++;
	}
	return;
}


/*
   What this function does.. is we check the status of the user->pause_time
   ignoring it if it's -1 otherwise.. we know we've paged a file via .finger
   or something, and we're waiting for the system command to finish writing
   it out to the appropriate file.. once we're done (time(0) is >= the
   user->pause_time flag) we page the file out to the user..
*/
void CheckSysCommands(void)
{
	User u,next;

	u = user_first;
	while (u != NULL)
	{
		next = u->next;
		/* set to -1.. no need to go any further. */
		if (u->pause_time == -1)
		{
			u = next;
			continue;
		}
		/* We haven't finished paging yet.. let's keep waiting.. */
		if (time(0)<u->pause_time)
		{
			u = next;
			continue;
		}
		/* We've finished.. so page the file out to the user.. */
		else if (time(0) >= u->pause_time)
		{
			u->pause_time = -1;
			switch (u->sys_com)
			{
				case 1:
				  uni_inger(u,NULL,1);
				  break;
				case 2:
				  uni_ois(u,NULL,1);
				  break;
				case 3:
				  ns_lookup(u,NULL,1);
				  break;
				case 4:
				  goto_webpage(u,NULL,1);
				  break;
				default:
				  u->sys_com = 0;
				  break;
			}
			u = next;
			continue;
		}
		u = next;
	}
}

/*
   This will do the lottery draw once a week, on thursdays at 6:49 pm system
   time. I figured I'd shape it up more after the Canadian way ;-)
*/
void LotteryDraw(void)
{
	User u;
	Lotto l;
	char filename[FSL];
	static int drawn = 0;
	int ball[6],i = 0,good = 0,cnt = 0;

	if (strcasecmp(day[twday],"Thurs.")) return; /* it ain't thursday */
	if (thour == 18 && tmin == 49) /* 6:49 pm - 24 hour time */
	{
		if (drawn == 1) return;
	}
	else
	{
		drawn = 0;
		return;
	}
	drawn = 1;
	for (i = 0; i < 6 ; ++i) ball[i] = 0;
	write_room(NULL,"~CW - ~FGIt is now time to draw this weeks lucky numbers for the lottery.\n");
	/* get the numbers */
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
	} while (ball[0] == ball[3] || ball[1] == ball[3]
	    || ball[2] == ball[3]);
	do
	{
		ball[4] = ran(75);
	} while (ball[0] == ball[4] || ball[1] == ball[4]
	    || ball[2] == ball[4] || ball[3] == ball[4]); 
	do
	{
		ball[5] = ran(75);
	} while (ball[0] == ball[5] || ball[1] == ball[5]
	    || ball[2] == ball[5] || ball[3] == ball[5] || ball[4] == ball[5]);
	qsort(ball,6,sizeof(ball[0]),intcmp);
	WriteRoom(NULL,"~CW - ~FGThis weeks lucky numbers are~CB:~FT %d, %d, %d, %d, %d and %d\n",ball[0],ball[1],ball[2],ball[3],ball[4],ball[5]);
	for_lotto(l)
	{
		if (l->ball[0] == ball[0] && l->ball[1] == ball[1]
		  && l->ball[2] == ball[2] && l->ball[3] == ball[3]
		  && l->ball[4] == ball[4] && l->ball[5] == ball[5]) 
		{
			++cnt;
			if ((u = get_user(l->name)) == NULL)
			{
				if ((u = create_user()) == NULL)
				{
					write_log(0,LOG1,"[ERROR] - Couldn't create a temp. user object in LotteryDraw().\n");
					continue;
				}
				reset_vars(u);
				good = LoadUser(u,l->name);
				if (good == 1)
				{
					u->money += jackpot;
					sprintf(text,"~FTYou've won this weeks lottery jackpot, a total of %ld dollars.\n",jackpot);
					send_oneline_mail(NULL,u->name,text);
					WriteRoom(NULL,"~CW-~FT %s ~FGhas won %ld dollars.\n",u->recap,jackpot);
					SaveUser(u,0);
				}
				destruct_user(u);
				destructed = 0;
				continue;
			}
			else
			{
				writeus(u,"~CW-~FT Congrats, you won the jackpot of %ld dollars.\n",jackpot);
				u->money += jackpot;
				WriteRoomExcept(NULL,u,"~CW-~FT %s~FG has won %ld dollars.\n",u->recap,jackpot);
				continue;
			}
		}
		jackpot = DefaultJackpot; /* reset the jackpot */
	}
	if (cnt == 0)
	{
		write_room(NULL,"~CW - ~FGBut it appears there are no winners, so the jackpot has been raised 500 dollars.\n");
		jackpot += 500;
	}
	write_room(NULL,"~CW-~FT Thanks for partaking in this weeks lottery, please donate again for next weeks.\n");
	sprintf(filename,"%s/lotto.dat",MiscDir);
	unlink(filename);
	clear_tickets();
	return;
}

/*
   This'll write out the talkers PID to a file so you'll be able to shutdown
   the talker from the shell without a problem :-)
*/
void write_pid_file(void)
{
	FILE *fp;
	char filename[FSL];

	sprintf(filename,"runtime/ramtits.pid");
	if (file_exists(filename)) unlink(filename);	/* Out with the old */
	if ((fp = fopen(filename,"w")) != NULL)
	{
		fprintf(fp,"%u",(unsigned)getpid());	/* In with the new  */
		FCLOSE(fp);
	}
	return;
}

/* END OF MAIN.C */
