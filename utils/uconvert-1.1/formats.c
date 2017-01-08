/*
  formats.c
    This is the file that will allow the user to decide which type of
    userfiles they are converting from.. Current support formats are:
	RaMTITS		: 1.1.2, 1.1.3
	Amnuts		: not done
	Moenuts		: not done
	TalkerOS	: not done
	NUTS		: not done
    The reason so many of those are not done is because I contacted the
    author's of some of those base codes and asked them to send me their
    load user details function as well as about 10-20 userfiles so that
    I can test, I never received any of them, so I said screw it.. if you
    want it added in, feel free to send me the load_user_details() function
    and 10-20 userfiles that use that format.
    If you have another talker base code.. feel free to send me a patched
    copy of this source file with your load_user() or relevant function
    that loads user files.  To choose the format you're converting from,
    edit the Makefile and define the proper version of the code you're
    currently using.

    Word of warning:  If you have modified your userfile setup from the
		original, this code will not work for you, you'll have
		to make some modifications yourself.
*/

#define UCONVERT_FORMATS_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <structs.h>	
#include <uconvert.proto.h>
#include <formats.proto.h>

User user_first, user_last;

/* Create user object */
User create_user(void)
{
	User user;

	if ((user = (User)malloc(sizeof(struct user_struct))) == NULL)
	{
        	return(NULL);
        }
	/* append into linked list. */
	if (user_first == NULL)
	{
		user_first = user;
		user->prev = NULL;
	}
	else
	{
		user_last->next = user;
		user->prev = user_last;
	}
	user->next = NULL;
	user_last = user;
	/* Init the user structure. */
	user->type = 1;			user->name[0] = '\0';
	user->socket = -1;		user->attempts = 0;
	user->login = 0;		user->port = 0;
	user->site_port = 0;		user->site[0] = '\0';
	user->ip_site[0] = '\0';	user->away_buff = NULL;
	user->tbuff = NULL;
	reset_vars(user);
	return(user);
}

/* Destruct a regular user object. */
void destruct_user(User user)
{

	if (user == NULL) return;
	/* remove from linked list */
	if (user == user_first)
	{
		user_first = user->next;
		if (user == user_last) user_last = NULL;
		else user_first->prev = NULL;
	}
	else
	{
		user->prev->next = user->next;
		if (user == user_last)
		{
			user_last = user->prev;
			user_last->next = NULL;
		}
		else user->next->prev = user->prev;
        }
	free(user);
	user = NULL;
}

void reset_vars(User user)
{
	int i;

	if (user == NULL) return;
	user->pass[0] = '\0';			user->last_site[0] = '\0';
	user->desc[0] = '\0';			user->in_phr[0] = '\0';
	user->out_phr[0] = '\0';		user->email[0] = '\0';
	user->recap[0] = '\0';			user->predesc[0] = '\0';
	user->lev_rank[0] = '\0';		user->webpage[0] = '\0';
	user->bday[0] = '\0';			user->married_to[0] = '\0';
	user->pet.name[0] = '\0';		user->login_msg[0] = '\0';
	user->logout_msg[0] = '\0';		user->bfafk[0] = '\0';
	user->sstyle[0] = '\0';			user->tzone[0] = '\0';
	user->autoexec[0] = '\0';		user->mychar1[0] = '\0';
	user->mychar2[0] = '\0';		user->mychar3[0] = '\0';
	user->mychar4[0] = '\0';		user->mychar5[0] = '\0';
	user->mychar6[0] = '\0';		user->mychar7[0] = '\0';
	user->mychar8[0] = '\0';		user->mychar9[0] = '\0';
	user->mychar10[0] = '\0';

	user->last_login = 0;			user->total_login = 0;
	user->last_login_len = 0;		user->level = 0;
	user->prompt = 0;			user->status = 0;
	user->char_echo = 0;			user->mode = 0;
	user->icq = 0;				user->money = 0;
	user->dep_money = 0;			user->chips = 0;
	user->dep_chips = 0;			user->ignore = 0;
	user->tic_win = 0;			user->tic_lose = 0;
	user->tic_draw = 0;			user->win = 0;
	user->lose = 0;				user->draw = 0;
	user->autofwd = 0;			user->logons = 0;
	user->age = 0;				user->examined = 0;
	user->login_type = 0;			user->condoms = 0;
	user->c4_win = 0;			user->c4_lose = 0;
	user->rows = 23;			user->cols = 80;
	user->atmos = 0;			user->hang_wins = 0;
	user->hang_loses = 0;			user->chess_wins = 0;
	user->chess_lose = 0;			user->chess_draws = 0;
	user->bship_wins = 0;			user->bship_lose = 0;
	user->gshout = 0;			user->gpriv = 0;
	user->gsocs = 0;			user->monitor = 0;
	user->no_fight = 0;			user->icqhide = 0;
	user->no_follow = 0;			user->tvar2 = 0;
	user->editor.editor = 0;		user->tvar3 = 0;
	user->has_room = 0;			user->carriage = 0;
	user->hat = 0;				user->protection = 0;
	user->tvar1 = 0;			user->hide_email = 0;
	user->ansi_on = 0;			user->help = 0;
	user->who = 0;				user->speech = 0;
	user->delprotect = 0;			user->gender = 0;
	user->whitespace = 0;			user->hide_room = 0;
	user->profile = 0;			user->menus = 0;
	user->quit = 0;				user->nukes = 0;
	user->pet.type = 0;			user->keys = 0;
	user->beers = 0;			user->bullets = 0;
	user->splits = 0;			user->myint1 = 0;
	user->myint2 = 0;			user->myint3 = 0;
	user->myint4 = 0;			user->myint5 = 0;
	user->myint6 = 0;			user->myint7 = 0;
	user->myint8 = 0;			user->myint9 = 0;
	user->myint10 = 0;

	for (i = 0; i < 50; ++i)
	{
		user->friends[i][0] = '\0';
		user->enemies[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		user->rmacros[i][0] = '\0';
		user->fmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		user->allowcom[i][0] = '\0';
		user->denycom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 28; ++i)
	{
		user->scut[i].scut = 0;
		user->scut[i].scom[0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
        	user->alias.find[i][0] = '\0';
		user->alias.alias[i][0] = '\0';
	} i = 0;
	user->pet.name[0] = '\0';
	for (i = 0; i < 20; ++i)
	{
        	user->pet.alias[i][0] = '\0';
		user->pet.trigger[i][0] = '\0';
	} i = 0;
	return;
}

/* Convert string to lower case */
void strtolower(char *str)
{
	while(*str)
	{
		*str = tolower(*str);
		str++;
	}
}

#if defined(RAMTITS112) || defined(RAMTITS113)
/*
   Start with RaMTITS 1.1.x userfiles.
*/
int LoadOldUser(User user,char *name)
{
	FILE *fp1, *fp2;
	char file1[256], file2[256], line[2000], backupfile[256];
	long temp1, temp2;
	int  temp3, temp4;

	temp1 = temp2 = temp3 = temp4 = 0;
	reset_vars(user);
	sprintf(file1,"userfiles/%s.C",user->name);
	sprintf(file2,"userfiles/%s.I",user->name);
	if ((fp1 = fopen(file1,"r")) == NULL) return(0);
	if ((fp2 = fopen(file2,"r")) == NULL)
	{
		FCLOSE(fp1);
		return(0);
	}
	fscanf(fp1,"%s",user->pass);
	fscanf(fp1,"%s\n",user->last_site);
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->desc,line,sizeof(user->desc)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->in_phr,line,sizeof(user->in_phr)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->out_phr,line,sizeof(user->out_phr)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->email,line,sizeof(user->email)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->recap,line,sizeof(user->recap)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->predesc,line,sizeof(user->predesc)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->lev_rank,line,sizeof(user->lev_rank)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->webpage,line,sizeof(user->webpage)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->bday,line,sizeof(user->bday)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->married_to,line,sizeof(user->married_to)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	/* This var isn't used any more.. so get rid of it */
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->login_msg,line,sizeof(user->login_msg)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->logout_msg,line,sizeof(user->logout_msg)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->bfafk,line,sizeof(user->bfafk)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->sstyle,line,sizeof(user->sstyle)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->tzone,line,sizeof(user->tzone)-1);
	line[0] = '\0';
	fgets(line,(2000-1),fp1);
	line[strlen(line)-1] = '\0';
	strncpy(user->autoexec,line,sizeof(user->autoexec)-1);
	line[0] = '\0';
	FCLOSE(fp1);
	sprintf(backupfile,"backup/%s.C",user->name);
	rename(file1,backupfile);

	/* numeric stuff */
	fscanf(fp2,"%ld %ld %ld %d %hd\n",&temp1, &temp2,
		&user->last_login_len, &temp4, &user->level);
	fscanf(fp2,"%hd %hd %hd %hd %d\n",&user->prompt, &user->status,
		&user->char_echo, &user->mode, &temp3);
	fscanf(fp2,"%ld %ld %ld %ld %ld\n",&user->icq, &user->money,
		&user->dep_money, &user->chips, &user->dep_chips);
	fscanf(fp2,"%hd %d %d %d %d\n",&user->ignore,&user->tic_win,
		&user->tic_lose, &user->tic_draw, &user->win);
	fscanf(fp2,"%d %d %hd %d %d\n",&user->lose, &user->draw,
		&user->autofwd, &temp3, &temp3);
	fscanf(fp2,"%d %d %d %hd %d\n",&temp3, &user->logons, &temp3,
		&user->age, &user->examined);
	fscanf(fp2,"%hd %d %d %d %hd\n",&user->login_type, &user->condoms,
		&user->c4_win, &user->c4_lose, &user->rows);
	fscanf(fp2,"%hd %hd %d %d %d\n",&user->cols, &user->atmos,
		&user->hang_wins, &user->hang_loses, &user->chess_wins);
	fscanf(fp2,"%d %d %d %d %d\n",&user->chess_lose, &user->chess_draws,
		&temp3, &temp3, &user->bship_wins);
	fscanf(fp2,"%d %hd %hd %hd %d\n",&user->bship_lose, &user->gshout,
		&user->gpriv, &user->gsocs, &temp3);
	fscanf(fp2,"%hd %hd %hd %hd %d\n",&user->monitor, &user->no_fight,
		&user->icqhide, &user->no_follow, &temp3);
	fscanf(fp2,"%d %d %d %hd %hd\n",&user->tvar2, &user->editor.editor,
		&user->tvar3, &user->has_room, &user->carriage);
	fscanf(fp2,"%hd %hd %d %hd %hd\n",&user->hat, &user->protection,
		&user->tvar1, &user->hide_email, &user->ansi_on);
	fscanf(fp2,"%hd %hd %hd %hd %hd\n",&user->help, &user->who,
		&user->speech, &user->delprotect, &user->gender);
	fscanf(fp2,"%hd %hd %hd %hd %hd\n",&user->whitespace, &user->hide_room,
		&user->profile, &user->menus, &user->quit);
	fscanf(fp2,"%hd %d %hd %d %d\n",&user->nukes, &user->pet.type,
		&user->keys, &user->beers, &user->bullets);
	fscanf(fp2,"%hd %d %d %d %d\n",&user->splits, &temp3,
		&temp3, &temp3, &temp3);
	user->last_login  = (time_t)temp1;
	user->total_login = (time_t)temp2;
	FCLOSE(fp2);
	sprintf(backupfile,"backup/%s.I",user->name);
	rename(file2,backupfile);
	load_user_stuff(user,1);
	load_user_stuff(user,2);
	load_user_stuff(user,3);
	load_user_stuff(user,4);
	LoadMacros(user);
	load_default_shortcuts(user);
	return(1);
}
#endif /* RAMTITS112 or RAMTITS113 userfiles */

#if defined(RAMTITS2)

int LoadOldUser(User u,char *name)
{
	FILE *fp;
	char filename[256], backupfile[256];
	int err = 0,version,i = 0;

	if (u == NULL) return(0);
	strtolower(name);
	name[0] = toupper(name[0]);
	sprintf(filename,"userfiles/%s.dat",name);
	fp = fopen(filename,"r");
	if (fp == NULL) return(0); /* user doesn't exist */
	strcpy(u->name,name);

	version = '1';
	rval(version);
	if (err) return(0);
	for (i = 0; i < 50; ++i)
	{
		rbuff(u->friends[i]);
		if (!strcmp(u->friends[i],"#NOTSET#"))
			u->friends[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		rbuff(u->enemies[i]);
		if (!strcmp(u->enemies[i],"#NOTSET#"))
			u->enemies[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		rbuff(u->rmacros[i]);
		if (!strcmp(u->rmacros[i],"#NOTSET#"))
			u->rmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		rbuff(u->fmacros[i]);
		if (!strcmp(u->fmacros[i],"#NOTSET#"))
			u->fmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->allowcom[i]);
		if (!strcmp(u->allowcom[i],"#NOTSET#"))
			u->allowcom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->denycom[i]);
		if (!strcmp(u->denycom[i],"#NOTSET#"))
			u->denycom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->alias.find[i]);
		if (!strcmp(u->alias.find[i],"#NOTSET#"))
			u->alias.find[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->alias.alias[i]);
		if (!strcmp(u->alias.alias[i],"#NOTSET#"))
			u->alias.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		rbuff(u->pet.alias[i]);
		if (!strcmp(u->pet.alias[i],"#NOTSET#"))
			u->pet.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		rbuff(u->pet.trigger[i]);
		if (!strcmp(u->pet.trigger[i],"#NOTSET#"))
			u->pet.trigger[i][0] = '\0';
	} i = 0;
	rbuff(u->scut);
	/* Main character variables. */
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

	/* integer variables. */
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
	rval(u->splits);
	FCLOSE(fp);
	sprintf(backupfile,"backup/%s.dat",u->name);
	rename(filename,backupfile);
	if (err) return(0);
	return(1);
}
#endif /* RAMTITS2 userfiles */



#undef UCONVERT_FORMATS_SOURCE
