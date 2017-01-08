/*
   module2.c
     Various functions that were added after RaMTITS 1.


    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Arnaud Abelard, Mike Irving, Phypor, and
	Andrew Collington

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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "include/blocking.h"
#define _MODULE2_C
  #include "include/needed.h"
#undef _MODULE2_C
extern void kill_ident_server(void);

/*
  Allows you to delete specific messages from ANY of the files... types are
  as follows:
  1 = message board     (.wipe)
  2 = users s-mail      (.dmail)
  3 = users sent s-mail (.dsent)
  4 = news messages..   (.news -delete)
  5 = suggestions..     (.suggest -delete)
  6 = bugs              (.bug -delete)
*/
void delete_messages(User user,char *inpstr,int type)
{
	Room rm;
	FILE *infp,*outfp;
	int cnt,cnt_um,valid,just_one,range,total;
	int num,num_one,num_two,num_three,num_four,num_five,num_six;
	int word_num1,word_num2,word_num3,word_num4,word_num5,word_num6,word_num7;
	char infile[80],line[ARR_SIZE],id[ARR_SIZE],*name,check[20],type1[20];

	range = num_two = num_three = num_four = 0;
	word_num1 = word_num2 = word_num3 = word_num4 = 0;
	word_num5 = word_num6 = word_num7 = 0;
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
	if (type<1 || type>7) return;       /* Some sorta error occured. */
	if (user->invis == Invis) name=invisname;  else name = user->recap;
	just_one = 1;  total = 1;
	/* Get the word[] variables in place. */
	switch(type)
	{
		case 1:
		case 2:
		case 3:
		  word_num1=1;  word_num2=2;  word_num3=3;  word_num4=4;
		  word_num5=5;  word_num6=6;  word_num7=7;  break;
		case 4:
		case 5:
		case 6:
		case 7:
		  word_num1=2;  word_num2=3;  word_num3=4;  word_num4=5;
		  word_num5=6;  word_num6=7;  word_num7=8;  break;
	}
	if (type == 1 && user->level<WIZ && user->room->access == FIXED_PUBLIC)
	{
		write_user(user,"~FR You cannot clear the wall in this room!\n");
		return;
	}
	if ((word_num1 == 1 && word_count<2)
	  || (word_num1 == 2 && word_count<3))
	{
		print_usage(user,type);
		return;
	}
	num = atoi(word[word_num1]);
	if ((num<1 && strncasecmp(word[word_num1],"-all",2)
	  && strncasecmp(word[word_num1],"-top",2)
	  && strncasecmp(word[word_num1],"-bottom",2)))
	{
		print_usage(user,type);
		return;
	}
	else
	{
		if (!strncasecmp(word[word_num1],"-top",2)
		  && (num = atoi(word[word_num2]))<1)
		{
			print_usage(user,type);
			return;
		}
		if (!strncasecmp(word[word_num1],"-bottom",2)
		  && (num = atoi(word[word_num2]))<1)
		{
			print_usage(user,type);
			return;
		}
		if (!strcmp(word[word_num2],"-"))
		{
			num_one = atoi(word[word_num3]);
			range = 1;  just_one = 0;
			total = (num_one-num)+1;
			if (total<2)
			{
				total = 0;  range = 0;  just_one = 0;
			}
			goto GOTCHA;
		}
		if ((num_one = atoi(word[word_num2]))<1)
		{
			just_one = 1;  range = 0;  total = 1;
			goto GOTCHA;
		}
		if ((num_two = atoi(word[word_num3]))<1)
		{
			just_one = 0;  range = 0;  total = 2; num_three = 0;
			num_four = 0;  num_five = 0;
			goto GOTCHA;
		}
		if ((num_three = atoi(word[word_num4]))<1)
		{
			just_one = 0;  range = 0;  total = 3;
			num_four = 0;  num_five = 0;
			goto GOTCHA;
		}
		if ((num_four = atoi(word[word_num5]))<1)
		{
			just_one = 0;  range = 0;  total = 4;  num_five = 0;
			goto GOTCHA;
		}
		if ((num_five = atoi(word[word_num6]))<1)
		{
			just_one = 0;  range = 0;  total = 5;
			goto GOTCHA;
		}
		if ((num_six = atoi(word[word_num7]))<1)
		{
			just_one = 0;  range = 0;  total = 0;
		}
	}
	GOTCHA:
	  switch(type)
	  {
		case 1:
		  sprintf(check,"PT:");
		  strcpy(type1,"etching");               break;
		case 2:
		  sprintf(check,"~FYWriter~CB:");
		  strcpy(type1,"s-mail message");        break;
		case 3:
		  sprintf(check,"~FYWrittenTo~CB:");
		  strcpy(type1,"sent s-mail message");   break;
		case 4:
		  sprintf(check,"~FREditor~CB:");
		  strcpy(type1,"news message");          break;
		case 5:
		  sprintf(check,"~FRIdealist~CB:");
		  strcpy(type1,"suggestion");            break;
		case 6:
		  sprintf(check,"~FRBugNoter~CB:");
		  strcpy(type1,"bug");                   break;
		case 7:
		  sprintf(check,"~FRWizNoter~CB:");
		  strcpy(type1,"wiznote");		 break;
	  }
	  if (total == 0 && just_one == 0 && range == 0)
	  {
		print_usage(user,type);
		return;
	  }
	if (type == 1) rm = user->room;
	else rm = NULL;
	switch(type)
	{
		case 1:
		  sprintf(infile,"%s/%s/%s.B",RoomDir,MesgBoard,rm->name);
		  break;
		case 2:
		  sprintf(infile,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		  break;
		case 3:
		  sprintf(infile,"%s/%s/%s.S",UserDir,UserMesg,user->name);
		  break;
		case 4:
		  sprintf(infile,"%s/%s",DataDir,NewsFile);
		  break;
		case 5:
		  sprintf(infile,"%s/%s",DataDir,SugFile);
		  break;
		case 6:
		  sprintf(infile,"%s/%s",DataDir,BugFile);
		  break;
		case 7:
		  sprintf(infile,"%s/wiznotes",DataDir);
		  break;
	  }
	  if ((infp = fopen(infile,"r")) == NULL)
	  {
		switch(type)
		  {
			case 1:
			  sprintf(text,"~FT-~CT=~CB> ~FRThis wall has already been cleaned. ~CB<~CT=~FT-\n");
			  break;
			case 2:
			  sprintf(text,"~FT-~CT=~CB> ~FRIt appears as tho you have no s-mail. ~CB<~CT=~FT-\n");
			  break;
			case 3:
			  sprintf(text,"~FT-~CT=~CB> ~FRIt appears as tho you have no sent s-mail. ~CB<~CT=~FT-\n");
			  break;
			case 4:
			  sprintf(text,"~FT-~CT=~CB> ~FRIt appears as tho there is no news. ~CB<~CT=~FT-\n");
			  break;
			case 5:
			  sprintf(text,"~FT-~CT=~CB> ~FRIt appears as tho there are no suggestions. ~CB<~CT=~FT-\n");
			  break;
			case 6:
			  sprintf(text,"~FT-~CT=~CB> ~FRIt appears as tho there are no bugs. ~CB<~CT=~FT-\n");
			  break;
			case 7:
			  sprintf(text,"~FT-~CT=~CB> ~FRIt appears as tho there are no wiznotes. ~CB<~CT=~FT-\n");
			  break;
		}
		write_user(user,center(text,80));
		return;
	  }
	  if (!strncasecmp(word[word_num1],"-all",2))
	  {
		FCLOSE(infp);
		unlink(infile);
		writeus(user,"~CW- ~FTALL~FT %ss have been deleted.\n",type1);
		if (type == 1)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the %s on this wall.\n",name,type1);
		}
		if (type == 1 || type > 3)
		{
			write_log(1,LOG1,"%s deleted the %s board\n",user->name,type1);
		}
		reset_messages(user,type);
		return;
	  }
	  if ((outfp = fopen("tempfile","w")) == NULL)
	  {
		write_user(user,"~CW- ~FGSorry, unable to delete any messages at this time.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in delete_messages()\n");
		FCLOSE(infp);
		return;
	  }
	  if (!strncasecmp(word[word_num1],"-top",2))
	  {
		cnt = 0;  valid = 1;
		fgets(line,ARR_SIZE-1,infp);
		while (!feof(infp))
		{
			if (cnt <= num)
			{
				if (*line == '\n') valid = 1;
				sscanf(line,"%s",id);
				if (valid && (!strcmp(id,check)))
				{
					if (++cnt>num) fputs(line,outfp);
					valid=0;
				}
			}
			else fputs(line,outfp);
			fgets(line,ARR_SIZE-1,infp);
		}
		FCLOSE(infp);
		FCLOSE(outfp);
		unlink(infile);
		if (cnt <= num)
		{
			unlink("tempfile");
			writeus(user,"~CW- ~CRALL~FT %ss have been deleted.\n",type1);
			if (type == 1)
			{
				WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the %ss on this wall!\n",name,type1);
			}
			if (type == 1 || type>3)
			{
				write_log(1,LOG1,"%s deleted the %s board.\n",user->name,type1);
			}
			reset_messages(user,type);
			return;
		}
		rename("tempfile",infile);
		if (type == 1) check_messages(user,NULL,1);  else count_messages(user,type);
		writeus(user,"~CW- ~FY%d~FM %s%s~FM deleted from the top.\n",num,type1,num>1?"s":"");
		if (type == 1 || type>3)
		{
			write_log(1,LOG1,"%s wiped %d %s%s\n",user->name,num,type1,num>1?"s":"");
		}
		if (type == 1)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s~FT clears off some etchings.\n",name);
		}
		return;
	  }
	  cnt_um = 0;  cnt = 0;  valid = 1;
	fgets(line,ARR_SIZE-1,infp);
	while (!feof(infp))
	{
		if (*line == '\n') valid = 1;
		sscanf(line,"%s",id);
		if (valid && (!strcmp(id,check)))
		{
			cnt_um++;
			valid = 0;
		}
		fgets(line,ARR_SIZE-1,infp);
	}
	FCLOSE(infp);
	if ((infp = fopen(infile,"r")) == NULL) return;
	if (!strncasecmp(word[word_num1],"-bottom",2))
	{
		cnt = 0;  valid = 1;	total = num;
		num = cnt_um-total;	num++;
		if (num<0)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			unlink(infile);
			writeus(user,"~CW- ~CRALL ~FT%ss have been deleted.\n",type1);
			if (type == 1)
			{
				WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the etchings on this wall.\n",name);
			}
			if (type == 1 || type>3)
			{
				write_log(1,LOG1,"%s deleted the %s board.\n",user->name,type1);
			}
			reset_messages(user,type);
			return;
		}
		fgets(line,ARR_SIZE-1,infp);
		while (!feof(infp))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s",id);
			if (valid && (!strcmp(id,check)))
			{
				if (++cnt<num) fputs(line,outfp);
				valid=0;
			}
			fgets(line,ARR_SIZE-1,infp);
			if (!valid && cnt<num) fputs(line,outfp);
		}
		FCLOSE(infp);
		FCLOSE(outfp);
		unlink(infile);
		if (cnt == total)
		{
			unlink("tempfile");
			writeus(user,"~CW- ~CRALL~FT %ss have been deleted.\n",type1);
			if (type == 1)
			{
				WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the %s on this wall.\n",name,type1);
			}
			if (type == 1 || type>3)
			{
				write_log(1,LOG1,"%s deleted the %s board.\n",user->name,type1);
			}
			reset_messages(user,type);
			return;
		}
		rename("tempfile",infile);
		if (type == 1) check_messages(user,NULL,1);  else count_messages(user,type);
		writeus(user,"~FY %d~FR %s%s~FR deleted from the bottom.\n",total,type1,total>1?"s":"");
		if (type == 1 || type>3)
		{
			write_log(1,LOG1,"%s wiped %d %s%s\n",user->name,num,type1,num>1?"s":"");
		}
		if (type == 1)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s~FT clears off some etchings.\n",name);
			user->room->mesg_cnt -= num;
		}
		return;
	  }
	  if (just_one)
	  {
		cnt = 0;  valid = 1;
		if (cnt_um == 1)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			unlink(infile);
			writeus(user,"~CW- ~CRALL~FT %ss have been deleted.\n",type1);
			if (type == 1)
			{
				WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the %s on this wall.\n",name,type1);
			}
			if (type == 1 || type>3)
			{
				write_log(1,LOG1,"%s deleted the %s board.\n",user->name,type1);
			}
			reset_messages(user,type);
			return;
		}
		if (num > cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FMThere is not a ~CR%d~FM %s to delete.\n",num,type1);
			print_usage(user,type);
			return;
		}
		fgets(line,ARR_SIZE-1,infp);
		while(!feof(infp))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s",id);
			if (valid && (!strcmp(id,check)))
			{
				if (++cnt != num) fputs(line,outfp);
				valid = 0;
			}
			fgets(line,ARR_SIZE-1,infp);
			if (!valid && cnt != num) fputs(line,outfp);
		}
		FCLOSE(infp);
		FCLOSE(outfp);
		unlink(infile);
		rename("tempfile",infile);
		if (type == 1) check_messages(user,NULL,1);
		else count_messages(user,type);
		writeus(user,"~FR One %s deleted.\n",type1);
		if (type == 1 || type>3)
		{
			write_log(1,LOG1,"%s deleted one %s.\n",user->name,type1);
		}
		if (type == 1)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTclears off some etchings.\n",name);
		}
		return;
	  }
	  if (range)
	  {
		cnt = 0;  valid = 1;
		if (cnt_um == total)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			unlink(infile);
			writeus(user,"~CW- ~CRALL~FT %ss have been deleted.\n",type1);
			if (type == 1)
			{
				WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the %s on this wall.\n",name,type1);
			}
			if (type == 1 || type>3)
			{
				write_log(1,LOG1,"%s deleted the %s board.\n",user->name,type1);
			}
			reset_messages(user,type);
			return;
		}
		if (num_one>cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FTThere is not a ~CR%d~FT %s to delete.\n",num_one,type1);
			print_usage(user,type);
			return;
		}
		fgets(line,ARR_SIZE-1,infp);
		while(!feof(infp))
		{
			if (*line == '\n') valid=1;
			sscanf(line,"%s",id);
			if (valid && (!strcmp(id,check)))
			{
				cnt++;
				if (cnt<num || cnt>num_one) fputs(line,outfp);
				valid = 0;
			}
			fgets(line,ARR_SIZE-1,infp);
			if (!valid && (cnt<num || cnt>num_one)) fputs(line,outfp);
		}
		FCLOSE(infp);
		FCLOSE(outfp);
		unlink(infile);
		rename("tempfile",infile);
		if (type == 1) check_messages(user,NULL,1); else count_messages(user,type);
		writeus(user,"~FR%d %s%s deleted.\n",total,type1,total>1?"s":"");
		if (type == 1 || type>3)
		{
			write_log(1,LOG1,"%s deleted %d %s%s.\n",user->name,total,type1,total>1?"s":"");
		}
		if (type == 1)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTclears off some etchings.\n",name);
		}
		return;
	  }
	  if (just_one == 0 || range == 0)
	  {
		cnt = 0;  valid = 1;
		if (num>cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FTThere is not a ~CR%d~FT %s to delete.\n",num,type1);
			print_usage(user,type);
			return;
		}
		if (num_one>cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FTThere is not a ~CR%d~FT %s to delete.\n",num_one,type1);
			print_usage(user,type);
			return;
		}
		if (num_two>cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FTThere is not a ~CR%d~FT %s to delete.\n",num_one,type1);
			print_usage(user,type);
			return;
		}
		if (num_three>cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FTThere is not a ~CR%d~FT %s to delete.\n",num_one,type1);
			print_usage(user,type);
			return;
		}
		if (num_four>cnt_um)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			writeus(user,"~CW- ~FTThere is not a ~CR%d~FT %s to delete.\n",num_one,type1);
			print_usage(user,type);
			return;
		}
		if (cnt_um == total)
		{
			FCLOSE(infp);
			FCLOSE(outfp);
			unlink(infile);
			writeus(user,"~CW- ~CRALL~FT %ss have been deleted.\n",type1);
			if (type==1)
			{
				WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTcleared all the %s on this wall.\n",name,type1);
			}
			if (type == 1 || type>3)
			{
				write_log(1,LOG1,"%s deleted the %s board.\n",user->name,type1);
			}
			reset_messages(user,type);
			return;
		}
		while(!feof(infp))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s",id);
			if (valid && (!strcmp(id,check)))
			{
				cnt++;
				switch(total)
				{
					case 2:
					  if (cnt != num && cnt != num_one)
					    fputs(line,outfp);
					  break;
					case 3:
					  if (cnt != num && cnt != num_one
					    && cnt != num_two)
					    fputs(line,outfp);
					  break;
					case 4:
					  if (cnt != num && cnt != num_one
					    && cnt != num_two
					    && cnt != num_three)
					    fputs(line,outfp);
					  break;
					case 5:
					  if (cnt != num && cnt != num_one
					    && cnt != num_two
					    && cnt != num_three
					    && cnt != num_four)
					    fputs(line,outfp);
					  break;
				}
				valid = 0;
			}
			fgets(line,ARR_SIZE-1,infp);
			switch(total)
			{
				case 2:
				  if (!valid && (cnt != num && cnt != num_one))
				    fputs(line,outfp);
				  break;
				case 3:
				  if (!valid && (cnt != num && cnt != num_one
				    && cnt != num_two)) fputs(line,outfp);
				  break;
				case 4:
				  if (!valid && (cnt != num && cnt != num_one
				    && cnt != num_two && cnt != num_three))
				    fputs(line,outfp);
				  break;
				case 5:
				  if (!valid && (cnt != num && cnt != num_one
				    && cnt != num_two && cnt != num_three
				    && cnt != num_four)) fputs(line,outfp);
				  break;
			}
		}
		FCLOSE(infp);
		FCLOSE(outfp);
		unlink(infile);
		rename("tempfile",infile);
		if (type == 1) check_messages(user,NULL,1);
		else count_messages(user,type);
		writeus(user,"~CW- ~CR%d ~FT%s%s deleted.\n",total,type1,total>1?"s":"");
		if (type == 1 || type>3)
		{
			write_log(1,LOG1,"%s deleted %d %s%s.\n",user->name,total,type1,total>1?"s":"");
		}
		if (type==1)
		{
			WriteRoomExcept(user->room,user,"~CW- ~FG%s ~FTclears off some etchings.\n",name);
		}
		return;
	  }
	  FCLOSE(infp);
	  FCLOSE(outfp);
	  write_user(user,"~CW- ~FTUnknown error in delete_messages()\n");
	  print_usage(user,type);
}

/*
   This function reset's the variables, such as room->mesg_cnt
   suggestion_num, bug_num
*/
void reset_messages(User user,int type)
{
	Room rm;

	rm = user->room;
	switch(type)
	{
		case 1 :
		  rm->mesg_cnt = 0;	break;
		case 5 :
		  suggestion_num = 0;	break;
		case 6 :
		  bug_num = 0;		break;
		case 7 :
		  wiznote_num = 0;	break;
		default:
		  return;
	}
	return;
}

/*
  This function goes through the specified file, and counts the messages
   that are found within that file. The files that it searches through
   are 2 = users s-mail, 3 = users sent smail, 4 = news files,
   5 = suggestion file, and 6 = bug file.
*/
void count_messages(User user,int type)
{
	FILE *infp,*outfp;
	char id[ARR_SIZE],filename[FSL],line[ARR_SIZE],name[81];
	char dash[20],check[20];
	int cnt,msg_num,valid,ccnt;

	cnt = 0;
	/* First determine which one to go through: */
	switch(type)
	{
		case 2 :
		  sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		  sprintf(check,"~FYWriter~CB:");    break;
		case 3 :
		  sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
		  sprintf(check,"~FYWrittenTo~CB:"); break;
		case 4 :
		  sprintf(filename,"%s/%s",DataDir,NewsFile);
		  sprintf(check,"~FREditor~CB:");    break;
		case 5 :
		  sprintf(filename,"%s/%s",DataDir,SugFile);
		  sprintf(check,"~FRIdealist~CB:");  break;
		case 6 :
		  sprintf(filename,"%s/%s",DataDir,BugFile);
		  sprintf(check,"~FRBugNoter~CB:");  break;
		case 7 :
		  sprintf(filename,"%s/wiznotes",DataDir);
		  sprintf(check,"~FRWizNoter~CB:");  break;
		default:
		  return;
	}
	if ((infp = fopen(filename,"r")) == NULL) return;
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile to write in count_messages()\n");
		write_user(user,"~CW- ~FGUnable to open file for writing at this time.\n");
		FCLOSE(infp);
		return;
	}
	valid = 1;
	fgets(line,ARR_SIZE-1,infp);
	while (!feof(infp))
	{
		if (*line == '\n') valid=1;
		sscanf(line,"%s %s %s %d ",id,name,dash,&msg_num);
		if (valid && (!strcmp(id,check)))
		{
			ccnt = colour_com_count(name);
			cnt++;
			msg_num = cnt;
			sprintf(line," %s %-*.*s ~CW-~CR %d\n",id,15+ccnt*3,15*ccnt+3,name,msg_num);
			fputs(line,outfp);
			valid = 0;
		}
		else fputs(line,outfp);
		fgets(line,ARR_SIZE-1,infp);
	}
	FCLOSE(infp);
	FCLOSE(outfp);
	unlink(filename);
	rename("tempfile",filename);
}

/*
   This function allows a user to jump to a certain message in a board file
   for easier reading.
*/
void read_specific(User user,char *rmname,int msg_num,int command)
{
	Room rm = NULL;
	FILE *fp;
	char filename[80],*name,line[ARR_SIZE],check[ARR_SIZE],check2[20],
	     title[20];
	int valid,cnt,pt,amount;

	check[0]='\0';  amount = valid = 0;
	/*
	   Allows a user to view a review buffer of the called one... when
	   used alone it just displays the review buffer of the room they are
	   in.
	*/
	if (rmname != NULL && command == 1)
	{
		if ((rm = get_room(rmname)) == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
		if (rm->mesg_cnt == 0)
		{
			write_user(user,center("~FRIt appears as tho noone has carved anything on this wall.\n",80));
			return;
		}
		if (msg_num == 0)
		{
			writeus(user,usage,".read","~CB[~CW<~CRroom name~CW>~CB] [~CW<~CRmessage number~CW>~CB]");
			return;
		}
		if (msg_num > rm->mesg_cnt)
		{
			write_user(user,"~FGPerhaps you should try out .bfrom.\n");
			return;
		}
		sprintf(filename,"%s/%s/%s.B",RoomDir,MesgBoard,rm->name);
		if ((fp = fopen(filename,"r")) == NULL)
		{
			write_user(user,"~FG Unable to open the message board file at this time.\n");
			write_log(0,LOG1,"[ERROR] - Unable to open [%s.B] in read_specific.\n",rm->name);
			return;
		}
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~FBÄÄ~CB¯~FT> ~FYThe ~FM%s~FY wall...~FT <~CB®~FBÄÄ\n",rm->name);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FT-~CT=~CB> ~FYThe ~FM%s~FY wall... ~CB<~CT=~FT-\n",rm->name);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		valid = cnt = 1;
		fgets(line,ARR_SIZE-1,fp);
		while (!feof(fp))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s %d",check,&pt);
			if (valid && !strcmp(check,"PT:"))
			{
				if (msg_num == cnt)
				{
					while (*line != '\n')
					{
						write_user(user,line);
						fgets(line,ARR_SIZE-1,fp);
					}
				}
				valid = 0;  cnt++;
			}
			fgets(line,ARR_SIZE-1,fp);
		}
		FCLOSE(fp);
		if (user->invis == Invis)  name = invisname;
		else name = user->recap;
		if (rm == user->room)
		{
			WriteRoomExcept(user->room,user,"~CY %s ~FGdecides to view the etchings on the wall!\n",name);
		}
		return;
	} /* Message board done. Time to get to the nitty gritty. */
	if (rm == NULL)
	{
		switch(command)
		{
			case 1 :
			  sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
			  sprintf(check2,"~FYWriter~CB:");
			  sprintf(title,"s-mail");        break;
			case 2 :
			  sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
			  sprintf(check2,"~FYWrittenTo~CB:");
			  sprintf(title,"sent s-mail");   break;
			case 3 :
			  sprintf(filename,"%s/%s",DataDir,SugFile);
			  sprintf(check2,"~FRIdealist~CB:");
			  sprintf(title,"suggestion");   break;
			case 4 :
			  sprintf(filename,"%s/%s",DataDir,BugFile);
			  sprintf(check2,"~FRBugNoter~CB:");
			  sprintf(title,"bug");          break;
			case 5 :
			  sprintf(filename,"%s/wiznotes",DataDir);
			  sprintf(check2,"~FRWizNoter~CB:");
			  sprintf(title,"wiznote");	 break;
			default:
			  return;
		}
		if (msg_num == 0)
		{
			write_user(user,"~CW-~FG Umm.. did you type that right?\n");
			return;
		}
		if (!(amount = count_mess(user,command)))
		{
			write_user(user,"~CW- ~FTIt appears there aren't even any messages.\n");
			return;
		}
		if (amount < msg_num)
		{
			write_user(user,"~CW- ~FTIt appears that there aren't even that many messages.\n");
			return;
		}
		if ((fp = fopen(filename,"r")) == NULL)
		{
			write_user(user,"~FGUnable to open the file at this time.\n");
			write_log(0,LOG1,"[ERROR] - Unable to open a file in read_specific() (rm==NULL)!\n");
			return;
		}
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			sprintf(text,"~FBÄÄ~CB¯~FT> ~FG%s messages...~FT <~CB®~FBÄÄ\n",title);
			write_user(user,center(text,80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			sprintf(text,"~FT-~CT=~CB> ~FG%s messages... ~CB<~CT=~FT-\n",title);
			write_user(user,center(text,80));
			write_user(user,ascii_bline);
		}
		valid = 1;  cnt = 1;
		fgets(line,ARR_SIZE-1,fp);
		while (!feof(fp))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s",check);
			if (valid && !strcmp(check,check2))
			{
				if (msg_num == cnt)
				{
					while (*line != '\n')
					{
						write_user(user,line);
						fgets(line,ARR_SIZE-1,fp);
					}
				}
				valid = 0;  cnt++;
			}
			fgets(line,ARR_SIZE-1,fp);
		}
		FCLOSE(fp);
		return;
	}
}

/* Counts the amount of messages in the given type. */
int count_mess(User user,int type)
{
	FILE *fp;
	int valid,cnt;
	char line[ARR_SIZE],filename[80],check[ARR_SIZE],check2[30];

	cnt = 0;
	switch(type)
	{
		case 1 :
		  sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		  sprintf(check2,"~FYWriter~CB:");	break;
		case 2 :
		  sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
		  sprintf(check2,"~FYWrittenTo~CB:");	break;
		case 3 :
		  sprintf(filename,"%s/%s",DataDir,SugFile);
		  sprintf(check2,"~FRIdealist~CB:");	break;
		case 4 :
		  sprintf(filename,"%s/%s",DataDir,BugFile);
		  sprintf(check2,"~FRBugNoter~CB:");	break;
		case 5 :
		  sprintf(filename,"%s/wiznotes",DataDir);
		  sprintf(check2,"~FRWizNoter~CB:");	break;
		default:
		  return(cnt);
	}
	if ((fp = fopen(filename,"r")) == NULL) return(cnt);
	valid = 1;
	fgets(line,ARR_SIZE-1,fp);
	while (!feof(fp))
	{
		if (*line == '\n') valid = 1;
		sscanf(line,"%s",check);
		if (valid && !strcmp(check,check2))
		{
			cnt++;  valid = 0;
		}
		fgets(line,ARR_SIZE-1,fp);
	}
	FCLOSE(fp);
	return(cnt);
}

/*
   This function allows the user to see who has posted messages/sent them
   s-mail, who they've sent s-mail to, etc etc.
*/
void view_from(User user,char *inpstr,int com)
{
	FILE *fp1,*fp2;
	Room rm;
	int valid,cnt,fsize=0;
	char	id[ARR_SIZE],line[ARR_SIZE],filename[FSL],check[ARR_SIZE],
		filename2[FSL],title[50];
	struct stat fb;

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
	if (com<1 || com>7) return;
	if (com == 1) check_messages(user,NULL,1);
	else count_messages(user,com);
	switch(com)
	{
		case 1:
		  sprintf(filename,"%s/%s/%s.B",RoomDir,MesgBoard,rm->name);
		  strcpy(check,"PT:");
		  strcpy(title,"message");	break;
		case 2:
		  sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		  sprintf(filename2,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
		  strcpy(check,"~FYWriter~CB:");
		  strcpy(title,"s-mail");	break;
		case 3:
		  sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
		  strcpy(check,"~FYWrittenTo~CB:");
		  strcpy(title,"sent s-mail");	break;
		case 4:
		  sprintf(filename,"%s/%s",DataDir,NewsFile);
		  strcpy(check,"~FREditor~CB:");
		  strcpy(title,"news message");	break;
		case 5:
		  sprintf(filename,"%s/%s",DataDir,SugFile);
		  strcpy(check,"~FRIdealist~CB:");
		  strcpy(title,"suggestion");	break;
		case 6:
		  sprintf(filename,"%s/%s",DataDir,BugFile);
		  strcpy(check,"~FRBugNoter~CB:");
		  strcpy(title,"bug");		break;
		case 7:
		  sprintf(filename,"%s/wiznotes",DataDir);
		  strcpy(check,"~FRWizNoter~CB:");
		  strcpy(title,"wiznote");	break;
	}
	if ((fp1 = fopen(filename,"r")) == NULL)
	{
		switch(com)
		{
			case 1:
			  sprintf(text,"~CW-~FG It appears there are no messages on this board.\n");
			  break;
			case 3:
			  sprintf(text,"~CW-~FG It appears you have no sent s-mail.\n");
			  break;
			case 4:
			  sprintf(text,"~CW-~FG It appears there isn't any news.\n");
			  break;
			case 5:
			  sprintf(text,"~CW-~FG It appears as tho there are no suggestions :~(\n");
			  break;
			case 6:
			  sprintf(text,"~CW-~FG No bugs.. WOOHOO.\n");
			  break;
			case 7:
			  sprintf(text,"~CW-~FG Looks like the staff haven't been using their commands...\n");
			  break;
		}
		if (com != 2) write_user(user,text);
		if (com == 1 || com > 2) return;
		if (com == 2)
		{
			if (!file_exists(filename2) && !file_exists(filename))
			{
				write_user(user,"~CW-~FG It appears you have no s-mail and no new s-mail.\n");
				return;
			}
		}
	}
	if (user->ansi_on)
	{
		write_user(user,ansi_tline);
		if (com == 2 || com == 3) sprintf(text,"~FBÄÄ~CB¯~FT> ~FGYour %s contains the following messages: ~FT<~CB®~FBÄÄ\n",title);
		else sprintf(text,"~FBÄÄ~CB¯~FT> ~FGThe %s board contains the following messages: ~FT<~CB®~FBÄÄ\n",title);
		write_user(user,center(text,80));
		write_user(user,ansi_bline);
	}
	else
	{
		write_user(user,ascii_tline);
		if (com == 2 || com == 3) sprintf(text,"~FT-~CT=~CB> ~FGYour %s contains the following messages: ~CB<~CT=~FT-\n",title);
		else sprintf(text,"~FT-~CT=~CB> ~FTThe %s board contains the following messages: ~CB<~CT=~FT-\n",title);
		write_user(user,center(text,80));
		write_user(user,ascii_bline);
	}
	valid = 1;  cnt = 0;
	if (file_exists(filename))
	{
		fgets(line,ARR_SIZE-1,fp1);
		while (!feof(fp1))
		{
			if (*line == '\n') valid = 1;
			sscanf(line,"%s",id);
			if (valid && (!strcmp(id,check)))
			{
				write_user(user,line);
				cnt++;  valid = 0;
			}
			fgets(line,ARR_SIZE-1,fp1);
		}
	}
	FCLOSE(fp1);
	if (com == 2)
	{
		sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
		if (stat(filename,&fb) == -1) fsize = 0;
		else fsize = fb.st_size;  filename[0] = '\0';
		sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
		if (stat(filename,&fb) == -1) fsize = fsize;
		else fsize += fb.st_size;
		if ((fp2 = fopen(filename,"r")) != NULL)
		{
			write_user(user,"~CB[ ~FGNew s-mail from:~CB ]\n");
			valid = 1;
			fgets(line,ARR_SIZE-1,fp2);
			while (!feof(fp2))
			{
				if (*line == '\n') valid = 1;
				sscanf(line,"%s",id);
				if (valid && (!strcmp(id,check)))
				{
					write_user(user,line);
					cnt++;  valid = 0;
				}
				fgets(line,ARR_SIZE-1,fp2);
			}
			FCLOSE(fp2);
		}
	}
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	writeus(user,"~FG Total of ~CY%d~FG message%s found.\n",cnt,cnt>1?"s":"");
	if (com == 1 || com>3)
	{
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
		return;
	}
	if (com == 2)
	{
		writeus(user,"~FG Your s-mail box is using ~CY%d ~FGbytes, out of a MAX of ~CY%d\n",fsize,MaxMbytes);
	}
	if (com == 3)
	{
		if (stat(filename,&fb) == -1) fsize = 0;
		else fsize = fb.st_size;
		writeus(user,"~FG Your sent s-mail box is using ~CY%d~FG bytes, out of a MAX of ~CY%d\n",fsize,MaxSMbytes);
	}
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
	return;
}

/*
  This function goes through and allows the user to review the called upon
  buffer. Re-wrote it this way to cut down on the amount of commands I have
*/
void review_buffer(User user,char *inpstr,int rt)
{
	Room rm;
	int i,cnt=0;
	char temp[ARR_SIZE*2], search[ARR_SIZE*2];

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (!strncasecmp(word[1],"-shouts",3)) /* .rev -sh (shout buffer) */
	{
		for (i = 0 ; i<NUM_LINES ; ++i) if (shoutbuff->conv[i]) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"\n~FG Someone needs to shout something first.\n");
			return;
		}
		if (!strncasecmp(word[2],"-search",2))
		{
			cnt = 0;
			inpstr = remove_first(inpstr);
			inpstr = remove_first(inpstr);
			strcpy(search,inpstr);
			if (!strlen(inpstr))
			{
				writeus(user,usage,command[com_num].name,"-shout -search ~CB[~CRpattern~CB]");
				return;
			}
			strcpy(search,ColourStrip(search));
			for (i = 0 ; i < NUM_LINES ; ++i)
			{
				temp[0] = '\0';
				if (shoutbuff->conv[i] == NULL) continue;
				strcpy(temp,ColourStrip(shoutbuff->conv[i]));
				if (!istrstr(temp,search)) continue;
				++cnt;
			}
			if (cnt == 0)
			{
				writeus(user,"~CW- ~FTNo occurrences of ~CW\"~FY%s~CW\"~FT were found in the shout review buffer.\n",search);
				return;
			}
			writeus(user,"~CW- ~FTShout buffer containing ~CW\"~FY%s~CW\"\n",search);
			write_user(user,ascii_tline);
			for (i = 0; i < NUM_LINES; ++i)
			{
				temp[0] = '\0';
				if (shoutbuff->conv[i] == NULL) continue;
				strcpy(temp,ColourStrip(shoutbuff->conv[i]));
				if (!istrstr(temp,search)) continue;
				write_user(user,shoutbuff->conv[i]);
			}
			write_user(user,ascii_bline);
			writeus(user,"~CW-~FT Total of ~FY%d~FT line%s found.\n",cnt,cnt == 1?"":"s");
			return;
		}
		if (user->ansi_on) sprintf(text,"~FBÄÄÄ~CB¯¯~FT> ~FYReview of all shouted messages... ~FT<~CB®®~FBÄÄÄ\n");
		else sprintf(text,"~FT-~CT=~CB> ~FYReview of all shouted messages.. ~FT]~CB=~FT-\n");
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_convbuff(user,shoutbuff);
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
		return;
	}
	if (!strncasecmp(word[1],"-tells",2))
	{
		revtell(user,inpstr,0); /* still have for using .t alone..etc */
		return;
	}
	if (!strncasecmp(word[1],"-channel",2)) /* .rev -c (channel buffer) */
	{
		for (i = 0 ; i < NUM_LINES ; ++i)
		  if (channelbuff->conv[i]) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"\n~FG Someone needs to say something in the channel first.\n");
			return;
		}
		cnt = 0;
		if (!strncasecmp(word[2],"-search",2))
		{
			inpstr = remove_first(inpstr);
			inpstr = remove_first(inpstr);
			strcpy(search,inpstr);
			if (!strlen(inpstr))
			{
				writeus(user,usage,command[com_num].name,"-channel -search ~CB[~CRpattern~CB]");
				return;
			}
			strcpy(search,ColourStrip(search));
			for (i = 0 ; i < NUM_LINES ; ++i)
			{
				temp[0] = '\0';
				if (channelbuff->conv[i] == NULL) continue;
				strcpy(temp,ColourStrip(channelbuff->conv[i]));
				if (!istrstr(temp,search)) continue;
				++cnt;
			}
			if (cnt == 0)
			{
				writeus(user,"~CW- ~FTNo occurrences of ~CW\"~FY%s~CW\"~FT were found in the channel buffer.\n",search);
				return;
			}
			writeus(user,"~CW- ~FTChannel message buffer containing ~CW\"~FY%s~CW\"\n",search);
			write_user(user,ascii_tline);
			for (i = 0; i < NUM_LINES; ++i)
			{
				temp[0] = '\0';
				if (channelbuff->conv[i] == NULL) continue;
				strcpy(temp,ColourStrip(channelbuff->conv[i]));
				if (!istrstr(temp,search)) continue;
				write_user(user,channelbuff->conv[i]);
			}
			write_user(user,ascii_bline);
			writeus(user,"~CW-~FT Total of ~FY%d~FT line%s found.\n",cnt,cnt == 1?"":"s");
			return;
		}
		if (user->ansi_on) sprintf(text,"~FBÄÄÄ~CB¯¯~FT> ~FYReview of all channel messages... ~FT<~CB®®~FBÄÄÄ\n");
		else sprintf(text,"~FT-~CB=~FT[ ~FYReview of all channel messages... ~CB<~CT=~FT-\n");
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_convbuff(user,channelbuff);
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
		return;
	}
	if (!strncasecmp(word[1],"-wiz",2)) /* .rev -w (wiz buffer) */
	{
		if (user->level<WIZ)
		{
			write_user(user,"~CW-~FG Only staff members may read the wiz review buffer.\n");
			return;
		}
		for (i = 0 ; i < NUM_LINES ; ++i) if (wizbuff->conv[i]) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"\n~FG Someone needs to speak on the wiz channel first.\n");
			return;
		}
		if (!strncasecmp(word[2],"-search",2))
		{
			cnt = 0;
			inpstr = remove_first(inpstr);
			inpstr = remove_first(inpstr);
			strcpy(search,inpstr);
			if (!strlen(inpstr))
			{
				writeus(user,usage,command[com_num].name,"-wiz -search ~CB[~CRpattern~CB]");
				return;
			}
			strcpy(search,ColourStrip(search));
			for (i = 0 ; i < NUM_LINES ; ++i)
			{
				temp[0] = '\0';
				if (wizbuff->conv[i] == NULL) continue;
				strcpy(temp,ColourStrip(wizbuff->conv[i]));
				if (!istrstr(temp,search)) continue;
				++cnt;
			}
			if (cnt == 0)
			{
				writeus(user,"~CW- ~FTNo occurrences of ~CW\"~FY%s~CW\"~FT were found in the staff message buffer.\n",search);
				return;
			}
			writeus(user,"~CW- ~FTStaff message buffer containing ~CW\"~FY%s~CW\"\n",search);
			write_user(user,ascii_tline);
			for (i = 0; i < NUM_LINES; ++i)
			{
				temp[0] = '\0';
				if (wizbuff->conv[i] == NULL) continue;
				strcpy(temp,ColourStrip(wizbuff->conv[i]));
				if (!istrstr(temp,search)) continue;
				write_user(user,wizbuff->conv[i]);
			}
			write_user(user,ascii_bline);
			writeus(user,"~CW-~FT Total of ~FY%d~FT line%s found.\n",cnt,cnt == 1?"":"s");
			return;
		}
		if (user->ansi_on) sprintf(text,"~FBÄÄÄ~CB¯¯~FT> ~FYReview of staff tells and emotes... ~FT<~CB®®~FBÄÄÄ\n");
		else sprintf(text,"~FT-~CT=~CB> ~FYReview of staff tells and emotes... ~CB<~CT=~FT-\n");
		write_user(user,center(text,80));
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_convbuff(user,wizbuff);
		if (user->ansi_on) write_user(user,ansi_bline);
		else write_user(user,ascii_bline);
		return;
	}
	if (!strncasecmp(word[1],"-search",2))
	{
		rm = user->room;
		inpstr = remove_first(inpstr);
		strcpy(search,inpstr);
		if (!strlen(inpstr))
		{
			writeus(user,usage,command[com_num].name,"-search ~CB[~CRpattern~CB]");
			return;
		}
		strcpy(search,ColourStrip(search));
		for (i = 0 ; i < NUM_LINES ; ++i)
		{
			temp[0] = '\0';
			if (rm->revbuff->conv[i] == NULL) continue;
			strcpy(temp,ColourStrip(rm->revbuff->conv[i]));
			if (!istrstr(temp,search)) continue;
			++cnt;
		}
		if (cnt == 0)
		{
			writeus(user,"~CW- ~FTNo occurrences of ~CW\"~FY%s~CW\"~FT were found.\n",search);
			return;
		}
		writeus(user,"~CW- ~FTReview buffer for this room containing ~CW\"~FY%s~CW\"\n",search);
		write_user(user,ascii_tline);
		for (i = 0; i < NUM_LINES; ++i)
		{
			temp[0] = '\0';
			if (rm->revbuff->conv[i] == NULL) continue;
			strcpy(temp,ColourStrip(rm->revbuff->conv[i]));
			if (!istrstr(temp,search)) continue;
			write_user(user,rm->revbuff->conv[i]);
		}
		write_user(user,ascii_bline);
		writeus(user,"~CW-~FT Total of ~FY%d~FT line%s found.\n",cnt,cnt == 1?"":"s");
		return;
	}
	if (word_count<2 || user->level<GOD) rm = user->room;
	else
	{
		if ((rm = get_room(inpstr)) == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
		if (rm->access == PRIVATE)
		{
			write_user(user,"~CW-~FT You can't review the conversation of a private room.\n");
			return;
		}
		if (rm->lock == 1)
		{
			write_user(user,"~CW-~FT You can't review the convesation of a locked room.\n");
			return;
		}
		if (rm->access & PERSONAL)
		{
			if (rm != user->room)
			{
				if (strcmp(rm->owner,user->name))
				{
			 		write_user(user,"~CW- ~FGThat's not your room, you can't review the conversation!\n");
					return;
				}
			}
		}
	}
	for (i = 0 ; i < NUM_LINES ; ++i) if (rm->revbuff->conv[i]) ++cnt;
	if (cnt == 0)
	{
		write_user(user,"\n~FG Someone needs to say something first.\n");
		return;
	}
	if (user->ansi_on) sprintf(text,"~FBÄÄÄ~CB¯¯~FT> ~FYReview buffer for the~CB: [~FG%s~CB] ~FT<~CB®®~FTÄÄÄ\n",rm->recap);
	else sprintf(text,"~FT-~CT=~CB> ~FYReview buffer for the~CB: [~FG%s~CB] ~FT]~CB=~FT-\n",rm->recap);
	write_user(user,center(text,80));
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	write_convbuff(user,rm->revbuff);
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
}

/* Clears the called buffer.. ie .cb -t would clear the users tell buffer. */
void clear_buffers(User user,char *inpstr,int rt)
{
	Room rm;
	char *name;

	if (is_jailed(user))
	{
		status_text(user,1,command[com_num].name);
		return;
	}
	if (!strncasecmp(word[1],"-tells",2))
	{
		clear_convbuff(user->tbuff);
		write_user(user,"~CW- ~RSYour tell buffer has been cleared.\n");
		return;
	}
	if (!strncasecmp(word[1],"-shouts",2))
	{
		clear_convbuff(shoutbuff);
		write_user(user,"~CW- ~RSThe shout buffer has been cleared.\n");
		return;
	}
	if (!strncasecmp(word[1],"-wiz",2))
	{
		if (user->level<GOD)
		{
			write_user(user,"~CW- ~RSYou can't use this option.\n");
			return;
		}
		clear_convbuff(wizbuff);
		write_user(user,"~CW- ~RSThe wiz buffer has been cleared.\n");
		return;
	}
	if (!strncasecmp(word[1],"-channels",2))
	{
		clear_convbuff(channelbuff);
		write_user(user,"~CW- ~RSThe channel buffer has been cleared.\n");
		return;
	}
	if (word_count<2 || user->level<ADMIN) rm = user->room;
	else
	{
		if ((rm = get_room(inpstr)) == NULL)
		{
			write_user(user,center(nosuchroom,80));
			return;
		}
		clear_convbuff(rm->revbuff);
		writeus(user,"~CW-~RS You've cleared the review buffer for the ~CW'~CR%s~CW'~RS room.\n",rm->recap);
		return;
	}
	clear_convbuff(user->room->revbuff);
	write_user(user,"~CW- ~RSYou've cleared the review buffer for this room.\n");
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	WriteRoomExcept(user->room,user,"~CW- ~FM%s ~RSclears the review buffer.\n",name);
}

/* This loads the default shortcuts of the talker. */
void load_default_shortcuts(User user)
{

	user->scut[0].scut='!';		strcpy(user->scut[0].scom,"semote");
	user->scut[1].scut=';';		strcpy(user->scut[1].scom,"emote");
	user->scut[2].scut='/';		strcpy(user->scut[2].scom,"pemote");
	user->scut[3].scut='>';		strcpy(user->scut[3].scom,"tell");
	user->scut[4].scut='<';		strcpy(user->scut[4].scom,"pemote");
	user->scut[5].scut='\'';	strcpy(user->scut[5].scom,"to");
	user->scut[6].scut='*';		strcpy(user->scut[6].scom,"cbuff");
	user->scut[7].scut='%';		strcpy(user->scut[7].scom,"chemote");
	user->scut[8].scut='^';		strcpy(user->scut[8].scom,"channel");
	user->scut[9].scut=']';		strcpy(user->scut[9].scom,"cexec");
	user->scut[10].scut='#';	strcpy(user->scut[10].scom,"shout");
	user->scut[11].scut='-';	strcpy(user->scut[11].scom,"echo");
	user->scut[12].scut='`';	strcpy(user->scut[12].scom,"say");
	user->scut[13].scut='(';	strcpy(user->scut[13].scom,"ftell");
	user->scut[14].scut=')';	strcpy(user->scut[14].scom,"femote");
	return;
}

/* 
  This allows a user to set their own short cuts. If the word_count<2 then
  it lists the shortcuts that the user has.  I re-wrote this again, to base
  it more like the Iforms custom abbreviations, to which I want to thank
  (Deep?) for that... The way I previously wrote it.. was full of loopholes
  and wasn't very dependable.. and going through this code, found it to be
  much better then my original.. Thank yah :-)
*/
void shortcuts(User user,char *inpstr,int rt)
{
	char line[ARR_SIZE],newscut,newscom[21];
	int i=0,cnt,cnt2,scnt,found;

	cnt = cnt2 = newscut = scnt = found = 0;
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
	if (word_count<2)
	{
		write_user(user,"~FGYou have the following shortcuts set~CB:\n");
		for (i=0;i<28;++i)
		{
			if (user->scut[i].scut != 0)
			{
				++scnt;
				sprintf(line,"%c ~CB=~RS %s",user->scut[i].scut,user->scut[i].scom);
				strcat(text,"   ");
				cnt2 = colour_com_count(line);
				writeus(user,"%-*.*s",38+cnt2*3,38+cnt2*3,line);
				cnt++;
				if (cnt >= 2)
				{
					cnt = 0;
					write_user(user,"\n");
				}
			}
		}
		writeus(user,"\n~FT Total of %d shortcut%s set. Allowed a max. of %d.\n",scnt,scnt>1?"s":"",28);
		return;
	}
	newscut = word[1][0];
	inpstr = remove_first(inpstr);
	sprintf(newscom,"%s",inpstr);
	for (found = 0,i = 0 ; i<28 && found == 0 ; ++i)
	{
		if (user->scut[i].scut == newscut)
		{
			found = 1;
		}
	}
	if (found)
	{
		--i;
		if (newscom[0] == '\0')	/* Remove the shortcut command. */
		{
			user->scut[i].scut = 0;
			user->scut[i].scom[0] = '\0';
			write_user(user,"~CW-~FT Shortcut has been deleted.\n");
			return;
		}
		else			/* Otherwise change it. */
		{
			strncpy(user->scut[i].scom,inpstr,sizeof(user->scut[i].scom));
			write_user(user,"~CW-~FT Shortcut has been changed.\n");
			return;
		}
	}
	else
	{
		if (newscut == '.' || newscut == ',' || isalnum(newscut))
		{
			write_user(user,"~FT You specified an invalid shortcut key.\n");
			return;
		}
		if (newscut == '/' && user->mode == IRC)
		{
			write_user(user,"~FT You specified an invalid shortcut key.\n");
			return;
		}
		/* Find a free space. */
		for (found = 0,i = 0 ; i<28 && found == 0 ; ++i)
		{
			if (user->scut[i].scut == 0)
			{
				found = 1;
			}
		}
		if (found == 0)
		{
			write_user(user,"~CW-~FT Sorry, but you are unable to set any more shortcuts.\n");
		}
		else
		{
			if (newscom[0] == '\0')
			{
				write_user(user,"~CW-~FT It usually helps if you say what you want the shortcut to do.\n");
				return;
			}
			--i;
			user->scut[i].scut = newscut;
			strncpy(user->scut[i].scom,inpstr,sizeof(user->scut[i].scom));
			write_user(user,"~CW-~FT Shortcut has been set.\n");	
		}
	}
}

/*
  Seamless reboot stuff.. this stuff is very delicate.. so I wouldn't really
  advise touching it unless you know what you're doing.
*/
/*
  This is the main seamless reboot function, (well.. this is the function
  that calls the other functions of the seamless reboot.
*/
void sreboot_com(User user,char *inpstr,int rt)
{

	if (user->tpromoted && user->orig_lev<ADMIN)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
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
	if (rs_which == 1)
	{
		write_user(user,"~FGThe reboot countdown is currently active, cancel it first.\n");
		return;
	}
	if (rs_which == 0)
	{
		write_user(user,"~FGThe shutdown countdown is currently active, cancel it first.\n");
		return;
	}
	if (!strncasecmp(word[1],"-cancel",2))
	{
		if (!rs_countdown || rs_which != 2)
		{
			write_user(user,"~FG The seamless reboot countdown isn't currently active!\n");
			return;
		}
		if (rs_countdown && rs_which == 2 && rs_user == NULL)
		{
			write_user(user,"~FGSomeone else has set the seamless reboot count down!\n");
			return;
		}
		write_room(NULL,"   ~CT[~CRRaMTITS~CT] ~CW - ~CRSeamless reboot has been cancelled!\n");
		write_log(1,LOG1,"%s cancelled the seamless reboot.\n",user->name);
		rs_countdown = rs_announce = 0;  rs_which = -1;  rs_user = NULL;
		return;
	}
	if (word_count<1 && !isnumber(word[1]))
	{
		writeus(user,usage,command[com_num].name,"~CB[~CW<~CRsecs~CW|~CR-cancel~CW>~CB]");
		return;
	}
	if (rs_countdown && rs_which == 2)
	{
		write_user(user,"~FGThe seamless reboot countdown is currently active, cancel it first.\n");
		return;
	}
	if (word_count<2)
	{
		rs_countdown = rs_announce = 0;  rs_which = -1;  rs_user = NULL;
	}
	else
	{
		rs_countdown = atoi(word[1]);
		rs_which = 2;
	}
	write_user(user,"\07~CB[~CRWarning~CB] ~CW- ~FGThis will reboot the talker.\n");
	write_user(user,"~FGAre you SURE you want to do this?:~RS ");
	user->misc_op = 33;
	no_prompt = 1;
}

#define REBOOTING_DIR	"reboot"
#define USER_LIST_FILE	"reboot/_ulist"
#define TALKER_SYS_INFO	"reboot/_sysinfo"
#define CHILDS_PID_FILE	"reboot/_child_pid"
#define ROOM_LIST_FILE	"reboot/_rlist"

char rebooter[UserNameLen+2];
char mrname[RoomNameLen+2];

typedef struct talker_sys_info
{
	int sockets[2];
	char mr_name[RoomNameLen+2];
	char rebooter[UserNameLen+2];
	int logins,new_users;
	time_t boot_time;
} talker_sys_info;

/* Builds the system information. */
int build_sysinfo(User user)
{
	talker_sys_info tsi;
	FILE *fp;

	memset(&tsi,0,sizeof(talker_sys_info));
	strcpy(tsi.mr_name,main_room);
	tsi.sockets[0] = listen_sock[0];
	tsi.sockets[1] = listen_sock[1];
	tsi.boot_time  = boot_time;
	tsi.new_users  = new_users;
	if (user != NULL) strcpy(tsi.rebooter,user->name);
	if ((fp = fopen(TALKER_SYS_INFO,"w")) == NULL)
	{
		if (user) writeus(user,"~CW[~CRERROR~CW]~FG Unable to open reboot system info file [%s]\n",strerror(errno));
		else write_log(0,LOG1,"[ERROR] - Unable to open reboot system info file [%s]\n",strerror(errno));
		return(-1);
	}
	fwrite((void *)&tsi,sizeof(talker_sys_info),1,fp);
	FCLOSE(fp);
	return(0);
}

/* Builds the logged in users room information. */
int build_room_list(User user)
{
	FILE *fp;
	Room rm;

	fp = fopen(ROOM_LIST_FILE,"w");
	if (fp == NULL)
	{
		if (user) writeus(user,"~CW[~CRERROR~CW]~FG Failed to open reboot user list file: [%s]\n",strerror(errno));
		else write_log(0,LOG1,"[ERROR] - Failed to open reboot user list file: [%s]\n",strerror(errno));
		return(-1);
	}
	for_rooms(rm) fprintf(fp,"%s\n",rm->name);
	FCLOSE(fp);
	return(0);
}

/* This will build a list of all the users and the room which they are in. */
int build_user_list(User user)
{
	FILE *fp;
	Room rm;
	User u;
	size_t i = 0;

	fp = fopen(USER_LIST_FILE,"w");
	if (fp == NULL)
	{
		if (user) writeus(user,"~CW[~CRERROR~CW]~FG Failed to open reboot user list file: [%s]\n",strerror(errno));
		else write_log(0,LOG1,"[ERROR] - Failed to open reboot user list file: [%s]\n",strerror(errno));
		return(-1);
	}
	for_rooms(rm)/* First go and put a _ in place of spaces in room names */
	{
		for (i = 0 ; i<strlen(rm->name) ; ++i)
		{
			if (rm->name[i] == ' ') rm->name[i] = '_';
		}
	}
	for_users(u)
	{
		if (u->login || u->type == CloneType) continue;
		fprintf(fp,"%s %s\n",u->name,u->room->name);
	}
	FCLOSE(fp);
	return(0);
}

/* Build the logged in users information */
int build_loggedin_users_info(User user)
{
	User u,u2 = NULL;
	Telnet telnet;
	FILE *fp;

	for_users(u)
	{
		if (u->type == CloneType) continue;
		if (u->editor.editing)
		{
			write_user(u,"~FG Seamless reboot in progress, must abort editor.. sorry 'bout that.\n");
			editor_done(u);
		}
		if ((telnet = get_telnet(u)) != NULL)
		{
			write_user(u,"~FG Reboot in progress, disconnecting your telnet connection.\n");
			disconnect_telnet(telnet);
		}
		if (u->login)
		{
			write_user(u,"\n~FG Reboot in progress, aborting your connection, please relog..\n");
			close(u->socket);
			continue;
		}
		if (u->tbuff != NULL)
		{
			delete_convbuff(u->tbuff);
			u->tbuff = NULL;
		}
		if (u->away_buff != NULL)
		{
			delete_convbuff(u->away_buff);
			u->away_buff = NULL;
		}
		/* Reset games if user is playing them otherwise crash city */
		if (u->pop != NULL)
		{
			write_user(u,"~CW-~FT Reboot in progress, quitting your poker game.\n");
			leave_poker(u,NULL,0);
		}
		if (u->chess != NULL)
		{
			sprintf(text,"~CW- ~FTSeamless reboot in progress.. quitting your chess game.\n");
			tell_chess(u->chess,text);
			kill_chess(u);
		}
		if (u->checkers != NULL)
		{
			sprintf(text,"~CW- ~FTSeamless reboot in progress.. quitting your chess game.\n");
			checkers_say(u->checkers,text);
			checkers_kill(u);
		}
		destruct_bjgame(u);
		if (fight.first_user == u || fight.second_user == u)
		  reset_game(u,4);
		if (u->tic_opponent != NULL)
		{
			write_user(u,"~CW-~FT Seamless reboot in progress.. quitting your tic tac toe game.\n");
			write_user(u->tic_opponent,"~CW-~FT Seamless reboot in progress.. quitting your tic tac toe game.\n");
			reset_game(u,1);
			reset_game(u->tic_opponent,1);
		}
		if (u->c4_opponent != NULL)
		{
			write_user(u,"~CW-~FT Seamless reboot in progress.. quitting your connect 4 game.\n");
			write_user(u->c4_opponent,"~CW-~FT Seamless reboot in progress.. quitting your connect 4 game.\n");
			reset_game(u,2);
			reset_game(u->c4_opponent,2);
		}
		if (u->hangman_stage != -2)
		{
			u2 = get_user(u->hangman_opp);
			write_user(u,"~CW-~FT Seamless reboot in progress.. quitting your hangman game.\n");
			if (u2 != NULL)
			{
				write_user(u2,"~CW-~FT Seamless reboot in progress.. quitting your hangman game.\n");
				reset_game(u2,3);
			}
			reset_game(u2,3);
		}

#ifdef HAVE_BSHIP
		if (get_bsuser(u->name) != NULL)
		{
			write_user(u,"~CW-~FT Seamless reboot in progress.. Quitting your battleship game.\n");
			reset_game(u,5);
		}
#endif

		sprintf(text,"%s/%s",REBOOTING_DIR,u->name);
		if ((fp = fopen(text,"w")) == NULL)
		{
			if (user) writeus(user,"~CT[~CRERROR~CT]~FG Unable to open reboot user info file for '~FM%s~FG': ~CW[~FT%s~CW]\n",u->name,strerror(errno));
			else write_log(0,LOG1,"[ERROR] Unable to open reboot user info file for '%s': [%s]\n",u->name,strerror(errno));
			return(-1);
		}
		release_inputbuff(u);
		fwrite((void *)u,sizeof(struct user_struct),1,fp);
		FCLOSE(fp);
		SaveUser(u,1);

	}
	destructed = 1;
	write_window(NULL,NULL,0,0);
	return(0);
}

/* Do the reboot */
void do_reboot(User user)
{
	User u;
	Room rm;
	FILE *fp;
	int cpid;
	char *args[]={ progname,confile,NULL };

	for_rooms(rm)
	{
		SaveRoom(rm);
	}
	save_talker_stuff();
	save_all_bans();
	save_lotto();
	/* first get rid of the clones */
	for_users(u)
	{
		if (u->login || u->type != CloneType) continue;
		write_user(u->owner,"~CW- ~FGSeamless reboot in progress, destroying your clone.\n");
		destroy_user_clones(u);
	}
	write_room(NULL,"~CB[~CRRaMTITS~CB]~RS Seamless reboot in progress...\n");
	if (build_sysinfo(user)<0)
	{
		if (user != NULL) write_user(user,"~FG Reboot failed <build_sysinfo()>\n");
		else write_log(1,LOG1,"[ERROR] - Reboot failed <build_sysinfo()>\n");
		return;
	}
	if (build_room_list(user)<0)
	{
		if (user) write_user(user,"~FG Reboot failed <build_room_list()>\n");
		else write_log(1,LOG1,"[ERROR] - Reboot failed <build_room_list()>\n");
		return;
	}
	if (build_user_list(user)<0)
	{
		if (user) write_user(user,"~FG Reboot failed <build_user_list()>\n");
		else write_log(1,LOG1,"[ERROR] - Reboot failed <build_user_list()>\n");
		return;
	}
	if (build_loggedin_users_info(user)<0)
	{
		if (user) write_user(user,"~FG Reboot failed <build_loggedin_users_info()>\n");
		else write_log(1,LOG1,"[ERROR] - Reboot failed <build_loggedin_users_info()>\n");
		return;
	}
	for_users(u)
	{
		if (u->login || u->type == CloneType) continue;
	}
	write_window(NULL,NULL,0,0);	/* Flush the cache	 */
	kill_ident_server();		/* Kill the Ident server */
	close_sockets();		/* Kill any open unused sockets */
	cpid = fork();
	switch(cpid)
	{
		case -1:
		  write_log(0,LOG1,"[ERROR] - Failed to fork() in rebooting.\n");
		  if (user) write_user(user,"~CRFailed to fork.\n");
		  break;
		case 0 :
		  execvp(progname,args);
		  /* run the background process and DIE */
		  write_log(0,LOG1,"[BOOT] - Exiting child process.\n");
		  break;
		default:
		  /*
		    Parents thread.. put the childs PID to file for reboot
		    for reboot matching.
		  */
		  fp = fopen(CHILDS_PID_FILE,"w");
		  if (fp != NULL)
		  {
			fprintf(fp,"%u\n",(unsigned)cpid);
			FCLOSE(fp);
		  }
		  else write_log(0,LOG1,"[ERROR] - Failed to open %s. Reason: [%s] Reboot failed.\n",CHILDS_PID_FILE,strerror(errno));
		  exit(0);
	}
}

/*
  The reboot's been performed.. now we need to make sure everything restores
  properly.
*/
void trans_to_quiet(User user,char rmname[RoomNameLen])
{
	Room aroom;

	aroom = get_room(rmname);
	if (aroom != NULL)
	{
		user->room = aroom;
		return;
	}
	else
	{
		user->room = get_room(main_room);
		return;
	}
}

/* Create a user */
User create_user_template(User user)
{
	User u,tmp;
	int inserted;

	u = (User)malloc(sizeof(struct user_struct));
	memcpy(u,user,sizeof(struct user_struct));
	if (user_first == NULL)
	{
		user_first = u;  u->prev = NULL;
	}
	else
	{
		user_last->next = u;  u->prev = user_last;
	}
	u->next = NULL;
	user_last = u;
	inserted = 0;
	if (user_afirst == NULL)
	{
		user_afirst = u;	u->aprev = NULL;
		u->anext = NULL;	user_alast = u;
	}
	else
	{
		tmp = user_afirst;
		while (tmp != NULL)
		{
			if ((strcmp(u->name,tmp->name)<0) && tmp==user_afirst)
			{
				user_afirst->aprev = u;	  u->aprev = NULL;
				u->anext = user_afirst;   user_afirst = u;
				inserted = 1;
			}
			else if (strcmp(u->name,tmp->name)<0)
			{
				tmp->aprev->anext = u;	u->aprev = tmp->aprev;
				tmp->aprev = u;		u->anext = tmp;
				inserted = 1;
			}
			if (inserted) break;
			tmp = tmp->anext;
		}
		if (inserted == 0)
		{
			user_alast->anext = u;	u->aprev = user_alast;
			u->anext = NULL;	user_alast = u;
		}
	}
	if (u->away_buff == NULL) u->away_buff = newconv_buffer();
	if (u->tbuff == NULL) u->tbuff = newconv_buffer();
	return(u);
}

/* Retrieve the talkers system information */
int retrieve_sysinfo(void)
{
	talker_sys_info tsi;
	FILE *fp;

	memset(&tsi,0,sizeof(talker_sys_info));
	fp = fopen(TALKER_SYS_INFO,"r");
	if (fp == NULL)
	{
		write_log(0,LOG1,"Failed to open reboot system info file for reading: [%s]\n",strerror(errno));
		return(-1);
	}
	fread((void *)&tsi,sizeof(talker_sys_info),1,fp);
	FCLOSE(fp);
	strcpy(mrname,tsi.mr_name);
	listen_sock[0] = tsi.sockets[0];
	listen_sock[1] = tsi.sockets[1];
	new_users = tsi.new_users;
	boot_time = tsi.boot_time;
	strncpy(rebooter,tsi.rebooter,UserNameLen);
	return(0);
}

/* First we retrieve the rooms. */
void retrieve_rooms(void)
{
	FILE *fp;
	Room rm;
	char rmname[ARR_SIZE+1];
	int err = 0;

	fp = fopen(ROOM_LIST_FILE,"r");
	if (fp == NULL)
	{
		write_log(0,LOG1,"[ERROR] - When rebooting, in retrieve_rooms(), fopen FAILED.\n");
		return;
	}
	/* First we go through and re-create all the rooms. */
	fgets(rmname,ARR_SIZE,fp);
	while (!feof(fp))
	{
		rmname[strlen(rmname)-1] = '\0';
		if ((rm = get_room(rmname))) goto NEXTROOM;
		if ((rm = create_room()) == NULL)
		{
			write_log(0,LOG1,"[ERROR] - Failed to create room object for room '%s'\n",rmname);
			goto NEXTROOM;
		}
		reset_room(rm);
		strcpy(rm->name,rmname);
		err = LoadRoom(rm,rmname);
		if (err == 0 || err == -1)
		{
			write_log(0,LOG1,"[ERROR] - Failed to load room details for room '%s' [err = %d]\n",rmname,err);
			destruct_room(rm);
			goto NEXTROOM;
		}
		NEXTROOM:
		  fgets(rmname,ARR_SIZE,fp);
	}
	FCLOSE(fp);
}

/* Now we do the users */
void retrieve_users(void)
{
	FILE *fp1,*fp2;
	User u;
	Room rm;
	struct user_struct spanky;
	char name[UserNameLen+2],rmname[RoomNameLen+2];
	size_t i=0;

	fp1 = fopen(USER_LIST_FILE,"r");
	if (fp1 == NULL)
	{
		write_log(0,LOG1,"[ERROR] - When rebooting, in retrieve_users(), fopen FAILED.\n");
		return;
	}
	fscanf(fp1,"%s %s",name,rmname);
	while (!feof(fp1))
	{
		sprintf(text,"%s/%s",REBOOTING_DIR,name);
		fp2 = fopen(text,"r");
		if (fp2 != NULL)
		{
			for (i = 0 ; i<strlen(rmname) ; ++i)
			  if (rmname[i] == '_') rmname[i] = ' ';
			fread((void *)&spanky,sizeof(struct user_struct),1,fp2);
			create_user_template(&spanky);
			num_of_users++;
			u = get_user(name);
			if (u != NULL)
			{
				rm = get_room(rmname);
				if (rm != NULL) u->room = rm;
				else u->room = get_room(main_room);
			}
			FCLOSE(fp2);
		}
		else write_log(0,LOG1,"[ERROR] - Failed to open reboot user info for '%s' [%s]",name,strerror(errno));
		fscanf(fp1,"%s %s",name,rmname);
	}
	FCLOSE(fp1);
	destructed = 0;
}

/* put the spaces back in the rooms. */
void redo_rooms(void)
{
	Room rm;
	size_t i = 0;

	for_rooms(rm)
	{
		for (i = 0 ; i < strlen(rm->name) ; i++)
		{
			if (rm->name[i] == '_') rm->name[i] = ' ';
		}
	}
}

/* Now we recuperate from what just happened ;-) */
int possibly_reboot(void)
{
	User u,chk;
	Room rm;
	FILE *fp;
	char r[16];

	write_log(0,LOG1,"[RaMTITS] - Checking for a reboot proof....\n");
	fp = fopen(CHILDS_PID_FILE,"r");
	if (fp == NULL)
	{
		write_log(0,LOG1,"[RaMTITS] - Can't find child's PID. This isn't a reboot.\n");
		return(0);
	}
	memset(r,0,16);
	fgets(r,15,fp);
	write_log(0,LOG1,"[RaMTITS] - Reading child's PID.. boot: [%u] - child: [%u]\n",(unsigned int)getpid(),(unsigned int)atoi(r));
	if (getpid() != atoi(r))
	{
		write_log(0,LOG1,"[RaMTITS] - This isn't a reboot.\n");
		return(0);
	}
	write_log(0,LOG1,"[RaMTITS] - [possibly_reboot()] Rebooting...\n");
	retrieve_sysinfo();
	retrieve_rooms();
	redo_rooms();
	retrieve_users();
	system("rm -f " REBOOTING_DIR "/*");
	/*
	  Now we check if the user is in a NULL room, if so move them to the
	  main room.
	*/
	for_users(chk)
	{
		if (chk->splits)
		{
			splitscreen_off(chk);
			splitscreen_on(chk);
		}
		release_inputbuff(chk);
		if (chk->room) continue;
		if (chk->room == NULL)
		{
			rm = get_room(main_room);
			if (rm != NULL) chk->room = rm;
			else write_log(1,LOG1,"[ERROR] - The main room wasn't found after seamless reboot.\n");
		}
	}
	if (rebooter[0])
	{
		u = get_user(rebooter);
		if (u == NULL) write_log(0,LOG1,"[%s] didn't survive the reboot :~(\n",rebooter);
	}
	write_room(NULL,"~CB[~CRRaMTITS~CB]~RS Seamless reboot completed...\n");
	write_window(NULL,NULL,0,0);
	time(&sreboot_time);
	return(1);
}


/* Loads up some talker variables */
void load_tvars(User user,char *inpstr,int rt)
{
	FILE *fp;
	Commands com;
	char file1[FSL],name[50];
	int i = 0,j;

	if (user != NULL) write_user(user,"~FG Re-loading the variables in memory.\n");
	for (j = 0; j < MaxStaff; ++j)
	{
		staffmember[j][0] = '\0';
	} j = 0;
	for (j = 0 ; j < 20 ; ++j)
	{
		disabled[j][0] = '\0';
	} j = 0;
	for (j = 0 ; j < 50 ; ++j)
	{
		member[j][0] = '\0';
	} j = 0;
	sprintf(file1,"%s/%s",DataDir,StaffFile);
	fp = fopen(file1,"r");
	if (fp != NULL)
	{
		fscanf(fp,"%s",name);
		while(!feof(fp))
		{
			if (i >= MaxStaff)
			{
				write_log(0,LOG1,"[ERROR] - Too many staff defined in the staff_file.\n");
				break;
			}
			strcpy(staffmember[i],name);
			i++;
			fscanf(fp,"%s",name);
		}
		FCLOSE(fp);
	}
	file1[0] = '\0';
	i = 0;
	sprintf(file1,"%s/disabled",MiscDir);
	fp = fopen(file1,"r");
	if (fp != NULL)
	{
		fscanf(fp,"%s",name);
		while(!feof(fp))
		{
			if ((com = get_comname(name)) != NULL) com->disabled = 1;
			else write_log(0,LOG1,"[ERROR] - Unknown command %s found in the disabled list.\n",name);
			fscanf(fp,"%s",name);
		}
		FCLOSE(fp);
	}
	file1[0] = '\0';
	i = 0;
	sprintf(file1,"%s/%s",DataDir,MemberFile);
	fp = fopen(file1,"r");
	if (fp != NULL)
	{
		fscanf(fp,"%s",name);
		while(!feof(fp))
		{
			if (i >= 50)
			{
				write_log(0,LOG1,"[ERROR] - Too many members defined in the member list.\n");
				break;
			}
			strcpy(member[i],name);
			i++;
			fscanf(fp,"%s",name);
		}
		FCLOSE(fp);
	}
	i = 0;
	return;
}

/*
  This command, when used alone, displays the users on the users friend
  list, or if a user is given and the user IS in their friend list, it
  removes the friend from the list, if the user ISN'T in their friend
  list, then it adds the friend.
*/
void friends(User user,char *inpstr,int rt)
{
	char name[UserNameLen+2],*rname;
	int i,cnt,cnt2;

	cnt2 = i = cnt = 0;
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
	name[0] = '\0';
	if (word_count<2)
	{
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_user(user,center("~FT-~CT=~CB> ~FGYour current list of friends...~CB <~CT=~FT-\n",80));
		if (user->ansi_on) write_user(user,ascii_bline);
		else write_user(user,ascii_bline);
		for (i = 0 ; i < 50 ; ++i)
		{
			if (user->friends[i][0] == '\0') continue;
			cnt++;  cnt2++;
			writeus(user,"~FG%-17s",center(user->friends[i],17));
			if (cnt2 >= 4)
			{
				write_user(user,"\n");
				cnt2 = 0;
			}
		}
		if (cnt2 > 0 && cnt2 < 4) write_user(user,"\n");
		if (cnt == 0)
		{
			write_user(user,"~CW- ~FGYou have no friends.\n");
		}
		if (user->ansi_on) write_user(user,ansi_line);
		else write_user(user,ascii_line);
		return;
	}
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	if (!strcasecmp(user->name,word[1]))
	{
		write_user(user,"~FG It's a pretty sad day when you need to add yourself to your friend list...\n");
		return;
	}
	if (!strcasecmp(word[1],"-none"))
	{
		for (i = 0 ; i < 50 ; ++i)
		{
			user->friends[i][0] = '\0';
		}  i = 0;
		write_user(user,"~CW-~FT Your friends have been removed.\n");
		return;
	}
	for (i = 0 ; i < 50 ; ++i)
	{
		if (!strcasecmp(user->friends[i],word[1]))
		{
			writeus(user,"~FT You have removed ~CM%s~FT from your friend list...\n",word[1]);
			user->friends[i][0] = '\0';
			return;
		}
	}
	if ((rname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (is_enemy_name(user,rname))
	{
		write_user(user,"~CW-~FT You can't make your enemies your friends..\n");
		return;
	}
	for (i = 0 ; i < 50 ; ++i)
	{
		if (user->friends[i][0] == '\0')
		{
			strcpy(user->friends[i],rname);
			writeus(user,"~FG%s ~FRadded to your friends list.\n",rname);
			sprintf(text,"~FT %s~FG has added you to %s friend list.\n",user->recap,gender1[user->gender]);
			write_user(get_user(rname),text);
			return;
		}
	}
	write_user(user,"~CRYour friends list is currently full.\n");
	return;
}

/*
  This function is a subsid of the above function, meaning, that it does
  the same thing, except it does it with enemies, and not friends ;-)
*/
void enemies(User user,char *inpstr,int rt)
{
	char name[UserNameLen+2],*uname,*rname;
	int i,cnt,cnt2;

	cnt2 = i = cnt = 0;
	if (user->invis == Invis) uname=invisname;  else uname = user->recap;
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
	name[0] = '\0';
	if (word_count<2)
	{
		if (user->ansi_on) write_user(user,ansi_tline);
		else write_user(user,ascii_tline);
		write_user(user,center("~FT-~CT=~CB> ~FGYour current list of enemies...~CB <~CT=~FT-\n",80));
		if (user->ansi_on) write_user(user,ascii_bline);
		else write_user(user,ascii_bline);
		for (i = 0 ; i < 50 ; ++i)
		{
			if (user->enemies[i][0] == '\0') continue;
			cnt++;
			writeus(user,"~FG%-17s",center(user->enemies[i],17));
			cnt2++;
			if (cnt2 >= 4)
			{
				write_user(user,"\n");
				cnt2 = 0;
			}
		}
		if (cnt2 > 0 && cnt2 < 4) write_user(user,"\n");
		if (cnt == 0) write_user(user,"~CW-~FG You have no enemies.\n");
		if (user->ansi_on) write_user(user,ansi_line);
		else write_user(user,ascii_line);
		return;
	}
	strtolower(word[1]);
	word[1][0] = toupper(word[1][0]);
	if (!strcasecmp(user->name,word[1]))
	{
		write_user(user,"~FG Do you hate yourself that much?...\n");
		return;
	}
	for (i = 0 ; i < 50 ; ++i)
	{
		if (!strcasecmp(user->enemies[i],word[1]))
		{
			writeus(user,"~FTYou have removed ~FM%s~FT from your enemy list...\n",word[1]);
			user->enemies[i][0] = '\0';
			return;
		}
	}
	if ((rname = get_user_full(user,word[1])) == NULL)
	{
		write_user(user,center(nosuchuser,80));
		return;
	}
	if (is_friend_name(user,rname))
	{
		write_user(user,"~CW-~FT You can't make your friends your enemies..\n");
		return;
	}
	for (i = 0 ; i < 50 ; ++i)
	{
		if (user->enemies[i][0] == '\0')
		{
			strcpy(user->enemies[i],rname);
			writeus(user,"~FG%s ~CRadded to your enemy list.\n",rname);
			sprintf(text,"~FT %s~CG has added you to their enemy list.\n",uname);
			write_user(get_user(word[1]),text);
			return;
		}
	}
	write_user(user,"~CRYour enemy list is currently full.\n");
	return;
}

/* Disable a talkers command */
void disable_command(User user,char *inpstr,int rt)
{
	FILE *fp;
	char filename[80],comname[20];
	Commands com = NULL,cmd = NULL;
	int cnt = 0,cnt2;

	cnt2 = 0;
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
		for_comms(cmd)
		{
			if (cmd->disabled == 0) continue;
			if (++cnt == 1)
			{
				if (user->ansi_on) write_user(user,ansi_tline);
				else write_user(user,ascii_tline);
				write_user(user,center("~FT-~CT=~CB> ~FGThe current list of disabled commands...~CB <~CT=~FT-\n",80));
				if (user->ansi_on) write_user(user,ascii_bline);
				else write_user(user,ascii_bline);
			}
			writeus(user,"~FG%-20.20s",cmd->name);
			cnt2++;
			if (cnt2 >= 4)
			{
				write_user(user,"\n");
				cnt2 = 0;
			}
		}
		if (cnt2) write_user(user,"\n");
		if (cnt == 0) write_user(user,center("~CRThere are no disabled commands..\n",80));
		return;
	}
	if (user->tpromoted && user->orig_lev < GOD)
	{
		writeus(user,tempnodo,command[com_num].name);
		return;
	}
	if ((com = get_command(user,word[1])) == NULL)
	{
		writeus(user,wtf,word[1]);
		return;
	}
	strcpy(comname,com->name);
	if (!strcasecmp(word[1],command[com_num].name))
	{
		writeus(user,"~FM FUNNY.. you can't disable the '%s' command ;-)\n",command[com_num].name);
		return;
	}
	if (com->min_lev>user->level)
	{
		write_user(user,"~FM You need the command before you can disable it.\n");
		return;
	}
	for_comms(cmd)
	{
		if ((com == cmd) && cmd->disabled == 1)
		{
			writeus(user,"~FYThe ~CR.%s~FY command has been reactivated.\n",com->name);
			write_log(1,LOG1,"%s REACTIVATED the %s command!\n",user->name,com->name);
			com->disabled = 0;
			WriteRoomExcept(NULL,user,"~CB[~CRRaMTITS~CB]~CW- ~FGThe ~FT.%s~FG command has been reactivated.\n",com->name);
			goto DI_FILE;
		}
	}
	for_comms(cmd)
	{
		if (com == cmd)
		{
			writeus(user,"~FYYou have disabled the ~CR.%s~FY command!\n",com->name);
			write_log(1,LOG1,"%s DISABLED the %s command!\n",user->name,com->name);
			com->disabled=1;
			WriteRoomExcept(NULL,user,"~CB[~CRRaMTITS~CB]~CW- ~FGThe ~FT.%s~FG command has been disabled.\n",com->name);
			goto DI_FILE;
		}
	}
	write_user(user,"~CRThere are already the max amount of disabled commands!\n");
	return;

	DI_FILE:
	  sprintf(filename,"%s/disabled",MiscDir);
	  if ((fp = fopen(filename,"w")) == NULL)
	  {
		write_user(user,"~CRUnable to open the disabled list at this point in time.\n");
		write_log(0,LOG1,"[ERROR] - Unable to open the disabled file to write in disable_command()!\n");
		return;
	  }
	  for_comms(cmd)
	  {
		if (cmd->disabled == 0) continue;
		sprintf(text,"%s\n",cmd->name);
		fputs(text,fp);
	  }
	  FCLOSE(fp);
	  for_comms(cmd)
	  {
		if (cmd->disabled == 1) ++cnt;
	  }
	  if (cnt == 0) unlink(filename);
}

void do_maintenance(void)
{

	/* reset clone_cnt, so we don't eventually go over the limit. */
	rename_logfiles();
	if (!backup_talker) write_log(1,LOG1,"[RaMTITS] - Talker backup has been disabled.\n");
	else backup_files();
}

/*
  Screen a site.. this allows you to screen certain sites so that they won't
  show up whenever a connection is made from a certain site.. I know some of
  you may know what i'm talking about.. ;-)
*/
void screen_site(User user,char *inpstr,int rt)
{
	Bans	b;
	int	cnt, hours, mins, secs, days, i;
	long	ban_length;
	char	line[251],time_buff[81],temp[41];

	cnt = hours = mins = secs = days = ban_length = i = 0;
	if (word_count<2)
	{
		writeus(user,usage,command[com_num].name,"~CB[~CR-list~CW|~CRsite.to.screen~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-list",2))
	{
		for_bans(b)
			if (b->type == Screen) ++cnt;
		if (cnt == 0)
		{
			write_user(user,"~CW- ~FTThere are no current screened sites.\n");
			return;
		}
		if (user->ansi_on)
		{
			write_user(user,ansi_tline);
			write_user(user,center("~FBÄÄÄ~CB¯¯~FT> ~FGCurrent list of screened sites... ~FT<~CB®®~FBÄÄÄ\n",80));
			write_user(user,ansi_bline);
		}
		else
		{
			write_user(user,ascii_tline);
			write_user(user,center("~FT-~CT=~CB> ~FGCurrent list of screened sites... ~CB<~CT=~FT-\n",80));
			write_user(user,ascii_bline);
		}
		cnt = 0;
		for_bans(b)
		{
			for (i = 0; i < sizeof(time_buff)-1; ++i) time_buff[i] = '\0';
			for (i = 0; i < sizeof(temp)-1; ++i) temp[i] = '\0';
			if (b->type != Screen) continue;
			ban_length = (time(0) - b->time_banned);
			days	= ban_length / 86400;
			hours	= (ban_length%86400)/3600;
			mins	= (ban_length%3600)/60;
			secs	= ban_length%60;
			strcpy(time_buff,"Screened for: ");
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
				if (strstr(time_buff,"hour") || strstr(time_buff,"day"))                                strcat(time_buff,", ");
				sprintf(temp,"%d minute%s",mins,mins == 1?"":"s");
				strcat(time_buff,temp);   
			}
			if (strstr(time_buff,"minute") || strstr(time_buff,"hour")
			  || strstr(time_buff,"day")) strcat(time_buff,", and ");
			sprintf(temp,"%d second%s",secs,secs == 1?"":"s");
			strcat(time_buff,temp);
			sprintf(line,"~FGSite/Domain~CB:~FT %s\n~CB           : ~FG%s\n",b->who,time_buff);
			write_user(user,line);
			++cnt;
		}
		writeus(user,"\n~CW-~FT Total of %d screened site%s/domain%s.\n",cnt,cnt == 1?"":"s",cnt == 1?"":"s");
		return;
	}
	b = find_ban(word[1],Screen);
	if (b != NULL)
	{
		nuke_ban(b);
		write_user(user,"~CW- ~FTSite has been unscreened.\n");
		return;
	}
	b = new_ban(word[1],Screen);
	if (b == NULL)
	{
		write_user(user,"~CW- ~FTAn error occurred.. unable to screen the site right now.\n");
		return;
	}
	b->time_banned = time(0);
	strncpy(b->by,user->name,sizeof(b->by)-1);
	strcpy(b->reason,"N/A");
	write_user(user,"~CW- ~FTSite/Domain has been screened.\n");
	write_log(1,LOG1,"[SCREEN] - User: [%s] Site: [%s]\n",user->name,word[1]);
}

/*
  We need to be able to process triggers.. such as replacing $1's with a
  specific string.. etc.. This really has no purpose right now, as it probably
  will in future versions.. it was something I started, but didn't really
  get around to finishing.
*/
char *process_trigger(char *trig,char *str)
{
	static char text2[ARR_SIZE*2];
	char *t,*s;
	int i;

	text2[0] = text[0] = '\0';
	/*
	  We first have to fill the word[] array, with str, because we need to
	  know what to replace the $0, $1, etc, etc with.
	*/
	clear_words();
	word_count = wordfind(str);
	t = text2;  s = trig;
	if (!word_count) return(str);
	/*
	  Now we need to go through the user->trigger[] and find the $0, $1's.
	*/
	while (*s)
	{
		for (i = 0 ; i<word_count ; ++i)
		{
			sprintf(text,"$%d",i);
			if (!strncasecmp(s,text,strlen(text)))
			{
				memcpy(t,word[i],strlen(word[i]));
				t += strlen(word[i])-1;
				s += strlen(text)-1;
				goto CONT;
			}
		}
		*t = *s;
		CONT:
		  ++s;  ++t;
	}
	*t = '\0';
	return(text2);
}

/* This allows you to toggle split screen on/off */
void set_splitscr(User user,char *inpstr,int rt)
{

	if (user->splits == 0)
	{
		user->splits = 1;
		cls(user,NULL,0);
		splitscreen_on(user);
		write_user(user,"~FG Split screen has now been turned ~CRON\n");
		return;
	}
	splitscreen_off(user);
	user->splits = 0;
	write_user(user,"~FG Split screen has now been turned ~CROFF\n");
	return;
}

/* END OF MODULE2.C */
