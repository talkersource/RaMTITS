/*
   menus.c
    The source within this file contains the functions for the various
    menus, such as the bank, store, and set menus. As well as the functions
    for the commands when they're used by themselves (ie .set desc).

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Neil Robertson, and all the users who have suggested
    wanting to be able to set certain things.

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
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "include/menus.h"
#define _MENUS_C
  #include "include/needed.h"
#undef _MENUS_C

int GetSetOp(char *swd)
{
	int x,len;

	x = len = 0;
	len = strlen(swd);
	while (setting[x].sword[0]!='*')
	{
		if (!strncasecmp(setting[x].sword,swd,len))
		{
			return(setting[x].set_op);
		}
		++x;
	}
	return(-1);
}

int GetStoreBuyOp(char *stwd)
{
	int x,len;

	x = len = 0;
	len = strlen(stwd);
	while (storestuff[x].stword[0] != '*')
	{
		if (!strncasecmp(storestuff[x].stword,stwd,len))
		{
			return(storestuff[x].buy_op);
		}
		++x;
	}
	return(-1);
}

int GetStoreSellOp(char *stwd)
{
	int x,len;

	x = len = 0;
	len = strlen(stwd);
	while (storestuff[x].stword[0] != '*')
	{
		if (!strncasecmp(storestuff[x].stword,stwd,len))
		{
			return(storestuff[x].sell_op);
		}
		++x;
	}
	return(-1);
}

int GetBankOp(char *bwd)
{
	int x,len;

	x = len = 0;
	len = strlen(bwd);
	while (banking[x].bword[0] != '*')
	{
		if (!strncasecmp(banking[x].bword,bwd,len))
		{
			return(banking[x].bank_op);
		}
		++x;
	}
	return(-1);
}

/* Do the actual .set command. */
void setmeup(User user,char *inpstr,int rt)
{
	char *name;
	int setop;

	if (user == NULL) return; /* Safety first */
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
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (word_count<2)
	{
		if (user->menus == 1)
		{
			PrintMenu(user,2);
			user->set_op	= 38;
			no_prompt	= 1;
			user->ignall_store = user->ignall;
			user->ignall	= 1;
			WriteRoomExcept(user->room,user,"~CW- ~FT%s~FY enters the ~FG%s ~FYsetup menu...\n",name,TalkerRecap);
			return;
		}
		else
		{
			PrintMenu(user,6);
			return;
		}
	}
	inpstr = remove_first(inpstr);
	if ((setop = GetSetOp(word[1])) == -1)
	{
		if (!strcasecmp(word[1],"show"))
		{
			write_user(user,center("~FT-~CT=~CB> ~FGYour setting attributes are set at: ~CB<~CT=~FT-\n",80));
			ShowSettings(user);  return;
		}
		PrintMenu(user,6);
		return;
	}
	user->set_op	= setop;
	clear_words();
	word_count	= wordfind(inpstr);
	SetOps(user,inpstr,0);
	user->set_op	= 0;
	return;
}

/*
   Now we actually do the menu thing.. The big difference here now is that
   yes, it does still read off of the user->*_op, but now, I have made it
   so that if a user does want to use menus, or just use .set <item> then
   this is extremely more flexible.. and easier to read, as well as easier
   to use.. Perhaps.. still not sure how it's going to turn out yet... We
   also have the int menu, which can be 1 of 2 things.. 0 = NO menu, 1 =
   yes.. the user is actually using a menu.
*/
void SetOps(User user,char *inpstr,int menus)
{
	Room rm;
	char *name,temp[ARR_SIZE],temp2[81];
	long val;
	int bmonth,bday,i,cnt,tchange;

	temp[0] = temp2[0] = '\0';
	bmonth = bday = i = cnt = val = tchange = 0;
	if (user == NULL) return;
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	/* Yes.. we do still handle it in a big switch statement :p */
	switch(user->set_op)
	{
	 /*
	    Now it used to be.. that when the user first entered the set menu
	    we would just set user->set_op to 1 and let the user take their
	    pick of the options.. but as you can see from the set structure,
	    that won't work out to well.. but I think I know of a few good
	    ways around this.. first by setting the user->set_op to 38.. :)
	*/
		case 1 : /* .set desc */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set desc ~CB[~CRnew desc~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW- ~FTWell, why did you say you wanted to set your desc in the first place?\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  strcpy(temp,ColourStrip(inpstr));
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW- ~FTYour description can't contain the $vars.. that could be bad...\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new description~CB:~RS ");
				user->set_op = 1;
				return;
			}
		  }
		  if (istrstr(temp,"NULL"))/* Can't have nullifying settings! */
		  {
			write_user(user,"~CW- ~FTWell you can't set your description to that... sheesh..\n");
			if (menus==0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new description~CB:~RS ");
				user->set_op = 1;
				return;
			}
		  }
		  if (strlen(temp)>UserDescLen
		    || strlen(inpstr)>sizeof(user->desc)-1)
		  {
			write_user(user,"~CW-~FT That description would be a tad bit too long...\n");
			if (menus==0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new description~CB:~RS ");
				user->set_op = 1;
				return;
			}
		  }
		  /* I've started becoming to careful w/ buffer overflows :-) */
		  strncpy(user->desc,inpstr,sizeof(user->desc)-1);
		  if (user->predesc[0] != '\0')
		    writeus(user,"~CW- ~FTYou will now be seen as~CB: ~RS%s %s%s%s\n",user->predesc,user->invis>Invis?user->temp_recap:user->recap,user->desc[0]=='\''?"":" ",user->desc);
		  else writeus(user,"~CW- ~FTYou will now be seen as~CB: ~RS%s%s%s\n",user->invis>Invis?user->temp_recap:user->recap,user->desc[0]=='\''?"":" ",user->desc);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op=39;
			return;
		  }
		case 2 : /* .set recap */
		  if (!strlen(inpstr))
		  {
			if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set recap ~CB[~CRnew recapped name~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW- ~FTWell why did you say you wanted to set your desc in the first place?\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  if (strstr(inpstr,"~FK") && user->level<GOD)
		  {
			write_user(user,"~CW- ~FTYou can't have the colour black in your recapped name..\n");
			if (menus==0) return;
			else
			{
				write_user(user,"~CW- ~FTRe-enter recapped name~CB:~RS ");
				user->set_op = 2;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->recap)-1)
		  {
			write_user(user,"~CW- ~FTThat name would be too long..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTRe-enter recapped name~CB:~RS ");
				user->set_op = 2;
				return;
			}
		  }
		  strcpy(temp,ColourStrip(inpstr));
		  if (strcasecmp(temp,user->name) && user->level<OWNER)
		  {
			write_user(user,"~CW- ~FTIt usually helps if your recapped name was the same as your login name.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTRe-enter recapped name~CB:~RS ");
				user->set_op = 2;
				return;
			}
		  }
		  if (user->invis>Invis) strcpy(user->temp_recap,inpstr);
		  else strcpy(user->recap,inpstr);
		  if (user->predesc[0] != '\0')
		    writeus(user,"~CW- ~FTYou will now be seen as~CB: ~RS%s %s%s%s\n",user->predesc,user->invis>Invis?user->temp_recap:user->recap,user->desc[0]=='\''?"":" ",user->desc);
		  else writeus(user,"~CW- ~FTYou will now be seen as~CB: ~RS%s%s%s\n",user->invis>Invis?user->temp_recap:user->recap,user->desc[0]=='\''?"":" ",user->desc);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 3 : /* .set pdesc */
		  if (!strlen(inpstr))
		  {
			if (menus==0)
			{
				write_user(user,"~CW- ~FTSet Usage~CB: ~CR.set pdesc ~CB[~CRnew predesc~CW|~CR-none~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW- ~FTIf you didn't want to set it.. why'd you choose it?\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strncasecmp(word[0],"-none",2))
		  {
			write_user(user,"~CW- ~FTYour pre-description has been removed.\n");
			user->predesc[0] = '\0';
			if (webpage_on == 1) do_user_webpage(user);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  strcpy(temp,ColourStrip(inpstr));
		  if (istrstr(temp,"NULL"))
		  {
			write_user(user,"~CW- ~FTYou can't nullify your pre-description.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW- ~FTYour pre-desc can't contain the $vars.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new pre-desc~CB:~RS ");
				user->set_op = 3;
				return;
			}
		  }
		  if (strlen(temp)>PreDescLen
		    || strlen(inpstr)>sizeof(user->predesc)-1)
		  {
			write_user(user,"~CW- ~FTThat's a tad bit too long..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new pre-desc~CB:~RS ");
				user->set_op = 3;
				return;
			}
		  }
		  strncpy(user->predesc,inpstr,sizeof(user->predesc)-1);
		  writeus(user,"~CW- ~FTYour pre-desc is now set to~CB: [~RS%s~CB]\n",user->predesc);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op=39;
			return;
		  }
		case 4 : /* .set lalias */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB:~CR .set lalias ~CB[~CRnew level alias~CW|~CR-none~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW- ~FTGeeze.. you gotta actually say what you want to set it to.\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  inpstr = ColourStrip(inpstr);
		  if (!strncasecmp(word[0],"-none",2))
		  {
			write_user(user,"~CW- ~FTYour level alias has been removed.\n");
			user->lev_rank[0] = '\0';
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW- ~FTYour level alias can't contain the $vars..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTEnter new level alias~CB:~RS ");
				user->set_op = 4;
				return;
			}
		  }
		  if (istrstr(inpstr,"NULL"))
		  {
			write_user(user,"~CW- ~FTYou can't nullify your level alias..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new level alias~CB:~RS ");
				user->set_op = 4;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->lev_rank)-1)
		  {
			write_user(user,"~CW- ~FTThat level alias is a tad bit too long..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new level alias~CB:~RS ");
				user->set_op = 4;
				return;
			}
		  }
		  strncpy(user->lev_rank,inpstr,sizeof(user->lev_rank)-1);
		  writeus(user,"~CW- ~FGYour level alias is now set at~CB: [~CR%s~CB]\n",user->lev_rank);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 5 : /* .set email */
		  if (!strlen(inpstr))
		  {
			if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set email ~CB[~CRnew email addy~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well next time don't say you want to set your e-mail addy.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  inpstr = ColourStrip(inpstr);
		  if (IsUserVar(inpstr))
		  {
			write_user(user,"~CW- ~FTYour e-mail addy can't contain the $vars..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new e-mail addy~CB:~RS ");
				user->set_op = 5;
				return;
			}
		  }
		  strncpy(user->email,inpstr,sizeof(user->email)-1);
		  user->autofwd		= 0;
		  user->verifynum	= -1;
		  writeus(user,"~CW- ~FTE-mail addy now set to~CB: [~RS%s~CB] %s\n",user->email,user->hide_email==1?"[~CRHIDDEN~CB]":"");
		  writeus(user,"~CW- ~CRNOTE~CB:~FT If you had e-mail autoforwarding on, you must re-verify with\n~FT your new e-mail address.\n");
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 6 : /* .set www */
		  if (!strlen(inpstr))
		  {
		  	if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set www ~CB[~CRnew webpage addy~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Sheesh.. why'd you say you wanted to set a www address then?\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW- ~FTYour www addy can't contain the $vars..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new webpage addy~CB:~RS http://");
				user->set_op=6;
				return;
			}
		  }
		  if (istrstr(inpstr,"NULL"))
		  {
			write_user(user,"~CW-~FT You can't nullify your webpage addy..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new webpage addy~CB:~RS http:// ");
				user->set_op = 6;
				return;
			}
		  }
		  if (strncmp(inpstr,"http://",7))
		    sprintf(text,"http://%s",inpstr);
		  else sprintf(text,"%s",inpstr);
		  strncpy(user->webpage,text,sizeof(user->webpage)-1);
		  writeus(user,"~CW-~FG Your webpage URL is now set to~CB: [~RS%s~CB]\n",user->webpage);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 7 : /* .set bday */
		  if (!strlen(inpstr))
		  {
			if (menus==0)
			{	write_user(user,"~CW-~FT Set Usage~CB: ~CR.set bday ~CB[~CRnew birthday~CB] (~CRMM/DD format~CB)\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT If you didn't want to set your birthday, why'd you say you wanted to.\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  inpstr = ColourStrip(inpstr);
		  if (inpstr[2] != '/')
		  {
			write_user(user,"~CW-~FT Your birthday must be set in MM/DD format.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter your birthday~CB:~RS ");
				user->set_op = 7;
				return;
			}
		  }
		  strncpy(temp,inpstr,2);  inpstr++;  inpstr++;  inpstr++;
		  strncpy(temp2,inpstr,sizeof(temp2)-1);
		  bmonth	= atoi(temp);
		  bday		= atoi(temp2);
		  if (bmonth < 1 || bmonth > 12)
		  {
			write_user(user,"~CW-~FT There is no such month.. Birthday format is MM/DD.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter your birthday~CB:~RS ");
				user->set_op = 7;
				return;
			}
		  }
		  if (bday<1)
		  {
			write_user(user,"~CW-~FT That was an invalid date. Birthday format must be in MM/DD.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter your birthday~CB:~RS ");
				user->set_op = 7;
				return;
			}
		  }
		  if ((bmonth == 2 && bday>28) || ((bmonth == 4 || bmonth == 6
		    || bmonth == 9 || bmonth == 11) && bday>30) || ((bmonth == 1
		    || bmonth == 3 || bmonth == 5 || bmonth == 7 || bmonth == 8
		    || bmonth == 10 || bmonth == 12) && bday>31))
		  {
			write_user(user,"~CW-~FT That was an invalid date. Birthday format must be in MM/DD.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter your birthday~CB:~RS ");
				user->set_op = 7;
				return;
			}
		  }
		  sprintf(text,"%2.2d/%2.2d",bmonth,bday);
		  strcpy(user->bday,text);
		  writeus(user,"~CW-~FG Your birthday is now set to~CB: [~CR%s ~CW<~CR%s %d~CW>~CB]\n",user->bday,month[bmonth-1],bday);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 8 : /* .set age */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set age ~CB[~CRhowever old you are~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Sheesh.. why'd you say you wanted to set your age then..\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  val = atoi(word[0]);
		  if (val<0 || val>99)
		  {
			write_user(user,"~CW-~FT That's an invalid age.. Your age can only be set between 1 and 99.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTEnter new age~CB:~RS ");
				user->set_op = 8;
				return;
			}
		  }
		  user->age = val;
		  writeus(user,"~CW-~FG Your age is now set to~CB: [~CR%d~CB]\n",user->age);
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 9 : /* .set icq */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set icq ~CB[~CRwhatever your ICQ UIN is~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Sheesh.. why'd you say you wanted to set your ICQ UIN then..\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  val = atol(word[0]);
		  if (val<1)
		  {
			write_user(user,"~CW-~FT That's an invalid ICQ UIN.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTEnter new ICQ UIN~CB:~RS ");
				user->set_op = 9;
				return;
			}
		  }
		  user->icq = val;
		  writeus(user,"~CW-~FG Your ICQ UIN is now set to~CB: [~CR%ld~CB] %s\n",user->icq,user->icqhide==1?"[~CRHIDDEN~CB]":"");
		  if (webpage_on == 1) do_user_webpage(user);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 10: /* .set inphr */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set inphr ~CB[~CRa phrase for when you enter a room~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Fine.. I didn't want to set your new in phrase anyway.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->in_phr)-1)
		  {
			write_user(user,"~CW-~FT That new in phrase would be too long.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new in phrase~CB:~RS ");
				user->set_op = 10;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW-~FT You can't use the $vars in your in phrase..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new in phrase~CB:~RS ");
				user->set_op = 10;
				return;
			}
		  }
		  strncpy(user->in_phr,inpstr,sizeof(user->in_phr)-1);
		  writeus(user,"~CW-~FG Your in phrase is now set to~CB: [~RS%s~CB]\n",user->in_phr);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 11: /* .set outphr */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set outphr ~CB[~CRa phrase when you leave a room~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Fine.. I didn't want to set your new out phrase anyway.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->out_phr)-1)
		  {
			write_user(user,"~CW-~FT That new out phrase would be too long.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new out phrase~CB:~RS ");
				user->set_op = 11;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW-~FT You can't use the $vars in your out phrase..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new out phrase~CB:~RS ");
				user->set_op = 11;
				return;
			}
		  }
		  strncpy(user->out_phr,inpstr,sizeof(user->out_phr)-1);
		  writeus(user,"~CW-~FG Your out phrase is now set to~CB: [~RS%s~CB]\n",user->out_phr);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 12: /* .set loginmsg */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set loginmsg ~CB[~CRa message when you first logon~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Fine.. I didn't want to set your new login message anyway.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->login_msg)-1)
		  {
			write_user(user,"~CW-~FT That new login message would be too long.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new login message~CB:~RS ");
				user->set_op = 12;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW-~FT You can't use the $vars in your login message..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new login message~CB:~RS ");
				user->set_op = 12;
				return;
			}
		  }
		  strncpy(user->login_msg,inpstr,sizeof(user->login_msg)-1);
		  writeus(user,"~CW-~FG Your login message is now set to~CB: [~RS%s~CB]\n",user->login_msg);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 13: /* .set logoutmsg */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set logoutmsg ~CB[~CRa message when you log off~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Fine.. I didn't want to set your new logout message anyway.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->logout_msg)-1)
		  {
			write_user(user,"~CW-~FT That new logout message would be too long.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new logout message~CB:~RS ");
				user->set_op = 13;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW-~FT You can't use the $vars in your logout message..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new logout message~CB:~RS ");
				user->set_op = 13;
				return;
			}
		  }
		  strncpy(user->logout_msg,inpstr,sizeof(user->logout_msg)-1);
		  writeus(user,"~CW-~FG Your logout message is now set to~CB: [~RS%s~CB]\n",user->logout_msg);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 14: /* .set gender */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set gender ~CB[~CRmale~CW|~CRfemale~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alright.. so you don't want to go through with the gender change..\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strncasecmp(word[0],"male",1))
		  {
			user->gender = 0;
			user->gender = Male;
			write_user(user,"~CW-~FG Alright, your gender is now set to male..\n");
			if (webpage_on == 1) do_user_webpage(user);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else if (!strncasecmp(word[0],"female",1))
		  {
			user->gender = 0;
			user->gender = Female;
			write_user(user,"~CW-~FG Alright your gender is now set to female.\n");
			if (webpage_on == 1) do_user_webpage(user);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW-~FT You can only set your gender to male or female.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter your gender ~CB[~CRmale~CW|~CRfemale~CB]:~RS ");
				user->set_op = 14;
				return;
			}
		  }
		case 15: /* .set colour */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set colour ~CB[~CRon~CW|~CRoff~CW|~CR-test~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well.. don't choose to set colour if you don't really want to..\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-test"))
		  {
			cnt = 0;
			for(i = 1 ; i < NumCols ; ++i)
			{
				writeus(user," ^~%s~CB - [~RS~%sRaMTITS Colour com. test~RS  ",colcom[i],colcom[i]);
				++cnt;
				if (cnt >= 2)
				{
					write_user(user,"\n");
					cnt = 0;
				}
			}
			if (cnt) write_user(user,"\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Your colour choice~CB [~CRon~CW|~CRoff~CW|~CR-test~CB]:~RS ");
				user->set_op = 15;
				i = 0;
				return;
			}
		  }
		  else if (!strcasecmp(word[0],"off"))
		  {
			write_user(user,"~CW- ~FTYou turn ansi colour ~CROFF.\n");
			user->colour = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else if (!strcasecmp(word[0],"on"))
		  {
			write_user(user,"~CW- ~FTYou turn ansi colour ~CRON.\n");
			user->colour = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW-~FT That was an invalid colour choice..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Your colour choice~CB [~CRon~CW|~CRoff~CW|~CR-test~CB]:~RS ");
				user->set_op = 15;
				return;
			}
		  }
		case 16: /* .set ansi */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set ansi ~CB[~CRon~CW|~CRoff~CW|~CR-test~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well why'd you say you wanted to set ansi screens then..\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"on"))
		  {
			write_user(user,"~CW-~FG You turn ansi screens on.\n");
			user->ansi_on = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else if (!strcasecmp(word[0],"off"))
		  {
			write_user(user,"~CW-~FG You turn ansi screens off.\n");
			user->ansi_on = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  else if (!strcasecmp(word[0],"-test"))
		  {
			write_user(user,"~FG If this test works properly, you will be able to see the 'RaMTITS' in big bold\n");
			write_user(user,"~FG block type letters, if not, you'll just get a bunch of gibberish on your\n");
			write_user(user,"~FG screen. Every telnet application should support this however. Make sure that\n");
			write_user(user,"~FG your emulation is set to ~CW\"~CRansi~CW\"~FG or if you use tinyfugue use\n");
			write_user(user,"~FG \"~CR/set emulation=raw~CW\".~FG  One other thing you will most likely have to\n");
			write_user(user,"~FG change, is your font as well, and set that to ~CW\"~CRterminal~CW\"~FG.  Then you *SHOULD*\n");
			write_user(user,"~FG be set!\n");
			write_user(user,"              ~CBÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»\n");
			write_user(user,"              ~CBº~CG  ÛßßÛÜ        ÛÛÜ ÜÛÛ ÛßÛßÛ ÛßÛßÛ ÛßÛßÛ  ÜÜ   ~CBº\n");
			write_user(user,"              ~CBº~CG  Û   Û        Û ÛÜÛ Û Û Û Û Û Û Û Û Û Û Û  ß  ~CBº\n");
			write_user(user,"              ~CBº~FG  ÛßßÛß  ÜßßßÜ Û  Û  Û   Û     Û     Û   ßßÜ   ~CBº\n");
			write_user(user,"              ~CBº~FG  Û  ßÛ  ÛÜÜÜÛ Û     Û   Û   Û Û Û   Û      Û  ~CBº\n");
			write_user(user,"              ~CBº~FT ÛßÛ ÛßÛ Û   Û Û     Û   Û   ÛÜÛÜÛ   Û   ßÜÜß  ~CBº\n");
			write_user(user,"              ~CBÈÍÍÍÍÍÍÍ»                                   ~CBÉÍÍÍ¼\n");
			write_user(user,"              ~CB        º~FM ÜßÜ ÛÜ Û Üß  Û    ÜÛÜ Üßß Üß  ÜÛÜ ~CBº\n");
			write_user(user,"              ~CB        º~FM ÛßÛ Û ßÛ  ßÜ Û     Û  Ûßß  ßÜ  Û  ~CBº\n");
			write_user(user,"              ~CB        º~FM           ß            ßß  ß      ~CBº\n");
			write_user(user,"              ~CB        ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Your ansi choice~CB [~CRon~CW|~CRoff~CW|~CRtest~CB]:~RS ");
				user->set_op = 16;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW-~FT That was an invalid ansi choice..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Your ansi choice~CB [~CRon~CW|~CRoff~CW|~CRtest~CB]:~RS ");
				user->set_op = 16;
				return;
			}
		  }
		case 17: /* .set who */
		  if (!strlen(inpstr))
		  {
		  	if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set who ~CB[~CR#~CW|~CR-list~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your who style.\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-list"))
		  {
			write_user(user,"~CW-~FG The types of who styles are~CB:\n");
			for (i = 1; i<NumWhos; ++i)
			{
				writeus(user,"  ~CR%d ~CW= ~FT%s\n",i,who_style[i]);
			}
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter who style choice~CB:~RS ");
				user->set_op = 17;
				return;
			}
		  }
		  val = atoi(word[0]);
		  if (val<1 || val>(NumWhos-1))
		  {
			writeus(user,"~CW-~FT Valid who styles are 1 - %d. ~CB[~CW'~CR-list~CW'~FT for a list~CB]\n",(NumWhos-1));
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter who style choice~CB:~RS ");
				user->set_op = 17;
				return;
			}
		  }
		  user->who = val;
		  writeus(user,"~CW-~FG Your who style has been set to~CB: [~CR%s~CB]\n",who_style[user->who]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op=39;
			return;
		  }
		case 18: /* .set help */
		  if (!strlen(inpstr))
		  {
		  	if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set help ~CB[~CR#~CW|~CR-list~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your help style.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-list"))
		  {
			write_user(user,"~CW-~FG The types of help styles are~CB:\n");
			for (i = 1 ; i < NumHelp ; ++i)
			{
				writeus(user,"  ~CR%d ~CW= ~FT%s\n",i,help_style[i]);
			}
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter help style choice~CB:~RS ");
				user->set_op = 18;
				return;
			}
		  }
		  val = atoi(word[0]);
		  if (val<1 || val>(NumHelp-1))
		  {
			writeus(user,"~CW-~FT Valid help styles are 1 - %d. ~CB[~CW'~CR-list~CW'~FT for a list~CB]\n",(NumHelp-1));
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter help style choice~CB:~RS ");
				user->set_op = 18;
				return;
			}
		  }
		  user->help = val;
		  writeus(user,"~CW-~FG Your help style has been set to~CB: [~CR%s~CB]\n",help_style[user->help]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op=39;
			return;
		  }
		case 19: /* .set hemail */
		  if (user->hide_email == 0)
		  {
			write_user(user,"~CW- ~FTYour e-mail addy will now be hidden..\n");
			user->hide_email = 1;
			if (webpage_on == 1) do_user_webpage(user);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYour e-mail addy will no longer be hidden..\n");
			user->hide_email = 0;
			if (webpage_on == 1) do_user_webpage(user);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 20: /* .set columns */
		  if (!strlen(inpstr))
		  {
		  	if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set columns ~CB[~CR#~CW|~CRoff~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your columns.\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  if (!strcmp(word[0],"off"))
		  {
			write_user(user,"~CW-~FT You turn off talker controlled word wrapping.\n");
			user->cols = -1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  val = atoi(word[0]);
		  if (val>150 || val<50)
		  {
			write_user(user,"~CW- ~FTYou can only set your columns between 50 and 150.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount of columns~CB [~CR50 ~CW-~CR 150~CW/~CRoff~CB]:~RS ");
				user->set_op = 20;
				val = 0;
				return;
			}
		  }
		  user->cols = val;
		  writeus(user,"~CW-~FT You set your columns to wrap after ~CR%d ~FTcharacters.\n",user->cols);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 21: /* .set charecho */
		  if (user->char_echo == 0)
		  {
			write_user(user,"~CW- ~FTYou turn echoing for character mode clients ~CRON.\n");
			user->char_echo = 1;
			if (menus == 0)  return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYou turn echoing for character mode clients ~CROFF.\n");
			user->char_echo = 0;
			if (menus == 0)  return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 22: /* .set prompt */
		  if (user->prompt == 0)
		  {
			write_user(user,"~CW- ~FTYour prompt has now been turned ~CRON.\n");
			user->prompt = 1;
			if (menus == 0)  return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYour prompt has now been turned ~CROFF.\n");
			user->prompt = 0;
			if (menus == 0)  return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 23: /* .set mode */
		  if (!strlen(inpstr))
		  {
		  	if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set mode ~CB[~CRnuts~CW|~CRirc~CW|~CRewtoo~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your mode.\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"ewtoo"))
		  {
			write_user(user,"~CW-~FT You are now in EW-Too mode.\n");
			user->mode = EWTOO;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else if (!strcasecmp(word[0],"irc"))
		  {
			write_user(user,"~CW-~FT You are now in IRC mode.\n");
			user->mode = IRC;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else if (!strcasecmp(word[0],"nuts"))
		  {
			write_user(user,"~CW-~FT You are now in NUTS mode.\n");
			user->mode = NUTS;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW-~FT That was an invalid mode choice.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter mode of choice~CB [~CRnuts~CW|~CRewtoo~CW|~CRirc~CB]:~RS ");
				user->set_op=23;
				return;
			}
		  }
		case 24: /* .set editor */
		  if (!strlen(inpstr))
		  {
		  	if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set editor ~CB[~CRnuts~CW|~CRramtits~CB]\n");
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set editor -visual\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your editor.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strncasecmp(word[0],"-visual",2))
		  {
			if (user->editor.visual == 1)
			{
				write_user(user,"~CW-~FT Alright, you turn off the visualization for the editor.\n");
				user->editor.visual = 0;
				if (menus == 0) return;
				else
				{
					write_user(user,continue1);
					user->set_op = 39;
					return;
				}
			}
			else
			{
				write_user(user,"~CW-~FT Alright, you turn on the visualization for the editor.\n");
				user->editor.visual = 1;
				if (menus == 0) return;
				else
				{
					write_user(user,continue1);
					user->set_op = 39;
					return;
				}
			}
		  }
		  if (!strncasecmp(word[0],"nuts",2))
		  {
			write_user(user,"~CW-~FT Alright, you're now using the NUTS editor.\n");
			user->editor.editor = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else if (!strncasecmp(word[0],"ramtits",2))
		  {
			write_user(user,"~CW-~FTAlright, you're now using the RaMTITS editor.\n");
			user->editor.editor = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW-~FT That was an invalid editor choice.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter choice~CB [~CRnuts~CW|~CRramtits~CW|~CR-visual~CB]:~RS ");
				user->set_op = 23;
				return;
			}
		  }
		case 25: /* .set rmhide */
		  if (user->has_room == 0 && user->level < WIZ
			&& free_rooms == 0)
		  {
			write_user(user,"~CW- ~FTYou don't even have a room to hide..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  home_room(user);
		  rm = get_room(user->name);
		  if (rm == NULL)
		  {
			write_user(user,"~CW-~FT Sorry, an error occurred while trying to load your room.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (user->hide_room == 0)
		  {
			write_user(user,"~CW- ~FTYour room will now be hidden..\n");
			user->hide_room = 1;
			rm->hidden = 1;
			SaveRoom(rm);
			destruct_room(rm);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYour room will no longer be hidden..\n");
			user->hide_room = 0;
			rm->hidden = 0;
			SaveRoom(rm);
			destruct_room(rm);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 26: /* .set sstyle */
		  if (!strlen(inpstr))
		  {
		  	if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set sstyle ~CB[~CRnew saystyle~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your say style.\n");
				write_user(user,continue1);
				user->set_op=39;
				return;
			}
		  }
		  strcpy(temp,ColourStrip(inpstr));
		  if (strlen(temp)>MaxStyleLen)
		  {
			write_user(user,"~CW-~FT That say style would be a tad bit too long.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new say style~CB:~RS ");
				user->set_op = 26;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->sstyle)-1)
		  {
			write_user(user,"~CW-~FT That say style would be a tad bit too long.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new say style~CB:~RS ");
				user->set_op = 26;
				return;
			}
		  }
		  if (istrstr(temp,"NULL") || strstr(inpstr,"~FK"))
		  {
			write_user(user,"~CW-~FT That say style is invalid.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter new say style~CB:~RS ");
				user->set_op = 26;
				return;
			}
		  }
		  strncpy(user->sstyle,inpstr,sizeof(user->sstyle)-1);
		  writeus(user,"~CW-~FT Your say style will now be shown as~CB: [~RS%s%s~CB]\n",user->recap,user->sstyle);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 27: /* .set wspace */
		  if (user->whitespace == 0)
		  {
			write_user(user,"~CW- ~FTYou turn on white space removal (remove leading spaces from input).\n");
			user->whitespace = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYou turn off white space removal (remove leading spaces from input).\n");
			user->whitespace = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 28: /* .set tzone */
		  if (!strlen(inpstr))
		  {
		  	if (menus==0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set tzone ~CB[~CR+~CW|~CR- hours from system time~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your time zone.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if ((inpstr[0] != '+') && (inpstr[0] != '-'))
		  {
			write_user(user,"~CW-~FT Format for timezone must be +/- hours from the system time.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour time zone~CB:~RS ");
				user->set_op = 28;
				return;
			}
		  }
		  if (inpstr[0] == '+') val = 1;  else val = 2;
		  inpstr++;
		  tchange = atoi(inpstr);
		  if (tchange<1 || tchange>24)
		  {
			write_user(user,"~CW- ~FTThat change can only be between 1 and 24 hours.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour time zone~CB:~RS ");
				user->set_op = 28;
				return;
			}
		  }
		  sprintf(text,"%s%d",val == 1?"+":"-",tchange);
		  strcpy(user->tzone,text);
		  writeus(user,"~CW-~FT Your timezone is %s %d hours from the system time.\n",val==1?"+":"-",tchange);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 29: /* .set quit */
		  if (user->quit==0)
		  {
			write_user(user,"~CW- ~FTYou will now have to type .quit to log off the talker.\n");
			user->quit = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYou will no longer have to type .quit to log off.\n");
			user->quit = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 30: /* .set profile */
		  if (user->profile == 0)
		  {
			write_user(user,"~CW- ~FTYou will now be prompted when examining someone.\n");
			user->profile = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYou will no longer be prompted when examining someone.\n");
			user->profile = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 31: /* .set autoexec */
		  if (!strlen(inpstr))
		  {
		  	if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set autoexec ~CB[~CRcommand to execute when you logon~CW|~CR-none~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your auto exec command.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strncasecmp(word[0],"-none",2))
		  {
			write_user(user,"~CW- ~FTAlright, your autoexec command has been removed.\n");
			user->autoexec[0] = '\0';
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->autoexec)-1)
		  {
			write_user(user,"~CW- ~FTThat auto-exec command would be too long..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Your new auto-exec command~CB:~RS ");
				user->set_op = 31;
				return;
			}
		  }
		  strncpy(user->autoexec,inpstr,sizeof(user->autoexec)-1);
		  writeus(user,"~CW- ~FTWhen you first log in the talker you will execute~CB: [~CR%s~CB]\n",user->autoexec);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 32: /* .set menus */
		  if (user->menus==0)
		  {
			write_user(user,"~CW- ~FTYou will now use the RaMTITS menu system.\n");
			user->menus = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYou will no longer use the RaMTITS menu system.\n");
			user->menus = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 33: /* .set carriage */
		  if (user->car_return == 0)
		  {
			write_user(user,"~CW- ~FTYou will now support carriage return (\\r)\n");
			user->car_return = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYou turn carriage return (\\r) off.\n");
			user->car_return = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 34: /* .set bfafk */
		  if (!strlen(inpstr))
		  {
		  	if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set bfafk ~CB[~CRa bfafk message~CW|~CR-none~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your bfafk message.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strncasecmp(word[0],"-none",2))
		  {
			write_user(user,"~CW- ~FTYour back from afk message has been removed.\n");
			user->bfafk[0] = '\0';
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (IsUserVar(inpstr) && user->level<OWNER)
		  {
			write_user(user,"~CW- ~FTYour back from afk message can't contain the $vars..\n");
			if (menus == 0) return;
			else
			{
			 	write_user(user,"~CW- ~FTYour new back from afk message~CB:~RS ");
				user->set_op = 34;
				return;
			}
		  }
		  if (strlen(inpstr)>sizeof(user->bfafk)-1)
		  {
			write_user(user,"~CW-~FT That back from afk message would be too long..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTYour new back from afk message~CB:~RS ");
				user->set_op = 34;
				return;
			}
		  }
		  strncpy(user->bfafk,inpstr,sizeof(user->bfafk)-1);
		  writeus(user,"~CW-~FG When you come back from afk people will see~CB:~RS %s %s\n",user->invis>Invis?user->temp_recap:user->recap,user->bfafk);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 35: /* .set icqhide */
		  if (user->icqhide == 0)
		  {
			write_user(user,"~CW- ~FTYour ICQ UIN will now be hidden.\n");
			user->icqhide = 1;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  else
		  {
			write_user(user,"~CW- ~FTYour ICQ UIN will no longer be hidden.\n");
			user->icqhide = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		case 36: /* .set rows */
		  if (!strlen(inpstr))
		  {
		  	if (menus == 0)
			{
				write_user(user,"~CW-~FT Set Usage~CB: ~CR.set rows ~CB[~CR# of rows~CW|~CR-off~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Alrighty, you don't want to set your rows.\n");
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  if (!strncasecmp(word[0],"-off",2))
		  {
			write_user(user,"~CW- ~FTYou turn off your prompting of files.\n");
			user->rows = 0;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->set_op = 39;
				return;
			}
		  }
		  val = atoi(word[0]);
		  if (val<15 || val>150)
		  {
			write_user(user,"~CW- ~FTYou can only set your rows between 15 and 150..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTNew amount of rows~CB:~RS ");
				user->set_op = 36;
				return;
			}
		  }
		  user->rows = val;
		  writeus(user,"~CW-~FG When reading files, your screen will now pause after %d rows.\n",user->rows);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->set_op = 39;
			return;
		  }
		case 38: /* The users actual choice if in menu */
		  switch (toupper(inpstr[0]))
		  {
			case '1':
			  switch (toupper(inpstr[1]))
			  {
				case '0':
				  writeus(user,"~CW- ~FG Your in phrase is currently~CB: [~RS%s~CB]\n",user->in_phr);
				  write_user(user,"~CW- ~FTEnter new in phrase~CB:~RS ");
				  user->set_op = 10;  return;
				case '1':
				  writeus(user,"~CW-~FG Your out phrase is currently~CB: [~RS%s~CB]\n",user->out_phr);
				  write_user(user,"~CW- ~FTEnter new out phrase~CB:~RS ");
				  user->set_op = 11;  return;
				case '2':
				  writeus(user,"~CW- ~FG Your login message is currently~CB: [~RS%s~CB]\n",user->login_msg);
				  write_user(user,"~CW- ~FTEnter new login message~CB:~RS ");
				  user->set_op = 12;  return;
				case '3':
				  writeus(user,"~CW- ~FGYour logout message is currently~CB: [~RS%s~CB]\n",user->logout_msg);
				  write_user(user,"~CW- ~FTEnter new logout message~CB:~RS ");
				  user->set_op = 13;  return;
				case '4':
				  writeus(user,"~CW- ~FGYou are currently a~CB: [~CR%s~CB]\n",gendert[user->gender]);
				  write_user(user,"~CW-~FT Enter new gender~CB [~CRmale~CW/~CRfemale~CB]:~RS ");
				  user->set_op = 14;  return;
				case '5':
				  writeus(user,"~CW- ~FGYou currently have colour~CB: [~CR%s~CB]\n",offon[user->colour]);
				  write_user(user,"~CW-~FT Colour choices~CB [~CRon~CW/~CRoff~CW/~CRtest~CB]:~RS ");
				  user->set_op = 15;  return;
				case '6':
				  writeus(user,"~CW- ~FGYou currently have ansi screens~CB: [~CR%s~CB]\n",offon[user->ansi_on]);
				  write_user(user,"~CW-~FT Ansi choices~CB [~CRon~CW/~CRoff~CW/~CRtest~CB]:~RS ");
				  user->set_op = 16;  return;
				case '7':
				  writeus(user,"~CW-~FGYour who style is set to~CB: [~CR%s~CB]\n",user->who?who_style[user->who]:"DragonS CovE");
				  write_user(user,"~CW- ~FTOther possible who styles~CB:\n");
				  for (i = 1 ; i<NumWhos ; ++i)
				  {
					writeus(user,"         ~FT%d ~CR=~FG %s\n",i,who_style[i]);
				  }
				  write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				  user->set_op = 17;  i = 0;  return;
				case '8':
				  writeus(user,"~CW-~FGYour help style is set to~CB: [~CR%s~CB]\n",user->help?help_style[user->help]:"Normal");
				  write_user(user,"~CW- ~FTOther possible help styles~CB:\n");
				  for (i = 1 ; i<NumHelp ; ++i)
				  {
					writeus(user,"         ~FT%d ~CR=~FG %s\n",i,help_style[i]);
				  }
				  write_user(user,"~CW- ~FTYour choice~CB:~RS ");
				  user->set_op = 18;  i = 0;  return;
				case '9':
				  write_user(user,"~CW-~FT Toggling hidden e-mail... hit enter..\n");
				  user->set_op = 19;  return;
				default :
				  writeus(user,"~CW-~FG Your desc is currently~CB: [~RS%s~CB]\n",user->desc);
				  write_user(user,"~CW-~FT Enter new desc~CB:~RS ");
				  user->set_op = 1;  return;
			  }
			case '2':
			  switch (toupper(inpstr[1]))
			  {
				case '0':
				  if (user->cols == -1) write_user(user,"~CW-~FG You currently have word wrapping disabled.\n");
				  else writeus(user,"~CW-~FG You currently have word wrapping set to ~CR%d~FG columns.\n",user->cols);
				  write_user(user,"~CW-~FT Enter new amount of columns~CB [~CR50 ~CW-~CR 150~CW/~CRoff~CB]:~RS ");
				  user->set_op = 20;  return;
				case '1':
				  write_user(user,"~CW-~FT Toggling character mode echoing.. hit enter.\n");
				  user->set_op = 21;  return;
				case '2':
				  write_user(user,"~CW-~FT Toggling prompting.. hit enter.\n");
				  user->set_op = 22;  return;
				case '3':
				  if (user->mode == EWTOO) write_user(user,"~CW-~FG You are currently in ~CRCOMMAND ~FGmode.\n");
				  else if (user->mode == IRC) write_user(user,"~CW-~FG You are currently in ~CRIRC ~FGmode.\n");
				  else write_user(user,"~CW-~FG You are currently in ~CRNUTS~FG mode.\n");
				  write_user(user,"~CW-~FT Enter mode of choice~CB [~CRnuts~CW/~CRewtoo~CW/~CRirc~CB]:~RS ");
				  user->set_op = 23;  return;
				case '4':
				  writeus(user,"~CW- ~FGYour editor of preference is the ~CR%s~FG editor. Visualization is ~CR%s\n",user->editor.editor==1?"NUTS":"RaMTITS",offon[user->editor.visual]);
				  write_user(user,"~CW-~FT Enter editor choice~CB [~CRnuts~CW|~CRramtits~CW|~CR-visual~CB]:~RS ");
				  user->set_op = 24;  return;
				case '5':
				  write_user(user,"~CW-~FT Toggling personal room hiding.. hit enter.\n");
				  user->set_op = 25;  return;
				case '6':
				  if (user->sstyle[0] != '\0') writeus(user,"~CW-~FG Your say style is currently seen as~CB: [~RS%s%s~CB]\n",user->invis>Invis?user->temp_recap:user->recap,user->sstyle);
				  write_user(user,"~CW-~FT Enter new say style~CB:~RS ");
				  user->set_op = 26;  return;
				case '7':
				  write_user(user,"~CW-~FT Toggling white space removal.. hit enter.\n");
				  user->set_op = 27;  return;
				case '8':
				  if (user->tzone[0] != '\0') writeus(user,"~CW-~FG Your time is ~CR%s~FG hours from the system time.\n",user->tzone);
				  write_user(user,"~CW-~FT Enter new time difference~CB [~CR+~CW/~CR- hours~CB]:~RS ");
				  user->set_op = 28;  return;
				case '9':
				  write_user(user,"~CW-~FT Toggling '.qu' to quit.. hit enter.\n");
				  user->set_op = 29;  return;
				default :
				  writeus(user,"~CW-~FG Your recapped name is currently~CB: [~RS%s~CB]\n",user->invis>Invis?user->temp_recap:user->recap);
				  write_user(user,"~CW-~FT Enter new re-cap~CB:~RS ");
				  user->set_op = 2;  return;
			  }
			case '3':
			  switch (toupper(inpstr[1]))
			  {
				case '0':
				  write_user(user,"~CW-~FT Toggling profile prompting.. hit enter.\n");
				  user->set_op = 30;  return;
				case '1':
				  if (user->autoexec[0] != '\0') writeus(user,"~CW-~FG Your current autoexec command is~CB: [~RS%s~CB]\n",user->autoexec);
				  write_user(user,"~CW-~FT Enter new auto-exec command~CB:~RS ");
				  user->set_op = 31;  return;
				case '2':
				  write_user(user,"~CW-~FT Toggling using the menu system.. hit enter.\n");
				  user->set_op = 32;  return;
				case '3':
				  write_user(user,"~CW-~FT Toggling carriage returns.. hit enter.\n");
				  user->set_op = 33;  return;
				case '4':
				  if (user->bfafk[0] != '\0') writeus(user,"~CW- ~FGYour back from afk message is currently~CB: [~RS%s~CB]\n",user->bfafk);
				  write_user(user,"~CW-~FT Enter new back from afk message~CB:~RS ");
				  user->set_op = 34;  return;
				case '5':
				  write_user(user,"~CW-~FT Toggling hiding your ICQ UIN.. hit enter.\n");
				  user->set_op = 35;  return;
				case '6':
				  if (user->rows == 0) write_user(user,"~CW-~FG You currently have paging files turned off.\n");
				  else writeus(user,"~CW- ~FGYour pager is set to pause after ~CR%d~FG rows.\n",user->rows);
				  write_user(user,"~CW-~FT Enter new amount of rows~CB [~CR15~CW-~CR150~CW/~CR-off~CB]:~RS ");
				  user->set_op = 36;  return;
				default :
				  if (user->predesc[0] != '\0') writeus(user,"~CW- ~FGYour pre-desc is currently~CB: [~RS%s~CB]\n",user->predesc);
				  write_user(user,"~CW-~FT Enter new pre-desc~CB:~RS ");
				  user->set_op = 3;  return;
			  }
			case '4':
			  if (user->lev_rank[0] != '\0') writeus(user,"~CW-~FG Your level alias is currently~CB: [~RS%s~CB]\n",user->lev_rank);
			  write_user(user,"~CW-~FT Enter new level alias~CB:~RS ");
			  user->set_op = 4;  return;
			case '5':
			  if (user->email[0] != '\0') writeus(user,"~CW-~FG Your e-mail addy is currently~CB: [~RS%s~CB]\n",user->email);
			  write_user(user,"~CW-~FT Enter new e-mail addy~CB:~RS ");
			  user->set_op = 5;  return;
			case '6':
			  if (user->webpage[0] != '\0') writeus(user,"~CW-~FG Your webpage URL is currently~CB: [~RS%s~CB]\n",user->webpage);
			  write_user(user,"~CW- ~FTEnter new webpage URL~CB:~RS http://");
			  user->set_op = 6;  return;
			case '7':
			  if (user->bday[0] != '\0') writeus(user,"~CW-~FG Your birthday is currently~CB: [~RS%s~CB]\n",user->bday);
			  write_user(user,"~CW-~FT Enter new birthday~CB:~RS ");
			  user->set_op = 7;  return;
			case '8':
			  if (user->age) writeus(user,"~CW-~FG Your age is currently set at~CB: [~CR%d~CB]\n",user->age);
			  write_user(user,"~CW-~FT Enter your age~CB:~RS ");
			  user->set_op = 8;  return;
			case '9':
			  if (user->icq) writeus(user,"~CW-~FG Your ICQ UIN is currently set at~CB: [~CR%ld~CB]\n",user->icq);
			  write_user(user,"~CW-~FT Enter your ICQ UIN~CB:~RS ");
			  user->set_op = 9;  return;
			case '?':
			  PrintMenu(user,2);  user->set_op = 38;  return;
			case 'S':
			  write_user(user,center("~FT-~CT=~CB> ~FGYour setting attributes are set at: ~CB<~CT=~FT-\n",80));
			  ShowSettings(user);	write_user(user,continue1);
			  user->set_op = 39;	return;
			case 'Q':
			  writeus(user,"~CW- ~CTThanks %s ~CTfor using the ~FG%s ~CTsetup menu...\n",user->invis>Invis?user->temp_recap:user->recap,TalkerRecap);
			  WriteRoomExcept(user->room,user,"~CW- ~FT%s ~FGleaves the ~FY%s~FG setup menu.\n",name,TalkerRecap);
			  user->set_op = 0;	rev_away(user);
			  if (webpage_on == 1) do_user_webpage(user);
			  prompt(user);		return;
			default :
			  write_user(user,"~CW-~FG Invalid option...~CB(~CT? for menu~CB)\n");
			  write_user(user,menuprompt);  user->set_op = 38;
			  return;
		  }
		case 39: /* Just a 'hit enter' prompt. */
		  if (toupper(inpstr[0]) == 'Q')
		  {
			writeus(user,"~CW-~CT Thanks %s~CT for using the ~FG%s~CT setup menu..\n",user->invis>Invis?user->temp_recap:user->recap,TalkerRecap);
			WriteRoomExcept(user->room,user,"~CW- ~FT%s~FG leaves the ~FY%s~FG setup menu.\n",name,TalkerRecap);
			user->set_op = 0;	rev_away(user);
			if (webpage_on == 1) do_user_webpage(user);
			prompt(user);		return;
		  }
		  temp[0] = temp2[0] = '\0';
		  tchange = bmonth = bday = cnt = val = i = 0;
		  PrintMenu(user,2);	user->set_op = 38;
		  return;
	}
	return;
}

/* This functions shows the users settings. */
void ShowSettings(User user)
{
	int cnt = 0;
	char modestr[21],rowstr[8],lev_name[16],colstr[6];

	modestr[0] = rowstr[0] = lev_name[0] = colstr[0] = '\0';
	if (user == NULL) return; /* more of a safety check. */
	cnt = colour_com_count(user->predesc);
	if (user->cols == -1) strcpy(colstr,"OFF");
	else sprintf(colstr,"%d",user->cols);
	if (user->rows == 0) strcpy(rowstr,"OFF");
	else sprintf(rowstr,"%d",user->rows);
	strcpy(lev_name,user->gender == Female?level[user->level].fname:level[user->level].mname);
	if (user->mode == EWTOO) strcpy(modestr,"EW-Too");
	else if (user->mode == IRC) strcpy(modestr,"IRC");
	else strcpy(modestr,"NUTS");

	writeus(user,"~FMPre-description ~CB= [~RS%-*.*s~CB]     ~FT-~FG=~CY] [~FG=~FT-  ~FMRecapped name  ~CB= [~RS%s~CB]\n",8+cnt*3,8+cnt*3,user->predesc,user->invis>Invis?user->temp_recap:user->recap);
	writeus(user,"~FMGender          ~CB= [~RS%-8.8s~CB]     ~FT-~FG=~CY] [~FG=~FT-  ~FMLevel Rank     ~CB= [~RS%s~CB]\n",gendert[user->gender],user->lev_rank[0]?user->lev_rank:lev_name);
	writeus(user,"~FMAge             ~CB= [~RS%-2.2d~CB]           ~FT-~FG=~CY] [~FG=~FT-  ~FMICQ Number     ~CB= [~RS%ld~CB]%s\n",user->age,user->icq,user->icqhide==1?" [~CRH~CB]":"");
	writeus(user,"~FMColour          ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMAnsi Screens   ~CB= [~RS%s~CB]\n",offon[user->colour],offon[user->ansi_on]);
	writeus(user,"~FMWho Style       ~CB= [~RS%-12.12s~CB] ~FT-~FG=~CY] [~FG=~FT-  ~FMHelp Style     ~CB= [~RS%s~CB]\n",user->who?who_style[user->who]:"DragonS CovE",user->help?help_style[user->help]:"Normal");
	writeus(user,"~FMColumn wrapping ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMRows Prompt    ~CB= [~RS%s~CB]\n",colstr,rowstr);
	writeus(user,"~FMChar. echoing   ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMPrompt         ~CB= [~RS%s~CB]\n",offon[user->char_echo],offon[user->prompt]);
	writeus(user,"~FMMode            ~CB= [~RS%-8.8s~CB]     ~FT-~FG=~CY] [~FG=~FT-  ~FMEditor         ~CB= [~RS%s~CB]%s\n",modestr,user->editor.editor==1?"NUTS":"RaMTITS",user->editor.visual==1?"[~CRV~CB]":"");
	writeus(user,"~FMRoom Hidden?    ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMSay Style      ~CB= [~RS%s~CB]\n",noyes1[user->hide_room],user->sstyle);
	writeus(user,"~FMWhite Space     ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMTime Zone      ~CB= [~RS%s~CB]\n",noyes1[user->whitespace],user->tzone[0]?user->tzone:"Unset");
	writeus(user,"~FM.qu to quit?    ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMProfile Prompt ~CB= [~RS%s~CB]\n",noyes1[user->quit],offon[user->profile]);
	writeus(user,"~FMUsing Menus?    ~CB= [~RS%-3.3s~CB]          ~FT-~FG=~CY] [~FG=~FT-  ~FMCar. Return    ~CB= [~RS%s~CB]\n",noyes1[user->menus],offon[user->car_return]);
	writeus(user,"~FME-mail addy     ~CB= [~RS%s~CB]%s\n",user->email,user->hide_email==1?" [~CRH~CB]":"");
	writeus(user,"~FMWebpage URL     ~CB= [~RS%s~CB]\n",user->webpage[0]?user->webpage:"Unset");
	writeus(user,"~FMAuto exec. com. ~CB= [~RS%s~CB]\n",user->autoexec[0]?user->autoexec:"Unset");
	writeus(user,"~FMDescription     ~CB= [~RS%s~CB]\n",user->desc);
	writeus(user,"~FMIn phrase       ~CB= [~RS%s~CB]\n",user->in_phr);
	writeus(user,"~FMOut phrase      ~CB= [~RS%s~CB]\n",user->out_phr);
	writeus(user,"~FMLogin message   ~CB= [~RS%s~CB]\n",user->login_msg);
	writeus(user,"~FMLogout message  ~CB= [~RS%s~CB]\n",user->logout_msg);
	writeus(user,"~FMBFAFK message   ~CB= [~RS%s~CB]\n",user->bfafk[0]?user->bfafk:"Unset");
	writeus(user,"~FMBirthday        ~CB= [~RS%s~CB]\n",user->bday);
	return;
}

/* This function is like the above but shows the users inventory. -
 - (stuff that they buy from the store).			  */
void ShowInventory(User user)
{
	int cnt = 0;

	if (user == NULL) return;
	if (user->hat)
	{
		writeus(user," ~CW- ~FGA coloured hat~CB: [%sthis colour~CB]\n",hatcolor[user->hat]);
		++cnt;
	}
	if (user->condoms)
	{
		writeus(user," ~CW- ~FGSome condoms for .netsexing~CB: [~FTA total of ~CY%d~CB]\n",user->condoms);
		++cnt;
	}
	if (user->has_room)
	{
		writeus(user," ~CW- ~FGYour own personal room.\n");
		++cnt;
	}
	if (user->atmos)
	{
		writeus(user," ~CW- ~FGSome atmospherics for your room~CB: [~FTA total of ~CY%d~CB]\n",user->atmos);
		++cnt;
	}
	if (user->chips)
	{
		writeus(user," ~CW- ~FGSome chips for gambling~CB: [~FTA total of ~CY%ld~CB]\n",user->chips);
		++cnt;
	}
	if (user->protection)
	{
		writeus(user," ~CW- ~FGA body guard to protect you from being mugged.\n");
		++cnt;
	}
	if (user->carriage)
	{
		writeus(user," ~CW- ~FGA carriage for easier travel between rooms.\n");
		++cnt;
	}
	if (user->nukes)
	{
		writeus(user," ~CW- ~FGSome nuclear bombs~CB: [~FTA total of ~CY%ld~CB]\n",user->nukes);
		++cnt;
	}
	if (user->pet.type)
	{
		writeus(user," ~CW- ~FGYour very own pet ~CR%s ~FGnamed~CB: [~FT%s~CB]\n",pet[user->pet.type],user->pet.name[0]?user->pet.name:pet[user->pet.type]);
		++cnt;
	}
	if (user->keys)
	{
		writeus(user," ~CW- ~FGKeys for your room~CB: [~FTA total of ~CY%d~CB]\n",user->keys);
		++cnt;
	}
	if (user->beers)
	{
		writeus(user," ~CW- ~FGSome good ole Canadian beer~CB: [~CR%d beer%s~CB]\n",user->beers,user->beers>1?"s":"");
		++cnt;
	}
	if (user->bullets)
	{
		writeus(user," ~CW- ~FGSome bullets for russian roullette~CB: [~FTTotal of ~CY%d~CB]\n",user->bullets);
		++cnt;
	}
	if (cnt == 0) write_user(user," ~CW- ~FGYou don't have anything in your inventory.\n");
	return;
}

/* We need to know which menu to print too. */
void PrintMenu(User user,int which_menu)
{

	if (user == NULL) return;
	if (which_menu != 10) cls(user,NULL,0);
	switch (which_menu)
	{
		case 1 : /* Bank menu */
		  sprintf(file,"bankmenu");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the bank menu.. sorry.\n");
			return;
		  }
		  write_user(user,center("~FT-~CT=~CB> ~FGYour Current Bank Balance~CB: ~CB<~CT=~FT-\n",80));
		  sprintf(text,"~CB| ~FGCash on Hand~CB:  ~FR%7.7ld ~CB| ~FGCash in Bank~CB:  ~FR%7.7ld ~CB|\n",user->money,user->dep_money);
		  write_user(user,center(text,80));
		  sprintf(text,"~CB| ~FGChips on Hand~CB: ~FR%7.7ld ~CB| ~FGChips in Bank~CB: ~FR%7.7ld ~CB|\n",user->chips,user->dep_chips);
		  write_user(user,center(text,80));
		  write_user(user,menuprompt);
		  show_prompt(user);
		  return;
		case 2 : /* Set menu */
		  sprintf(file,"setmenu");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the settings menu.. sorry.\n");
			return;
		  }
		  write_user(user,menuprompt);
		  show_prompt(user);
		  return;
		case 3 : /* Store menu (buy) */
		  sprintf(file,"storebuy1");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the store menu for buying.. sorry.\n");
			return;
		  }
		  write_user(user,menuprompt);
		  show_prompt(user);
		  return;
		case 4 : /* Store menu (sell) */
		  sprintf(file,"storesell1");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the bank menu.. sorry.\n");
			return;
		  }
		  write_user(user,menuprompt);
		  show_prompt(user);
		  return;
		case 5 : /* Bank options */
		  sprintf(file,"bankopts");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the bank options.. sorry.\n");
			return;
		  }
		  write_user(user,center("~FT-~CT=~CB> ~FGYour Current Bank Balance~CB: ~CB<~CT=~FT-\n",80));
		  sprintf(text,"~CB| ~FGCash on Hand~CB:  ~FR%7ld ~CB| ~FGCash in Bank~CB:  ~FR%7ld ~CB|\n",user->money,user->dep_money);
		  write_user(user,center(text,80));
		  sprintf(text,"~CB| ~FGChips on Hand~CB: ~FR%7ld ~CB| ~FGChips in Bank~CB: ~FR%7ld ~CB|\n",user->chips,user->dep_chips);
		  write_user(user,center(text,80));
		  show_prompt(user);  return;
		case 6 : /* Set options */
		  sprintf(file,"setopts");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the setting options.. sorry.\n");
			return;
		  }
		  show_prompt(user);  return;
		case 7 : /* Store options (buy) */
		  sprintf(file,"storebuy2");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the store options for buying.. sorry.\n");
			return;
		  }
		  show_prompt(user);  return;
		case 8 : /* Store options (sell) */
		  sprintf(file,"storesell2");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the store options for selling.. sorry.\n");
			return;
		  }
		  show_prompt(user);  return;
		case 9 : /* Mail options */
		  sprintf(file,"mailmenu");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG I was unable to find the mail menu.. sorry.\n");
			return;
		  }
		  show_prompt(user);  return;
		case 10:
		  sprintf(file,"loginmenu");
		  if (!(show_screen(user)))
		  {
			write_user(user,"~FG ACK, there isn't a login options menu.\n");
			return;
		  }
		  show_prompt(user);  return;
	}
	return;
}

void mail_menu(User user,char *inpstr,int rt)
{
	char *name;

	if (user == NULL) return;
	if (user->invis == Invis) name = invisname;  else name = user->recap;
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
	PrintMenu(user,9);
	write_user(user,menuprompt);
	user->mail_op		= 1;
	user->ignall_store	= user->ignall;
	user->ignall		= 1;
	no_prompt		= 1;
	WriteRoomExcept(user->room,user,"~CW- ~FT%s~FY enters the %s ~FYmail management menu.\n",name,TalkerRecap);
	return;
}

void MailOps(User user,char *inpstr)
{
	User u = NULL;
	FILE *infp,*outfp;
	char d,filename[FSL],*name,*uname;
	int num,num2,num3,num4,num5,on,fsize,err=0;
	struct stat fb;

	if (user == NULL) return;
	num = num2 = num3 = num4 = num5 = on = fsize = 0;
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	switch(user->mail_op)
	{
		case 1 :
		  switch(toupper(inpstr[0]))
		  {
			case '1': /* Read s-mail */
			  write_user(user,"~CW-~FT To read ALL your s-mail, just hit enter, otherwise specify the number of\n~CW- ~FTthe message you would like to read, or hit 'new' for your new s-mail.\n");
			  write_user(user,"~CW-~FT Your choice~CB:~RS ");
			  user->mail_op = 2;  return;
			case '2': /* Read sent s-mail */
			  write_user(user,"~CW-~FT To read ALL your sent s-mail, just hit enter, otherwise specify the number of\n~CW- ~FTthe message you would like to read.\n");
			  write_user(user,"~CW-~FT Your choice~CB:~RS ");
			  user->mail_op = 3;  return;
			case '3': /* View from for s-mail */
			  view_from(user,NULL,2);  write_user(user,continue1);
			  user->mail_op = 7;  return;
			case '4': /* View from for sent s-mail */
			  view_from(user,NULL,3);  write_user(user,continue1);
			  user->mail_op = 7;  return;
			case '5': /* Delete s-mail */
			  write_user(user,"~CW-~FT To delete all your messages type~CRall\n~CW-~FT To delete the top X messages type: ~CR top # ~CW(~FGie top 2~CW)\n");
			  write_user(user,"~CW-~FT To delete the bottom X messages type: ~CRbottom # ~CW(~FGie bottom 2~CW)\n~CW- ~FTTo delete an X message type: ~CR# ~CW(~FGie 3 to delete the third message.~CW)\n");
			  write_user(user,"~CW-~FT To delete a range of messages type: ~CR# - # ~CW(~FGie 3 - 6~CW)\n~CW-~FT To delete a variety of messages type: ~CR# # # # # ~CW(~FGmax of 5~CW)\n");
			  write_user(user,"~CW-~FTDelete s-mail - Your choice~CB:~RS ");
			  user->mail_op = 4;  return;
			case '6': /* Delete sent s-mail */
			  write_user(user,"~CW-~FT To delete all your messages type~CRall\n~CW-~FT To delete the top X messages type: ~CR top # ~CW(~FGie top 2~CW)\n");
			  write_user(user,"~CW-~FT To delete the bottom X messages type: ~CRbottom # ~CW(~FGie bottom 2~CW)\n~CW- ~FTTo delete an X message type: ~CR# ~CW(~FGie 3 to delete the third message.~CW)\n");
			  write_user(user,"~CW-~FT To delete a range of messages type: ~CR# - # ~CW(~FGie 3 - 6~CW)\n~CW-~FT To delete a variety of messages type: ~CR# # # # # ~CW(~FGmax of 5~CW)\n");
			  write_user(user,"~CW-~FTDelete sent s-mail - Your choice~CB:~RS ");
			  user->mail_op = 5;  return;
			case '7': /* Send s-mail to someone */
			  write_user(user,"~CW-~FT To s-mail your friends type~CB: ~CR-friends\n");
			  if (user->level>=WIZ)
			  {
				write_user(user,"~CW-~FT To s-mail the staff type~CB: ~CR-staff\n~CW-~FT To s-mail everyone type~CB: ~CR-all\n");
			  }
			  write_user(user,"~CW-~FT Who would you like to s-mail?~CB:~RS ");
			  user->mail_op = 6;  return;
			case 'Q':
			  writeus(user,"~CW- ~CTThanks %s ~CTfor using the ~FG%s~CT mail management menu.\n",user->recap,TalkerRecap);
			  WriteRoomExcept(user->room,user,"~FT %s ~FGleaves the ~FY%s~FG mail menu.\n",name,TalkerRecap);
			  user->mail_op = 0;
			  rev_away(user);	prompt(user);
			  return;
			case '?':
			  PrintMenu(user,9);
			  write_user(user,menuprompt);
			  user->mail_op = 1;
			  return;
			default :
			  write_user(user,"~CW-~FT Invalid option.. ~CB[~CT? for menu~CB]\n");
			  write_user(user,menuprompt);
			  user->mail_op = 1;
		  }
		  return;
		case 2 :
		  if (!strcasecmp(word[0],"new"))
		  {
			sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
			switch(more(user,user->socket,filename,3))
			{
				case 0:
				  write_user(user,center("~FGYou have no new s-mail to read.\n",80));
				  break;
				case 1:
				  user->misc_op = 2;  return;
			}
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  num = atoi(word[0]);
		  if (num > 0)
		  {
			read_specific(user,NULL,atoi(word[0]),1);
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  /* First we have to copy the new mail file in */
		  sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
		  if (file_exists(filename))
		  {
			if ((outfp = fopen("tempfile","w")) != NULL)
			{
				/* Copy the new mail file in */
				if ((infp = fopen(filename,"r")) != NULL)
				{
					d = getc(infp);
					while (!feof(infp))
					{
						putc(d,outfp);
						d = getc(infp);
					}
					FCLOSE(infp);
				}
				unlink(filename);  /* we've added it */
				/* Copy the old mail file in */
				sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,user->name);
				if ((infp = fopen(filename,"r")) != NULL)
				{
					d = getc(infp);
					while (!feof(infp))
					{
						putc(d,outfp);
						d = getc(infp);
					}
					FCLOSE(infp);
				}
				FCLOSE(outfp);
			}
			rename("tempfile",filename);
		  }
		  switch(more(user,user->socket,filename,3))
		  {
			case 0:
			  write_user(user,center("~FG You have no s-mail to read..\n",80));
			  write_user(user,continue1);
			  user->mail_op = 7;
			  break;
			case 1:
			  user->misc_op = 2;
		  }
		  return;
		case 3 :
		  num = atoi(word[0]);
		  if (num>0)
		  {
			read_specific(user,NULL,atoi(word[0]),2);
			write_user(user,continue1);	user->mail_op = 7;
			return;
		  }
		  sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
		  switch (more(user,user->socket,filename,2))
		  {
			case 0:
			  write_user(user,center("~FG You have no sent s-mail to read..\n",80));
			  write_user(user,continue1);	user->mail_op = 7;
			  break;
			case 1:
			  user->misc_op = 2;
		  }
		  return;
		case 4 :
		  if (!strncasecmp(word[0],"top",2))
		  {
			num = atoi(word[1]);
			if (num)
			{
				sprintf(text,"dmail -t %d",num);
				clear_words();
				word_count = wordfind(text);
				exec_com(user,text);
			}
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strncasecmp(word[0],"bottom",2))
		  {
			num = atoi(word[1]);
			if (num)
			{
				sprintf(text,"dmail -b %d",num);
				clear_words();
				word_count = wordfind(text);
				exec_com(user,text);
			}
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strcasecmp(word[0],"all"))
		  {
			sprintf(text,"dmail -all");
			clear_words();
			word_count=wordfind(text);
			exec_com(user,text);
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  num = atoi(word[0]);
		  if (num <= 0)
		  {
			write_user(user,"~CW- ~FTAborting deleting s-mail..\n");
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strcmp(word[1],"-"))
		  {
			num2 = atoi(word[2]);
			if (num2 <= 0)
			{
				write_user(user,"~CW- ~FTAborting deleting s-mail..\n");
				write_user(user,continue1);
				user->mail_op=7;
				return;
			}
			sprintf(text,"dmail %d - %d",num,num2);
			clear_words();
			word_count = wordfind(text);
			exec_com(user,text);
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  num2 = atoi(word[1]);		num3 = atoi(word[2]);
		  num4 = atoi(word[3]);		num5 = atoi(word[4]);
		  sprintf(text,"dmail %d %s %s %s %s",num,num2?word[1]:"",num3?word[2]:"",num4?word[3]:"",num5?word[4]:"");
		  clear_words();	word_count=wordfind(text);
		  exec_com(user,text);	write_user(user,continue1);
		  user->mail_op = 7;	return;
		case 5:
		  if (!strncasecmp(word[0],"top",2))
		  {
			num = atoi(word[1]);
			if (num)
			{
				sprintf(text,"dsent -t %d",num);
				clear_words();
				word_count = wordfind(text);
				exec_com(user,text);
			}
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strncasecmp(word[0],"bottom",2))
		  {
			num = atoi(word[1]);
			if (num)
			{
				sprintf(text,"dsent -b %d",num);
				clear_words();
				word_count = wordfind(text);
				exec_com(user,text);
			}
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strcasecmp(word[0],"all"))
		  {
			sprintf(text,"dsent -all");	clear_words();
			word_count=wordfind(text);	exec_com(user,text);
			write_user(user,continue1);	user->mail_op = 7;
			return;
		  }
		  num = atoi(word[0]);
		  if (num <= 0)
		  {
			write_user(user,"~CW- ~FTAborting deleting s-mail..\n");
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strcmp(word[1],"-"))
		  {
			num2 = atoi(word[2]);
			if (num2 <= 0)
			{
				write_user(user,"~CW- ~FTAborting deleting s-mail..\n");
				write_user(user,continue1);
				user->mail_op = 7;
				return;
			}
			sprintf(text,"dsent %d - %d",num,num2);
			clear_words();
			word_count = wordfind(text);
			exec_com(user,text);
			write_user(user,continue1);
			user->mail_op = 7;
			return;	
		  }
		  num2 = atoi(word[1]);		num3 = atoi(word[2]);
		  num4 = atoi(word[3]);		num5 = atoi(word[4]);
		  sprintf(text,"dsent %d %s %s %s %s",num,num2?word[1]:"",num3?word[2]:"",num4?word[3]:"",num5?word[4]:"");
		  clear_words();	word_count = wordfind(text);
		  exec_com(user,text);	write_user(user,continue1);
		  user->mail_op = 7;	return;
		case 6 :
	          if (!strlen(inpstr))
		  {
			write_user(user,"~CW- ~FTAborting sending s-mail.\n");
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (!strncasecmp(word[0],"-friends",2))
		  {
			friend_smail(user,0);
			return;
		  }
		  else if (!strncasecmp(word[0],"-all",2))
		  {
			if (user->level<WIZ)
			{
		 		write_user(user,center(nosuchuser,80));
				write_user(user,continue1);
				user->mail_op = 7;
				return;
			}
			users_smail(user,0);
			return;
		  }
		  else if (!strncasecmp(word[0],"-staff",2))
		  {
			if (user->level<WIZ)
			{
		 		write_user(user,center(nosuchuser,80));
				write_user(user,continue1);
				user->mail_op = 7;
				return;
			}
			staff_smail(user,0);
			return;
		  }
	  /*
	     Now we need to go on and check to see if the user is online, or
	     not so we can determine if their mailbox is full, or they are
	     an enemy of the user. If so we don't let them send it.
	  */
		  strtolower(word[0]);
		  word[0][0] = toupper(word[0][0]);
		  if ((uname = get_user_full(user,word[0])) == NULL)
		  {
			write_user(user,center(nosuchuser,80));
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if ((u = get_user(uname)) == NULL)
		  {
			if ((u = create_user()) == NULL)
			{
				write_user(user,"~CW- ~FTSorry, unable to send s-mail right now..\n");
				write_log(0,LOG1,"[ERROR] - Unable to create temp user object in mail_ops().\n");
				write_user(user,continue1);
				user->mail_op = 7;
				return;
			}
			err = LoadUser(u,uname);
			if (err == 0)
			{
				write_user(user,center(nosuchuser,80));
				destruct_user(u);	destructed = 0;
				write_user(user,continue1);
				user->mail_op = 7;
				return;
			}
			else if (err == -1)
			{
				write_user(user,"~CW-~FT It appears as tho that user's userfile is corrupted.\n");
				write_log(1,LOG1,"[ERROR] - Corrupt userfile: [%s]\n",u->name);
				destruct_user(u);		destructed = 0;
				write_user(user,continue1);
				user->mail_op = 7;
				return;
			}
			on = 0;
		  }
		  else on = 1;
		  if (is_enemy(user,u))
		  {
			writeus(user,"~CW- ~FTYou have that user as your enemy.. you can't smail %s\n",gender2[u->gender]);
			if (on == 0) {  destruct_user(u);  destructed = 0;  }
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  if (is_enemy(u,user))
		  {
			writeus(user,"~CW- ~FTThat user has declared you an enemy, you can't smail %s.\n",gender2[u->gender]);
			if (on == 0) {  destruct_user(u);  destructed = 0;  }
			write_user(user,continue1);
			user->mail_op = 7;
			return;
		  }
		  sprintf(filename,"%s/%s/%s.M",UserDir,UserMesg,u->name);
		  if (stat(filename,&fb) == -1) fsize = 0;
		  else fsize = fb.st_size;
		  filename[0] = '\0';
		  sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,u->name);
		  if (stat(filename,&fb) == -1) fsize=fsize;
		  else fsize += fb.st_size;
		  filename[0] = '\0';
		  if (fsize >= MaxMbytes && user->level<OWNER)
		  {
			write_user(user,"~CW-~FT That user's s-mail box is allocating the max amount of bytes.\n");
			write_user(user,"~CW- ~FTSo you can't send them an s-mail message until they delete some.\n");
			if (on == 0) {  destruct_user(u);  destructed = 0;  }
			write_user(user,continue1);	   user->mail_op = 7;
			return;
		  }
		  sprintf(text,"~FT-~CT=~CB> ~FGWriting an s-mail message to~CB [~FR%s~CB] ~CB<~CT=~FT-\n",u->recap);
		  write_user(user,center(text,80));
		  user->editor.editing = 1;  user->misc_op = 4;
		  strcpy(user->mail_to,u->name);
		  if (on == 0) {  destruct_user(u);  destructed = 0;  }
		  if (user->editor.editor == 1) nuts_editor(user,NULL);
		  else ramtits_editor(user,NULL);
		  return;
		case 7 :
		  if (toupper(inpstr[0]) == 'Q')
		  {
			  writeus(user,"~CW- ~CTThanks %s ~CTfor using the ~FG%s~CT mail management menu.\n",user->recap,TalkerRecap);
			  WriteRoomExcept(user->room,user,"~FT %s ~FGleaves the ~FY%s~FG mail menu.\n",name,TalkerRecap);
			  user->mail_op = 0;
			  rev_away(user);
			  prompt(user);
			  return;
		  }
		  filename[0] = '\0';
		  num = num2 = num3 = num4 = num5 = on = 0;
		  PrintMenu(user,9);
		  write_user(user,menuprompt);
		  user->mail_op = 1;
		  return;
	}
	return;
}

/* The .bank command. */
void bank(User user,char *inpstr,int rt)
{
	char *name;
	int bankop,chance1,chance2;
	long amount;

	chance1 = chance2 = amount = 0;
	if (user == NULL) return;
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
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (word_count<2)
	{
		if (user->menus == 1)
		{
			WriteRoomExcept(user->room,user,"~CW-~FG %s ~FTenters the ~FG%s~FT national bank.\n",name,TalkerRecap);
			user->bank_op = 10;
			PrintMenu(user,1);
			return;
		}
		else
		{
			PrintMenu(user,5);
			return;
		}
	}
	inpstr = remove_first(inpstr);
	if ((bankop = GetBankOp(word[1])) == -1)
	{
		/* Let's see how many people figure this out */
		if (!strcmp(word[1],"-rob"))
		{
			if (!strcmp(user->lev_rank,"Thief")
			  && (user->colour == 1) && (user->mode == NUTS)
			  && (user->prompt == 1))
			{
				writeus(user,"~CW-~FT You creep along the outside of the bank late at night...\n");
				writeus(user,"~CW-~FT You crawl through a window someone carelessly left open..\n");
				writeus(user,"~CW-~FT You find the safe.. and try your luck cracking it...\n");
				chance1	= ran(5);
				chance2 = ran(5);
				if (chance1 == chance2) /* success */
				{
					/* They can steal between 1 and 10000 bucks */
					amount = ran(10000);
					writeus(user,"~CW-~FT Success... You grab a sack of money and run out.\n");
					writeus(user,"~CW-~FT You learn after counting that you got %d bucks.\n",amount);
					user->money += amount;
					return;
				}
				else
				{
					writeus(user,"~CW-~FT Shit.. you set off the alarm.. so you run out as fast as you can.\n");
					return;
				}
			}
		}
		PrintMenu(user,5);
		return;
	}
	user->bank_op	= bankop;
	clear_words();
	word_count	= wordfind(inpstr);
	BankOps(user,inpstr,0);
	user->bank_op	= 0;
	return;
}

/* Now the actual bank commands.. */
void BankOps(User user,char *inpstr,int menus)
{
	User u=NULL;
	char *name;
	long val;

	if (user == NULL) return;
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	switch(user->bank_op)
	{
		case 1 : /* .bank dmoney   */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Bank Usage~CB: ~CR.bank dmoney ~CB[~CRAmount of money to deposit~CW|~CR-all~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well.. next time don't say you want to deposit money.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-all")) val = 1;
		  else val = atol(word[0]);
		  if (val < 0)
		  {
			write_user(user,"~CW-~FT Umm.. that was an invalid amount..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to deposit~CB:~RS ");
				user->bank_op = 1;
				return;
			}
		  }
		  if (val == 1)
		  {
			writeus(user,"~CW-~FT You deposit all ~FG$~CR%ld~FT, into the bank.\n",user->money);
			user->dep_money += user->money;
			user->money = 0;
			writeus(user,"~CW-~FT You now hold ~FG$~FR%ld~FT in the bank.\n",user->dep_money);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (val>user->money)
		  {
			write_user(user,"~CW-~FT You don't even have that much money on hand.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to deposit~CB:~RS ");
				user->bank_op = 1;
				return;
			}
		  }
		  if (val == 0) write_user(user,"~CW-~FT You deposit nothing.\n");
		  else writeus(user,"~CW-~FT You deposit ~FG$~FR%ld~FT into the bank.\n",val);
		  user->dep_money	+= val;
		  user->money		-= val;
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		case 2 : /* .bank wmoney   */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Bank Usage~CB: ~CR.bank wmoney ~CB[~CRAmount of money to withdrawal~CW|~CR-all~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well.. next time don't say you want to withdrawal wmoney.\n");
				write_user(user,continue1);
				user->bank_op=11;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-all")) val = 1;
		  else val = atol(word[0]);
		  if (val < 0)
		  {
			write_user(user,"~CW-~FT Umm.. that was an invalid amount..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to withdrawal~CB:~RS ");
				user->bank_op = 2;
				return;
			}
		  }
		  if (val == 1)
		  {
			writeus(user,"~CW-~FT You take out all ~FG$~FR%ld ~FTthat you had in the bank.\n",user->dep_money);
			user->money	+= user->dep_money;
			user->dep_money	=  0;
			writeus(user,"~CW-~FT You now have ~FG$~FR%ld~FT on hand.\n",user->money);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (val > user->dep_money)
		  {
			write_user(user,"~CW-~FT You don't even have that much money in the bank.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to withdrawal~CB:~RS ");
				user->bank_op = 2;
				return;
			}
		  }
		  if (val == 0) write_user(user,"~CW-~FT You withdrawal nothing.\n");
		  else writeus(user,"~CW-~FT You withdrawal ~FG$~FR%ld~FT from the bank.\n",val);
		  user->dep_money	-= val;
		  user->money		+= val;
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		case 3 : /* .bank dchips   */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Bank Usage~CB: ~CR.bank dchips ~CB[~CRAmount of chips to deposit~CW|~CR-all~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well.. next time don't say you want to deposit chips.\n");
				write_user(user,continue1);
				user->bank_op=11;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-all")) val = 1;
		  else val = atol(word[0]);
		  if (val<0)
		  {
			write_user(user,"~CW-~FT Umm.. that was an invalid amount..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to deposit~CB:~RS ");
				user->bank_op = 3;
				return;
			}
		  }
		  if (val == 1)
		  {
			writeus(user,"~CW-~FT You deposit all ~CR%ld ~FGchips~FT, into the bank.\n",user->chips);
			user->dep_chips		+= user->chips;
			user->chips		=  0;
			writeus(user,"~CW-~FT You now hold ~FR%ld~FG chips~FT in the bank.\n",user->dep_chips);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (val>user->chips)
		  {
			write_user(user,"~CW-~FT You don't even have that many chips on hand.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to deposit~CB:~RS ");
				user->bank_op = 3;
				return;
			}
		  }
		  if (val == 0) write_user(user,"~CW-~FT You deposit nothing.\n");
		  else writeus(user,"~CW-~FT You deposit ~FR%ld ~FGchips~FT into the bank.\n",val);
		  user->dep_chips	+= val;
		  user->chips		-= val;
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		case 4 : /* .bank wchips   */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Bank Usage~CB: ~CR.bank wchips ~CB[~CRAmount of chips to withdrawal~CW|~CR-all~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well.. next time don't say you want to withdrawal chips.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (!strcasecmp(word[0],"-all")) val = 1;
		  else val = atol(word[0]);
		  if (val < 0)
		  {
			write_user(user,"~CW-~FT Umm.. that was an invalid amount..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to withdrawal~CB:~RS ");
				user->bank_op = 4;
				return;
			}
		  }
		  if (val == 1)
		  {
			writeus(user,"~CW-~FT You take out all ~CR%ld ~FGchips~FT that you had in the bank.\n",user->dep_chips);
			user->chips	+= user->dep_chips;
			user->dep_chips	=  0;
			writeus(user,"~CW-~FT You now have ~FR%ld~FG chips~FT on hand.\n",user->chips);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (val>user->dep_chips)
		  {
			write_user(user,"~CW-~FT You don't even have that many chips in the bank.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Enter amount to withdrawal~CB:~RS ");
				user->bank_op = 4;
				return;
			}
		  }
		  if (val == 0) write_user(user,"~CW-~FT You withdrawal nothing.\n");
		  else writeus(user,"~CW-~FT You withdrawal ~FR%ld ~FGchips~FT from the bank.\n",val);
		  user->dep_chips	-= val;
		  user->chips		+= val;
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		case 5 : /* .bank transfer */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FT Bank Usage~CB: ~CR.bank transfer ~CB[~CRuser~CB] [~CRmoney~CW|~CRchips~CB] [~CRamount~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Well.. next time don't say you want to withdrawal chips.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			}
		  }
		  if (menus == 0)
		  {
			if ((u = get_name(user,word[0])) == NULL)
			{
				write_user(user,center(notloggedon,80));
				return;
			}
			if (!strncasecmp(word[1],"chips",2))
			{
				if (user->dep_chips == 0)
				{
					write_user(user,"~CW-~FT You have no chips in the bank to transfer.\n");
					return;
				}
				val = atol(word[2]);
				if (user->dep_chips<val)
				{
					write_user(user,"~CW-~FT You don't even have that many chips in the bank.\n");
					return;
				}
				if (val <= 0)
				{
					write_user(user,"~CW-~FT You entered an invalid amount..\n");
					return;
				}
				u->dep_chips	+= val;
				user->dep_chips -= val;
				writeus(user,"~CW-~FT You transfer ~FR%ld~FG chips~FT to ~FM%s%s~FT account.\n",val,u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
				writeus(u,"~CW-~FM %s~FT transfers ~FR%ld~FG chips~FT to your bank account.\n",name,val);
				return;
			}
			else if (!strncasecmp(word[1],"money",2))
			{
				if (user->dep_money == 0)
				{
					write_user(user,"~CW-~FT You have no money in the bank to transfer.\n");
					return;
				}
				val = atol(word[2]);
				if (user->dep_money<val)
				{
					write_user(user,"~CW-~FT You don't even have that much money in the bank.\n");
					return;
				}
				if (val <= 0)
				{
					write_user(user,"~CW-~FT You entered an invalid amount..\n");
					return;
				}
				u->dep_money	+= val;
				user->dep_money -= val;
				writeus(user,"~CW-~FT You transfer ~FG$~FR%ld~FT to ~FM%s%s~FT account.\n",val,u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
				writeus(u,"~CW-~FM %s~FT transfers ~FG$~FR%ld~FT to your bank account.\n",name,val);
				return;
			}
			else write_user(user,"~CW-~FT Bank Usage~CB: ~CR.bank transfer ~CB[~CRuser~CB] [~CRmoney~CW|~CRchips~CB] [~CRamount~CB]\n");
			return;
		  }
		  if ((u = get_name(user,word[0])) == NULL)
		  {
			write_user(user,center(notloggedon,80));
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		  user->check[0] = '\0';
		  strcpy(user->check,u->name);
		  write_user(user,"~CW-~FT Transfer what? ~CB[~CRM~CB]~FGoney ~FTor~CB [~CRC~CB]~FGhips~CB:~RS ");
		  user->bank_op = 6;
		  return;
		case 6 : /* Choosing money or chips. */
		  switch(toupper(inpstr[0]))
		  {
			case 'M':
			  if (user->dep_money == 0)
			  {
				write_user(user,"~CW-~FT You need money in your bank account first.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			  }
			  write_user(user,"~CW-~FT How much money to transfer?~CB:~RS ");
			  user->bank_op = 7;
			  return;
			case 'C':
			  if (user->dep_chips == 0)
			  {
				write_user(user,"~CW-~FT You need chips in your bank account first.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			  }
			  write_user(user,"~CW-~FT How many chips to transfer?~CB:~RS ");
			  user->bank_op = 8;
			  return;
			default :
			  write_user(user,"~CW-~FT Transfer what? ~CB[~CRM~CB]~FGoney ~FTor~CB [~CRC~CB]~FGhips~CB:~RS ");
			  user->bank_op = 6;  return;
		  }
		  return;
		case 7 : /* How much money. */
		  if (!strlen(inpstr))
		  {
			write_user(user,"~CW-~FT Well.. fine.. we don't want you to transfer money.\n");
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		  val = atol(word[0]);
		  if (val > user->dep_money)
		  {
			write_user(user,"~CW-~FT You don't even have that much money in the bank.\n");
			write_user(user,"~CW-~FT Re-enter amount to transfer~CB:~RS ");
			user->bank_op = 7;
			return;
		  }
		  if (val <= 0)
		  {
			write_user(user,"~CW-~FT There's no point in trying to transfer nothing..\n");
			write_user(user,"~CW-~FT Re-enter amount to transfer~CB:~RS ");
			user->bank_op = 7;
			return;
		  }
		  if ((u = get_user(user->check)) == NULL)
		  {
			write_user(user,"~CW-~FT The person who you wanted to transfer to left..\n");
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		  writeus(user,"~CW-~FT You transfer ~FG$~FR%ld~FT to ~FM%s%s ~FTaccount.\n",val,u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
		  writeus(u,"~CW-~FM %s~FT transfers ~FG$~FR%ld~FT to your account.\n",name,val);
		  u->dep_money		+= val;
		  user->dep_money	-= val;
		  write_user(user,continue1);
		  user->bank_op = 11;
		  return;
		case 8 : /* How many chips. */
		  if (!strlen(inpstr))
		  {
			write_user(user,"~CW-~FT Well.. fine.. we don't want you to transfer chips.\n");
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		  val = atol(word[0]);
		  if (val>user->dep_chips)
		  {
			write_user(user,"~CW-~FT You don't even have that many chips in the bank.\n");
			write_user(user,"~CW-~FT Re-enter amount to transfer~CB:~RS ");
			user->bank_op = 8;
			return;
		  }
		  if (val <= 0)
		  {
			write_user(user,"~CW-~FT There's no point in trying to transfer nothing..\n");
			write_user(user,"~CW-~FT Re-enter amount to transfer~CB:~RS ");
			user->bank_op = 8;
			return;
		  }
		  if ((u = get_user(user->check)) == NULL)
		  {
			write_user(user,"~CW-~FT The person who you wanted to transfer to left..\n");
			write_user(user,continue1);
			user->bank_op = 11;
			return;
		  }
		  writeus(user,"~CW-~FT You transfer ~FR%ld~FG chips~FT to ~FM%s%s ~FTaccount.\n",val,u->recap,u->name[strlen(u->name)-1]=='s'?"'":"'s");
		  writeus(u,"~CW-~FM %s~FT transfers ~FR%ld~FG chips~FT to your account.\n",name,val);
		  u->dep_chips		+= val;
		  user->dep_chips	-= val;
		  write_user(user,continue1);
		  user->bank_op = 11;
		  return;
		case 10: /* Actual menu */
		  switch(toupper(inpstr[0]))
		  {
			case '1':
			  if (user->money == 0)
			  {
				write_user(user,"~CW-~FT You have no money to withdrawal.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			  }
			  write_user(user,"~CW-~FT Enter amount to deposit ~CB[~CR1 for ALL~CB]:~RS ");
			  user->bank_op = 1;  return;
			case '2':
			  if (user->dep_money == 0)
			  {
				write_user(user,"~CW-~FT You have no money to withdrawal.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			  }
			  write_user(user,"~CW-~FT Enter amount to withdrawal ~CB[~CR1 for ALL~CB]:~RS ");
			  user->bank_op = 2;  return;
			case '3':
			  if (user->chips == 0)
			  {
				write_user(user,"~CW-~FT You have no chips to deposit.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			  }
			  write_user(user,"~CW-~FT Enter amount to deposit ~CB[~CR1 for ALL~CB]:~RS ");
			  user->bank_op = 3;  return;
			case '4':
			  if (user->dep_chips == 0)
			  {
				write_user(user,"~CW-~FT You have no chips to withdrawal.\n");
				write_user(user,continue1);
				user->bank_op = 11;
				return;
			  }
			  write_user(user,"~CW-~FT Enter amount to withdrawal ~CB[~CR1 for ALL~CB]:~RS ");
			  user->bank_op = 4;  return;
			case '5':
			  write_user(user,"~CW-~FT Enter user to transfer to~CB:~RS ");
			  user->bank_op = 5;  return;
			case 'Q':
			  writeus(user,"~CW-~FT Thanks %s ~FTfor banking with ~FG%s~FT International.\n",user->recap,TalkerRecap);
			  WriteRoomExcept(user->room,user,"~CW- ~FT%s~FG leaves the ~FY%s ~FTInternational bank.\n",name,TalkerRecap);
			  user->bank_op = 0;
			  rev_away(user);
			  prompt(user);
			  return;
			case '?':
			  PrintMenu(user,1);  user->bank_op = 10;  return;
			default :
			  write_user(user,"~CW-~FG Invalid option..~CB [~CT? for menu~CB]\n");
			  write_user(user,menuprompt);
			  user->bank_op = 10;
		  }
		  return;
		case 11: /* Simple prompt. */
		  if (toupper(inpstr[0]) == 'Q')
		  {
			writeus(user,"~CW-~FT Thanks %s ~FTfor banking with ~FG%s~FT International.\n",user->recap,TalkerRecap);
			WriteRoomExcept(user->room,user,"~CW- ~FT%s~FG leaves the ~FY%s ~FGInternational bank.\n",name,TalkerRecap);
			user->bank_op = 0;
			rev_away(user);			
			prompt(user);
			return;
		  }
		  val = 0;  user->check[0] = '\0';	PrintMenu(user,1);
		  user->bank_op = 10;			return;
	}
	return;
}

/* The store command */
void store(User user,char *inpstr,int rt)
{
	char *name;
	int store_op = 0;

	if (user == NULL) return;
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
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	if (word_count<2)
	{
		write_user(user,"~CW-~FT Store Usage~CB: ~CR.store ~CB[~CR-buy~CW|~CR-sell~CW|~CR-inventory~CB]\n");
		return;
	}
	if (word_count<3)
	{
		if (!strncasecmp(word[1],"-inventory",2))
		{
			write_user(user,center("~FT-~CT=~CB>~FG You own the following items~CB: ~CB<~CT=~FT-\n",80));
			ShowInventory(user);
			return;
		}
		else if (!strncasecmp(word[1],"-buy",2))
		{
			if (user->menus == 1)
			{
				WriteRoomExcept(user->room,user,"~CW-~FG %s~FT enters the ~FG%s~FT general store.\n",name,TalkerRecap);
				user->ignall_store = user->ignall;
				user->ignall = 1;
				PrintMenu(user,3);
				user->store_op = 20;
				return;
			}
			else {	PrintMenu(user,7);  return;  }
		}
		else if (!strncasecmp(word[1],"-sell",2))
		{
			if (user->menus == 1)
			{
				WriteRoomExcept(user->room,user,"~CW-~FG %s~FT enters the ~FG%s~FT general store.\n",name,TalkerRecap);
				user->ignall_store = user->ignall;
				user->ignall = 1;
				PrintMenu(user,4);
				user->store_op = 21;
				return;
			}
			else {	PrintMenu(user,8);  return;  }
		}
		else write_user(user,"~CW-~FT Store Usage~CB: ~CR.store ~CB[~CR-buy~CW|~CR-sell~CW|~CR-inventory~CB]\n");
		return;
	}
	inpstr = remove_first(inpstr);
	inpstr = remove_first(inpstr);
	if (!strncasecmp(word[1],"-buy",2))
	{
		if ((store_op = GetStoreBuyOp(word[2])) == -1)
		{
			PrintMenu(user,7);
			return;
		}
	}
	else if (!strncasecmp(word[1],"-sell",2))
	{
		if ((store_op = GetStoreSellOp(word[2])) <= -1)
		{
			PrintMenu(user,8);
			return;
		}
	}
	user->store_op	= store_op;
	clear_words();
	word_count	= wordfind(inpstr);
	StoreOps(user,inpstr,0);
	user->store_op	= 0;
	return;
}

/* The store options/menu. */
void StoreOps(User user,char *inpstr,int menus)
{
	Room rm = NULL;
	User u = NULL;
	char *name;
	int val,i = 0;
	long mval;

	if (user == NULL) return;
	if (user->invis == Invis) name = invisname;  else name = user->recap;
	switch(user->store_op)
	{
		case 1 : /* .store -buy  room		 */
		  if (free_rooms)
		  {
			write_user(user,"~CW-~FT The purchasing of rooms isn't required.. you have one for free.\n");
			if (menus==0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->has_room == 1 || user->level >= WIZ)
		  {
			write_user(user,"~CW-~FT You already have a personal room.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->money<2500)
		  {
			write_user(user,"~CW-~FT You don't have enough money on hand to purchase a room.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  home_room(user);
		  rm = get_room(user->name);
		  if (rm==NULL)
		  {
			write_user(user,"~CW-~FTAn error occurred trying to process your room..\n");
			write_log(1,LOG1,"[ERROR] - Unable to create user room for %s\n",user->name);
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op = 22;
				return;
			}
		  }
		  user->room = rm;
		  strcpy(rm->recap,user->name);
		  SaveRoom(rm);
		  user->has_room = 1;
		  user->money -= 2500;
		  write_user(user,"~CW-~FT Your room has been created, to rename it, try ~CW'~CR.rmname~CW'\n");
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s~FG buys %s own personal room..\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 22;
			return;
		  }
		case 2 : /* .store -sell room		 */
		  if (free_rooms)
		  {
			write_user(user,"~CW-~FT Rooms are free.. you can't sell your room.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  if (user->has_room != 1)
		  {
			write_user(user,"~CW- ~FTHow do you expect to sell your room if you don't even have one?\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  if (user->level >= WIZ)
		  {
			write_user(user,"~CW-~FT Staff members can't sell their rooms..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  user->money += 1250;  user->has_room = 0;
		  if ((rm = get_room(user->name)) != NULL)
		  {
			user->room = get_room(main_room);
			for_users(u)
			{
				if (u->room == rm) u->room = get_room(main_room);
			}
			destruct_room(rm);
		  }
		  write_user(user,"~CW-~FT Your room has been sold.. you get ~FG$~FR1250 ~FTback.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG has sold %s personal room.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 23;
			return;
		  }
		case 3 : /* .store -buy  atmos		 */
		  if (user->atmos >= MaxAtmos)
		  {
			write_user(user,"~CW-~FT You already have the max. amount of atmospherics.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->has_room != 1 && user->level<WIZ)
		  {
			write_user(user,"~CW-~FT You don't even have a room to hold the atmospherics.\n");
			if (menus == 0) return;
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->money<100)
		  {
			write_user(user,"~CW-~FT You don't have enough for an atmospheric.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->money -= 100;  ++user->atmos;
		  write_user(user,"~CW-~FT You bought an atmospheric.. to edit it, use ~CW'~CR.atmos~CW'\n");
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s~FG buys an atmospheric for %s room.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=22;
			return;
		  }
		case 4 : /* .store -buy  carriage	 */
		  if (user->carriage != 0)
		  {
			write_user(user,"~CW-~FT You already have your own horse carriage.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->money<1000)
		  {
			write_user(user,"~CW-~FT You kinda need a thousand bucks to buy your own horse carriage.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->money -= 1000;  user->carriage = 1;
		  write_user(user,"~CW-~FT You lug your horse carriage out of the store..\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG buys %s own horse carriage.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=22;
			return;
		  }
		case 5 : /* .store -sell carriage	 */
		  if (user->carriage == 0)
		  {
			write_user(user,"~CW-~FT You don't even have a horse carriage to sell.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  user->money += 500;  user->carriage = 0;
		  write_user(user,"~CW-~FT Your carriage has been sold for ~FG$~FR500~FT.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG sells %s horse drawn carriage.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=23;
			return;
		  }
		case 6 : /* .store -buy  hat		 */
		  if (user->hat != 0)
		  {
			write_user(user,"~CW-~FT Why would you need more then one hat? Do you have two heads???\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->money<250)
		  {
			write_user(user,"~CW-~FT Hey... hat's are expensive here.. you need more cash on hand..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->hat = 1;  user->money -= 250;
		  writeus(user,"~CW-~FT You buy a %scoloured ~FThat.. to change the colour, use ~CW'~CR.hat~CW'\n",hatcolor[user->hat]);
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s~FG buys %sself a %scoloured~FG hat.\n",name,gender2[user->gender],hatcolor[user->hat]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=22;
			return;
		  }
		case 7 : /* .store -sell hat		 */
		  if (user->hat == 0)
		  {
			write_user(user,"~CW-~FT You don't even have a coloured hat to sell.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  user->money += 125;  user->hat = 0;
		  write_user(user,"~CW-~FT You no longer wear a coloured hat.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG sold %s coloured hat.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=23;
			return;
		  }
		case 8 : /* .store -buy  condoms	 */
		  if (user->money<20)
		  {
			write_user(user,"~CW-~FT You don't have enough for condoms.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->condoms >= 65000)	/* More of a safety thing. */
		  {
			write_user(user,"~CW-~FT Don't you think you should use the condoms you have first?\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->condoms += 5;  user->money -= 20;
		  writeus(user,"~CW-~FT You buy a box of condoms, giving you a total of ~CR%d~FT.\n",user->condoms);
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s~FG buys a box of condoms....\n",name);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 22;
			return;
		  }
		case 9 : /* .store -buy  chips   <amnt>	 */
		  if (!strlen(inpstr))
		  {
			if (menus==0)
			{
				write_user(user,"~CW-~FG Store Usage~CB: ~CR.store -buy chips ~CB[~CRamount to buy~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Fine.. don't buy any chips...\n");
				write_user(user,continue1);
				user->store_op = 22;
				return;
			}
		  }
		  if (user->money<2)
		  {
			write_user(user,"~CW-~FT You don't have enough money for ONE chip..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  mval = atol(word[0]);
		  if (mval<1)
		  {
			write_user(user,"~CW-~FT You can't buy a negative amount of chips.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Amount of chips to buy~CB:~RS ");
				user->store_op = 9;
				return;
			}
		  }
		  if (user->money < (mval*2))
		  {
			write_user(user,"~CW-~FT You don't have enough cash for that amount.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->chips += mval;  user->money -= (mval*2);
		  writeus(user,"~CW-~FT Total cost of chips~CB: ~FG$~FY%ld~FT. You now hold ~FR%ld~FG chips.\n",(mval*2),user->chips);
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG buys up some gambling chips..\n",name);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 22;
			return;
		  }
		case 10: /* .store -sell chips   <amnt>	 */
		  if (!strlen(inpstr))
		  {
			if (menus == 0)
			{
				write_user(user,"~CW-~FG Store Usage~CB: ~CR.store -sell chips ~CB[~CRamount to sell~CB]\n");
				return;
			}
			else
			{
				write_user(user,"~CW-~FT Fine.. you don't want to sell any chips.\n");
				write_user(user,continue1); user->store_op = 23;
				return;
			}
		  }
		  mval = atol(word[0]);
		  if (mval<1)
		  {
			write_user(user,"~CW-~FT You need to specify a PROPER amount (greater then ZERO)\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW- ~FTAmount of chips to sell~CB:~RS ");
				user->store_op = 10;  return;
			}
		  }
		  if (mval > user->chips)
		  {
			write_user(user,"~CW-~FT You don't even have that many chips to sell.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,"~CW-~FT Amount of chips to sell~CB:~RS ");
				user->store_op = 10;  return;
			}
		  }
		  user->chips -= mval;  user->money += (mval*2);
		  writeus(user,"~CW-~FT You sold ~CR%ld ~FTchips for ~FG$~FR%ld~FT.\n",mval,(mval*2));
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG sells some gambling chips.\n",name);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=23;
			return;
		  }
		case 11: /* .store -buy  bguard		 */
		  if (user->protection)
		  {
			write_user(user,"~CW-~FT You already have a body guard to protect you.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->money < 1000)
		  {
			write_user(user,"~CW-~FT You don't even have enough cash for a body guard.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->money -= 1000;  user->protection = 1;
		  write_user(user,"~CW-~FT You hire a bodyguard to protect you from the evils of people..\n");
		  write_user(user,"~CW-~FT Be aware.. you are still vulnerable to other peoples attacks.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG hires %sself a body guard.\n",name,gender2[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=22;
			return;
		  }
		case 12: /* .store -sell bguard		 */
		  if (user->protection == 0)
		  {
			write_user(user,"~CW- ~FT You don't have a body guard to sell..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  user->money += 500;  user->protection = 0;
		  write_user(user,"~CW-~FT You tell your body guard to take a hike.\n");
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s~FG fires %s body guard.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=23;
			return;
		  }
		case 13: /* .store -buy  bullets 	 */
		  if (user->money < 50)
		  {
			write_user(user,"~CW-~FT You don't have enough for a box of bullets.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->bullets >= 65000)
		  {
			write_user(user,"~CW-~FT You should use the bullets you have first.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->money -= 50;  user->bullets += 10;
		  write_user(user,"~CW-~FT You buy yourself a box of 10 bullets.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG buys a box of bullets.\n",name);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=22;
			return;
		  }
		case 14: /* .store -buy  nukes		 */
		  if (user->money < 500000)
		  {
			write_user(user,"~CW-~FT Sheesh.. nukes don't come cheap ya know..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->nukes >= 50)
		  {
			write_user(user,"~CW-~FT You have too many nukes... set some off first.\n");
			if (menus==0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->money -= 500000;  ++user->nukes;
		  write_user(user,"~CW-~FT You begin to glow (literally) as you are now a nuclear power..\n");
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s ~FGglows with nuclear power..\n",name);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=22;
			return;
		  }
		case 15: /* .store -sell nukes		 */
		  if (user->nukes == 0)
		  {
			write_user(user,"~CW-~FT You can't sell something that you have none of.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  user->money += 5;  user->nukes--;
		  write_user(user,"~CW-~FT You sell one of your nukes and feel ripped off.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM%s~FG gets ripped off by selling a nuclear bomb!\n",name);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op=23;
			return;
		  }
		case 16: /* .store -buy  pet		 */
		  if (user->pet.type == 1)
		  {
			write_user(user,"~CW- ~FTSorry, you can only have one pet.\n");
			if (menus==0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->money<150)
		  {
			write_user(user,"~CW-~FT Pets don't come cheap on here..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->pet.type = 1;  user->money -= 150;
		  write_user(user,"~CW-~FT You buy yourself a new pet.. to choose which type, and the pet's name, use ~CW'~CR.pet~CW'\n");
		  WriteRoomExcept(user->room,user,"~CB -- ~FM%s~FG thinks %s is responsible enough to own a pet..\n",name,gender3[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 22;
			return;
		  }
		case 17: /* .store -sell pet		 */
		  if (user->pet.type == 0)
		  {
			write_user(user,"~CW-~FT You don't even have a pet to sell.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=23;
				return;
			}
		  }
		  user->money += 75;  user->pet.type = 0;
		  user->pet.name[0] = '\0';
		  for (i = 0 ; i < 20 ; ++i)
		  {
			user->pet.alias[i][0]='\0';
			user->pet.trigger[i][0]='\0';
		  }
		  write_user(user,"~CW-~FT You sell your pet.. leaving it feeling all alone :(\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s ~FGgets rid of %s pet.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 23;
			return;
		  }
		case 18: /* .store -buy  key		 */
		  if (user->money<250)
		  {
			write_user(user,"~CW-~FT You don't have enough cash to buy a key.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->keys >= MaxKeys)
		  {
			write_user(user,"~CW-~FT You already have the maximum amount of keys.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  ++user->keys;  user->money -= 250;
		  write_user(user,"~CW-~FT You buy yourself a key to allow others into your room.\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s ~FGbuys a key to let others enter %s room.\n",name,gender1[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 22;
			return;
		  }
		case 19: /* .store -buy beer		 */
		  if (user->money < 10)
		  {
			write_user(user,"~CW-~FT You don't have enough for some booze.\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  if (user->beers >= 65000)
		  {
			write_user(user,"~CW-~FT You should drink the beer that you have first..\n");
			if (menus == 0) return;
			else
			{
				write_user(user,continue1); user->store_op=22;
				return;
			}
		  }
		  user->money -= 10;  user->beers += 6;
		  write_user(user,"~CW-~FT You buy a six pack of beer.. time to get loaded!\n");
		  WriteRoomExcept(user->room,user,"~CB --~FM %s~FG buys %sself a six pack of beer.\n",name,gender2[user->gender]);
		  if (menus == 0) return;
		  else
		  {
			write_user(user,continue1);  user->store_op = 22;
			return;
		  }
		case 20: /* Menu choice. [buy]		 */
		  switch(toupper(inpstr[0]))
		  {
			case '1':
			  switch(toupper(inpstr[1]))
			  {
				case '0':
				  write_user(user,"~CW-~FT Buying a pet.. press enter.\n");
				  user->store_op = 16;  return;
				case '1':
				  write_user(user,"~CW-~FT Buying a room key.. press enter.\n");
				  user->store_op = 18;  return;
				case '2':
				  write_user(user,"~CW-~FT Buying some beer.. press enter.\n");
				  user->store_op = 19;  return;
				default :
				  write_user(user,"~CW-~FT Buying a personal room.. press enter.\n");
				  user->store_op = 1;   return;
			  }
			case '2':
			  write_user(user,"~CW- ~FTBuying some atmospherics.. press enter.\n");
			  user->store_op = 3;  return;
			case '3':
			  write_user(user,"~CW- ~FTBuying a horse drawn carriage.. press enter.\n");
			  user->store_op = 4;  return;
			case '4':
			  write_user(user,"~CW-~FT Buying a coloured hat.. press enter.\n");
			  user->store_op = 6;  return;
			case '5':
			  write_user(user,"~CW-~FT Buying some latex condoms.. press enter.\n");
			  user->store_op = 8;  return;
			case '6':
			  write_user(user,"~CW-~FT Amount of chips to buy~CB:~RS ");
			  user->store_op = 9;  return;
			case '7':
			  write_user(user,"~CW-~FT Buying a body guard.. press enter.\n");
			  user->store_op = 11;  return;
			case '8':
			  write_user(user,"~CW-~FT Buying a box of bullets.. press enter.\n");
			  user->store_op = 13;  return;
			case '9':
			  write_user(user,"~CW- ~FTBuying a nuclear bomb.. press enter.\n");
			  user->store_op = 14;  return;
			case '?':
			  PrintMenu(user,3);  user->store_op = 20;  return;
			case 'Q':
			  writeus(user,"~CW-~FT Thanks %s~FT for coming to the ~FG%s~FT general store..\n",user->recap,TalkerRecap);
			  WriteRoomExcept(user->room,user,"~CW- ~FT%s~FG leaves the ~FY%s~FG general store..\n",name,TalkerRecap);
			  rev_away(user);
			  user->store_op = 0;  prompt(user);  return;
			case 'I':
			  write_user(user,center("~FT-~CT=~CB> ~FGYou own the following items~CB: ~CB<~CT=~FT-\n",80));
			  ShowInventory(user);  write_user(user,continue1);
			  user->store_op = 22;    return;
			default :
			  write_user(user,"~CW-~FT Invalid option. ~CB[~CR? for menu~CB]\n");
			  write_user(user,menuprompt);  user->store_op = 20;
			  return;
		  }
		case 21: /* Menu choice. [sell]		 */
		  switch(toupper(inpstr[0]))
		  {
			case '1':
			  write_user(user,"~CW-~FT Selling your personal room.. Press Enter.\n");
			  user->store_op = 2;  return;
			case '2':
			  write_user(user,"~CW-~FT Selling your horse carriage.. Press Enter.\n");
			  user->store_op = 5;  return;
			case '3':
			  write_user(user,"~CW-~FT Selling your coloured hat.. Press Enter.\n");
			  user->store_op = 7;  return;
			case '4':
			  write_user(user,"~CW-~FT Amount of chips to sell~CB:~RS ");
			  user->store_op = 10;  return;
			case '5':
			  write_user(user,"~CW-~FT Selling off your body guard.. Press Enter.\n");
			  user->store_op = 12;  return;
			case '6':
			  write_user(user,"~CW-~FT Selling your nuclear bomb.. Press Enter.\n");
			  user->store_op = 15;  return;
			case '7':
			  write_user(user,"~CW-~FT Selling your adorable lil pet.. Press Enter.\n");
			  user->store_op = 17;  return;
			case '?':
			  PrintMenu(user,4);  user->store_op = 21;  return;
			case 'Q':
			  writeus(user,"~CW-~FT Thanks %s~FT for coming to the ~FG%s~FT general store..\n",user->recap,TalkerRecap);
			  WriteRoomExcept(user->room,user,"~CW- ~FT%s~FG leaves the ~FY%s~FG general store..\n",name,TalkerRecap);
			  rev_away(user);
			  user->store_op = 0;  prompt(user);  return;
			case 'I':
			  write_user(user,center("~FT-~CT=~CB> ~FGYou own the following items~CB: ~CB<~CT=~FT-\n",80));
			  ShowInventory(user);  write_user(user,continue1);
			  user->store_op = 23;  return;
			default :
			  write_user(user,"~CW-~FT Invalid option. ~CB[~CR? for menu~CB]\n");
			  write_user(user,menuprompt);  user->store_op = 21;
			  return;
		  }
		case 22: /* Small prompt.		 */
		  if (toupper(inpstr[0])=='Q')
		  {
			writeus(user,"~CW-~FT Thanks %s~FT for coming to the ~FG%s~FT general store.\n",user->recap,TalkerRecap);
			WriteRoomExcept(user->room,user,"~CW-~FT %s~FG leaves the ~FY%s~FG general store.\n",name,TalkerRecap);
			rev_away(user);
			user->store_op = 0;  prompt(user);  return;
		  }
		  val = mval = 0;  user->store_op = 20;  PrintMenu(user,3);
		  return;
		case 23:
		  if (toupper(inpstr[0] == 'Q'))
		  {
			writeus(user,"~CW-~FT Thanks %s~FT for coming to the ~FG%s~FT general store.\n",user->recap,TalkerRecap);
			WriteRoomExcept(user->room,user,"~CW-~FT %s~FG leaves the ~FY%s~FG general store.\n",name,TalkerRecap);
			rev_away(user);
			user->store_op = 0;  prompt(user);  return;
		  }
		  val = mval = 0;  user->store_op = 21;  PrintMenu(user,4);
		  return;
	}
	return;
}

/* END OF MENUS.C */
