/*
  editor.c
    Source code for the two different editors.

    The source within this file contains Copyrights 1996 - 2001 by
 	Neil Robertson, Frank Chavez, Rob Melhuish, Arnaud Abelard,
 	and Andrew Collington.

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
#define _EDITOR_C
  #include "include/needed.h"
#undef _EDITOR_C

/* The normal NUTS editor.. w/ a few minor tweaks. */
void nuts_editor(User user,char *inpstr)
{
	int cnt,line;
	char *ptr,*name;

	cnt = 0;
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (user->editor.maxlines == -1)
	{
		switch (user->misc_op)
		{
			case 4 :
			case 10:
			case 11:
			case 21:  user->editor.maxlines=25;		break;
			case 6 :  user->editor.maxlines=10;		break;
			case 7 :  user->editor.maxlines=50;		break;
			case 22:  user->editor.maxlines=8;		break;
			case 40:  user->editor.maxlines=20;		break;
			case 41:  user->editor.maxlines=40;		break;
			case 48:  user->editor.maxlines=50;		break;
			case 50:  user->editor.maxlines=15;		break;
			default:  user->editor.maxlines=DefaultLines;	break;
		}
	}
	if (user->editor.maxlines>MaxLines) user->editor.maxlines = MaxLines;
	if (user->editor.edit_op)
	{
		switch (toupper(*inpstr))
		{
			case 'S':  /* Save */
			  if (user->mail_op == 0)
				  WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FTfinishes up %s carvings...\n",name,gender1[user->gender]);
			  switch (user->misc_op)
			  {
				case 3 :  write_board(user,NULL,1);	break;
				case 4 :  smail(user,NULL,1);		break;
				case 5 :  enter_profile(user,NULL,1);	break;
				case 7 :  add_news(user,NULL,1);	break;
				case 8 :  room_desc(user,NULL,1);	break;
				case 9 :  suggestions(user,NULL,1);	break;
				case 10:  friend_smail(user,1);		break;
				case 11:  staff_smail(user,1);		break;
				case 12:  bugs(user,NULL,1);		break;
				case 21:  users_smail(user,1);		break;
				case 22:  enter_vote(user,NULL,1);	break;
				case 40:  wiznote(user,NULL,1);		break;
				case 41:  email_user(user,NULL,1);	break;
				case 48:  make_helpfile(user,1);	break;
				case 50:  enter_riddle(user,NULL,1);	break;
			  }
			  editor_done(user);
			  return;
			case 'R':  /* Re-do */
			  user->editor.edit_op	  = 0;
			  user->editor.edit_line  = 1;
			  user->editor.malloc_end = user->editor.malloc_start;
			  user->editor.charcnt	  = 0;
			  *user->editor.malloc_start = '\0';
			  cls(user,NULL,user->socket);
			  /* Bring the cursor to the top of the window. */
			  if (!user->splits) write_window(user,"\033[H\033[J",strlen("\033[H\033[J"),WriteDefault);
			  write_user(user,center("~FT-~CT=~CB>~FG Starting your message all over... ~CB<~CT=~FT-\n",80));
			  sprintf(text,"~FT-~CT=~CB> ~FGYou can write a maximum of ~CR%d~FG lines. ~CB<~CT=~FT-\n",user->editor.maxlines);
			  write_user(user,center(text,80));
			  write_user(user,center("~FT-~CT=~CB>~FG When finished, use a ~CW'~CR.~CW'~FG on a line by itself. ~CB<~CT=~FT-\n",80));
			  write_user(user,center("~FT-~CT=~CB>~FG For best results, stay between the editor's margins. ~CB<~CT=~FT-\n",80));
			  write_user(user,"   ~CB[~FM-=-=-=-=-~FT1~FM-=-=-=-=-~FT2~FM-=-=-=-=-~FT3~FM-=-=-=-=-~FT4~FM-=-=-=-=-~FT5~FM-=-=-=-=-~FT6~FM-=-=-=-=-~FT7~FM-=-~CB]\n");
			  writeus(user,"~FG%2d~CB]~RS",user->editor.edit_line);
			  return;
			case 'V': /* View */
			  user->tvar3 = 1;
			  redraw_edit(user,NULL);
			  user->tvar3 = 0;
			  write_user(user,edprompt);
			  return;
			case 'A': /* Abort */
			  write_user(user,"\n\n");
			  write_user(user,center("~FT-~CT=~CB> ~FGAborting your message... ~CB<~CT=~FT-\n",80));
			  sprintf(text,"~CW-~FR %s~FT realizes that %s can't write worth crap...\n",name,gender3[user->gender]);
			  write_room_except(user->room,user,text);
			  user->tvar2 = 1;
			  editor_done(user);
			  return;
			default : /* Something else */
			  redraw_edit(user,NULL);
			  write_user(user,edprompt);
			  return;
		}
	}
	/* If user has just started editing.. allocate the memory */
	if (user->editor.malloc_start==NULL)
	{
		if ((user->editor.malloc_start = (char *)malloc(user->editor.maxlines*79)) == NULL)
		{
			write_user(user,"~FG Sorry.. an error occurred while trying to create an editor.\n");
			write_log(0,LOG1,"[ERROR] - malloc FAILED in nuts_editor()\n");
			user->misc_op=0;
			prompt(user);
			return;
		}
		if (user->mail_op == 0)
		{
			user->ignall_store = user->ignall;
			user->ignall = 1;
		}
		user->editor.editing = 1;
		user->editor.edit_line = 1;
		user->editor.malloc_end = user->editor.malloc_start;
		user->editor.charcnt = 0;
		*user->editor.malloc_start = '\0';
		cls(user,NULL,user->socket);
		/* Bring the cursor to the top of the window. */
		if (!user->splits) write_window(user,"\033[H\033[J",strlen("\033[H\033[J"),WriteDefault);
		sprintf(text,"~FT-~CT=~CB> ~FGYou can write a maximum of ~CR%d~FG lines. ~CB<~CT=~FT-\n",user->editor.maxlines);
		write_user(user,center(text,80));
		write_user(user,center("~FT-~CT=~CB>~FG When finished, use a ~CW'~CR.~CW'~FG on a line by itself. ~CB<~CT=~FT-\n",80));
		write_user(user,center("~FT-~CT=~CB>~FG For best results, stay between the editor's margins. ~CB<~CT=~FT-\n",80));
		write_user(user,"   ~CB[~FM-=-=-=-=-~FT1~FM-=-=-=-=-~FT2~FM-=-=-=-=-~FT3~FM-=-=-=-=-~FT4~FM-=-=-=-=-~FT5~FM-=-=-=-=-~FT6~FM-=-=-=-=-~FT7~FM-=-~CB]\n");
		write_user(user,"~FG 1~CB]~RS");
		if (user->mail_op == 0)
			WriteRoomExcept(user->room,user,"~CW-~FR %s ~FTstarts writing up a message...\n",name);
		return;
	}
	/* Check for an empty line. */
	if (!word_count)
	{
		if (!user->editor.charcnt)
		{
			writeus(user,"~FG%2d~CB]~RS",user->editor.edit_line);
			redraw_edit(user,NULL);
			return;
		}
		*user->editor.malloc_end++ = '\n';
		if (user->editor.edit_line >= user->editor.maxlines) goto END;
		writeus(user,"~FG%2d~CB]~RS",++user->editor.edit_line);
		user->editor.charcnt = 0;
		redraw_edit(user,NULL);
		return;
	}
	/* if nothing carried over and a dot is entered then end. */
	if (!user->editor.charcnt && !strcmp(inpstr,".")) goto END;
	line = user->editor.edit_line;
	/* loop through input and store in allocated memory. */
	while (*inpstr)
	{
		*user->editor.malloc_end++ = *inpstr++;
		if (++cnt >= 79)
		{
			user->editor.edit_line++;
			cnt = 0;
		}
		if (user->editor.edit_line>user->editor.maxlines
		  || user->editor.malloc_end-user->editor.malloc_start >= user->editor.maxlines*79)
		  goto END;
	}
	if (line!=user->editor.edit_line)
	{
		ptr = (char *)(user->editor.malloc_end-cnt);
		*user->editor.malloc_end = '\0';
		sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s",user->editor.edit_line,ptr);
		write_window(user,text,strlen(text),WriteDefault);
		user->editor.charcnt=cnt;
		return;
	}
	else
	{
		*user->editor.malloc_end++ = '\n';
		user->editor.charcnt = 0;
	}
	if (user->editor.edit_line != user->editor.maxlines)
	{
		++user->editor.edit_line;
		redraw_edit(user,NULL);
		return;
	}
	/* User has finished his message, prompt the bitch. */
	END:
	  *user->editor.malloc_end = '\0';
	  if (*user->editor.malloc_start)
	  {
		user->editor.edit_op = 1;
		redraw_edit(user,NULL);
		write_user(user,edprompt);
		return;
	  }
	  write_user(user,"\n\n");
	  write_user(user,center("~FT-~CT=~CB> ~FGAborting your message... ~CB<~CT=~FT-\n",80));
	  if (user->mail_op == 0)
		  WriteRoomExcept(user->room,user,"~CW-~FR %s~FT realizes that %s can't write worth crap...\n",name,gender3[user->gender]);
	  editor_done(user);
}

/*
   Now the RaMTITS editor.. users have a choice of which editor they would
   rather use.. this one has more functionality to it.. and I've improved
   on it from the original version.. A big thanks goes to Nuke for helping
   me with this, and showing me what to do with it.
*/
void ramtits_editor(User user,char *inpstr)
{
	FILE *fp;
	Editor edit = get_editor(user);
	int cnt = 0,line = 0,i;
	char line2[ARR_SIZE],filename[FSL];
	char *str2,*name;

	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (user->editor.maxlines == -1)
	{
		switch (user->misc_op)
		{
			case 4 :
			case 10:
			case 11:
			case 21:  user->editor.maxlines=25;		break;
			case 6 :  user->editor.maxlines=10;		break;
			case 7 :  user->editor.maxlines=50;		break;
			case 22:  user->editor.maxlines=8;		break;
			case 40:  user->editor.maxlines=20;		break;
			case 41:  user->editor.maxlines=40;		break;
			case 48:  user->editor.maxlines=50;		break;
			case 50:  user->editor.maxlines=15;		break;
			default:  user->editor.maxlines=DefaultLines;	break;
		}
	}
	if (user->editor.maxlines>MaxLines) user->editor.maxlines = MaxLines;
	if (user->editor.edit_op) goto EDITING;
	/* Create the editor if it doesn't already exist. */
	if (edit == NULL)
	{
		if ((edit = create_editor()) == NULL)
		{
			write_user(user,"~FG Sorry, I was unable to open an editor right now..\n");
			write_log(0,LOG1,"[ERROR] - Memory allocation failure in ramtits_editor()\n");
			user->misc_op = 0;	prompt(user);
			return;
		}
		edit->user		= user;
		edit->current_line	= 0;
		edit->last_line		= 0;
		edit->max_lines		= user->editor.maxlines;
		edit->charcnt		= 0;
		edit->current_char 	= 0;
		for (i = 0 ; i<MaxLines ; ++i) edit->line[i][0] = '\0';
		cls(user,NULL,user->socket);
		edit->edited		= 0;
		user->editor.edit_line	= 1;
		user->editor.charcnt	= 0;
		if (user->mail_op == 0)
		{
			user->ignall_store	= user->ignall;
			user->ignall		= 1;
		}
		/* Bring the cursor to the top of the window. */
		if (!user->splits) write_window(user,"\033[H\033[J",strlen("\033[H\033[J"),WriteDefault);
		sprintf(text,"~FT-~CT=~CB> ~FGYou can write a maximum of ~CR%d~FG lines. ~CB<~CT=~FT-\n",user->editor.maxlines);
		write_user(user,center(text,80));
		write_user(user,center("~FT-~CT=~CB>~FG When finished, use ~CW'~CR.s~CW'~FG on a line by itself. ~CB<~CT=~FT-\n",80));
		write_user(user,center("~FT-~CT=~CB>~FG For best results, stay between the editor's margins. ~CB<~CT=~FT-\n",80));
		write_user(user,"   ~CB[~FM-=-=-=-=-~FT1~FM-=-=-=-=-~FT2~FM-=-=-=-=-~FT3~FM-=-=-=-=-~FT4~FM-=-=-=-=-~FT5~FM-=-=-=-=-~FT6~FM-=-=-=-=-~FT7~FM-=-~CB]\n");
		write_user(user,"~FG 1~CB]~RS");
		if (user->mail_op == 0)
			WriteRoomExcept(user->room,user,"~CW-~FR %s ~FTstarts writing up a message...\n",name);
		return;
	}
	/* Check for empty line */
	if (!word_count)
	{
		if (!edit->charcnt)
		{
			edit->line[edit->current_line][0] = '\0';
			if (edit->line[edit->current_line][0] == '\0')
			  strcpy(edit->line[edit->current_line],"  \n");
			if (edit->current_line>=user->editor.maxlines) goto END;
			if (edit->current_line == edit->last_line) edit->last_line++;
			edit->current_line++;
			edit->charcnt = 0;
			if (edit->current_line >= user->editor.maxlines) goto END;
			edit->current_char = 0;
			edit->edited = 1;
			redraw_edit(user,edit);
			return;
		}
		edit->line[edit->current_line][0] = '\0';
		edit->line[edit->current_line][edit->current_char] = '\n';
		if (edit->current_line >= user->editor.maxlines) goto END;
		if (edit->current_line == edit->last_line) edit->last_line++;
		edit->current_line++;	edit->charcnt = 0;
		if (edit->current_line >= user->editor.maxlines) goto END;
		edit->current_char = 0;
		redraw_edit(user,edit);
		return;
	}
	EDITING:
	  if ((!edit->charcnt && inpstr[0] == '.') || user->editor.edit_op)
	  {
		switch (toupper(inpstr[1]))
		{
			case 'H':
			  if (inpstr[2]) break;
			  write_user(user,center("~FT-~CT=~CB> ~CRRaMTITS~FG Editor command help.. ~CB<~CT=~FT-\n",80));
			  write_user(user,"~CW- ~FTBe advised.. these options aren't case sensitive.\n");
			  write_user(user,"~CW- ~CR.h  ~CB= ~FTThis help menu.\n");
			  write_user(user,"~CW- ~CR.s  ~CB= ~FTSave your message.\n");
			  write_user(user,"~CW- ~CR.a  ~CB= ~FTAborts your message.\n");
			  write_user(user,"~CW- ~CR.l  ~CB= ~FTLoads a file. ~CW(~CRRoom desc. & profiles only~CW)\n");
			  write_user(user,"~CW- ~CR.r  ~CB= ~FTLets you restart your message.\n");
			  write_user(user,"~CW- ~CR.v  ~CB= ~FTLets you review what you've written so far.\n");
			  write_user(user,"~CW- ~CR.+  ~CB= ~FTLets you advance a line.\n");
			  write_user(user,"~CW- ~CR.-  ~CB= ~FTLets you go to the previous line.\n");
			  write_user(user,"~CW- ~CR.#  ~CB= ~FTLets you jump to a line number.\n");
			  write_user(user,"~CW- ~CR.d# ~CB= ~FTLets you delete a certain line number.\n");
			  write_user(user,"   ~CB[~FM-=-=-=-=-~FT1~FM-=-=-=-=-~FT2~FM-=-=-=-=-~FT3~FM-=-=-=-=-~FT4~FM-=-=-=-=-~FT5~FM-=-=-=-=-~FT6~FM-=-=-=-=-~FT7~FM-=-~CB]\n");
			  if (!user->editor.edit_op) writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
			  else write_user(user,"~CW- ~FTEditor command~CW '~CR.h~CW'~FT for help~CB:~RS ");
			  return;
			case 'L':
			  if (inpstr[2]) break;
			  switch (user->misc_op)
			  {
				case 5 :
				  sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,user->name);
				  break;
				case 8 :
				  sprintf(filename,"%s/%s/%s.R",RoomDir,Descript,user->room->name);
				  break;
				default:
				  write_user(user,"\n\n~CW-~FG You can only load room descriptions and profiles.\n");
				  return;
			  }
			  if ((fp = fopen(filename,"r")) == NULL)
			  {
				write_user(user,"\n\n~CW- ~FGThere wasn't an old file to load up.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  for (i = 0 ; i<MaxLines ; ++i) edit->line[i][0] = '\0';
			  line2[0] = '\0';
			  cnt = 0;
			  edit->current_line = 0;
			  edit->current_char = 0;
			  fgets(line2,ARR_SIZE-1,fp);
			  while (!(feof(fp)))	 /* Lets load it up */
			  {
				str2 = line2;
				while (*str2)
				{
					if (++cnt >= 79 || *str2 == '\n')
					{
						if (edit->current_line==edit->last_line) edit->last_line++;
						edit->line[edit->current_line][edit->current_char] = '\n';
						edit->line[edit->current_line][edit->current_char+1] = '\0';
						str2++;
						edit->current_line++;
						cnt = 0;
						edit->current_char = 0;
						writeus(user,"\n~FG%2d~CB]~RS",edit->current_line+1);
						continue;
					}
					writeus(user,"%c",*str2);
					edit->line[edit->current_line][edit->current_char++] = *str2++;
					if (edit->current_line+1>=user->editor.maxlines)
					{
						FCLOSE(fp);
						goto END2;
					}
				}
				fgets(line2,ARR_SIZE-1,fp);
			  } /* End of while */
			  FCLOSE(fp);
			  user->tvar3 = 1;
			  redraw_edit(user,edit);
			  user->tvar3 = 0;
			  END2:
			    if (cnt) edit->charcnt=cnt;
			    if (edit->current_char) edit->line[edit->current_line][edit->current_char] = '\0';
			    edit->edited = 1;
			    user->tvar3 = 1;
			    redraw_edit(user,edit);
			    user->tvar3 = 0;
			    return;
			case 'S':
			  if (inpstr[2]) break;
			  edit->line[edit->current_line][edit->current_char] = '\0';
			  if (edit->edited)
			  {
				if (user->mail_op == 0)
					WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FTfinishes up %s carvings...\n",name,gender1[user->gender]);
				switch (user->misc_op)
				{
					case 3 :
					  write_board(user,NULL,1);
					  break;
					case 4 :
					  smail(user,NULL,1);
					  break;
					case 5 :
					  enter_profile(user,NULL,1);
					  break;
					case 7 :
					  add_news(user,NULL,1);
					  break;
					case 8 :
					  room_desc(user,NULL,1);
					  break;
					case 9 :
					  suggestions(user,NULL,1);
					  break;
					case 10:
					  friend_smail(user,1);
					  break;
					case 11:
					  staff_smail(user,1);
					  break;
					case 12:
					  bugs(user,NULL,1);
					  break;
					case 21:
					  users_smail(user,1);
					  break;
					case 22:
					  enter_vote(user,NULL,1);
					  break;
					case 40:
					  wiznote(user,NULL,1);
					  break;
					case 41:
					  email_user(user,NULL,1);
					  break;
					case 48:
					  make_helpfile(user,1);
					  break;
					case 50:
					  enter_riddle(user,NULL,1);
					  break;
				}
				editor_done(user);
				return;
			  }
			  write_user(user,"\n\n");
			  write_user(user,center("~FT-~CT=~CB> ~FGAborting your message... ~CB<~CT=~FT-\n",80));
			  if (user->mail_op == 0)
				  WriteRoomExcept(user->room,user,"~CW-~FR %s~FT realizes that %s can't write worth crap...\n",name,gender3[user->gender]);
			  editor_done(user);
			  return;
			case 'R':
			  if (inpstr[2]) break;
			  user->editor.edit_op = 0;
			  user->editor.edit_line = 1;
			  user->editor.charcnt = 0;
			  for (i = 0 ; i<MaxLines ; ++i) edit->line[i][0] = '\0';
			  edit->current_line = 0;
			  edit->current_char = 0;
			  edit->last_line = 0;
			  edit->charcnt = 0;
			  cls(user,NULL,user->socket);
			  /* Bring the cursor to the top of the window. */
			  if (!user->splits) write_window(user,"\033[H\033[J",strlen("\033[H\033[J"),WriteDefault);
			  write_user(user,center("~FT-~CT=~CB>~FG Starting your message all over... ~CB<~CT=~FT-\n",80));
			  sprintf(text,"~FT-~CT=~CB> ~FGYou can write a maximum of ~CR%d~FG lines. ~CB<~CT=~FT-\n",user->editor.maxlines);
			  write_user(user,center(text,80));
			  write_user(user,center("~FT-~CT=~CB>~FG When finished, use a ~CW'~CR.~CW'~FG on a line by itself. ~CB<~CT=~FT-\n",80));
			  write_user(user,center("~FT-~CT=~CB>~FG For best results, stay between the editor's margins. ~CB<~CT=~FT-\n",80));
			  write_user(user,"   ~CB[~FM-=-=-=-=-~FT1~FM-=-=-=-=-~FT2~FM-=-=-=-=-~FT3~FM-=-=-=-=-~FT4~FM-=-=-=-=-~FT5~FM-=-=-=-=-~FT6~FM-=-=-=-=-~FT7~FM-=-~CB]\n");
			  writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
			  return;
			case 'V':
			  user->tvar3 = 1;
			  redraw_edit(user,edit);
			  user->tvar3 = 0;
			  return;
			case 'A':
			  if (inpstr[2]) break;
			  write_user(user,"\n\n");
			  write_user(user,center("~FT-~CT=~CB> ~FGAborting your message... ~CB<~CT=~FT-\n",80));
			  if (user->mail_op == 0)
				  WriteRoomExcept(user->room,user,"~CW-~FR %s~FT realizes that %s can't write worth crap...\n",name,gender3[user->gender]);
			  user->tvar2 = 1;
			  editor_done(user);
			  return;
			case 'D':
			  if (!isdigit(inpstr[2])) break;
			  inpstr += 2;		line = 0;
			  line = atoi(inpstr);
			  while (inpstr[0])
			  {
				if (!isdigit(inpstr[0]))
				{
					write_user(user,"~CW-~FT Line must be a number only.\n");
					writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
					return;
				}
				inpstr++;
			  }
			  line = line-1;
			  if (line == -1 || line>user->editor.maxlines)
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  if (!edit->line[line][0]) write_user(user,"~CW-~FT That line is currently empty.\n");
			  else
			  {
				edit->line[line][0] = '\0';	edit->edited++;
				strcpy(edit->line[line],"  \n");
				writeus(user,"~CW-~FT Line ~CR%d~FT has been deleted.\n",line+1);
				redraw_edit(user,edit);
			  }
			  return;
			case '+':
			  user->tvar2 = 0;
			  if (inpstr[2]) break;
			  line = 0;  line = edit->current_line+1;
			  if (line>user->editor.maxlines)
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  if (edit->line[line][0])
			  {
				user->editor.edit_op = 0;
				text[0] = '\0';
				/* write the actual string.. unprocessed */
				sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s",line+1,edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='n') strcat(text,"\n");
				write_window(user,text,strlen(text),WriteDefault);
				edit->current_line = line;
				edit->current_char = 0;
				redraw_edit(user,edit);
				return;
			  }
			  if (line<user->editor.maxlines)
			  {
				user->editor.edit_op = 0;
				strcpy(edit->line[edit->current_line]," \n");
				edit->current_line = line;
				edit->current_char = 0;
				redraw_edit(user,edit);
				return;
			  }
			  else		/* More of a safety check */
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  return;
			case '-':
			  user->tvar2 = 0;
			  if (inpstr[2]) break;
			  line = 0;
			  line = edit->current_line-1;
			  if (line == -1 || line>user->editor.maxlines)
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  if (edit->line[line][0])
			  {
				user->editor.edit_op = 0;
				text[0] = '\0';
				/* write the actual string.. unprocessed */
				sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s",line+1,edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='n') strcat(text,"\n");
				write_window(user,text,strlen(text),WriteDefault);
				edit->current_line = line;
				edit->current_char = 0;
				redraw_edit(user,edit);
				return;
			  }
			  if (line<user->editor.maxlines)
			  {
				user->editor.edit_op = 0;
				strcpy(edit->line[edit->current_line]," \n");
				edit->current_line = line;
				edit->current_char = 0;
				redraw_edit(user,edit);
				return;
			  }
			  else			/* More of a safety check */
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  return;
			default : /* a '.' was hit on a line by itself or .# */
			  if (!inpstr[1])
			  {
				user->editor.edit_op = 1;
				redraw_edit(user,edit);
				write_user(user,"~CW- ~FTEditor command~CW '~CR.h~CW'~FT for help~CB:~RS ");
				return;
			  }
			  if (!isdigit(inpstr[1])) break;
			  inpstr++;
			  line = 0;
			  line = atoi(inpstr);
			  while (inpstr[0])
			  {
				if (!isdigit(inpstr[0]))
				{
					write_user(user,"~CW- ~FTLine must be a number only.\n");
					writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
					return;
				}
				inpstr++;
			  }
			  line = line-1;
			  if (line == -1 || line>user->editor.maxlines)
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  if (edit->line[line][0])
			  {
				user->editor.edit_op = 0;
				text[0] = '\0';
				/* write the actual string.. unprocessed */
				sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s",line+1,edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') strcat(text,"\n");
				write_window(user,text,strlen(text),WriteDefault);
				edit->current_line = line;
				edit->current_char = 0;
				redraw_edit(user,edit);
				return;
			  }
			  if (line<user->editor.maxlines)
			  {
				for (i = 0 ; i<line ; ++i)
				{
					if (edit->line[i][0] != '\0') continue;
					else strcpy(edit->line[i]," \n");
				}
				user->editor.edit_op = 0;
				edit->current_line = line;
				edit->current_char = 0;
				redraw_edit(user,edit);
				return;
			  }
			  else			/* More of a safety check */
			  {
				write_user(user,"~CW-~FT That line isn't in the editor.\n");
				writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
				return;
			  }
			  return;
		}
		if (user->editor.edit_op)
		{
			user->editor.edit_op = 1;
			redraw_edit(user,edit);
			write_user(user,"\n~CW- ~FTEditor command~CW '~CR.h~CW'~FT for help~CB:~RS ");
			return;
		}
	  }
	  line = edit->current_line;
	  cnt = edit->charcnt;
	  /* Loop through the input and store in editor structure */
	  while (*inpstr)
	  {
		edit->edited++;
		if (++cnt >= 79)
		{
			if (edit->current_line+1 >= user->editor.maxlines)
			  goto END;
			if (edit->current_line == edit->last_line)
			  edit->last_line++;
			edit->line[edit->current_line][edit->current_char] = '\n';
			edit->line[edit->current_line][edit->current_char+1] = '\0';
			edit->current_line++;
			cnt = 0;
			edit->current_char = 0;
			writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
			continue;
		}
		edit->line[edit->current_line][edit->current_char++] = *inpstr++;
	  }
	  if (line != edit->current_line)
	  {
		edit->line[edit->current_line][edit->current_char] = '\0';
		text[0] = '\0';
		if (user->colour) sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s",edit->current_line+1,edit->line[edit->current_line]);
		else sprintf(text,"%2d]%s",edit->current_line+1,edit->line[edit->current_line]);
		write_window(user,text,strlen(text),WriteDefault);
		edit->charcnt = cnt;
		return;
	  }
	  else
	  {
		edit->line[edit->current_line][edit->current_char] = '\n';
		edit->line[edit->current_line][edit->current_char+1] = '\0';
		edit->current_char = 0;
		edit->charcnt = 0;
	  }
	  if (edit->current_line<=user->editor.maxlines)
	  {
		edit->current_line++;
		if (edit->current_line>=user->editor.maxlines) goto END;
		redraw_edit(user,edit);
		return;
	  }
	  else goto END;
	  return;
	END:
	  user->editor.edit_op = 1;
	  edit->line[edit->current_line][edit->current_char] = '\0';
	  redraw_edit(user,edit);
	  write_user(user,"~CW- ~FTEditor command~CW '~CR.h~CW'~FT for help~CB:~RS ");
}

/* Re-draws that top screen.. */
void redraw_edit(User user,Editor edit)
{
	int i = 0,x = 1,buffpos = 0,chars = 0;
	char *tmp,buff[MaxLines*79];

	if (user->editor.visual == 0 && user->tvar3 == 0)
	{
		if (user->editor.editor==1) writeus(user,"~FG%2d~CB]~RS",user->editor.edit_line);
		else writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
		return;
	}
	cls(user,NULL,user->socket);
	write_user(user,"Hmm?\n");
	/* Bring the cursor to the top of the window. */
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (user->editor.editor == 1)
	{
		if (user->tvar3 == 1)
		{
			write_user(user,user->editor.malloc_start);
			if (user->ansi_on) write_user(user,ansi_line);
			else write_user(user,ascii_line);
			return;
		}
		if ((tmp = (char *)malloc(user->editor.maxlines*79)) == NULL)
		{
			write_user(user,"~FG Sorry, an error occurred.. failed to allocate buffer memory.\n");
			write_log(0,LOG1,"[ERROR] - Failed to allocate memory in redraw_edit()\n");
			return;
		}
		tmp = user->editor.malloc_start;
		buffpos = chars = 0;
		while (tmp != user->editor.malloc_end)
		{
			chars++;
			if (*tmp == '\n')
			{
				chars = 0;
				buff[buffpos] = '\0';
				if (user->colour)
				{
					if (x != user->editor.edit_line) sprintf(text,"\033[0;35m%2d\033[1;34m]\033[0m%s\n",x,buff);
					else sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s",x,buff);
				}
				else
				{
					if (x != user->editor.edit_line) sprintf(text,"%2d]%s\n",x,buff);
					else sprintf(text,"%2d]%s",x,buff);
				}
				if (user->car_return) strcat(text,"\r");
				write_window(user,text,strlen(text),WriteDefault);
				x++;
				buffpos = 0;
				++tmp;
				if (i>user->editor.maxlines)
				{
					*tmp = '\0';
					break;
				}
				continue;
			}
			*(buff+buffpos) = *tmp;
			buffpos++;
			++tmp;
		}
	}
	else
	{
		for (i = 0 ; i<user->editor.maxlines ; ++i)
		{
			if (edit->line[i][0] != '\0' && (user->editor.visual == 1 && user->tvar3 != 1))
			{
				text[0] = '\0';
				if (user->colour) sprintf(text,"\033[0;32m%2d\033[1;34m]\033[0m%s\r",i+1,edit->line[i]);
				else sprintf(text,"%2d]%s\r",i+1,edit->line[i]);
				if (edit->line[i][strlen(edit->line[i])-1] != '\n') strcat(text,"\n");
				write_window(user,text,strlen(text),WriteDefault);
			}
			if (edit->line[i][0] != '\0' && user->tvar3 == 1)
			{
				text[0] = '\0';
				sprintf(text,"~FG%2d~CB]~RS%s",i+1,edit->line[i]);
				write_user(user,text);
			}
		}
	}
	if (user->ansi_on) write_user(user,ansi_line);
	else write_user(user,ascii_line);
	if (user->editor.edit_op == 0)
	{
		sprintf(text,"~FT-~CT=~CB> ~FTTotal of ~CR%d ~FTlines allowed. ~CB<~CT=~FT-\n",user->editor.maxlines);
		write_user(user,center(text,80));
		write_user(user,"   ~CB[~FM-=-=-=-=-~FT1~FM-=-=-=-=-~FT2~FM-=-=-=-=-~FT3~FM-=-=-=-=-~FT4~FM-=-=-=-=-~FT5~FM-=-=-=-=-~FT6~FM-=-=-=-=-~FT7~FM-=-~CB]\n");
		if (user->editor.editor==1) writeus(user,"~FG%2d~CB]~RS",user->editor.edit_line);
		else writeus(user,"~FG%2d~CB]~RS",edit->current_line+1);
	}
	else write_user(user,"\n");
	if (user->splits) show_prompt(user);
}

/* Reset some values at the end of editing. */
void editor_done(User user)
{

	if (user->editor.editor == 1)
	{
		free(user->editor.malloc_start);
		user->editor.malloc_start = NULL;
		user->editor.malloc_end	  = NULL;
	}
	else
	{
		destruct_editor(get_editor(user));
	}
	user->editor.editing	= 0;
	user->misc_op		= 0;
	user->editor.edit_line	= 0;
	user->editor.edit_op	= 0;
	user->editor.maxlines	= -1;
	prompt(user);
	if (user->mail_op)
	{
		write_user(user,continue1);
		user->mail_op = 7;
		return;
	}
	if (user->tvar1 == 1)
	{
		write_user(user,"~CW-~FG Hit enter to continue.\n");
		user->misc_op	= 49;
		user->tvar1	= 0;
		return;
	}
	rev_away(user);
	if (user->chkrev)
	{
		writeus(user,new_tells1,user->recap);
		write_user(user,new_tells2);
		user->chkrev = 0;
	}
}

/* Get an editor from a user object */
Editor get_editor(User user)
{
	Editor edit;

	for_edit(edit)
	{
		if (edit->user == user) return(edit);
	}
	return(NULL);
}

/* Create an edit object */
Editor create_editor(void)
{
	Editor edit;
	int i;

	if ((edit = (Editor)malloc(sizeof(struct editor_struct))) == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - malloc FAILED in create_editor()\n");
		return(NULL);
	}
	/* Append into linked list */
	if (edit_first == NULL)
	{
		edit_first = edit;
		edit->prev = NULL;
	}
	else
	{
		edit_last->next = edit;
		edit->prev = edit_last;
	}
	edit->next		= NULL;	edit_last	= edit;
	edit->current_line	= 0;	edit->max_lines	= 0;
	edit->last_line		= 0;	edit->charcnt	= 0;
	edit->current_char	= 0;	edit->edited	= 0;
	edit->user		= NULL;
	for (i = 0 ; i<MaxLines ; ++i) edit->line[i][0] = '\0';
	return(edit);
}

/* Now we need to destruct the editor. */
void destruct_editor(Editor edit)
{

	if (edit == NULL) return;
	/* Remove from linked list. */
	if (edit == edit_first)
	{
		edit_first = edit->next;
		if (edit == edit_last) edit_last = NULL;
		else edit_first->prev = NULL;
	}
	else
	{
		edit->prev->next = edit->next;
		if (edit == edit_last)
		{
			edit_last = edit->prev;
			edit_last->next = NULL;
		}
		else edit->next->prev = edit->prev;
	}
	free(edit);
	edit = NULL;
}

/* END OF EDITOR.C */
