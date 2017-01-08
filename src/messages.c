/*
  messages.c
    The source within this file contains the functions that call upon the
    editor, such as .smail, .write, etc.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, and Neil Robertson

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
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#define _MESSAGES_C
  #include "include/needed.h"
#undef _MESSAGES_C

/* Displays the way to use the deleting commands ie .wipe, .dmail */
void print_usage(User user,int type)
{
	char name[20];

	switch(type)
	{
		case 1 :
		  strcpy(name,"wipe");
		  break;
		case 2 :
		  strcpy(name,"dmail");
		  break;
		case 3 :
		  strcpy(name,"dsent");
		  break;
		case 4 :
		  strcpy(name,"addnews -d");
		  break;
		case 5 :
		  strcpy(name,"sugg -d");
		  break;
		case 6 :
		  strcpy(name,"bug -d");
		  break;
		case 7 :
		  strcpy(name,"wiznote -d");
		  break;
		default:
		  strcpy(name,"unknown");
		  break;
	}
	writeus(user,"~CW - ~FGCommand usage~CB: ~CR.%s ~CB[~CR-all~CW|~CR-top ~CW(~CR# of mess.~CW)|~CR-bottom~CW (~CR# of mess.~CW)~CB]\n",name);
	writeus(user,"~FT         Example~CB:~CR .%s -all\n",name);
	writeus(user,"~FT         Example~CB:~CR .%s -top 6 ~CW<~CRdeletes the top 6 mess.~CW>\n",name);
	writeus(user,"~FT         Example~CB:~CR .%s -bottom 6 ~CW<~CRdeletes the bottom 6 mess.~CW>\n",name);
	writeus(user,"~CW - ~FGCommand usage~CB: ~CR.%s ~CB[~CRNumber of mess. to delete if its 1 msg.~CB]\n",name);
	writeus(user,"~FT         Example~CB:~CR .%s 3 ~CB[~CRThis will delete the third mess.~CB]\n",name);
	writeus(user,"~CW - ~FGCommand usage~CB: ~CR.%s ~CB[~CRstart~CB] ~CR- ~CB[~CRend~CB]\n",name);
	writeus(user,"~FT         Example~CB:~CR .%s 5 - 8 ~CB[~CRDeletes the mess. between the 2 numbers~CB]\n",name);
	writeus(user,"~CW - ~FGCommand usage~CB: ~CR.%s ~CB[~CRNumber of mess. to delete ~CW(~CRup to 5 #'s~CW)~CB]\n",name);
	writeus(user,"~FT         Example~CB:~CR .%s 1 3 4 7 9 or .%s 2 6 9\n",name,name);
	return;
}

/* Start off with simply writing a message on the board */
void write_board(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *infp,*outfp;
	int cnt,inp,line;
	char d,*ptr,*name,filename[FSL],*c;

	if (user == NULL) return;  edit = get_editor(user);
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
	if (done_editing == 0)
	{
		if (word_count < 2)
		{
			WriteRoomExcept(user->room,user,"~FG %s ~FMhas started to carve some etchings to go on the wall.\n",name);
			write_user(user,center("~FT-~CT=~CB> ~FGCarving some etchings onto the wall.~CB <~CT=~FT-\n",80));
			user->editor.editing = 1;
			user->misc_op = 3;
			if (user->editor.editor == 1) nuts_editor(user,NULL);
			else ramtits_editor(user,NULL);
			return;
		}
		ptr = inpstr;
		inp = 1;
	}
	else
	{
		inp = 0;
		ptr = NULL;
	}

	if (user->status & Silenced)
	{
		write_user(user,center("~FT-~CT=~CB> ~FGYou carve some etchings onto the wall.~CB <~CT=~FT-\n",80));
		return;
	}
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~FG Error in carving in the wall.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile to write in write_board()\n");
		return;
	}
	/* First put the new message in */
	if (user->invis == Invis) name = invismesg; else name = user->recap;
	fprintf(outfp,"PT: %d\r",(int)(time(0)));
	fprintf(outfp," ~FTCarver~CB: ~RS%s\n ~FTCarved On~CB: ~FR%s\n",name,long_date(0));
	fputs(ascii_line,outfp);
	cnt = 0;
	if (inp == 1)
	{
		if (ptr == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		while(*ptr != '\0')
		{
			putc(*ptr,outfp);
			++ptr;
		}
		fputs("\n",outfp);
		fputs(ascii_line,outfp);
		fputs("\n",outfp);
	}
	else
	{
		if (user->editor.editor == 1)
		{
			c = user->editor.malloc_start;
			while(c!=user->editor.malloc_end) putc(*c++,outfp);
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
				if (edit->line[line][0])
				{
					fprintf(outfp,"%s",edit->line[line]);
					if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",outfp);
				}
				line++;
			}
		}
		fputs(ascii_line,outfp);
		fputs("\n",outfp);
	}
	/* Now put in the rest of the board if it exists */
	sprintf(filename,"%s/%s/%s.B",RoomDir,MesgBoard,user->room->name);
	if ((infp = fopen(filename,"r")))
	{
		d = getc(infp);
		while(!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	rename("tempfile",filename);
	write_user(user,center("~FT-~CT=~CB>~FG You carve some etchings onto the wall.~CB <~CT=~FT-\n",80));
	WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FMcarves some etchings onto the wall.\n",name);
	user->room->mesg_cnt++;
}

/* Lets a user read the message board */
void read_board(User user,char *inpstr,int rt)
{
	Room rm = NULL;
	char filename[FSL],*name,*rmname;

	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count < 2) rm = user->room;
	else
	{
		if (word_count >= 3)
		{
			if ((rmname = get_room_full(user,word[1])) == NULL)
			{
				write_user(user,center(nosuchroom,80));
				return;
			}
			read_specific(user,rmname,atoi(word[2]),1);
			return;
		}
		if (word_count == 2)
		{
			if (atoi(word[1]))
			{
				read_specific(user,user->room->name,atoi(word[1]),1);
				return;
			}
			else
			{
				if ((rm = get_room(word[1])) == NULL)
				{
					write_user(user,center(nosuchroom,80));
					return;
				}
			}
		}
	}
	sprintf(filename,"%s/%s/%s.B",RoomDir,MesgBoard,rm->name);
	if (!file_exists(filename))
	{
		write_user(user,"~CW- ~FTIt appears as tho noone has carved on this wall.\n");
		return;
	}
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		sprintf(text,"~FBÄÄ~CB¯~FT> ~FYThe ~FR%s ~FYwall...~FT <~CB®~FBÄÄ\n",rm->recap);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		sprintf(text,"~FT-~CT=~CB> ~FYThe ~FR%s~FY wall...~CB <~CT=~FT-\n",rm->recap);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	check_messages(user,NULL,1);
	switch(more(user,user->socket,filename,0))
	{
		case 0:
		  write_user(user,"~CW- ~FTIt appears as tho noone has carved on this wall.\n");
		  break;
		case 1:
		  user->misc_op=2;
		}
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (rm == user->room)
	{
		WriteRoomExcept(user->room,user,"~CW-~CY %s ~FGdecides to view the etchings on the wall.\n",name);
	}
}

/* Lets a user enter their profile */
void enter_profile(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *fp;
	char *c,filename[FSL],*name;
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
	if (done_editing == 0)
	{
		if (!strncasecmp(word[1],"-delete",2))
		{
			sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,user->name);
			unlink(filename);
			write_user(user,"~FG Your profile has been deleted.\n");
			return;
		}
		if (!strncasecmp(word[1],"-read",2))
		{
			sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,user->name);
			write_user(user,center("~FT-~CT=~CB> ~FGYour current profile..~CB <~CT=~FT-\n",80));
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,"~CW- ~FGYou don't have a profile to view.\n");
				  break;
				case 1:
				  user->misc_op=2;
			}
			return;
		}
		WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FMhas decided to enter some stuff about %sself.\n",name,gender2[user->gender]);
		write_user(user,center("~FT-~CT=~CB> ~FGEntering a profile about yourself.~CB <~CT=~FT-\n",80));
		user->editor.editing=1;
		user->misc_op=5;
		if (user->editor.editor==1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	sprintf(filename,"%s/%s/%s.P",UserDir,UserMesg,user->name);
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW- ~FGThere was an error trying to save your profile.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open '%s.P' to write in enter_profile()\n",user->name);
		return;
	}
	if (user->editor.editor == 1)
	{
		c=user->editor.malloc_start;
		while(c != user->editor.malloc_end) putc(*c++,fp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line < MaxLines)
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
	if (webpage_on) do_user_webpage(user);
	write_user(user,center("~FT-~CT=~CB> ~FGYour profile has been stored. ~CB<~CT=~FT-\n",80));
	WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FMhas finished writing about %sself.\n",name,gender2[user->gender]);
}

/*
  Add some news, so this way u don't gotta do it through the shell all the
  the time! This command should only be used if the user is entering VALID
  news about the talker and what not!
*/
void add_news(User user,char *inpstr,int done_editing)
{
	Editor edit = get_editor(user);
	FILE *infp,*outfp;
	char d,filename[FSL],*name,*c;
	int line;

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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (done_editing == 0)
	{
		if (!strncasecmp(word[1],"-from",2))
		{
			view_from(user,NULL,4);
			return;
		}
		if (!strncasecmp(word[1],"-delete",2))
		{
			if (user->level < GOD)
			{
				write_user(user,"~CW -~FG Command usage~CB: ~CR.addnews\n");
				return;
			}
			delete_messages(user,NULL,4);
			return;
		}
		WriteRoomExcept(user->room,user,"~CW-~FR %s ~FMdecides they have some news worth sharing...\n",name);
		write_user(user,center("~FT-~CT=~CB> ~FGAdding some new news...~CB <~CT=~FT-\n",80));
		user->editor.editing = 1;
		user->misc_op = 7;
		if (user->editor.editor == 1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	if (user->status & Silenced)
	{
		sprintf(text,"~CB-=[ ~FG Thank you ~FT%s~FG for adding some news!~CW :-)~CB ]=-\n",user->recap);
		write_user(user,center(text,80));
		return;
	}
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW-~FG There was an error in writing the news.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in add_news(). [%s]\n",strerror(errno));
		return;
	}
	/* put the new message in first. */
	sprintf(text," ~FREditor~CB: ~FY%s\n",user->recap);
	fputs(text,outfp);
	sprintf(text," ~FRWritten on~CB: ~FG%s\n",long_date(0));
	fputs(text,outfp);
	fputs(ascii_line,outfp);
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while(c != user->editor.malloc_end) putc(*c++,outfp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line < MaxLines)
		{
			if (edit->line[line][0] != '\0')
			{
				fprintf(outfp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",outfp);
			}
			line++;
		}
	}
	fputs(ascii_line,outfp);
	fprintf(outfp,"\n");
	/* Now the old file. */
	sprintf(filename,"%s/%s",DataDir,NewsFile);
	if ((infp = fopen(filename,"r")))
	{
		d = getc(infp);
		while(!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	rename("tempfile",filename);
	sprintf(text,"~FT-~CT=~CB> ~FGThank you ~FM%s~FG for adding some news.~CW :-)~CB <~CT=~FT-\n",user->recap);
	write_user(user,center(text,80));
	WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FMhas added some new news.\n",name);
}

/* Add a room desc */
void room_desc(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *fp;
	int line;
	char *c,filename[FSL],*name,*rmname;

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
	if (done_editing == 0)
	{
		if (user->level < WIZ && strcmp(user->room->owner,user->name))
		{
			write_user(user,"~CW- ~FGThis is not your room.. you can't edit the room description for it.\n");
			return;
		}
		if (word_count < 2)
		{
			rmname = user->room->name;
			strcpy(user->check,rmname);
		}
		else
		{
			if ((rmname = get_room_full(user,word[1])) == NULL)
			{
				write_user(user,center(nosuchroom,80));
				return;
			}
			strcpy(user->check,rmname);
		}
		if (!strcmp(rmname,user->room->name))
		{
			WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FGdecides to write a new description for this room.\n",name);
		}
		write_user(user,center("~FT-~CT=~CB> ~FGEntering a new room description.~CB <~CT=~FT-\n",80));
		user->editor.editing = 1;
		user->misc_op = 8;
		if (user->editor.editor == 1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	if ((rmname = get_room_full(user,user->check)) == NULL)
	{
		write_user(user,"~CW- ~FGFailed to enter room description.\n");
		return;
	}
	sprintf(filename,"%s/%s/%s.R",RoomDir,Descript,rmname);
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW-~FG An error occurred while entering the new room description.\n");
		write_log(0,LOG1,"[ERROR] - Unable to write new room description in room_desc()\n");
		return;
	}
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while(c != user->editor.malloc_end) putc(*c++,fp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line < MaxLines)
		{
			if (edit->line[line][0])
			{
				fprintf(fp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",fp);
			}
			line++;
		}
	}
	FCLOSE(fp);
	writeus(user,"~CW - ~FGYou have finished the room desc for the~CB: ~FT%s~FG room.\n",rmname);
	user->check[0] = '\0';
	if (!strcmp(rmname,user->room->name))
	{
		look(user,NULL,0);
		WriteRoomExcept(user->room,user,"~CW - ~FR%s ~FMfinishes up the new description for this room.\n",name);
	}
}

/* Write suggestions to the board so users can suggest whatever they want */
void suggestions(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *infp,*outfp;
	char *c,d,filename[FSL],*name;
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
	if (done_editing == 0)
	{
		if (!strncasecmp(word[1],"-from",2))
		{
			view_from(user,NULL,5);
			return;
		}
		if (!strncasecmp(word[1],"-delete",2))
		{
			if (user->level < GOD)
			{
				writeus(user,usage,command[com_num].name,"~CB[<~CR-read~CW|~CR-from~CB>]");
				return;
			}
			delete_messages(user,NULL,5);
			return;
		}
		if (!strncasecmp(word[1],"-read",2))
		{
			count_messages(user,5);
			if (word_count == 3)
			{
				read_specific(user,NULL,atoi(word[2]),3);
				return;
			}
			sprintf(filename,"%s/%s",DataDir,SugFile);
			if (user->ansi_on)
			{
				write_user(user,ansi_tline);
				write_user(user,center("~FBÄÄ~CB¯~FT> ~FGThe suggestion board...~FT <~CB®~FBÄÄ\n",80));
				write_user(user,ansi_bline);
			}
			else
			{
				write_user(user,ascii_tline);
				write_user(user,center("~FT-~CT=~CB> ~FGThe suggestion board...~CB <~CT=~FT-\n",80));
				write_user(user,ascii_bline);
			}
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,center("~FR No suggestions were found! ~CW:~(\n",80));
				  return;
				case 1:
				  user->misc_op = 2;
			}
			return;
		}
		WriteRoomExcept(user->room,user,"~FR %s~FM thinks they have some cool suggestions for the talker!\n",name);
		write_user(user,center("~FT-~CT=~CB> ~FGContributing your ideas.~CB <~CT=~FT-\n",80));
		user->editor.editing = 1;
		user->misc_op = 9;
		if (user->editor.editor==1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	if (user->status & Silenced)
	{
		sprintf(text,"~FT-~CT=~CB> ~FGThank you ~FT%s~FG for adding your suggestions.~CW :-)~CB <~CT=~FT-\n",user->recap);
		write_user(user,center(text,80));
		return;
	}
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW- ~FG There was an error writing your suggestion.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in suggestions().\n");
		return;
	}
	/* Put the new message in first. */
	sprintf(text," ~FRIdealist~CB: ~FY%s\n",user->recap);
	fputs(text,outfp);
	sprintf(text," ~FRWritten on~CB: ~FG%s\n",long_date(0));
	fputs(text,outfp);
	fputs(ascii_line,outfp);
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while(c != user->editor.malloc_end) putc(*c++,outfp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while (line < MaxLines)
		{
			if (edit->line[line][0])
			{
				fprintf(outfp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1] != '\n') fputs("\n",outfp);
			}
			line++;
		}
	}
	fputs(ascii_line,outfp);
	fprintf(outfp,"\n");
	/* Now the rest of the file. */
	sprintf(filename,"%s/%s",DataDir,SugFile);
	if ((infp = fopen(filename,"r")))
	{
		d = getc(infp);
		while(!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	rename("tempfile",filename);
	sprintf(text,"~FT-~CT=~CB> ~FGThank you ~FT%s~FG for adding your suggestions.~CW :-)~CB <~CT=~FT-\n",user->recap);
	write_user(user,center(text,80));
	sprintf(text,"~CW- ~FR%s ~FMadds a new suggestion.\n",name);
	write_room_except(user->room,user,center(text,80));
	suggestion_num++;
}

/* Lets a user write a letter to all their friends! */
void friend_smail(User user,int done_editing)
{
	User u;
	char name[UserNameLen+2], filename[FSL], *tname;
	int on = 0, fsize = 0, err = 0, i = 0;
	struct stat fb;

	if (user == NULL) return;
	if (user->invis == Invis) tname = invisname;  else tname = user->recap;
	if (done_editing == 1)
	{
		for (i = 0; i < 50; ++i)
		{
			if (user->friends[i][0] == '\0') continue;
			user->chksmail = 1;
			if ((u = get_user(user->friends[i])) == NULL)
			{
				if ((u = create_user()) == NULL)
				{
					write_log(0,LOG1,"[ERROR] - Unable to create temp user object in friend_smail().\n");
					return;
				}
				err = LoadUser(u,user->friends[i]);
				if (err == 0)
				{
					writeus(user,"~CW-~FT Friend ~CW'~FY%s~CW'~FT doesn't seem to have an account on this talker anymore.\n",name);
					destruct_user(u);
					destructed = 0;
					continue;
				}
				else if (err == -1)
				{
					write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",name);
					destruct_user(u);
					destructed = 0;
					continue;
				}
				on = 0;
			}
			else on = 1;
			sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,u->name);
			if (stat(filename,&fb) == -1) fsize = 0;
			else fsize = fb.st_size;
			sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,u->name);
			if (stat(filename,&fb) == -1) fsize = fsize;
			else fsize += fb.st_size;
			if (fsize >= MaxMbytes)
			{
				writeus(user,"~FG Unable to send the friend s-mail to ~CW'~FG%s~CW'\n",u->recap);
				write_user(user,"~FG Their s-mail box is currently full.\n");
				if (on == 0)
				{
					destruct_user(u);
					destructed = 0;
				}
				continue;
			}
			send_mail(user,u->name);
			if (on == 0)
			{
				destruct_user(u);
				destructed = 0;
			}

		}
		write_user(user,center("~FT-~CT=~CB> ~FGYour friend s-mail has been successfully sent. ~CB<~CT=~FT-\n",80));
		user->chksmail = 0;
		return;
	}
	write_user(user,center("~FT-~CT=~CB> ~FGWriting a s-mail message to your friends.~CB <~CT=~FT-\n",80));
	user->editor.editing = 1;
	user->misc_op = 10;
	if (user->editor.editor == 1) nuts_editor(user,NULL);
	else ramtits_editor(user,NULL);
	return;
}

/* Lets a user write a letter to the staff members */
void staff_smail(User user,int done_editing)
{
	FILE *fp;
	char name[UserNameLen+2],filename[FSL];

	if (user == NULL) return;
	if (done_editing == 1)
	{
		sprintf(filename,"%s/%s",DataDir,StaffFile);
		if ((fp = fopen(filename,"r")) == NULL) return;
		user->chksmail = 2;
		fscanf(fp,"%s",name);
		while(!feof(fp))
		{
			if (strcmp(name,user->name)) send_mail(user,name);
			fscanf(fp,"%s",name);
		}
		FCLOSE(fp);
		write_user(user,center("~FT-~CT=~CB> ~FGThe staff s-mail has been successfully sent. ~CB<~CT=~FT-\n",80));
		user->chksmail = 0;
		return;
	}
	sprintf(filename,"%s/%s",DataDir,StaffFile);
	if (!file_exists(filename))
	{
		write_user(user,"~CW- ~FGIt appears there isn't any staff to s-mail.\n");
		return;
	}
	write_user(user,center("~FT-~CT=~CB> ~FGWriting a s-mail message to all the staff.~CB <~CT=~FT-\n",80));
	user->editor.editing = 1;
	user->misc_op = 11;
	if (user->editor.editor == 1) nuts_editor(user,NULL);
	else ramtits_editor(user,NULL);
}

/* Lets a user write a letter to all the other users */
void users_smail(User user,int done_editing)
{
	Ulobj ul;

	if (user == NULL) return;
	if (done_editing)
	{
		user->chksmail = 3;
		for_list(ul)
		{
			if (strcmp(ul->name,user->name)) send_mail(user,ul->name);
		}
		write_user(user,center("~FT-~CT=~CB> ~FGMail message to all users has been successfully sent. ~CB<~CT=~FT-\n",80));
		user->chksmail = 0;
		return;
	}
	write_user(user,center("~FT-~CT=~CB> ~FGWriting a s-mail message to all the users.~FT ]~CB=~FT-\n",80));
	user->editor.editing = 1;
	user->misc_op = 21;
	if (user->editor.editor == 1) nuts_editor(user,NULL);
	else ramtits_editor(user,NULL);
}

/* Lets a user jot down some bugs that they might notice */
void bugs(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *infp,*outfp;
	char *c,d,filename[FSL],*name;
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
	if (!done_editing)
	{
		if (!strncasecmp(word[1],"-from",2))
		{
			view_from(user,NULL,6);
			return;
		}
		if (!strncasecmp(word[1],"-delete",2))
		{
			if (user->level < GOD)
			{
				writeus(user,usage,command[com_num].name,"~CB[<~CR-read~CB>]");
				return;
			}
			delete_messages(user,NULL,6);
			return;
		}
		if (!strncasecmp(word[1],"-read",2))
		{
			count_messages(user,6);
			if (user->level < POWER)
			{
				write_user(user,"~CW- ~FRYou cannot read the bugs.\n");
				return;
			}
			if (word_count == 3)
			{
				read_specific(user,NULL,atoi(word[2]),4);
				return;
			}
			sprintf(filename,"%s/%s",DataDir,BugFile);
			if (user->ansi_on)
			{
				write_user(user,ansi_tline);
				write_user(user,center("~FBÄÄ~CB¯~FT> ~FTBugs that people have noticed.~FT <~CB®~FBÄÄ\n",80));
				write_user(user,ansi_bline);
			}
			else
			{
				write_user(user,ascii_tline);
				write_user(user,center("~FT-~CB=~FT[ ~FTBugs that people have noticed.~CB <~CT=~FT-\n",80));
				write_user(user,ascii_bline);
			}
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,"~CW- ~FTKick ass.. there are no bugs..\n");
				  return;
				case 1:
				  user->misc_op = 2;
			}
			return;
		}
		WriteRoomExcept(user->room,user,"~CW- ~FR%s~FM thinks they found a bug with the talker.\n",name);
		write_user(user,center("~FT-~CT=~CB> ~FGEntering a bug you noticed.~CB <~CT=~FT-\n",80));
		user->editor.editing = 1;
		user->misc_op = 12;
		if (user->editor.editor == 1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW- ~FGThere was an error in writing the bug.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in bugs().\n");
		return;
	}
	/* put the new message in first. */
	sprintf(text," ~FRBugNoter~CB: ~FY%s\n",user->recap);
	fputs(text,outfp);
	sprintf(text," ~FRWritten on~CB: ~FG%s\n",long_date(0));
	fputs(text,outfp);
	fputs(ascii_line,outfp);
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while (c != user->editor.malloc_end) putc(*c++,outfp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line < MaxLines)
		{
			if (edit->line[line][0])
			{
				fprintf(outfp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",outfp);
			}
			line++;
		}
	}
	fputs(ascii_line,outfp);
	fprintf(outfp,"\n");
	/* Now the old file. */
	sprintf(filename,"%s/%s",DataDir,BugFile);
	if ((infp = fopen(filename,"r")))
	{
		d = getc(infp);
		while(!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	rename("tempfile",filename);
	sprintf(text,"~FT-~CT=~CB>~FG Thank you ~FM%s~FG for pointing that bug out.~CW :-)~CB <~CT=~FT-\n",user->recap);
	write_user(user,center(text,80));
	bug_num++;
}

/* Sends a mail message */
void smail(User user,char *inpstr,int done_editing)
{
	User u;
	char *name,*uname;
	char filename[FSL];
	int on = 0,i,fsize = 0,err = 0,cnt = 0;
	struct stat fb;

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
	if (is_muzzled(user))
	{
		status_text(user,3,command[com_num].name);
		return;
	}
	if (done_editing == 1)
	{
		send_mail(user,user->mail_to);
		user->mail_to[0] = '\0';
		user->tvar1	 = 1;
		for (i = 0; i < 5; ++i) 
		{
			if (user->ccopy[i][0])
			{
				user->tvar2 = 1;
				send_mail(user,user->ccopy[i]);
			}
		}
		user->chksmail = 0;
		user->tvar1 = 0;
		user->tvar2 = 0;
		return;
	}
	if (word_count < 2)
	{
		if (user->level<WIZ)
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CW<~CRmessage~CW>~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CR-friends~CB]");
			return;
		}
		else
		{
			writeus(user,usage,command[com_num].name,"~CB[~CRuser~CB] [~CW<~CRmessage~CW>~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CR-friends~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CR-staff~CB]");
			writeus(user,usage,command[com_num].name,"~CB[~CR-all~CB]");
			return;
		}
		return;
	}
	if (!strncasecmp(word[1],"-friends",2))
	{
		for (i = 0; i < 50; ++i)
		{
			if (user->friends[i][0] == '\0') continue;
			++cnt;
		}
		if (cnt == 0)
		{
			write_user(user,"~CW-~FT You don't even have any friends to smail.\n");
			return;
		}
		friend_smail(user,0);
		return;
	}
	if (!strncasecmp(word[1],"-staff",2))
	{
		if (user->level<WIZ)
		{
			write_user(user,center(nosuchuser,80));
			return;
		}
		staff_smail(user,0);
		return;
	}
	if (!strncasecmp(word[1],"-all",2))
	{
		if (user->level < WIZ)
		{
			write_user(user,center(nosuchuser,80));
			return;
		}
		users_smail(user,0);
		return;
	}
	if ((uname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if ((u = get_user(uname)) == NULL)
	{
		if ((u = create_user()) == NULL)
		{
			write_user(user,"~CW- ~FGAn error occurred while trying to send mail..\n");
			write_log(0,LOG1,"[ERROR] - Unable to create temporary user object in smail()!\n");
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
			write_user(user,"~CW-~FT It appears as tho that users userfile is corrupted.\n");
			write_log(1,LOG1,"[UserFile - Error] - User: [%s] File corrupted.\n",uname);
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
	}
	else on = 1;
	inpstr = remove_first(inpstr);
	if (is_enemy(u,user) && user->level<OWNER)
	{
		write_user(user,"~CW- ~FGYou can't s-mail people who have made you an enemy..\n");
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	if (is_enemy(user,u) && user->level < OWNER)
	{
		write_user(user,"~CW- ~FGYou can't s-mail your enemies.\n");
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,u->name);
	if (stat(filename,&fb) == -1) fsize = 0;
	else fsize=fb.st_size;
	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,u->name);
	if (stat(filename,&fb) == -1) fsize = fsize;
	else fsize += fb.st_size;
	if (fsize >= MaxMbytes)
	{
		write_user(user,"~FG That users s-mail box is allocating the maximum amount of bytes.\n");
		write_user(user,"~FG So you cannot send them a s-mail message until they delete some.\n");
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		return;
	}
	if (strlen(inpstr))	/* one lined s-mail */
	{
		strcat(inpstr,"\n");
		send_oneline_mail(user,u->name,inpstr);
		user->tvar1 = 1;
		if (on == 0)
		{
			destruct_user(u);
			destructed = 0;
		}
		for (i=0;i<5;++i)
		{
			if (user->ccopy[i][0])
			{
				user->tvar2 = 1;
				send_oneline_mail(user,user->ccopy[i],inpstr);
			}
		}
		user->tvar1 = 0;
		user->tvar2 = 0;
		return;
	}
	WriteRoomExcept(user->room,user,"~CW- ~FR%s~FM starts writing a s-mail letter.\n",name);
	sprintf(text,"~FT-~CT=~CB> ~FGWriting a s-mail message to ~CB[~FR%s~CB]~CB <~CT=~FT-\n",u->recap);
	write_user(user,center(text,80));
	user->editor.editing = 1;
	user->misc_op = 4;
	strcpy(user->mail_to,u->name);
	if (on == 0)
	{
		destruct_user(u);
		destructed = 0;
	}
	if (user->editor.editor==1) nuts_editor(user,NULL);
	else ramtits_editor(user,NULL);
}

/* Enter a voting topic. */
void enter_vote(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *fp;
	char filename[FSL],*c,*name;
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
	if (done_editing == 0)
	{
		WriteRoomExcept(user->room,user,"~CW- ~FR%s~FM is adding a new vote topic.\n",name);
		write_user(user,center("~FT-~CT=~CB> ~FGEntering a new voting topic.~CB <~CT=~FT-\n",80));
		write_user(user,center("~FT-~CT=~CB> ~FGIf you haven't already, leave the editor and use ~CW'~CR.vote -clear~CW'~CB <~CT=~FT-\n",80));
		user->editor.editing = 1;
		user->misc_op = 22;
		if (user->editor.editor == 1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	sprintf(filename,"%s/votefile",MiscDir);
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW-~FG An error occurred trying to write your vote topic.\n");
		write_log(0,LOG1,"[ERROR] - Unable to open vote file in enter_vote()!\n");
		return;
	}
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while(c != user->editor.malloc_end) putc(*c++,fp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line < MaxLines)
		{
			if (edit->line[line][0])
			{
				fprintf(fp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",fp);
			}
			line++;
		}
	}
	fprintf(fp,"\n");
	FCLOSE(fp);
	sprintf(text,"~FT-~CT=~CB> ~FGThank you ~FM%s~FG, the vote topic is now added.~CW :-)~FT ]~CB=~FT-\n",user->recap);
	write_user(user,center(text,80));
	WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FMhas finished entering a vote topic.\n",name);
}

/* Enter a riddle. */
void enter_riddle(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *fp;
	char filename[FSL],*c,*name;
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
	if (done_editing == 0)
	{
		WriteRoomExcept(user->room,user,"~CW- ~FR%s~RS is adding a new riddle.\n",name);
		write_user(user,center("~FT-~CT=~CB> ~FGEntering a new riddle.~CB <~CT=~FT-\n",80));
		write_user(user,center("~FT-~CT=~CB> ~FGIf you haven't already, leave the editor and use ~CW'~CR.riddle -clear~CW'~CB <~CT=~FT-\n",80));
		user->editor.editing = 1;
		user->misc_op = 50;
		if (user->editor.editor == 1) nuts_editor(user,NULL);
		else ramtits_editor(user,NULL);
		return;
	}
	sprintf(filename,"%s/riddle",MiscDir);
	if ((fp = fopen(filename,"w")) == NULL)
	{
		write_user(user,"~CW-~FG An error occurred trying to write your riddle.\n");
		write_log(0,LOG1,"[ERROR] - Unable to open riddle file in enter_riddle().\n");
		return;
	}
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while(c != user->editor.malloc_end) putc(*c++,fp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while(line < MaxLines)
		{
			if (edit->line[line][0])
			{
				fprintf(fp,"%s",edit->line[line]);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",fp);
			}
			line++;
		}
	}
	fprintf(fp,"\n");
	FCLOSE(fp);
	sprintf(text,"~FT-~CT=~CB> ~FGThank you ~FM%s~FG, the new riddle is now added.~CW :-)~FT ]~CB=~FT-\n",user->recap);
	write_user(user,center(text,80));
	WriteRoomExcept(user->room,user,"~CW- ~FR%s ~FMhas finished entering a riddle.\n",name);
}

/* Lets a user read their s-mail */
void rmail(User user,char *inpstr,int isnew)
{
	FILE *infp,*outfp;
	char d,filename[FSL],filename2[FSL],tempf[FSL];
	int rows;

	if (user == NULL) return;
	if (is_clone(user))
	{
		status_text(user,2,command[com_num].name);
		return;
	}
	if (word_count > 2 && isnew == 0)
	{
		write_user(user,"~CW -~FG Command usage~CB: ~CR.rmail~CW [<~CRmessage number~CW|~CR-new~CW>]\n");
		return;
	}
	if (word_count == 2 || isnew)
	{
		if (strncasecmp(word[1],"-new",2) && isnew == 0)
		{
			read_specific(user,NULL,atoi(word[1]),1);
			return;
		}
		else
		{
			if (user->ansi_on)
			{
				write_user(user,"\n");
				write_user(user,ansi_tline);
				write_user(user,center("~FBÄÄ~CB¯~FT> ~FGYour NEW s-mail...~FT <~CB®~FBÄÄ\n",80));
				write_user(user,ansi_bline);
			}
			else
			{
				write_user(user,"\n");
				write_user(user,ascii_tline);
				write_user(user,center("~FT-~CT=~CB> ~FGYour NEW s-mail...~CB <~CT=~FT-\n",80));
				write_user(user,ascii_bline);
			}
			sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
			sprintf(filename2,"%s/%s/%s.M",UserDir,UserMesg,user->name);
			if (!file_exists(filename))
			{
				write_user(user,"~CW-~FG It appears as though you have no new s-mail to read.\n");
				return;
			}
			if ((outfp = fopen("tempfile","w")) == NULL)
			{
				write_user(user,"~CW- ~FGAn error occured... Unable to show you your new s-mail.\n");
				write_log(0,LOG1,"[ERROR] - Unable to open tempfile in rmail()!\n");
				return;
			}
			/* Copy the new mail file in. */
			if ((infp = fopen(filename,"r")))
			{
				d = getc(infp);
				while(!feof(infp))
				{
					putc(d,outfp);
					d = getc(infp);
				}
				FCLOSE(infp);
			}
			/* Copy the old mail file in. */
			if ((infp = fopen(filename2,"r")))
			{
				d = getc(infp);
				while(!feof(infp))
				{
					putc(d,outfp);
					d = getc(infp);
				}
				FCLOSE(infp);
			}
			FCLOSE(outfp);
			rename("tempfile",filename2);
			if (user->login)
			{
				rows = user->rows;
				user->rows = 0;
				more(user,user->socket,filename,1);
				unlink(filename);
				user->rows = rows;
				return;
			}
			switch(more(user,user->socket,filename,1))
			{
				case 0:
				  write_user(user,"~CW-~FG It appears as though you have no new s-mail to read.\n");
				  unlink(filename);
				  break;
				case 1:
				  user->misc_op = 2;
			}
		}
		return;
	}
	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
	sprintf(filename2,"%s/%s/%s.M",UserDir,UserMesg,user->name);
	if (!file_exists(filename)) goto READIT;
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW-~FG An error occured... Unable to show you your s-mail.\n");
		write_log(0,LOG1,"[ERROR] - Unable to open tempfile in rmail()\n");
		goto READIT;
	}
	/* Copy the new mail file in. */
	if ((infp = fopen(filename,"r")))
	{
		d = getc(infp);
		while(!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	unlink(filename);  /* it's been appended. */
	/* Copy the old mail file in. */
	if ((infp = fopen(filename2,"r")))
	{
		d = getc(infp);
		while(!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	rename("tempfile",filename2); /* Rename the tempfile. */
	filename[0] = filename2[0] = tempf[0] = '\0';

	READIT:
	  sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
	  if (!file_exists(filename))
	  {
		write_user(user,"~CW-~FG It appears as though you have no mail to read.\n");
		return;
	  }
	  count_messages(user,2);
	  if (user->ansi_on)
	  {
		write_user(user,ansi_tline);
		write_user(user,center("~FBÄÄ~CB¯~FT> ~FGYour s-mail box...~FT <~CB®~FBÄÄ\n",80));
		write_user(user,ansi_bline);
	  }
	  else
	  {
		write_user(user,ascii_line);
		write_user(user,center("~FT-~CT=~CB> ~FGYour s-mail box...~CB <~CT=~FT-\n",80));
		write_user(user,ascii_line);
	  }
	  switch(more(user,user->socket,filename,1))
	  {
		case 0:
		  write_user(user,"~CW-~FG It looks like your s-mail box is empty...\n");
		  break;
		case 1:
		  user->misc_op = 2;
	}
}

/*
   Re-wrote this function... so now we can do alot more with it.. instead
   of just forwarding your whole s-mail box to your e-mail addy.. you can
   now forward certain message numbers (ie .fsmail 1 which means the
   the first message in your s-mail box will get sent to your user->email
   address. There's more information on this in the .h fsmail
*/
void forward_smail(User user,char *inpstr,int rt)
{
	FILE *infp,*outfp;
	char *args="~CB[~CRall~CW|~CRmessage number~CW|~CR-autofwd on~CW|~CRoff~CW|~CRverify~CB]";
	char filename[FSL],outfile[FSL],line[ARR_SIZE+1],addy[EmailLen],check[25];
	int mnum=-1,amount,valid,cnt,vnum;

	if (user == NULL) return;
	amount = valid = cnt = vnum = 0;
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
		writeus(user,usage,command[com_num].name,args);
		return;
	}
	strncpy(addy,user->email,sizeof(addy)-1);
	if (!strcasecmp(word[1],"all") || isdigit(word[1][0]))
	{
		if (!valid_addy(user,addy,0))
		{
			write_user(user,"~CW- ~FTYou need to have a real e-mail address set (use .set)\n");
			return;
		}
		if (isdigit(word[1][0])) mnum = atoi(word[1]);
		/* First check for the .fsmail # */
		if (!(amount=count_mess(user,1)))
		{
			write_user(user,"~CW- ~FTYou have no s-mail to forward..\n");
			return;
		}
		if (mnum != -1 && amount < mnum)
		{
			writeus(user,"~CW- ~FTYou don't have a %d message to forward..\n",mnum);
			return;
		}
		/*
		   Now open up the file.. if 'all' was given.. write the mail
		   file line by line cause we gotta ColourStrip each line. Or
		   if a message number was specified, then we just write the
		   one message... (Beats doing everything more then once.)
		*/
		sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		if ((infp = fopen(filename,"r")) == NULL)
		{
			write_user(user,"~CW- ~FTYou have no s-mail to forward..\n");
			return;
		}
		sprintf(outfile,"%s/%s.FWD",TempFiles,user->name);
		if ((outfp = fopen(outfile,"w")) == NULL)
		{
			write_user(user,"~CW- ~FTAn error occurred in e-mailing.. sorry.\n");
			write_log(0,LOG1,"[ERROR] - Failed to open '%s.FWD' for writing. Reason: [%s]\n",user->name,strerror(errno));
			FCLOSE(infp);
			return;
		}
		fprintf(outfp,"From:    %s <%s>\n",TalkerName,TalkerEmail);
		fprintf(outfp,"To:      %s <%s>\n",user->name,addy);
		fprintf(outfp,"Subject: %s s-mail forwarding.\n",TalkerName);
		fprintf(outfp,"\n\n");
		fgets(line,ARR_SIZE,infp);
		while(!feof(infp))
		{
			if (mnum == -1)
			{
				fputs(ColourStrip(line),outfp);
			}
			else
			{
				if (*line == '\n') valid = 1;
				sscanf(line,"%s",check);
				if (valid && (!strcmp(check,"~FYWriter~CB:")
				  || !strcmp(check,"Writer:")
				  || !strcmp(check," ~FYWriter~CB:")))
				{
					if (mnum == cnt)
					{
						while (*line != '\n')
						{
							fputs(ColourStrip(line),outfp);
							fgets(line,ARR_SIZE-1,infp);
						}
					}
					valid = 0;
					cnt++;
				}
			}
			fgets(line,ARR_SIZE-1,infp);
		}
		FCLOSE(infp);
		fputs("\n\n",outfp);
		fprintf(outfp,signature,TalkerName,TalkerAddy,TalkerEmail,TalkerName,TalkerEmail);
		FCLOSE(outfp);
		send_email(addy,outfile);
		write_user(user,"~CW-~FG Your s-mail has been forwarded.\n");
		write_log(1,LOG3,"[FSMAIL] - User: [%s] Addy: [%s]\n",user->name,addy);
		return;
	}
	if (!strncasecmp(word[1],"-autofwd",2))
	{
		if (!strcasecmp(word[2],"on"))
		{
			if (user->autofwd == 1)
			{
				writeus(user,"~CW- ~FTYour new s-mail is already being sent to ~CW'~CR%s~CW'\n",user->email);
				return;
			}
			if (word_count > 3)
			{
				vnum = atoi(word[3]);
				if (user->verifynum != vnum)
				{
					write_user(user,"~CW- ~FTYou specified a wrong verification number.\n");
					return;
				}
				user->autofwd = 1;
				writeus(user,"~CW- ~FTYou have now turned on auto-forwarding of s-mail, Please note however,\n");
				writeus(user,"~CW- ~FTeverytime you change your e-mail address with .set you will have to verify\n");
				writeus(user,"~CW- ~FTyour e-mail address again. The reason is this is so that we can be on the\n");
				writeus(user,"~CW- ~FTalert for bogus e-mail addresses which some people like to do for some\n");
				writeus(user,"~CW- ~FTreason. Everytime you receive new s-mail, it will be sent to:\n  ~CR%s~FT.\n",user->email);
				writeus(user,"~CW- ~FTIf this is incorrect, then you'll have to change your e-mail addy via the\n");
				writeus(user,"~CW- ~FT.set command.\n");
				return;
			}
			if (user->verifynum == -1)
			{
				write_user(user,"~CW- ~FTYou haven't verified your e-mail address yet.. in order to do this, you\n");
				write_user(user,"~CW- ~FTmust first receive an e-mail from the talker stating what your verification\n");
				write_user(user,"~CW- ~FTnumber is. Then after receiving this number, you can use this same command,\n");
				write_user(user,"~CW- ~FTbut use it in the form '.fsmail -autofwd on <verification number>' and this\n");
				write_user(user,"~CW- ~FTwill turn it on for you.. Once this is done, you won't have to do this\n");
				write_user(user,"~CW- ~FTanymore, unless you change your e-mail address.\n");
				return;
			}
			user->autofwd = 1;
			writeus(user,"~CW- ~FTYou have now turned on auto-forwarding of s-mail, Please note however,\n");
			writeus(user,"~CW- ~FTeverytime you change your e-mail address with .set you will have to verify\n");
			writeus(user,"~CW- ~FTyour e-mail address again. The reason is this is so that we can be on the\n");
			writeus(user,"~CW- ~FTalert for bogus e-mail addresses which some people like to do for some\n");
			writeus(user,"~CW- ~FTreason. Everytime you receive new s-mail, it will be sent to ~CR%s~FT.\n",user->email);
			writeus(user,"~CW- ~FTIf this is incorrect, then you'll have to change your e-mail addy via the\n");
			writeus(user,"~CW- ~FT.set command.\n");
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (user->autofwd == 0)
			{
				write_user(user,"~CW- ~FTYou don't have new s-mail auto-forwarding on..\n");
				return;
			}
			user->autofwd = 0;
			write_user(user,"~CW- ~FTYour new s-mail will no longer be forwarded to your e-mail addy.\n");
			return;
		}
		if (!strcasecmp(word[2],"verify"))
		{
			sprintf(filename,"%s/%s.verify",TempFiles,user->name);
			if ((outfp = fopen(filename,"w")) == NULL)
			{
				write_user(user,"~CW- ~FTSorry.. verification failed.. please try back in a while..\n");
				write_log(0,LOG1,"[ERROR] - Failed to open '%s.verify' to write. Reason: [%s]\n",user->name,strerror(errno));
				return;
			}
			if (!valid_addy(user,addy,0))
			{
				write_user(user,"~CW- ~FTYou need a valid e-mail address first.\n");
				return;
			}
			fprintf(outfp,"From:    %s <%s>\n",TalkerName,TalkerEmail);
			fprintf(outfp,"To:      %s <%s>\n",user->name,addy);
			fprintf(outfp,"Subject: Auto-Forward Verification.\n");
			fprintf(outfp,"\n\n");
			/* Number between 1 and 1000 enuf.. I think so.. */
			user->verifynum = ran(1000);
			fprintf(outfp,"Hello %s:\n  This message is just to send you your verification number, so that you\nwill be able to take advantage of the RaMTITS auto-forwarding feature.\n",user->name);
			fprintf(outfp,"The point of this e-mail is to mainly verify that your e-mail address,\nthat you have set is valid.  If it isn't, then well, this message will more\nthen likely bounce ;-).  Anyways, to get on to the point, your verification\n");
			fprintf(outfp,"number is '%d' (without the quotes) and to enable new s-mail forwarding,\nWhen you go back to the talker %s, simply type in the line below:\n\n  '.fsmail -auto on %d'\n\n",user->verifynum,TalkerName,user->verifynum);
			fprintf(outfp,"Again, without the quotes. And then everytime you receive new s-mail, it\nwill be forwarded to this e-mail address. Please note, that whenever you\nchange your e-mail address with the .set command, you will have to re-verify.\n");
			fprintf(outfp,"The reason for this is again, to avoid e-mail messages bouncing, as well as\nto secure your s-mail.  If you have any questions regarding this, you can\ncontact the owner of %s (%s), or you can contact the RaMTITS\n",TalkerName,TalkerOwner);
			fprintf(outfp,"programmer at 'squirt@ramtits.ca'. Happy telnetting :-)\n");
			fputs("\n",outfp);
			fprintf(outfp,signature,TalkerName,TalkerAddy,TalkerEmail,TalkerName,TalkerEmail);
			FCLOSE(outfp);
			send_email(addy,filename);
			writeus(user,"~CW-~FT Verification e-mail has been sent. Check your '%s' e-mail addy.\n",user->email);
			return;
		}
	}
	writeus(user,usage,command[com_num].name,args);
	return;
}

/* Lets a user read the s-mail that they sent */
void read_sent(User user,char *inpstr,int rt)
{
	char filename[FSL];

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
	if (word_count == 2)
	{
		read_specific(user,NULL,atoi(word[1]),2);
		return;
	}
	sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
	count_messages(user,3);
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		write_user(user,center("~FBÄÄ~CB¯~FT> ~FGS-mail that you have sent...~FT <~CB®~FBÄÄ\n",80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_line);
		write_user(user,center("~FT-~CT=~CB> ~FGS-mail that you have sent...~CB <~CT=~FT-\n",80));
		write_user(user,ascii_line);
	}
	switch(more(user,user->socket,filename,2))
	{
		case 0:
		  write_user(user,"~CW- ~FTYou haven't sent anyone any s-mail.\n");
		  break;
		case 1:
		  user->misc_op = 2;
	}
}

/* Lets a user edit their atmospherics */
void edit_atmos(User user,char *inpstr,int rt)
{
	Room rm;
	int i,edit_at;

	if (user == NULL) return;
	edit_at = i = 0;
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
		writeus(user,usage,command[com_num].name,"~CB[~CR-list~CW|~CR# of atmos ~CW<~CRatmospheric~CW>~CB]");
		writeus(user,"~FT     Example~CB:~CR .%s -list   ~CB[ ~FGlist the atmos for this room~CB ]\n",command[com_num].name);
		writeus(user,"~FT     Example~CB: ~CR.%s 1 ~CW<~CRatmos~CW> ~FG Sets atmos # 1 to ~CW<~CRtext given~CW>\n",command[com_num].name);
		return;
	}
	rm = user->room;
	if (!strncasecmp(word[1],"-list",2))
	{
		if (strcmp(rm->owner,user->name) && user->level<WIZ)
		{
			write_user(user,"~FR You can only list the atmospherics in your room.\n");
			return;
		}
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			write_user(user,center("~CBÄÄ¯> ~FTThe atmospherics for this room! ~CB<®ÄÄ\n",80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			write_user(user,center("~CB-=[ ~FTThe atmospherics for this room!~CB ]=-\n",80));
			write_user(user,ascii_bline);
		}
		if (strcmp(rm->owner,user->name))
		{
			for(i = 0; i < MaxAtmos; ++i)
			{
				writeus(user,"~FG%2d~CB = [~RS%s~CB]\n",(i+1),rm->atmos[(i+1)]);
			}
		}
		else
		{
			for (i = 0; i < user->atmos; ++i)
			{
				writeus(user,"~FG%2d~CB = [~FG%s~CB]\n",(i+1),rm->atmos[(i+1)]);
			}
		}
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			write_user(user,center("~FBÄÄ~CB¯~FT> ~FGEnd of atmospherics.. ~FT<~CB®~FBÄÄ\n",80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			write_user(user,center("~FT-~CT=~CB> ~FGEnd of atmospherics..~CB <~CT=~FT-\n",80));
			write_user(user,ascii_bline);
		}
		return;
	}
	if (strcasecmp(rm->owner,user->name) && user->level<ADMIN)
	{
		write_user(user,"~CW-~FR This isn't your room, you can't edit the atmospherics of it.\n");
		return;
	}
	edit_at = atoi(word[1]);
	if (word_count < 3)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CR-list~CW|~CR# of atmos ~CW<~CRatmospheric~CW>~CB]");
		writeus(user,"~FT     Example~CB:~CR .%s -list   ~CB[ ~FGlist the atmos for this room~CB ]\n",command[com_num].name);
		writeus(user,"~FT     Example~CB: ~CR.%s 1 ~CW<~CRatmos~CW> ~FG Sets atmos # 1 to ~CW<~CRtext given~CW>\n",command[com_num].name);
		return;
	}
	inpstr = remove_first(inpstr);
	if (edit_at < 1)
	{
		write_user(user,"~CW-~FT I'm afraid  you can't edit a negative numbered atmosphere.\n");
		return;
	}
	sprintf(text,"%s",inpstr);
	if (edit_at > user->atmos)
	{
		if (strcasecmp(rm->owner,user->name) && edit_at > MaxAtmos)
		{
			writeus(user,"~CW-~FT You can only edit atmospherics between ~CR1 ~CW- ~CR%d~FT.\n",MaxAtmos);
			return;
		}
		if (user->level < ADMIN)
		{
			write_user(user,"~CW-~FT To edit that atmosphere, you have to purchase one at the store.\n");
			return;
		}
	}
	writeus(user,"~FG Atmosphere ~FY%d~CB:\n~CW -- ~RS%s\n",edit_at,text);
	strncpy(rm->atmos[edit_at],text,sizeof(rm->atmos[edit_at])-1);
	if (!save_atmos(rm)) write_user(user,"~CW- ~FGFailed to save atmospherics.\n");
}

/*
   This will allow users to set carbon copies of who they want to send s-mail
   to other then just the person you're specifying.
*/
void set_ccopy(User user,char *inpstr,int rt)
{
	int i,cnt = 0,num = 0;

	if (user == NULL) return;
	if (isdigit(word[1][0]))
	{
		num = atoi(word[1]);
		if (num > 5 || num < 0)
		{
			write_user(user,"~CW-~FG You can't set a carbon copy for that number.\n");
			return;
		}
		word[2][0] = toupper(word[2][0]);
		if (!user_exists(word[2]))
		{
			write_user(user,center(nosuchuser,80));
			return;
		}
		for (i = 0; i < 5; ++i)
		{
			if (!strcasecmp(user->ccopy[i],word[2]))
			{
				writeus(user,"~CW-~FT You're already sending a carbon copy of s-mail to ~FG%s~FT.\n",word[2]);
				return;
			}
		}
		strncpy(user->ccopy[num],word[2],sizeof(user->ccopy[num])-1);
		writeus(user,"~FG Alright, when you send s-mail, %s will also receive a copy.\n",word[2]);
		return;
	}
	if (!strncasecmp(word[1],"-del",2))
	{
		if (!strcasecmp(word[2],"all"))
		{
			for (i = 0; i < 5; ++i) user->ccopy[i][0] = '\0';
			write_user(user,"~CW- ~FGYou are no longer sending out carbon copies out with your s-mail.\n");
			return;
		}
		num = atoi(word[2]);
		if (num > 5 || num < 0)
		{
			writeus(user,"~CW- ~FGYou don't have a ~CR%d ~FGcarbon copy set.\n",num);
			return;
		}
		user->ccopy[num][0] = '\0';
		writeus(user,"~CW- ~FGCarbon copy number ~CR%d ~FGhas been deleted.\n",num);
		return;
	}
	for (i = 0; i < 5; ++i) if (user->ccopy[i][0]) ++cnt;
	if (cnt)
	{
		write_user(user,"~CW- ~FGYou currently have carbon copies going out to~CB:\n");
		for (i = 0; i < 5; ++i)
		{
			if (user->ccopy[i][0] == '\0') continue;
			writeus(user," ~CB[~CR%d~CB] - ~FG%s\n",i,user->ccopy[i]);
		}
	}
	write_user(user,"~FG To add a carbon copy~CB:\n");
	writeus(user,usage,command[com_num].name,"~CR#~CB [~CRuser~CB]");
	write_user(user,"~FG To delete a carbon copy~CB:\n");
	writeus(user,usage,command[com_num].name,"~CR-del ~CB[~CR# of ccopy to delete~CW|~CRall~CB]");
	return;
}

/*
   Wiznote.. everytime a staff command is used.. it should be posted on this
   board.. otherwise it could be just assumed as abusive of commands
*/
void wiznote(User user,char *inpstr,int done_editing)
{
	Editor edit;
	FILE *infp,*outfp;
	char *c,d,filename[FSL],*name;
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
	if (done_editing == 0)
	{
		if (word_count < 2)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FR%s~FM has a message to post on the staff board...\n",name);
			write_user(user,center("~FT-~CT=~CB> ~FTContributing a staff note...~CB <~CT=~FT-\n",80));
			user->editor.editing = 1;
			user->misc_op = 40;
			if (user->editor.editor==1) nuts_editor(user,NULL);
			else ramtits_editor(user,NULL);
			return;
		}
		if (!strncasecmp(word[1],"-from",2))
		{
			view_from(user,NULL,7);
			return;
		}
		if (!strncasecmp(word[1],"-delete",2))
		{
			if (user->level < GOD)
			{
				writeus(user,usage,command[com_num].name,"~CW[<~CR-read~CW|~CR-from~CW>]");
				return;
			}
			delete_messages(user,NULL,7);
			return;
		}
		if (!strncasecmp(word[1],"-read",2))
		{
			count_messages(user,7);
			if (word_count == 3)
			{
				read_specific(user,NULL,atoi(word[2]),5);
				return;
			}
			sprintf(filename,"%s/wiznotes",DataDir);
			if (user->ansi_on)
			{
				write_user(user,ansi_tline);
				write_user(user,center("~FBÄÄ~CB¯~FT> ~FGThe wiznotes staff have contributed..~FT <~CB®~FBÄÄ\n",80));
				write_user(user,ansi_bline);
			}
			else
			{
				write_user(user,ascii_tline);
				write_user(user,center("~FT-~CT=~CB> ~FGThe wiznotes staff have contributed...~CB <~CT=~FT-\n",80));
				write_user(user,ascii_bline);
			}
			switch(more(user,user->socket,filename,0))
			{
				case 0:
				  write_user(user,"~CW- ~FTNo wiznotes have been found...\n");
				  return;
				case 1:
				  user->misc_op=2;
			}
			return;
		}
		return;
	}
	if (user->status & Silenced)
	{
		write_user(user,"~CW- ~FTYour wiznote has been added...\n");
		return;
	}
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW- ~FGThere was an error writing your wiznote..\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in wiznote()\n");
		return;
	}
	/* Put the new message in first. */
	fprintf(outfp," ~FRWizNoter~CB: ~FY%s\n",user->recap);
	fprintf(outfp," ~FRWritten On~CB: ~FG%s\n",long_date(0));
	fputs(ascii_line,outfp);
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while (c != user->editor.malloc_end) putc(*c++,outfp);
	}
	else
	{
		if (edit == NULL)
		{
			write_user(user,"~CW-~FT There was an error processing your request.\n");
			return;
		}
		line = 0;
		while (line < MaxLines)
		{
			if (edit->line[line][0] != '\0')
			{
				fputs(edit->line[line],outfp);
				if (edit->line[line][strlen(edit->line[line])-1]!='\n') fputs("\n",outfp);
			}
			line++;
		}
	}
	fputs(ascii_line,outfp);
	fputs("\n",outfp);
	/* Now the rest of the old wiznotes. */
	sprintf(filename,"%s/wiznotes",DataDir);
	if ((infp = fopen(filename,"r")))
	{
		d = getc(infp);
		while(!(feof(infp)))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	rename("tempfile",filename);
	write_user(user,"~CW- ~FTYour wiznote has been added...\n");
	WriteLevel(WIZ,user,"~CB[~FTWizNote~CB] ~CW- ~FT%s ~FMhas contributed a wiznote..\n",name);
	wiznote_num++;
}

/* END OF MESSAGES.C */
