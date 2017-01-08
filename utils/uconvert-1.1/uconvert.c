/*
   uconvert.c:
     This program allows a talker owner to convert userfiles to the NEW
     RaMTITS 2.x format.  Modify the Makefile to your current userfile
     setup.. if you're userfile type isn't supported, you'll have to modify
     the formats.c file yourself and make modifications to the most similiar
     userfile format that you're userfiles currently use. Then just type
     'make' and the program should compile itself into an executable called
     'uconvert'.  Copy all the userfiles into the 'userfiles' directory (in
     this directory) and the new userfiles will be saved into the 'userfiles'
     directory and the old ones will be stored in the 'backup' directory.

     What it does is reads in the old format for userfiles, then writes out
     the new userfiles to 'userfiles/username.dat' WHATEVER YOU DO, DO NOT
     EDIT THE .DAT FILES MANUALLY!!!! that is the only thing that I know of
     that can cause the userfile to be corrupted. I may eventually make an
     utility to modify the userfiles through the shell.. but right now,
     this is it. Once it finishes the conversion, it checks all the userfiles
     it converted.. just to make sure the userfiles were converted properly
     and outputs everything to a log file.. to check out the logfile, just
     edit ./userconvert.log. Many thanks go to Wookey for showing me how to
     make this.. possibly optimizing speed a fair bit.  Send bug reports to
     'squirt@tdf.ca'. This program is (c)1999-2000, Rob Melhuish.
*/
#define UCONVERT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <structs.h>
#include <uconvert.proto.h>
#include <formats.proto.h>

char word[10][45];
int word_count;

/*
#define wbuff(buff)  if (!xwrite(fp,(char *)buff,sizeof(buff))) ++err; err += (ferror(fp)!=0);
#define rbuff(buff)  if (!xread(fp, (char *)buff,sizeof(buff))) ++err; err += (ferror(fp)!=0);
#define wval(val)    if (!xwrite(fp,(char *)&val,sizeof(val)))  ++err; err += (ferror(fp)!=0);
#define rval(val)    if (!xread(fp, (char *)&val,sizeof(val)))  ++err; err += (ferror(fp)!=0);
*/

int main(void)
{
	User u;
	char dirname[256], uname[20], logfile[256];
	FILE *logfp;
	struct dirent *dp;
	DIR *dir;
	int cnt = 0,logging = 0,i,bad_cnt = 0;

	i = 0;
	sprintf(logfile,"userconvert.log");
	if ((logfp = fopen(logfile,"w")) == NULL)
	{
		printf("Unable to open logfile, will continue without.\n");
		logging = 0;
	}
	else logging = 1;
	sprintf(dirname,"userfiles");
	dir = opendir(dirname);
	if (dir == NULL)
	{
		printf("Unable to open userfiles directory. Aborting.\n");
		fprintf(logfp,"Unable to open userfiles directory. Operation aborted.\n");
		exit(1);
	}
	if ((u = create_user()) == NULL)
	{
		printf("Unable to create temporary user object.\n");
		fprintf(logfp,"Unable to create a temporary user object. Operation aborted.\n");
		(void)closedir(dir);
		exit(2);
	}
	while ((dp=readdir(dir)) != NULL)
	{
#if defined(RAMTITS112) || defined(RAMTITS113)
		if (strstr(dp->d_name,".C"))
#elif defined(RAMTITS2)
		if (strstr(dp->d_name,".dat"))
#else
		if (strstr(dp->d_name,".D"))
#endif
		{
			uname[0] = '\0';
			strcpy(uname,dp->d_name);
#if defined(RAMTITS2)
			uname[strlen(uname)-4] = '\0';
#else
			uname[strlen(uname)-2] = '\0';
#endif
			strcpy(u->name,uname);
			if ((LoadOldUser(u,uname)) == 0)
			{
				printf("Unable to load userfile for: %s\n",u->name);
				fprintf(logfp,"Failed to load userfile: %s\n",u->name);
				u->name[0]	= '\0';
				uname[0]	= '\0';
				reset_vars(u);
				bad_cnt++;
				continue;
			}
			if ((SaveNewUser(u)) == 0)
			{
				printf("Unable to save new user: %s\n",u->name);
				fprintf(logfp,"Unable to save new user: %s\n",u->name);
				u->name[0]	= '\0';
				uname[0]	= '\0';
				reset_vars(u);
				bad_cnt++;
				continue;
			}
			reset_vars(u);
			printf("Converted: %s, now verifying...",u->name);
			if ((LoadNewUser(u,uname)) == 0)
			{
				printf("FAILED\n");
				fprintf(logfp,"Unable to load new user: %s - Reason: [%s]\n",uname,strerror(errno));
				u->name[0]	= '\0';
				uname[0]	= '\0';
				reset_vars(u);
				bad_cnt++;
				continue;
			}
			printf("SUCCESS\n");
			reset_vars(u);
			++cnt;
		}
		else continue;
	}
	(void)closedir(dir);
	reset_vars(u);
	printf("Converted %d user%s.",cnt,cnt>1?"s":"");
	if (bad_cnt == 0) printf("\n");
	else printf(" Total of %d bad account%s\n",bad_cnt,bad_cnt>1?"s":"");
	destruct_user(u);
	fclose(logfp);
	if (bad_cnt == 0) unlink(logfile);
	exit(0);
}

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

int SaveNewUser(User u)
{
	FILE *fp;
	char filename[256];
	int err = 0, version, i = 0;

	if (u == NULL) return(0);
	version = '2';
	sprintf(filename,"userfiles/%s.dat",u->name);
	fp = fopen("tempfile","w");
	if (fp == NULL) return(0);

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
	/*
	   Temp int's for your own use.. just remember what they are.
	   I'm not sure of the effect of changing the variable names.. but
	   it might be bad.. so just remember... what they are..
	*/
	wval(u->myint1);		wval(u->myint2);
	wval(u->myint3);		wval(u->myint4);
	wval(u->myint5);		wval(u->myint6);
	wval(u->myint7);		wval(u->myint8);
	wval(u->myint9);		wval(u->myint10);

	FCLOSE(fp);
	if (err)
	{
		remove("tempfile");
		printf("Couldn't save file for user: %s\n",u->name);
		return(0);
	}
	else if (rename("tempfile",filename))
	{
		printf("Error renaming 'tempfile' to '%s': Reason %s\n",u->name,strerror(errno));
		return(0);
	}
	return(1);
}

int LoadNewUser(User u,char *name)
{
	FILE *fp;
	char filename[256];
	int err = 0,version,i = 0;

	if (u == NULL) return(0);
	version='2';
	sprintf(filename,"userfiles/%s.dat",name);
	fp = fopen(filename,"r");
	if (fp == NULL) return(0);
	strcpy(u->name,name);

	rval(version);
	if (err) return(-1);

	for (i = 0; i < 50; ++i)
	{
		rbuff(u->friends[i]);
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		rbuff(u->enemies[i]);
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		rbuff(u->rmacros[i]);
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		rbuff(u->fmacros[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->allowcom[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->denycom[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->alias.find[i]);
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		rbuff(u->alias.alias[i]);
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		rbuff(u->pet.alias[i]);
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		rbuff(u->pet.trigger[i]);
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		if (!strcmp(u->friends[i],"#NOTSET#"))
			u->friends[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 50; ++i)
	{
		if (!strcmp(u->enemies[i],"#NOTSET#"))
			u->enemies[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (!strcmp(u->rmacros[i],"#NOTSET#"))
			u->rmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 17; ++i)
	{
		if (!strcmp(u->fmacros[i],"#NOTSET#"))
			u->fmacros[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcmp(u->allowcom[i],"#NOTSET#"))
			u->allowcom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcmp(u->denycom[i],"#NOTSET#"))
			u->denycom[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcmp(u->alias.find[i],"#NOTSET#"))
			u->alias.find[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 25; ++i)
	{
		if (!strcmp(u->alias.alias[i],"#NOTSET#"))
			u->alias.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (!strcmp(u->pet.alias[i],"#NOTSET#"))
			u->pet.alias[i][0] = '\0';
	} i = 0;
	for (i = 0; i < 20; ++i)
	{
		if (!strcmp(u->pet.trigger[i],"#NOTSET#"))
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
	/*
	   Temp int's for your own use.. just remember what they are.
	   I'm not sure of the effect of changing the variable names.. but
	   it might be bad.. so just remember... what they are..
	*/
	rval(u->myint1);		rval(u->myint2);  
	rval(u->myint3);		rval(u->myint4);
	rval(u->myint5);		rval(u->myint6);    
	rval(u->myint7);		rval(u->myint8);
	rval(u->myint9);		rval(u->myint10);

	FCLOSE(fp);
	if (err) return(0);
	return(1);
}

void clear_words(void)
{
	int w;
	for (w = 0; w < 10; ++w) word[w][0] = '\0';
	word_count = 0;
}

int wordfind(char *inpstr)
{
	int wn,wpos;

	wn = wpos = 0;
	do
	{
		while(*inpstr<33) if (!*inpstr++) return(wn);
		while(*inpstr>32 && wpos<40-1)
		{
			word[wn][wpos] = *inpstr++;
			wpos++;
		}
		word[wn][wpos] = '\0';
		wn++;
		wpos = 0;
	} while (wn<10);
	return(wn-1);
}

/* Return pos. of second word in inpstr */
char *remove_first(char *inpstr)
{
	char *pos = inpstr;   

	while (*pos<33 && *pos) ++pos;
	while (*pos>32) ++pos;
	while (*pos<33 && *pos) ++pos;
	return(pos);
}

/* Load the user's old macros. */
int LoadMacros(User user)
{
	FILE *ffp,*rfp;
	int i;
	char file1[256],file2[256],temp[26],tempfile[FSL];

	sprintf(file1,"userfiles/settings/%s.FM",user->name);
	sprintf(file2,"userfiles/settings/%s.RM",user->name);
	if ((ffp = fopen(file1,"r")) == NULL)
	{
		return(0);
	}
	if ((rfp = fopen(file2,"r")) == NULL)
	{
		fclose(ffp);
		return(0);
	}
	for (i = 0; i < 17; ++i)
	{
		fgets(user->fmacros[i],sizeof(user->fmacros[i])-1,ffp);
		user->fmacros[i][strlen(user->fmacros[i])-1]='\0';
		sprintf(temp,"Unset %2.2d",i);
		if (user->fmacros[i][0] == '\0') strcpy(user->fmacros[i],temp);
	}
	FCLOSE(ffp);
	rename(file1,tempfile);
	for (i = 0; i < 17; ++i)
	{
		fgets(user->rmacros[i],sizeof(user->rmacros[i])-1,rfp);
		user->rmacros[i][strlen(user->rmacros[i])-1] = '\0';
		sprintf(temp,"Unset %2.2d",i);
		if (user->rmacros[i][0] == '\0') strcpy(user->rmacros[i],temp);
	}
	FCLOSE(rfp);
	rename(file2,tempfile);
	return(1);
}

/* Loads user->friends, enemies, allowcom, and denycoms */
int load_user_stuff(User user,int which)
{
	FILE *fp;
	char file1[256],name[51];
	int i=0;

	file1[0] = name[0] = '\0';
	switch (which)
	{
		case 1:
		  sprintf(file1,"userfiles/usermisc/%s.F",user->name);
		  if ((fp = fopen(file1,"r")) == NULL) return(0);
		  fscanf(fp,"%s",name);
		  while (!(feof(fp)))
		  {
			if (i>50) break;
			strcpy(user->friends[i],name);
			++i;  name[0] = '\0';
			fscanf(fp,"%s",name);
		  }
		  if (fp != NULL) fclose(fp);
		  unlink(file1);
		  return(1);
		case 2:
		  sprintf(file1,"userfiles/usermisc/%s.E",user->name);
		  if ((fp = fopen(file1,"r")) == NULL) return(0);
		  fscanf(fp,"%s",name);
		  while (!(feof(fp)))
		  {
			if (i>50) break;
			strcpy(user->enemies[i],name);
			++i;  name[0] = '\0';
			fscanf(fp,"%s",name);
		  }
		  if (fp != NULL) fclose(fp);
		  unlink(file1);
		  return(1);
		case 3:
		  sprintf(file1,"userfiles/usermisc/%s.G",user->name);
		  if ((fp = fopen(file1,"r")) == NULL) return(0);
		  fscanf(fp,"%s",name);
		  while (!(feof(fp)))
		  {
			if (i>25) break;
			strcpy(user->allowcom[i],name);
			++i;  name[0] = '\0';
			fscanf(fp,"%s",name);
		  }
		  if (fp != NULL) fclose(fp);
		  unlink(file1);
		  return(1);
		case 4:
		  sprintf(file1,"userfiles/usermisc/%s.D",user->name);
		  if ((fp = fopen(file1,"r")) == NULL) return(0);
		  fscanf(fp,"%s",name);
		  while (!(feof(fp)))
		  {
			if (i>25) break;
			strcpy(user->denycom[i],name);
			++i;  name[0] = '\0';
			fscanf(fp,"%s",name);
		  }
		  if (fp != NULL) fclose(fp);
		  unlink(file1);
		  return(1);
	}
	return(0);
}

/*
   This loads the default shortcuts of the talker.
*/
void load_default_shortcuts(User user)
{

	user->scut[0].scut='!'; 	strcpy(user->scut[0].scom,"semote");
	user->scut[1].scut=';'; 	strcpy(user->scut[1].scom,"emote");
	user->scut[2].scut='/'; 	strcpy(user->scut[2].scom,"pemote");
	user->scut[3].scut='>'; 	strcpy(user->scut[3].scom,"tell");
	user->scut[4].scut='<'; 	strcpy(user->scut[4].scom,"pemote");
	user->scut[5].scut='\'';	strcpy(user->scut[5].scom,"to");
	user->scut[6].scut='*'; 	strcpy(user->scut[6].scom,"cbuff");
	user->scut[7].scut='%'; 	strcpy(user->scut[7].scom,"chemote");
	user->scut[8].scut='^';		strcpy(user->scut[8].scom,"channel");
	user->scut[9].scut=']'; 	strcpy(user->scut[9].scom,"cexec");
	user->scut[10].scut='#';	strcpy(user->scut[10].scom,"shout");
	user->scut[11].scut='-';	strcpy(user->scut[11].scom,"echo");
	user->scut[12].scut='`';	strcpy(user->scut[12].scom,"say");
	user->scut[13].scut='(';	strcpy(user->scut[13].scom,"ftell");
	user->scut[14].scut=')';	strcpy(user->scut[14].scom,"femote");
	return;
}

#undef UCONVERT_SOURCE
/* END OF UCONVERT.C */
