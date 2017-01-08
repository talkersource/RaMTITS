/*
  ramtits.c
    Source code for various functions that are needed by the RaMTITS
    code. Makes up a fair amount of the internal functioning of the
    daemon.

    The source within this file is Copyright 1998 - 2001 by
 	Rob Melhuish, Tim van der Leeuw, Mike Irving, and Cygnus (coder
 	of ncohafmuta).

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
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#define _RAMTITS_C
  #include "include/needed.h"
#undef _RAMTITS_C

/*
   Again messed around with the way userfiles are loaded/saved, please read
   ~/ramtits-2.x/docs/README.userfiles I'm hoping that this way will be the
   absolute most safest way to do userfiles..
*/
int LoadUser(User u,char *name)
{
	FILE *fp;
	char filename[256];
	int err = 0,version,i = 0;

	if (u == NULL) return(0);
	strtolower(name);
	name[0] = toupper(name[0]);
	sprintf(filename,"%s/%s.dat",UserDir,name);
	fp = fopen(filename,"r");
	if (fp == NULL) return(0); /* user doesn't exist */
	strcpy(u->name,name);
	/* This MUST be whatever you use in uconvert.c file, or SaveUser */
	version = '2';

	rval(version);
	if (err) return(-1);

	for (i = 0; i < 50; ++i)
	{
		rbuff(u->friends[i]);
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		if (!strcasecmp(u->friends[i],"#NOTSET#"))
			u->friends[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		rbuff(u->enemies[i]);
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		if (!strcasecmp(u->enemies[i],"#NOTSET#"))
			u->enemies[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		rbuff(u->rmacros[i]);
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (!strcasecmp(u->rmacros[i],"#NOTSET#"))
			u->rmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		rbuff(u->fmacros[i]);
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (!strcasecmp(u->fmacros[i],"#NOTSET#"))
			u->fmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->allowcom[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->allowcom[i],"#NOTSET#"))
			u->allowcom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->denycom[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->denycom[i],"#NOTSET#"))
			u->denycom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->alias.find[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->alias.find[i],"#NOTSET#"))
			u->alias.find[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->alias.alias[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcasecmp(u->alias.alias[i],"#NOTSET#"))
			u->alias.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		rbuff(u->pet.alias[i]);
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (!strcasecmp(u->pet.alias[i],"#NOTSET#"))
			u->pet.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		rbuff(u->pet.trigger[i]);
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (!strcasecmp(u->pet.trigger[i],"#NOTSET#"))
			u->pet.trigger[i][0] = '\0';
	} i = 0;
	rbuff(u->scut);

	rbuff(u->name);			rbuff(u->pass);
	rbuff(u->last_site);		rbuff(u->desc);
	rbuff(u->in_phr);		rbuff(u->out_phr);
	rbuff(u->email);		rbuff(u->recap);
	rbuff(u->predesc);		rbuff(u->lev_rank);
	rbuff(u->webpage);		rbuff(u->bday);
	rbuff(u->married_to);		rbuff(u->pet.name);
	rbuff(u->login_msg);		rbuff(u->logout_msg);
	rbuff(u->bfafk);		rbuff(u->sstyle);
	rbuff(u->tzone);		rbuff(u->autoexec);
	/*
	   Temp char's for your own use.. just remember what they are.
	   I'm not sure of the effect of changing the variable names.. but
	   it might be bad.. so just remember what they are..
	*/
	rbuff(u->mychar1);		rbuff(u->mychar2);
	rbuff(u->mychar3);		rbuff(u->mychar4);
	rbuff(u->mychar5);		rbuff(u->mychar6);
	rbuff(u->mychar7);		rbuff(u->mychar8);
	rbuff(u->mychar9);		rbuff(u->mychar10);

	rval(u->last_login);		rval(u->total_login);
	rval(u->last_login_len);	rval(u->level);
	rval(u->prompt);		rval(u->status);
	rval(u->char_echo);		rval(u->mode);
	rval(u->icq);			rval(u->money);
	rval(u->dep_money);		rval(u->chips);
	rval(u->dep_chips);		rval(u->ignore);
	rval(u->tic_win);		rval(u->tic_lose);
	rval(u->tic_draw);		rval(u->win);
	rval(u->lose);			rval(u->draw);
	rval(u->autofwd);		rval(u->logons);
	rval(u->age);			rval(u->examined);
	rval(u->login_type);		rval(u->condoms);
	rval(u->c4_win);		rval(u->c4_lose);
	rval(u->rows);			rval(u->cols);
	rval(u->atmos);			rval(u->hang_wins);
	rval(u->hang_loses);		rval(u->chess_wins);
	rval(u->chess_lose);		rval(u->chess_draws);
	rval(u->bship_wins);		rval(u->bship_lose);
	rval(u->gshout);		rval(u->gpriv);
	rval(u->gsocs);			rval(u->monitor);
	rval(u->no_fight);		rval(u->icqhide);
	rval(u->no_follow);		rval(u->tvar2);
	rval(u->editor.editor);		rval(u->tvar3);
	rval(u->has_room);		rval(u->carriage);
	rval(u->hat);			rval(u->protection);
	rval(u->tvar1);			rval(u->hide_email);
	rval(u->ansi_on);		rval(u->help);
	rval(u->who);			rval(u->speech);
	rval(u->delprotect);		rval(u->gender);
	rval(u->whitespace);		rval(u->hide_room);
	rval(u->profile);		rval(u->menus);
	rval(u->quit);			rval(u->nukes);
	rval(u->pet.type);		rval(u->keys);
	rval(u->beers);			rval(u->bullets);
	rval(u->splits);		rval(u->editor.visual);
	rval(u->myint1);		rval(u->myint2);
	/*
	   Temp int's for your own use.. just remember what they are.
	   I'm not sure of the effect of changing the variable names.. but
	   it might be bad.. so just remember... what they are..
	*/
	rval(u->myint3);		rval(u->myint4);
	rval(u->myint5);		rval(u->myint6);
	rval(u->myint7);		rval(u->myint8);
	rval(u->myint9);		rval(u->myint10);

	FCLOSE(fp);
	u->cloak_lev = u->level;
	check_bad_vars(u);
	reset_junk(u);
	if (err) return(-1);
	return(1);
}

/* Now we save it in this new format. */
int SaveUser(User u,int save_current)
{
	FILE *fp;
	char filename[256];
	int err = 0, version, i = 0;

	if (u == NULL) return(0);
	if (u->type == CloneType || u->login) return(0);
	/* Don't touch this number unless you now what you're doing. */
	version = '2';
	sprintf(filename,"%s/%s.dat",UserDir,u->name);
	fp = fopen("tempfile","w");
	if (fp == NULL)
	{
		write_user(u,"~CW-~FT I was unable to open your userfile.\n");
		write_log(1,LOG1,"[ERROR] - Unable to save userfile '%s' Reason: [%s]\n",u->name,strerror(errno));
		return(0);
	}
	strcpy(u->last_site,u->ip_site);
	wval(version);
	for (i = 0; i < 50; ++i)
	{
		if (u->friends[i][0] == '\0') strcpy(u->friends[i],"#NOTSET#");
		wbuff(u->friends[i]);
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		if (u->enemies[i][0] == '\0') strcpy(u->enemies[i],"#NOTSET#");
		wbuff(u->enemies[i]);
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (u->rmacros[i][0] == '\0') strcpy(u->rmacros[i],"#NOTSET#");
		wbuff(u->rmacros[i]);
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (u->fmacros[i][0] == '\0') strcpy(u->fmacros[i],"#NOTSET#");
		wbuff(u->fmacros[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (u->allowcom[i][0] == '\0')
			strcpy(u->allowcom[i],"#NOTSET#");
		wbuff(u->allowcom[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (u->denycom[i][0] == '\0')
			strcpy(u->denycom[i],"#NOTSET#");
		wbuff(u->denycom[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (u->alias.find[i][0] == '\0')
		strcpy(u->alias.find[i],"#NOTSET#");
			wbuff(u->alias.find[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (u->alias.alias[i][0] == '\0')
			strcpy(u->alias.alias[i],"#NOTSET#");
		wbuff(u->alias.alias[i]);
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (u->pet.alias[i][0] == '\0')
			strcpy(u->pet.alias[i],"#NOTSET#");
		wbuff(u->pet.alias[i]);
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (u->pet.trigger[i][0] == '\0')
			strcpy(u->pet.trigger[i],"#NOTSET#");
		wbuff(u->pet.trigger[i]);
	} i = 0;
	wbuff(u->scut);

	wbuff(u->name);			wbuff(u->pass);
	wbuff(u->last_site);		wbuff(u->desc);
	wbuff(u->in_phr);		wbuff(u->out_phr);
	wbuff(u->email);		wbuff(u->recap);
	wbuff(u->predesc);		wbuff(u->lev_rank);
	wbuff(u->webpage);		wbuff(u->bday);
	wbuff(u->married_to);		wbuff(u->pet.name);
	wbuff(u->login_msg);		wbuff(u->logout_msg);
	wbuff(u->bfafk);		wbuff(u->sstyle);
	wbuff(u->tzone);		wbuff(u->autoexec);
	/*
	   Temp char's for your own use.. just remember what they are.
	   I'm not sure of the effect of changing the variable names.. but
	   it might be bad.. so just remember... what they are..
	*/
	wbuff(u->mychar1);		wbuff(u->mychar2);
	wbuff(u->mychar3);		wbuff(u->mychar4);
	wbuff(u->mychar5);		wbuff(u->mychar6);
	wbuff(u->mychar7);		wbuff(u->mychar8);
	wbuff(u->mychar9);		wbuff(u->mychar10);

	wval(u->last_login);		wval(u->total_login);
	wval(u->last_login_len);	wval(u->level);
	wval(u->prompt);		wval(u->status);
	wval(u->char_echo);		wval(u->mode);
	wval(u->icq);			wval(u->money);
	wval(u->dep_money);		wval(u->chips);
	wval(u->dep_chips);		wval(u->ignore);
	wval(u->tic_win);		wval(u->tic_lose);
	wval(u->tic_draw);		wval(u->win);
	wval(u->lose);			wval(u->draw);
	wval(u->autofwd);		wval(u->logons);
	wval(u->age);			wval(u->examined);
	wval(u->login_type);		wval(u->condoms);
	wval(u->c4_win);		wval(u->c4_lose);
	wval(u->rows);			wval(u->cols);
	wval(u->atmos);			wval(u->hang_wins);
	wval(u->hang_loses);		wval(u->chess_wins);
	wval(u->chess_lose);		wval(u->chess_draws);
	wval(u->bship_wins);		wval(u->bship_lose);
	wval(u->gshout);		wval(u->gpriv);
	wval(u->gsocs);			wval(u->monitor);
	wval(u->no_fight);		wval(u->icqhide);
	wval(u->no_follow);		wval(u->tvar2);
	wval(u->editor.editor);		wval(u->tvar3);
	wval(u->has_room);		wval(u->carriage);
	wval(u->hat);			wval(u->protection);
	wval(u->tvar1);			wval(u->hide_email);
	wval(u->ansi_on);		wval(u->help);
	wval(u->who);			wval(u->speech);
	wval(u->delprotect);		wval(u->gender);
	wval(u->whitespace);		wval(u->hide_room);
	wval(u->profile);		wval(u->menus);
	wval(u->quit);			wval(u->nukes);
	wval(u->pet.type);		wval(u->keys);
	wval(u->beers);			wval(u->bullets);
	wval(u->splits);		wval(u->editor.visual);
	wval(u->myint1);		wval(u->myint2);
	/*
	   Temp int's for your own use.. just remember what they are.
	   I'm not sure of the effect of changing the variable names.. but
	   it might be bad.. so just remember... what they are..
	*/
	wval(u->myint3);		wval(u->myint4);
	wval(u->myint5);		wval(u->myint6);
	wval(u->myint7);		wval(u->myint8);
	wval(u->myint9);		wval(u->myint10);

	FCLOSE(fp);
	if (err)
	{
		remove("tempfile");
		write_log(1,LOG1,"[ERROR] - Couldn't save userfile '%s' Reason: [%s]\n",u->name,strerror(errno));
		write_user(u,"~CW-~FT An error occurred trying to save your userfile.\n");
		return(0);
	}
	else if (rename("tempfile",filename))
	{
		write_log(1,LOG1,"[ERROR] - Couldn't rename tempfile to '%s.dat' Reason: [%s]\n",u->name,strerror(errno));
		write_user(u,"~CW- ~FGAn error occurred trying to save your userfile.\n");
		return(0);
	}
	return(1);
}

/*
   The function below you should only need if you are upgrading your ramtits
   versions, I'll be making this function obsolete after a couple of months
*/

/*
   Refund the user for any items they might have bought in the old version.
   of course.. only refund them half of what they paid for the item ;-)
*/
void refund_user(User user)
{
	char text2[ARR_SIZE*2];

	switch (user->tvar1)		/* AKA user->weapon */
	{
		case 1 :
		  user->money += 125;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour dagger has been sold for $~CY125~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 2 :
		  user->money += 250;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour short sword has been sold for $~CY250~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 3 :
		  user->money += 500;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour scimitar has been sold for $~CY500~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 4 :
		  user->money += 750;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour mace has been sold for $~CY750~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 5 :
		  user->money += 1500;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour crossbow has been sold for $~CY1500~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 6 :
		  user->money += 1750;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour longsword has been sold for $~CY1750~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 7 :
		  user->money += 2500;	user->tvar1 = 0;
		  sprintf(text2,"~CW- ~FGYour battle axe has been sold for $~CY2500~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		default:
		  break;
	}
	switch (user->tvar2)		/* AKA user->ring */
	{
		case 1 :
		  user->money += 1000;	user->tvar2 = 0;
		  sprintf(text2,"~CW- ~FGYour magical ring has been sold for $~CY1000~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 2 :
		  user->money += 2000;	user->tvar2 = 0;
		  sprintf(text2,"~CW- ~FGYour mystical ring has been sold for $~CY2000~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		default:
		  break;
	}
	switch (user->tvar3)		/* AKA user->armour */
	{
		case 1:
		  user->money += 125;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour studded leather has been sold for $~CY125~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 2 :
		  user->money += 250;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour chain mail has been sold for $~CY250~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 3 : 
		  user->money += 500;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour ring mail has been sold for $~CY500~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 4 :
		  user->money += 750;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour banded mail has been sold for $~CY750~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 5 :
		  user->money += 1500;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour plate mail has been sold for $~CY1500~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 6 :
		  user->money += 1750;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour field plate has been sold for $~CY1750~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		case 7 :
		  user->money += 2500;	user->tvar3 = 0;
		  sprintf(text2,"~CW- ~FGYour full plate has been sold for $~CY2500~FG.\n");
		  send_oneline_mail(NULL,user->name,text2);
		  break;
		default:
		  break;
	}
	user->socket = -2;
	strcpy(user->site,user->last_site);
	SaveUser(user,1);
	return;
}

/* see if user is jailed or not. */
int is_jailed(User user)
{
	if (user->status & Jailed) return(1);
	return(0);
}

/* See if user is clone or not. */
int is_clone(User user)
{
	if (user->type == CloneType) return(1);
	return(0);
}

/* See if user is muzzled or not. */
int is_muzzled(User user)
{
	if (user->status & Muzzled) return(1);
	return(0);
}

/*
   Calls upon the specified text, and writes it out to the user. Did this
   in order to cut down on the code a fair amount... Three different types:
   1 = JAILED 2 = CLONE 3 = MUZZLED
*/
void status_text(User user,int type,char *com)
{

	switch (type)
	{
		case 1:
		  writeus(user,wtf,com);
		  return;
		case 2:
		  writeus(user,clnodo,com);
		  return;
		case 3:
		  writeus(user,muzztext,com);
		  return;
	}
	return;
}

/* Display a review buffer for a user if they come out of a menu or something */
void rev_away(User user)
{
	int i,cnt=0;

	/* First go through and see if we need to review. */
	user->ignall = user->ignall_store;
	user->ignall_store = 0;
	for (i=0;i<NUM_LINES;++i)
	{
		if (user->away_buff->conv[i]!=NULL) ++cnt;
	}
	if (cnt == 0) return;
	if (user->ansi_on) sprintf(text,"~CBÄÄÄ¯¯> ~FYWhile you were busy... ~CB<®®ÄÄÄ\n");
	else sprintf(text,"~FT-~CT=~CB> ~FYWhile you were busy... ~CB<~CT=~FT-\n");
	write_user(user,center(text,80));
	if (user->ansi_on) write_user(user,ansi_tline);
	else write_user(user,ascii_tline);
	write_convbuff(user,user->away_buff);
	if (user->ansi_on) write_user(user,ansi_bline);
	else write_user(user,ascii_bline);
	/* now we reset it. */
	clear_convbuff(user->away_buff);
	return;
}

/*
   The following functions will return 1, if the user is in these lists,
   otherwise it'll just return 0, stating that the user, or name isn't
   in the list.
*/
int is_staff(User user)
{
	int i=0;

	for (i = 0; i < MaxStaff; ++i)
	  if (!strcasecmp(staffmember[i],user->name)) return(1);
	return(0);
}

int is_member(User user)
{
	int i=0;

	for (i = 0; i < 50; ++i)
	  if (!strcasecmp(member[i],user->name)) return(1);
	return(0);
}

int disabled_com(char *comword)
{
	int i=0;

	for (i=0;i<20;++i)
	  if (!strcasecmp(disabled[i],comword)) return(1);
	return(0);
}

int is_friend(User user,User u)
{
	int i=0;

	for (i=0;i<50;++i)
	  if (!strcasecmp(user->friends[i],u->name)) return(1);
	return(0);
}

int is_friend_name(User user,char *name)
{
	int i = 0;
	if (user == NULL) return(0);
	for (i = 0; i < 50; ++i)
	  if (!strcasecmp(user->friends[i],name)) return(1);
	return(0);
}

int is_enemy(User user,User u)
{
	int i=0;

	if (user == NULL || u == NULL) return(0);
	for (i=0;i<50;++i)
	  if (!strcasecmp(user->enemies[i],u->name)) return(1);
	return(0);
}

int is_enemy_name(User user,char *name)
{
	int i = 0;
	if (user == NULL) return(0);
	for (i = 0; i < 50; ++i)
	  if (!strcasecmp(user->enemies[i],name)) return(1);
	return(0);
}

int is_allowed(User user,char *comword)
{
	int i = 0;

	for (i = 0; i < 10; ++i)
	  if (!strcasecmp(user->allowcom[i],comword)) return(1);
	return(0);
}

int is_denied(User user,char *comword)
{
	int i=0;

	for (i=0;i<10;++i)
	  if (!strcasecmp(user->denycom[i],comword)) return(1);
	return(0);
}

/* This determines if a user is busy or not.. */
int is_busy(User user)
{

	if (user == NULL) return(0);
	if (user->editor.editing || user->misc_op || user->store_op
	  || user->bank_op || user->game_op || user->mail_op || user->set_op
	  || user->afk || user->triv_mode || user->ignall) return(1);
	return(0);
}

/* Does the user exist? */
int user_exists(char *name)
{
	char filename[FSL];

	strtolower(name);
	name[0] = toupper(name[0]);
	sprintf(filename,"%s/%s.dat",UserDir,name);
	if (!file_exists(filename)) return(0);
	return(1);
}

/* Strips white spaces and chars < 33 from the beginning of a string. */
void white_space(char text[])
{
	if (!text[0]) return;
	while (text[0]<33)
	{
		if (!text[0]) return;
		strcpy(&text[0],&text[1]);
	}
}

/*
   Check to see whether a site is banned, screened, etc, and which type of
   ban it is..
*/
int is_banned(char *site,int type)
{
	Bans b;

	strtolower(site);
	for_bans(b)
	{
		if (b->type != type) continue;
		if (istrstr(site,b->who)) return(1);
	}
	return(0);
}

/*
   Get words from sentence. This function prevents the words in the
   sentence from writing off the end of a word array element. This is
   difficult to do with sscanf() hence I use this function instead.
*/
int wordfind(char *inpstr)
{
	int wn,wpos;

	wn = wpos = 0;
	do
	{
		while(*inpstr<33) if (!*inpstr++) return(wn);
		while(*inpstr>32 && wpos<WordLen-1)
		{
			word[wn][wpos] = *inpstr++;
			wpos++;
		}
		word[wn][wpos] = '\0';
		wn++;  wpos = 0;
	} while (wn<MaxWords);
	return(wn-1);
}

int wordshift(char *inpstr)
{
	int wn,wpos;

	wn = 1; wpos = 0;
	do
	{
		while(*inpstr<33) if (!*inpstr++) return(wn);
		while(*inpstr>32 && wpos<WordLen-1)
		{
			word[wn][wpos] = *inpstr++;
			wpos++;
		}
		word[wn][wpos] = '\0';
		wn++;  wpos = 0;
	} while (wn<MaxWords);
	return(wn-1);
}

/* clear word array etc. */
void clear_words(void)
{
	int w;

	for(w = 0 ; w < MaxWords ; ++w) word[w][0] = '\0';
	word_count = 0;
}

/* yes or no */
int yn_check(char *wd)
{

	if (!strcmp(wd,"YES")) return(1);
	if (!strcmp(wd,"NO")) return(0);
	return(-1);
}

/* on or off? */
int onoff_check(char *wd)
{

	if (!strcmp(wd,"ON")) return(1);
	if (!strcmp(wd,"OFF")) return(0);
	return(-1);
}

/* Record speech and emotes in the room. */
void record(Room rm,char *str)
{
	if (rm->no_buff == 1) return;
	addto_convbuff(rm->revbuff,str);
}

/* Records tells and pemotes sent to the user. */
void record_tell(User user,char *str)
{
	addto_convbuff(user->tbuff,str);
}

/* Records tells and pemotes sent to the user. */
void record_away(User user,char *str)
{
	addto_convbuff(user->away_buff,str);
}

/* Record wizard messages between one another */
void record_wiz(char *str)
{
	addto_convbuff(wizbuff,str);
}

/* Records most messages where write_room(NULL..) occurs. */
void record_shout(char *str)
{
	addto_convbuff(shoutbuff,str);
}

/* Record to the debug buffer */
void RecordDebug(char *str)
{

	if (debugbuff == NULL) return;
	if (debugbuff->conv[debugbuff->count] != NULL)
	{
		free(debugbuff->conv[debugbuff->count]);
		debugbuff->conv[debugbuff->count] = NULL;
	}
	debugbuff->conv[debugbuff->count] = strdup(str);
	debugbuff->count = (++(debugbuff->count))%15;
}

/* Records logins. */
void RecordLogin(char *str)
{

	if (loginbuff == NULL) return;
	if (loginbuff->conv[loginbuff->count] != NULL)
	{
		free(loginbuff->conv[loginbuff->count]);
		loginbuff->conv[loginbuff->count] = NULL;
	}
	loginbuff->conv[loginbuff->count] = strdup(str);
	loginbuff->count = (++(loginbuff->count))%10;
}

/* Records speech in the channels. */
void record_channel(char *str)
{
	addto_convbuff(channelbuff,str);
}

/*
   See if user has unread mail, mail file has last read time on its
   first line.
*/
int has_unread_mail(User user)
{
	char filename[FSL];

	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
	if (!file_exists(filename)) return(0);
	else return(1);
}

/* Date string for board messages, mail, .who */
char *long_date(int which)
{
	static char dstr[80];
	char ttype[5];
	int hour;

	hour = 0;
	if (thour >= 12)
	{
		(thour>12)?(hour = (int)thour-12):(hour = 12);
		strcpy(ttype,"p.m.");
	}
	else
	{
		(thour == 0)?(hour = 12):(hour = (int)thour);
		strcpy(ttype,"a.m.");
	}
	if (which == 1) sprintf(dstr,"on %s %s %d, %d at %02d:%02d %s",day[twday],month[tmonth],tmday,tyear,hour,tmin,ttype);
	else if (which == 2) sprintf(dstr,"[%02d/%02d %02d:%02d %s]",tmonth+1,tmday,hour,tmin,ttype);
	else sprintf(dstr," ~CB[~FM%s %s %d, %d at~FT %02d:%02d %s~CB]",day[twday],month[tmonth],tmday,tyear,hour,tmin,ttype);
	return(dstr);
}

/*
   is the e-mail addy they supplied actually valid? - Based from Moenuts
   Mike Irving.
*/
int valid_addy(User user,char *addy,int verbose)
{

	if (contains_swearing(addy))
	{
		if (verbose) write_user(user,"~CB[~CRError~CB] - ~FGE-mail addy contained swearing.\n");
		return(0);
	}
	if (istrstr(addy,"localhost"))
	{
		if (verbose) write_user(user,"~CB[~CRError~CB] - ~FGE-mail addy can't be sent to localhost.\n");
		return(0);
	}
	if (istrstr(addy,"root@"))
	{
		if (verbose) write_user(user,"~CB[~CRError~CB] - ~FGE-mail addy can't be sent to root.\n");
		return(0);
	}
	if (addy[0] == '@')
	{
		if (verbose) write_user(user,"~CB[~CRError~CB] - ~FGE-mail addy didn't contain a username.\n");
		return(0);
	}
	if (!strstr(addy,"@"))
	{
		if (verbose) write_user(user,"~CB[~CRError~CB] - ~FGInvalid e-mail address format.\n");
		return(0);
	}
	if (strstr(addy,"@") && !strstr(addy,"."))
	{
		if (verbose) write_user(user,"~CB[~CRError~CB] - ~FGInvalid e-mail address format.\n");
		return(0);
	}
	if (verbose) write_user(user,"~CW- ~FTE-mail address passed the check...\n");
	return(1);
}

/* Does the file exist? */
int file_exists(char *fname)
{
	FILE *fp;

	if ((fp = fopen(fname,"r")) == NULL) return(0);
	FCLOSE(fp);
	return(1);
}

/* Generates a random number */
int intrand(int max)
{
	int n,mask;

	for (mask = 1 ; mask<max ; max *= 2);
	mask -= 1;
	do
	{
		n = random()&mask;
	} while (n >= max);
	return(n);
}

/*
   Get a random number... This works better then the rand() C function..
   because it's actually RANDOM! ;-) Works better then the above.  Thanks
   to Arny for giving me hints on how to do this..
*/
int ran(unsigned long ranamnt)
{
	unsigned long amnt = 0;
	unsigned long mseed = 0;
	static int which;

	which += 4;
	if (which >= 35000) which = 4;
	srand(which);
	mseed = (1+(int) (100000*rand()) % 25565);
	srand((++mseed)+time(0)*2); /* Get something truly random ;-) */
	/* Use a do {} while() cuz i've found it works better. :-) */
	do
	{
		srand((++mseed)+time(0)*2);
		amnt = rand() % (ranamnt+1);

	} while (amnt>ranamnt || amnt == 0);
	return(amnt);
}

/* Count the lines in a file */
int count_lines(char *fname)
{
	int lines,c;
	FILE *fp;

	lines = 0;
	if ((fp = fopen(fname,"r")) == NULL) return(0);
	c = getc(fp);
	while (!(feof(fp)))
	{
		if (c == '\n') lines++;
		c = getc(fp);
	}
	FCLOSE(fp);
	return(lines);
}

/*
   Checks to see if the *format is set properly.. returns -1 if not, 0
   if the format isn't equal to todays date, and 1 if the format is.
*/
int is_bday_today(char *format)
{
	char temp[80],temp2[80];
	int bmonth,bday;

	bmonth = bday = 0;
	if (!strlen(format)) return(-1);
	format = ColourStrip(format);
	if (format[2] != '/') return(-1);
	strncpy(temp,format,2);
	format += 3;
	strncpy(temp2,format,strlen(format));
	bmonth = atoi(temp);
	bday = atoi(temp2);
	if (bmonth == tmonth+1 && bday == tmday) return(1);
	return(0);
}

/*
   Creates a birthday file.. gets displayed at the login if it turns out
   that someone is actually celebrating a birthday :-)
*/
void create_bday_file(User user)
{
	FILE *fp;
	char filename[FSL],line[151];

	sprintf(text,"~CW - ~CB[ ~FGIt is~RD %s's ~FGbirthday today :-)~CB ]\n",user->name);
	/*
	   First go through the file and make sure that the user
	   isn't already in the birthday file.
	*/
	sprintf(filename,"%s/%s",MiscDir,BirthDays);
	fp = fopen(filename,"r");
	if (fp != NULL)
	{
		fgets(line,150,fp);
		while (!(feof(fp)))
		{
			if (istrstr(line,user->name))
			{
				FCLOSE(fp);
				return;
			}
			fgets(line,150,fp);
		}
		FCLOSE(fp);
	}
	/* It doesn't exist... so we add it. */
	if ((fp = fopen(filename,"a")))
	{
		fputs(center(text,80),fp);
		FCLOSE(fp);
	}
	return;
}

/*
   checks to see if any of a users friends are coming on, and
   if so lets them know.. unless they are ignoring alerts..
*/
void friend_check(User user,int type)
{
	User u;

	if (type == 1) sprintf(text,"\07~CB[~FTFriend Alert~CB]~CM %s~FG has logged on.\n",user->recap);
	else if (type == 2) sprintf(text,"\07~CB[~FTFriend Alert~CB]~CM %s~FG has logged off.\n",user->recap);
	for_users(u)
	{
		if (u->login || u == NULL || u->type == CloneType) continue;
		if ((u->ignore & Friends) || (u->ignore & Alert)) continue;
		if (is_friend(u,user))
		{
			if (is_busy(u) && !u->afk)
			{
				record_away(u,text);
				continue;
			}
			write_user(u,text);
			record_tell(u,text);
		}
	}
}

/* waiting check..notifies a user when the person they're waiting for logs on */
void wait_check(User user)
{
	User u;

	for_users(u)
	{
		if (u->login || u == NULL || u->type == CloneType) continue;
		if (!strcasecmp(u->waiting,user->name))
		{
			u->waiting[0] = '\0';
			writeus(u,"\07~CB[~CRWAIT~CB] - ~FR%s~FT has entered the talker.\n",user->recap);
			writeus(user,"~FG%s~FM has been waiting for you to come online.\n",u->recap);
			return;
		}
	}
}

/*
   Removes the files that a user has paged via system commands, ie .finger
   etc... this routine is called at the end of more() and in the misc_ops
*/
void RemoveSysfiles(User user)
{
	char filename[FSL];

	if (user == NULL) return;
	sprintf(filename,"tempfiles/%s.URL",user->name);
	if (file_exists(filename)) unlink(filename);
	filename[0] = '\0';
	sprintf(filename,"tempfiles/%s.F",user->name);
	if (file_exists(filename)) unlink(filename);
	filename[0] = '\0';
	sprintf(filename,"tempfiles/%s.W",user->name);
	if (file_exists(filename)) unlink(filename);
	filename[0] = '\0';
	sprintf(filename,"tempfiles/%s.NS",user->name);
	if (file_exists(filename)) unlink(filename);
	filename[0] = '\0';
	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,user->name);
	if (file_exists(filename)) unlink(filename);
	filename[0] = '\0';
	return;
}

/* Returns 1 if the string contains the $vars and 0 if not.. */
int IsUserVar(char *str)
{
int i;

	str = ColourStrip(str);
	for (i = 0; i < NumVars; ++i)
	{
		if (istrstr(str,user_vars[i])) return(1);
	}
	return(0);
}

/* writes a block of memory to a file. */
int xwrite(FILE *fp,char *buff,int size)
{
	int bytes;

	bytes = fwrite(buff,1,size,fp);
	if (bytes == -1)
	{
		return(0);
	}
	if (bytes != size)
	{
		return(0);
	}
	return(1);
}

/* reads a block of memory from a file. */
int xread(FILE *fp,char *buff,int size)
{
	int bytes;

	bytes = fread(buff,1,size,fp);
	if (bytes == -1)
	{
		return(0);
	}
	if (bytes != size)
	{
		return(0);
	}
	if (ferror(fp)) return(0);
	return(1);
}

int intcmp(const void *v1,const void *v2)
{
	return (*(int *)v1 - *(int *)v2);
}

int comp(const void *s1,const void *s2)
{
	return (strcmp(*(char **)s1,*(char **)s2));
}

/*
   The following functions help prevent portfuckers to kill your talker,
   thanks to Cygnus for this.
*/
int InConnlist(char *site)
{
	int z=0;

	for (z = 0 ; z<20 ; ++z)
	{
		if (!strcmp(connlist[z].site,site)) return(z);
	}
	return(-1);
}

int FindFreeConnslot(void)
{
	int z,found,lowest,lp;

	z = found = lp = 0;  lowest = 1;
	for (z = 0 ; z<20 ; ++z)
	{
		if (connlist[z].connections == 0) return(z);
	} z = 0;
	/* all slots used.. bump lowest one. */
	for (z = 19 ; z >= 0 ; --z)
	{
		if (connlist[z].connections <= lowest)
		{
			lp = z;
			lowest = connlist[z].connections;
		}
	}
	return(lp);
}

void ClearConnlist(void)
{
	int z=0;

	if (check_time<60)
	{
		check_time += heartbeat;
		return;
	}
	for (z = 0 ; z < 20 ; ++z)
	{
		connlist[z].connections = 0;
		connlist[z].site[0] = '\0';
	}
	check_time = 0;
}

/*
   This will check to make sure a user's variables when loading their
   userfiles are proper, and the userfile isn't corrupted.. if it
   encounters a bad value.. it will reset it accordingly
*/
void check_bad_vars(User u)
{

	if (u->money<0 || u->money>10000000) u->money = 500;
	if (u->dep_money<0 || u->dep_money>10000000) u->dep_money = 500;
	if (u->chips<0 || u->chips>10000000) u->chips = 500;
	if (u->dep_chips<0 || u->dep_chips>10000000) u->dep_chips = 500;
	if (u->icq<0) u->icq = 0;
	if (u->level<0 || u->level>OWNER) u->level = USER;
	if (u->status<0) u->status = 0;
	if (u->ignore<0) u->ignore = 0;
	if (u->tic_win<0 || u->tic_win>65000) u->tic_win = 0;
	if (u->tic_lose<0 || u->tic_lose>65000) u->tic_lose = 0;
	if (u->tic_draw<0 || u->tic_draw>65000) u->tic_draw = 0;
	if (u->lose<0 || u->lose>65000) u->lose = 0;
	if (u->draw<0 || u->draw>65000) u->draw = 0;
	if (u->win<0 || u->win>65000) u->win = 0;
	if (u->logons<0 || u->logons>65000) u->logons = 400;
	if (u->age<0 || u->age>100) u->age = 0;
	if (u->examined<0 || u->examined>65000) u->examined = 0;
	if (u->condoms<0 || u->condoms>65000) u->condoms = 0;
	if (u->c4_win<0 || u->c4_win>65000) u->c4_win = 0;
	if (u->c4_lose<0 || u->c4_lose>65000) u->c4_lose = 0;
	if ((u->rows<15 && u->rows!=0) || u->rows>150) u->rows = 23;
	if ((u->cols<50 && u->cols!=-1) || u->cols>150) u->cols = 80;
	if (u->atmos>MaxAtmos || u->atmos<0) u->atmos = 0;
	if (u->chess_wins<0 || u->chess_wins>65000) u->chess_wins = 0;
	if (u->chess_lose<0 || u->chess_lose>65000) u->chess_lose = 0;
	if (u->chess_draws<0 || u->chess_draws>65000) u->chess_draws = 0;
	if (u->bship_wins<0 || u->bship_wins>65000) u->bship_wins = 0;
	if (u->bship_lose<0 || u->bship_lose>65000) u->bship_lose = 0;
	if (u->bullets<0 || u->bullets>65000) u->bullets = 0;
	if (u->beers<0 || u->beers>65000) u->beers = 0;
	if (strlen(u->desc) > sizeof(u->desc)-1) strncpy(u->desc,u->desc,sizeof(u->desc)-1);
	if (strlen(u->in_phr) > sizeof(u->in_phr)-1) strncpy(u->in_phr,u->in_phr,sizeof(u->in_phr)-1);
	if (strlen(u->out_phr) > sizeof(u->out_phr)-1) strncpy(u->out_phr,u->out_phr,sizeof(u->out_phr)-1);
	if (strlen(u->email) > sizeof(u->email)-1) strncpy(u->email,u->email,sizeof(u->email)-1);
	if (strlen(u->recap) > sizeof(u->recap)-1) strncpy(u->recap,u->recap,sizeof(u->recap)-1);
	if (strlen(u->predesc) > sizeof(u->predesc)-1) strncpy(u->predesc,u->predesc,sizeof(u->predesc)-1);
	if (strlen(u->lev_rank) > sizeof(u->lev_rank)-1) strncpy(u->lev_rank,u->lev_rank,sizeof(u->lev_rank)-1);
	if (strlen(u->webpage) > sizeof(u->webpage)-1) strncpy(u->webpage,u->webpage,sizeof(u->webpage)-1);
	if (strlen(u->bday) > sizeof(u->bday)-1) strncpy(u->bday,u->bday,sizeof(u->bday)-1);
	if (strlen(u->pet.name) > sizeof(u->pet.name)-1) strncpy(u->pet.name,u->pet.name,sizeof(u->pet.name)-1);
	if (strlen(u->login_msg) > sizeof(u->login_msg)-1) strncpy(u->login_msg,u->login_msg,sizeof(u->login_msg)-1);
	if (strlen(u->logout_msg) > sizeof(u->logout_msg)-1) strncpy(u->logout_msg,u->logout_msg,sizeof(u->logout_msg)-1);
	if (strlen(u->bfafk) > sizeof(u->bfafk)-1) strncpy(u->bfafk,u->bfafk,sizeof(u->bfafk)-1);
	if (strlen(u->sstyle) > sizeof(u->sstyle)-1) strncpy(u->sstyle,u->sstyle,sizeof(u->sstyle)-1);
	if (strlen(u->tzone) > sizeof(u->tzone)-1) strncpy(u->tzone,u->tzone,sizeof(u->tzone)-1);
	if (strlen(u->autoexec) > sizeof(u->autoexec)-1) strncpy(u->autoexec,u->autoexec,sizeof(u->autoexec)-1);

	
	if (((get_user_full(NULL,u->married_to)) == NULL)
	  && (!istrstr(u->married_to,"Noone")
	  || !istrstr(u->married_to,"Divorce")))
	{
		strcpy(u->married_to,"Noone");
	}
	/* Should be enough...*/
	return;
}

/* The function that sends mail to users when the editor is used. */
void send_mail(User user,char *to)
{
	User u = NULL;
	Editor edit = get_editor(user);
	FILE *infp,*outfp,*sfp,*sfp2,*mfp;
	char *c,d,filename[FSL],mailfile[FSL];
	struct stat fb;
	int fsize,err,on,i,forward;

	mfp = NULL;
	fsize = err = i = forward = on = 0;
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW- ~FG There was an error in the mail delivery.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in send_mail(). 1\n");
		return;
	}
	if ((u = get_user(to)) == NULL)
	{
		if ((u = create_user()) == NULL)
		{
			write_log(1,LOG1,"[ERROR] - Failed to create a temp. user object in send_mail().\n");
			write_user(user,"~CW- ~FGThere was an error in the mail delivery.\n");
			return;
		}
		strcpy(u->name,to);
		err = LoadUser(u,to);
		if ((err == 0) || (err == -1))
		{
			writeus(user,"~CW- ~FGThere was an error in the mail delivery. ~CB[~FTNo such user ~FY%s\n",to);
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
	}
	else on = 1;
	/*
	  This is just basically a check to see if the user we're sending
	  s-mail to wants their new messages forwarded to them.. if so then
	  we send it..
	*/
	if (u->autofwd == 0)
	{
		forward = 0;
	}
	else
	{
		if (u->email[0] == '\0') forward = 0;
		else
		{
			forward = 1;
			sprintf(mailfile,"tempfiles/%s.mail",u->name);
			if ((mfp = fopen(mailfile,"w")) == NULL)
			{
				write_user(user,"~CW- ~FGThere was an error in the mail delivery.\n");
				write_log(0,LOG1,"[ERROR] - Couldn't open '%s.mail' in send_mail()\n",u->name);
				forward = 0;
				return;
			}
			fprintf(mfp,"From:    %s <%s>\n",TalkerName,TalkerEmail);
			fprintf(mfp,"To:      %s <%s>\n",u->name,u->email);
			fprintf(mfp,"Subject: New s-mail from %s.\n",user->name);
			fprintf(mfp,"\n\n");
		}
	}
	/* Put new mail into the tempfile, as well as the forward file */
	switch (user->chksmail)
	{
		case 1 :
		  fprintf(outfp," ~FYWriter~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",user->invis>1?user->temp_recap:user->recap,long_date(0));
		  fprintf(outfp,center("~FT-~CT=~CB> ~FRFriend S-mail ~CB<~CT=~FT-\n",80));
		  if (forward == 1)
		  {
			fprintf(mfp," Writer: %s\n Written on: %s\n",user->name,ColourStrip(long_date(0)));
			fprintf(mfp,center("-=[ Friend S-mail ]=-\n",80));
		  }
		  break;
		case 2 :
		  fprintf(outfp," ~FYWriter~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",user->invis>1?user->temp_recap:user->recap,long_date(0));
		  fprintf(outfp,center("~FT-~CT=~CB> ~FRStaff S-mail ~CB<~CT=~FT-\n",80));
		  if (forward == 1)
		  {
			fprintf(mfp," Writer: %s\n Written on: %s\n",user->name,ColourStrip(long_date(0)));
			fprintf(mfp,center("-=[ Staff S-mail ]=-\n",80));
		  }
		  break;
		case 3 :
		  fprintf(outfp," ~FYWriter~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",user->invis>1?user->temp_recap:user->recap,long_date(0));
		  fprintf(outfp,center("~FT-~CT=~CB> ~FRAll users s-mail ~CB<~CT=~FT-\n",80));
		  if (forward == 1)
		  {
			fprintf(mfp," Writer: %s\n Written on: %s\n",user->name,ColourStrip(long_date(0)));
			fprintf(mfp,center("-=[ All users s-mail ]=-\n",80));
		  }
		  break;
		default:
		  fprintf(outfp," ~FYWriter~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",user->invis>1?user->temp_recap:user->recap,long_date(0));
		  if (forward == 1)
		  {
			fprintf(mfp," Writer: %s\n Written on: %s\n",user->name,ColourStrip(long_date(0)));
		  }
		  break;
	}
	fputs(ascii_line,outfp);
	if (forward == 1)
	{
		fputs(ColourStrip(ascii_line),mfp);
	}
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while (c != user->editor.malloc_end) putc(*c++,outfp);
		if (forward == 1)
		{
			/* Gotta redo from start for mfp */
			c = user->editor.malloc_start;
			while (c != user->editor.malloc_end) putc(*c++,mfp);
		}
	}
	else
	{
		i = 0;
		while (i < MaxLines)
		{
			if (edit->line[i][0] != '\0')
			{
				fprintf(outfp,"%s",edit->line[i]);
				if (forward == 1) fprintf(mfp,"%s",edit->line[i]);
				if (edit->line[i][strlen(edit->line[i])-1] != '\n')
				{
					fputs("\n",outfp);
					if (forward == 1) fputs("\n",mfp);
				}
			}
			i++;
		}
	}
	fputs(ascii_line,outfp);
	fputs("\n",outfp);
	if (forward == 1)
	{
		fputs(ColourStrip(ascii_line),mfp);
		fputs("\n",mfp);
		/* We're basically done now, put our signature, and close */
		fprintf(mfp,signature,TalkerName,TalkerAddy,TalkerEmail,TalkerName,TalkerEmail);
		FCLOSE(mfp);
		send_email(u->email,mailfile);
		write_log(1,LOG3,"[AutoForward] - S-mail From: [%s] To: [%s]\n",user->name,u->name);
	}
	/*
	  Copy current new mail file into tempfile if it exists.. I switched
	  the order of everything around, so this way we get the most recent
	  messages first, which makes for easier sorting and what not.
	*/
	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,to);
	if ((infp = fopen(filename,"r")) != NULL)
	{
		/* Copy it in */
		d = getc(infp);
		while (!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	if ((on == 0) && (u != NULL))
	{
		destruct_user(u);
		destructed = 0;
	}
	rename("tempfile",filename);
	if (!user->chksmail && !user->tvar1)
	{
		writeus(user,"~CW- ~FGYou give your letter to ~FT%s~FG to the ~FR%s~FG Postal service.\n",to,TalkerRecap);
		write_log(1,LOG1,"[S-MAIL] - User: [%s] Recipient: [%s]\n",user->name,to);
	}
	if (user == NULL) return;
	u = get_user(to);
	if (u != NULL) writeus(u,"\07~CW-~FT The ramtits postal express drops off a letter to you.\n");
	/* Now we write our sent s-mail file */
	if (user->tvar2 == 1 || user->chksmail) return;
	filename[0] = '\0';
	sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
	if (stat(filename,&fb) == -1) fsize = 0;
	else fsize = fb.st_size;
	if (fsize > MaxSMbytes)
	{
		write_user(user,"~CW - ~FTUnable to write to your sent s-mail file.\n");
		writeus(user,"~CW - ~FGThe reason:~FT Your sent s-mail box is allocating ~FR%d~FT bytes out of ~FR%d MAX~FT.\n",fsize,MaxSMbytes);
		return;
	}
	if ((sfp2 = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW - ~FTThere was an error writing to your sent s-mail box.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in send_mail(). 2\n");
		return;
	}
	fprintf(sfp2," ~FYWrittenTo~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",to,long_date(0));
	fputs(ascii_line,sfp2);
	if (user->editor.editor == 1)
	{
		c = user->editor.malloc_start;
		while (c != user->editor.malloc_end) putc(*c++,sfp2);
	}
	else
	{
		i = 0;
		while (i<MaxLines)
		{
			if (edit->line[i][0] != '\0')
			{
				fprintf(sfp2,"%s",edit->line[i]);
				if (edit->line[i][strlen(edit->line[i])-1] != '\n') fputs("\n",sfp2);
			}
			i++;
		}
	}
	fputs(ascii_line,sfp2);
	fputs("\n",sfp2);
	sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
	if ((sfp = fopen(filename,"r")) != NULL)
	{
		d = getc(sfp);
		while (!feof(sfp))
		{
			putc(d,sfp2);
			d = getc(sfp);
		}
		FCLOSE(sfp);
	}
	FCLOSE(sfp2);
	rename("tempfile",filename);
	write_user(user,center("~FT-~CT=~CB>~FR To view your sent s-mail type ~CW'~CR.rsent~CW' ~CB<~CT=~FT-\n",80));
}

/* The function that sends mail to users if a string is given. */
void send_oneline_mail(User user,char *to,char *str)
{
	User u;
	FILE *infp,*outfp,*sfp,*sfp2,*mfp;
	char filename[FSL],mailfile[FSL],d;
	struct stat fb;
	int fsize,err,on,i,forward;

	fsize = err = on = i = forward = 0;
	mfp = NULL;
	if ((outfp = fopen("tempfile","w")) == NULL)
	{
		if (user != NULL) write_user(user,"~CW- ~FG There was an error in the mail delivery.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in send_mail(). 1\n");
		return;
	}
	/* Check to see if we are forwarding this message */
	if ((u = get_user(to)) == NULL)
	{
		if ((u = create_user()) == NULL)
		{
			write_log(1,LOG1,"[ERROR] - Failed to create a temp. user object in send_mail().\n");
			if (user != NULL) write_user(user,"~CW- ~FGThere was an error in the mail delivery.\n");
			return;
		}
		strcpy(u->name,to);
		err = LoadUser(u,to);
		if (err == 0 || err == -1)
		{
			if (user != NULL) write_user(user,"~CW- ~FGThere was an error in the mail delivery.\n");
			destruct_user(u);
			destructed = 0;
			return;
		}
		on = 0;
	}
	else on = 1;
	/*
	  This is just basically a check to see if the user we're sending
	  s-mail to wants their new messages forwarded to them.. if so then
	  we send it.
	*/
	if (u->autofwd == 0)
	{
		forward = 0;
	}
	else
	{
		if (u->email[0] == '\0') forward = 0;
		else
		{
			forward = 1;
			sprintf(mailfile,"tempfiles/%s.mail",u->name);
			if ((mfp = fopen(mailfile,"w")) == NULL)
			{
				if (user != NULL) write_user(user,"~CW- ~FGThere was an error in the mail delivery.\n");
				write_log(0,LOG1,"[ERROR] - Couldn't open '%s.mail' in send_mail()\n",u->name);
				forward = 0;
				return;
			}
			fprintf(mfp,"From:    %s <%s>\n",TalkerName,TalkerEmail);
			fprintf(mfp,"To:      %s <%s>\n",u->name,u->email);
			fprintf(mfp,"Subject: New s-mail from %s.\n",user == NULL?TalkerName:user->name);
			fprintf(mfp,"\n\n");
		}
	}
	/* Put new mail into the tempfile, as well as the forward file */
	if (user == NULL) fprintf(outfp," ~FYWriter~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",TalkerRecap,long_date(0));
	else fprintf(outfp," ~FYWriter~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",user->invis>1?user->temp_recap:user->recap,long_date(0));
	if (forward == 1)
	{
		fprintf(mfp," Writer: %s\n Written on: %s\n",user == NULL?TalkerMailName:user->name,ColourStrip(long_date(0)));
	}

	fputs(ascii_line,outfp);
	fputs(str,outfp);
	fputs(ascii_line,outfp);
	fputs("\n",outfp);
	if (forward == 1)
	{
		fputs(ColourStrip(ascii_line),mfp);
		fputs(ColourStrip(str),mfp);
		fputs(ColourStrip(ascii_line),mfp);
		fputs("\n",mfp);
		/* We're basically done now, put our signature, and close */
		fprintf(mfp,signature,TalkerName,TalkerAddy,TalkerEmail,TalkerName,TalkerEmail);
		FCLOSE(mfp);
		send_email(u->email,mailfile);
		write_log(1,LOG3,"[AutoForward] - S-mail From: [%s] To: [%s]\n",user == NULL?TalkerName:user->name,u->name);
	}
	/*
	  Copy current new mail file into tempfile if it exists.. I switched
	  the order of everything around, so this way we get the most recent
	  messages first, which makes for easier sorting and what not.
	*/
	sprintf(filename,"%s/%s/%s.NM",UserDir,UserMesg,to);
	if ((infp = fopen(filename,"r")) != NULL)
	{
		/* Copy it in */
		d = getc(infp);
		while (!feof(infp))
		{
			putc(d,outfp);
			d = getc(infp);
		}
		FCLOSE(infp);
	}
	FCLOSE(outfp);
	if ((on == 0) && (u != NULL))
	{
		destruct_user(u);
		destructed = 0;
	}
	rename("tempfile",filename);
	if (user != NULL && !user->tvar1)
	{
		writeus(user,"~CW- ~FGYou give your letter to ~FT%s~FG to the ~FR%s~FG Postal service.\n",to,TalkerRecap);
		write_log(1,LOG1,"[S-MAIL] - User: [%s] Recipient: [%s]\n",user->name,to);
	}
	if (user == NULL) return;
	u = get_user(to);
	if (u != NULL) writeus(u,"\07~CW-~FT The ramtitss postal express drops off a letter to you.\n");
	/* Now we write our sent s-mail file */
	if (user->tvar2 == 1 || user->chksmail) return;
	filename[0] = '\0';
	sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
	if (stat(filename,&fb) == -1) fsize = 0;
	else fsize = fb.st_size;
	if (fsize > MaxSMbytes)
	{
		write_user(user,"~CW - ~FTUnable to write to your sent s-mail file.\n");
		writeus(user,"~CW - ~FGThe reason:~FT Your sent s-mail box is allocating ~FR%d~FT bytes out of ~FR%d MAX~FT.\n",fsize,MaxSMbytes);
		return;
	}
	if ((sfp2 = fopen("tempfile","w")) == NULL)
	{
		write_user(user,"~CW - ~FTThere was an error writing to your sent s-mail box.\n");
		write_log(0,LOG1,"[ERROR] - Couldn't open tempfile in send_mail(). 2\n");
		return;
	}
	fprintf(sfp2," ~FYWrittenTo~CB: ~FR%s\n ~FYWritten on~CB: ~FR%s\n",to,long_date(0));
	fputs(ascii_line,sfp2);
	fputs(str,sfp2);
	if (str[strlen(str)-1] != '\n') fputs("\n",sfp2);
	fputs(ascii_line,sfp2);
	fputs("\n",sfp2);
	sprintf(filename,"%s/%s/%s.S",UserDir,UserMesg,user->name);
	if ((sfp = fopen(filename,"r")) != NULL)
	{
		d = getc(sfp);
		while (!feof(sfp))
		{
			putc(d,sfp2);
			d = getc(sfp);
		}
		FCLOSE(sfp);
	}
	FCLOSE(sfp2);
	rename("tempfile",filename);
	write_user(user,center("~FT-~CT=~CB>~FR To view your sent s-mail type ~CW'~CR.rsent~CW' ~CB<~CT=~FT-\n",80));
}

int save_bans(int type)
{
	Bans b;
	FILE *fp;
	char filename[FSL];

	switch(type)
	{
		case SiteBan:
		  sprintf(filename,"%s/%s",DataDir,BanSites);
		  break;
		case NewSBan:
		  sprintf(filename,"%s/%s",DataDir,BanNew);
		  break;
		case WhoSBan:
		  sprintf(filename,"%s/%s",DataDir,BanWho);
		  break;
		case UserBan:
		  sprintf(filename,"%s/%s",DataDir,BanUsers);
		  break;
		case Screen :
		  sprintf(filename,"%s/%s",DataDir,SiteScreen);
		  break;
	}
	fp = fopen("tempfile","w");
	if (fp == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Couldn't open tempfile for writing bans (type %d)\n"
				 "          Reason: %s\n",type,strerror(errno));
		return(-1);
	}
	for_bans(b)
	{
		if (b->type != type) continue;
		fprintf(fp,"%s %ld %d %s %s\n",b->who,b->time_banned,b->ban_length,b->by,b->reason);
	}
	FCLOSE(fp);
	if (rename("tempfile",filename) == -1)
	{
		write_log(1,LOG1,"[ERROR] - Couldn't rename 'tempfile' to '%s'\n"
				 "	    Reason: %s\n",filename,strerror(errno));
		return(-1);
	}
	return(0);
}

void save_all_bans(void)
{
	int	success, all_success = 0;

	success = 0;
	success = save_bans(SiteBan);
	if (success == -1) write_log(1,LOG1,"[ERROR] - Failed to save site bans.\n");
	else ++all_success;
	success = save_bans(NewSBan);
	if (success == -1) write_log(1,LOG1,"[ERROR] - Failed to save new site bans.\n");
	else ++all_success;
	success = save_bans(WhoSBan);
	if (success == -1) write_log(1,LOG1,"[ERROR] - Failed to save who site bans.\n");
	else ++all_success;
	success = save_bans(UserBan);
	if (success == -1) write_log(1,LOG1,"[ERROR] - Failed to save user bans.\n");
	else ++all_success;
	success = save_bans(Screen);
	if (success == -1) write_log(1,LOG1,"[ERROR] - Failed to save screened sites.\n");
	else ++all_success;
	if (all_success != 5)
		write_log(1,LOG1,"[ERROR] - Not all bans saved, review the sys log for more details.\n");

}

/* END OF RAMTITS.C */
