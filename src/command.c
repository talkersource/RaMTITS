/*
  command.c
    Source code for inner workings of command execution.

    The source within this file is Copyright 1998 - 2001 by
 	Rob Melhuish, Arnaud Abelard, Mike Irving, and Tim van der Leeuw.

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
#include <ctype.h>
#define _COMMAND_C
  #include "include/needed.h"
#undef _COMMAND_C

/*
  This function finds the command to execute, then if it's found, it
  executes it with the cmd->function... figured this would be easier
  then the older exec_com
*/
void exec_com(User user,char *inpstr)
{
	Commands com;
	int i,len,found=0;
	char *comword=NULL,levelname[16];

	if (user == NULL) return;
	if (user->whitespace)
	{
		while(inpstr[0] == ' ') inpstr++;
	}
	if (strstr(inpstr,"&&"))
	{
		ExecMulti(user,inpstr);
		return;
	}
	if (word[0][0] == '.' || word[0][0] == ',') comword = (word[0]+1);
	else if (word[0][0] == '/' && user->mode == IRC) comword = (word[0]+1);
	else
	{
		for (found = 0,i = 0 ; i<28 && found == 0 ; ++i)
		{
			if (inpstr[0] == user->scut[i].scut)
			{
				comword = user->scut[i].scom;
				found = 1;
				inpstr++;
				white_space(inpstr);
			}
		}
		if (found == 0) comword = word[0];
	}
	if (comword[0] == '\0')
	{
		write_user(user,"~FG It usually helps if you enter something valid ;-)\n");
		return;
	}
	if (user->status & Gagged)
	{
		write_user(user,center("~FT-~CT=~CB> ~FTYou're gagged, you can't do ANYTHING... ~CB<~CT=~FT-\n",80));
		return;
	}
	i = 0;
	len = strlen(comword);
	com = get_comname(comword);
	if (com == NULL)
	{
		if (is_social(user,comword))
		{
			if (user->level<USER || (user->status & Jailed))
			{
				writeus(user,wtf,comword);
				return;
			}
			else ExecSocial(user,inpstr);
		}
		else if (is_alias(user,comword))
		{
			ExecAlias(user,comword,inpstr);
			return;
		}
		else writeus(user,wtf,comword);
		return;
	}
	if (com->min_lev>user->level && !is_allowed(user,com->name))
	{
		if (user->gender == Female) strcpy(levelname,level[com->min_lev].fname);
		else strcpy(levelname,level[com->min_lev].mname);
		writeus(user,notallowed,com->name,levelname);
		return;
	}
	if (com->disabled)
	{
		writeus(user,"~FT Sorry.. but the %s command is currently disabled.\n",com->name);
		return;
	}
	if (found == 1)
	{
		clear_words();
		strcpy(word[0],comword);
		word_count = wordshift(inpstr);
	}
	if (debug_talker)
	{
		if (word[1][0] == 0) sprintf(text,"User: [%-12.12s] - Command: [%-12.12s]\n",user->name,com->name);
		else sprintf(text,"User: [%-12.12s] - Command: [%-12.12s] Arg 1: [%s] [%d words]\n",user->name,com->name,word[1],word_count);
		RecordDebug(text);
	}
	++com->count;
	com_num = com->id;
	if (found == 0) inpstr = remove_first(inpstr);
	com->funct(user,inpstr,com->args);
	return;
}

/* Get the command id. */
int get_comnum(char *cmd)
{
	Commands com;

	com = get_comname(cmd);
	if (com == NULL) return(-1);
	return(com->id);
}

/* Return a command value based on a command name. */
Commands get_comname(char *cmdname)
{
	Commands com;
	char com2[21];
	int x,len;

	x = len = 0;
	len = strlen(cmdname);
	while (command[x].name[0]!='*')
	{
		if (!strncasecmp(command[x].name,cmdname,len))
		{
			strcpy(com2,command[x].name);
			break;
		}
		++x;
	}
	for_comms(com)
	{
		if (!strcasecmp(com->name,com2)) return(com);
	}
	return(NULL);
}

/* Is it a shortcut? */
int is_scut(User user,char *inpstr)
{
	int i=0;

	for (i=0;i<28;++i)
	{
		if (user->scut[i].scut == inpstr[0]) return(1);
	}
	return(0);
}

/*
  Now we need to be able to execute socials if in fact there is a social
  to be executed.
*/
int ExecSocial(User user,char *inpstr)
{
	Socials soc;
	User u;
	FILE *fp;
	char *comword,social[5][251],filename[FSL],line[251];
	char *name1,*name2,*socstr;
	int i;

	for (i = 0 ; i < 5 ; ++i) social[i][0] = '\0';
	if (user->whitespace)
	{
		while(inpstr[0]==' ') inpstr++;
	}
	if (word[0][0] == '.' || word[0][0] == ',') comword = (word[0]+1);
	else if (word[0][0] == '/' && user->mode == IRC) comword = (word[0]+1);
	else comword=word[0];
	soc = get_socname(comword);
	if (soc == NULL)
	{
		/* if (!exec_user_social(user,inpstr)) return(0); */
		return(0);
	}
	if (user->gsocs)
	{
		write_user(user,"~CW-~FG You have had socials taken away from you.\n");
		return(1);
	}
	sprintf(filename,"%s/%s.dat",SocialDir,soc->name);
	if ((fp = fopen(filename,"r")) == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Failed to open %s.dat in exec_social()\n",soc->name);
		return(0);
	}
	i = 0;
	fgets(line,250,fp);
	while (!(feof(fp)))
	{
		if (i>4) break;
		line[strlen(line)-1] = '\0'; /* Remove \n */
		if (strlen(line)>2) strcpy(social[i],line);
		++i;
		fgets(line,250,fp);
	}
	FCLOSE(fp);
	if (user->invis == Invis) name1 = invisname;  else name1 = user->recap;
	if (word_count<2)
	{
		sprintf(text,social[0],name1);
		if (text[strlen(text)-1] != '\n') strcat(text,"\n");
		socstr = check_gender(user,NULL,text);
		write_user(user,socstr);
		write_room_except(user->room,user,socstr);
		return(1);
	}
	if ((u = get_name(user,word[1])) == NULL)
	{
		write_user(user,center(notloggedon,80));
		return(1);
	}
	if (u->invis == Invis) name2 = invisname;  else name2 = u->recap;
	if (u == user)
	{
		sprintf(text,social[1],name1);
		if (text[strlen(text)-1] != '\n') strcat(text,"\n");
		socstr = check_gender(user,NULL,text);
		write_user(user,socstr);
		write_room_except(user->room,user,socstr);
		return(1);
	}
	if (is_enemy(u,user))
	{
		writeus(user,"~CW-~FG %s ~FTdoesn't want anything to do with you.\n",name2);
		return(1);
	}
	if (is_enemy(user,u))
	{
		write_user(user,"~FG You can't execute socials on your enemies.\n");
		return(1);
	}
	if (is_busy(u))
	{
		writeus(user,"~CW- ~FG%s ~FTis currently busy... try again in a bit :-)\n",name2);
		return(1);
	}
	if (u->room != user->room)
	{
		sprintf(text,social[2],name1);
		if (text[strlen(text)-1] != '\n') strcat(text,"\n");
		socstr = check_gender(user,u,text);
		write_user(u,socstr);
		record_tell(u,socstr);
		text[0] = '\0';
		sprintf(text,social[3],name2);
		if (text[strlen(text)-1] != '\n') strcat(text,"\n");
		socstr = check_gender(user,u,text);
		write_user(user,socstr);
		record_tell(user,socstr);
		return(1);
	}
	text[0] = '\0';
	sprintf(text,social[4],name1,name2);
	if (text[strlen(text)-1] != '\n') strcat(text,"\n");
	socstr = check_gender(user,u,text);
	write_user(user,socstr);
	write_user(u,socstr);
	write_room_except2(user->room,user,u,socstr);
	record(user->room,socstr);
	return(1);
}

/* Tells us whether or not the comword is a social or not. */
int is_social(User user,char *comword)
{
	Socials s;
	char filename[FSL];

	for_socials(s)
	{
		if (!strncasecmp(comword,s->name,strlen(comword))) return(1);
	}
	sprintf(filename,"%s/%s.%s",SocialDir,comword,user->name);
	if (file_exists(filename)) return(1);
	return(0);
}

/* Tells us whether or not the comword is an alias. */
int is_alias(User user,char *comword)
{
	int i;

	if (user == NULL) return(0);
	for (i = 0 ; i < 25 ; ++i)
	{
		if (!strncasecmp(user->alias.find[i],comword,strlen(comword)))
		  return(1);
	}
return(0);
}

/* This performs the multiple executions of commands. */
void ExecMulti(User user,char *str)
{
	char exec[15][151]; /* Max 15 multiple executions */
	int i,j,x,len;

	i = j = x = 0;
	len = strlen(str);
	if (user == NULL) return;
	for (i = 0 ; i < 15 ; ++i) exec[i][0] = '\0';
	i = 0;
	while (i <= len)
	{
		if (j >= 15) break;
		if (str[i] == '&' && str[i+1] == '&')
		{
			if (str[i-1] == ' ') exec[j][x-1] = '\0';
			i += 2;
			while (str[i] == ' ') i++;
			exec[j][x] = '\0';
			j++;
			x = 0;
		}
		else
		{
			exec[j][x] = str[i];
			i++;
			x++;
			if (x >= 149 || i >= len)
			{
				exec[j][x+1] = '\0';
			}
		}
	}
	for (i = 0 ; i < 15 ; ++i)
	{
		if (exec[i][0] == '\0') continue;
		clear_words();
		word_count = wordfind(exec[i]);
		exec_com(user,exec[i]);
	}
}

/*
  Executes a command on all users, i.e .execall hug would hug ever user on
  the talker.
*/
void exec_all(User user,char *inpstr,int rt)
{
	User u;
	char temp[ARR_SIZE],cmd[20],temp2[ARR_SIZE*2];
	int cnt=0;

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
	if (word_count<2) {
		writeus(user,usage,command[com_num].name,"~CB[~CRcommand~CB] [<~CRarguments~CB>]");
		return;
	}
	strncpy(cmd,word[1],sizeof(cmd)-1);
	inpstr = remove_first(inpstr);
	strncpy(temp,inpstr,sizeof(temp)-1);
	for_users(u)
	{
		if (u->type==CloneType || is_busy(u) || (u->ignore & Execall)
		  || (u->invis>=Hidden && user->level<GOD) || u->login
		  || (u == user)) continue;
		cnt++;
		sprintf(temp2,"%s %s %s",cmd,u->name,temp);
		clear_words();
		word_count = wordfind(temp2);
		exec_com(user,temp2);
	}
}

/* This will execute the users alias if it's found.. */
int ExecAlias(User user,char *command,char *str)
{
	int i,found;
	char alias[ARR_SIZE];

	i = found = 0;
	if (user == NULL) return(0);
	for (i = 0 ; i < 25 ; i++)
	{
		if (!strncasecmp(user->alias.find[i],command,strlen(command)))
		{
			strncpy(alias,user->alias.alias[i],sizeof(alias)-1);
			found = 1;
			break;
		}
		found = 0;
	}
	if (found == 0) return(0);	/* alias wasn't found */
	strncpy(alias,StringConvert(alias,remove_first(str)),sizeof(alias)-1);
	clear_words();
	word_count = wordfind(alias);
	exec_com(user,alias);
	return(1);
}

/* Gives us a gender for socials */
char *which_gender(User u,User u2,char *which_one)
{
	if (!strcasecmp(which_one,"$G1")) return(gender1[u->gender]);
	else if (!strcasecmp(which_one,"$G2")) return(gender2[u->gender]);
	else if (!strcasecmp(which_one,"$G3")) return(gender3[u->gender]);
	else if (!strcasecmp(which_one,"$G4") && u2 !=NULL)
	  return(gender1[u2->gender]);
	else if (!strcasecmp(which_one,"$G5") && u2 != NULL)
	  return(gender2[u2->gender]);
	else if (!strcasecmp(which_one,"$G6") && u2 != NULL)
	  return(gender3[u2->gender]);
	else return(NULL);
}

/*
  This *should* look for the $G1 $G2, etc and replace it with the gender
  names defined in custom1.h.
*/
char *check_gender(User user,User user2,char *str)
{
	char *str2=NULL;
	int x=0;

	while (gender_types[x][0] != '*')
	{
		while (str != NULL)
		{
                	str2 = str;
			str = andys_replace_string(str2,gender_types[x],which_gender(user,user2 != NULL?user2:NULL,gender_types[x]));
		}
		++x;
		str = str2;
	}
	return(str2);
}

/* END OF COMMAND.C */
