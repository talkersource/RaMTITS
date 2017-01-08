/*
   config.c
     This source file contains the source code for configuring the talker,
     such as adding rooms, and changing things in the config file.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, and Mike Irving.

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
#include <errno.h>
#include <unistd.h>
#define _CONFIG_C
  #include "include/needed.h"
#undef _CONFIG_C

/*
  Changes the elements in the config... I've decided to go about making a
  config file a whole seperate way.. to have the ability to make things
  more definable globally. Now when this command is called, and after
  something is changed, it will create a config.talker file which will
  be the main config file throughout this is probably going to take some
  massive work and renovations.. but if you ask me.. it should be all
  worthwhile in the long run.. I also plan on having a .rmadmin command
  which will allow you to delete the default rooms that come with the
  talker, and create your own rooms.. I don't know how well this exactly
  will work out... just cross your fingers ;-)
*/
void change_system(User user,char *inpstr,int rt)
{
	int temp,val,lev;
	char *ca[]={ " NONE  ","IGNORE ","REBOOT ","SREBOOT" };
	char temp2[81],cusage[80];

	temp = val = lev = 0;  temp2[0] = cusage[0] = '\0';
	if (user->tpromoted && user->orig_lev<GOD)
	{
		writeus(user,tempnodo,command[com_num].name);
	        return;
        }
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
	if (!strcasecmp(word[1],"-users"))
	{
		temp = max_users;
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-users ~CB[~CR10~CW - ~CR99~CB]");
			return;
		}
		val = atoi(word[2]);
		if (val < 10 || val > 99)
		{
			writeus(user,"~FG That value was to %s, value must be between 10 and 99either too hi or too low.\n",val>99?"high":"low");
			return;
		}
		max_users = val;
		writeus(user,"~FG You change max_users from~CB: [~CR%d~CB] ~FGto ~CB[~CR%d~CB]\n",temp,max_users);
		write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: max_users - [%d] to [%d]\n",user->name,temp,max_users);
		save_config();
		return;
	}
	if (!strncasecmp(word[1],"-clones",3))
	{
		temp = max_clones;
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-cl ~CB[~CR1~CW-~CR6~CB]");
			return;
		}
		val = atoi(word[2]);
		if (val < 1 || val > 6)
		{
			writeus(user,"~CW-~FG That value is to %s, value can only be between 1 - 6.\n",val>6?"high":"low");
			return;
		}
		max_clones = val;
		writeus(user,"~FG You change max_clones from~CB: [~CR%d~CB] ~FGto ~CB[~CR%d~CB]\n",temp,max_clones);
		write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: max_clones - [%d] to [%d]\n",user->name,temp,max_clones);
		save_config();
		return;
	}
	if (!strncasecmp(word[1],"-passwd",5))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-pass ~CB[~CRyes~CW|~CRno~CB]");
			return;
		}
		if (!strncasecmp(word[2],"yes",1))
		{
			if (password_echo)
			{
				write_user(user,"~FG Password echoing is already set to yes.\n");
				return;
			}
			password_echo = 1;
			write_user(user,"~FG You change password_echo from~CB: ~CRNO~FG to ~CRYES\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: password_echo - NO to YES\n",user->name);
			save_config();
			return;
		}
		if (!strncasecmp(word[2],"no",1))
		{
			if (password_echo == 0)
			{
				write_user(user,"~FG Password echoing is already set to no.\n");
				return;
			}
			password_echo = 0;
			write_user(user,"~FG You change password_echo from~CB: ~CRYES~FG to ~CRNO\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: password_echo - YES to NO\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-pass ~CB[~CRyes~CW|~CRno~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-mesgchtime",5))
	{
		sprintf(temp2,"%02d:%02d",mesg_check_hour,mesg_check_min);
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-mesg ~CB[~CRtime to check messages~CB] (~CRHH~CW:~CRMM format~CB)");
			return;
		}
		if (word[2][2]!=':' || strlen(word[2])>5 || !isdigit(word[2][0])
		  || !isdigit(word[2][1]) || !isdigit(word[2][3])
		  || !isdigit(word[2][4]))
		{
			write_user(user,"~FG You specified an invalid message check time.\n~FG Format must be in HH:MM\n");
			return;
		}
		sscanf(word[2],"%d:%d",&temp,&val);
		if (temp > 23 || val > 59)
		{
			write_user(user,"~FG You specified an invalid message check time.\n");
			return;
		}
		mesg_check_hour = temp;  mesg_check_min = val;
		writeus(user,"~FG You change mesg_check_time from~CB: [~CR%s~CB]~FG to ~CB[~CR%s~CB]\n",temp2,word[2]);
		write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: mesg_check_time - [%s] - [%s]\n",user->name,temp2,word[2]);
		save_config();
		return;
	}
	if (!strncasecmp(word[1],"-life",5))
	{
		temp = mesg_life;
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-life ~CB[~CR1~CW - ~CR15~CB]");
			return;
		}
		val = atoi(word[2]);
		if (val<1 || val>15)
		{
			writeus(user,"~FG That value was to %s, value must be between 1 and 15.\n",val>15?"high":"low");
			return;
		}
		mesg_life = val;
		writeus(user,"~FG You change mesg_life from~CB: [~CR%d~CB]~FG to ~CB[~CR%d~CB]\n",temp,mesg_life);
		write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: mesg_life - [%d] to [%d]\n",user->name,temp,mesg_life);
		save_config();
		return;
	}
	if (!strcasecmp(word[1],"-afks"))
	{
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-afks ~CB[~CRyes~CW|~CRno~CB]");
			return;
		}
		if (!strncasecmp(word[2],"no",1))
		{
			if (time_out_afks == 0)
			{
				write_user(user,"~FG Time out AFKS is already set to NO.\n");
				return;
			}
			time_out_afks = 0;
			write_user(user,"~FG You change time_out_afks from~CB: ~CRYES~FG to ~CRNO\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: time_out_afks - YES to NO\n",user->name);
			save_config();
			return;
		}
		if (!strncasecmp(word[2],"yes",1))
		{
			if (time_out_afks)
			{
				write_user(user,"~FG Time out AFKS is already set to YES.\n");
				return;
			}
			time_out_afks = 1;
			write_user(user,"~FG You change time_out_afks from~CB: ~CRNO~FG to ~CRYES\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: time_out_afks - NO to YES\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-afks ~CB[~CRyes~CW|~CRno~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-wizplev",5))
	{
		temp = wizport_level;
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-wizp ~CB[~CRlevel~CB]");
			return;
		}
		if ((lev = get_level(word[2])) == -1)
		{
			write_user(user,"~FG You have specified an invalid level.\n");
			return;
		}
		if (lev < WIZ)
		{
			write_user(user,"~FG The level you specified is below the WIZ level.\n");
			return;
		}
		wizport_level = lev;
		writeus(user,"~FG You change the wizport_level from~CB: [~CR%s~CB]~FG to ~CB[~CR%s~CB]\n",level[temp].name,level[wizport_level].name);
		write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: wizport_level - [%s] to [%s]\n",user->name,level[temp].name,level[wizport_level].name);
		save_config();
		return;
	}
	if (!strncasecmp(word[1],"-charecho",5))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-char ~CB[~CRoff~CW|~CRon~CB]");
			return;
		}
		if (!strcasecmp(word[2],"on"))
		{
			if (charecho_def)
			{
				write_user(user,"~FG Character echo default is already set to ON.\n");
				return;
			}
			charecho_def = 1;
			write_user(user,"~FG You change charecho_def from~CB: ~CROFF~FG to ~CRON\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: charecho_def - OFF to ON\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (charecho_def == 0)
			{
				write_user(user,"~FG Character echo default is already set to OFF.\n");
				return;
			}
			charecho_def = 0;
			write_user(user,"~FG You change charecho_def from~CB: ~CRON~FG to ~CROFF\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: charecho_def - ON to OFF\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-char ~CB[~CRoff~CW|~CRno~CB]");
	}
	if (!strncasecmp(word[1],"-prompt",3))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-pr ~CB[~CRoff~CW|~CRon~CB]");
			return;
		}
		if (!strcasecmp(word[2],"on"))
		{
			if (prompt_def)
			{
				write_user(user,"~FG Prompt default is already set to ON.\n");
				return;
			}
			prompt_def = 1;
			write_user(user,"~FG You change prompt_def from~CB: ~CROFF~FG to ~CRON\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: prompt_def - OFF to ON\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (prompt_def == 0)
			{
				write_user(user,"~FG Prompt default is already set to OFF.\n");
				return;
			}
			prompt_def = 0;
			write_user(user,"~FG You change prompt_def from~CB: ~CRON~FG to ~CROFF\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: prompt_def - ON to OFF\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-pr ~CB[~CRoff~CW|~CRon~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-crashaction",6))
	{
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-crash ~CB[~CRreboot~CW|~CRignore~CW|~CRsreboot~CW|~CRnone~CB]");
			return;
		}
		if (!strncasecmp(word[2],"reboot",3))
		{
			crash_action = 2;
			write_user(user,"~FG You change crash_action to~CB: ~CRREBOOT\n");
			return;
		}
		if (!strcasecmp(word[2],"none"))
		{
			crash_action = 0;
			write_user(user,"~FG You change crash_action to~CB:~CR NONE\n");
			return;
		}
		if (!strncasecmp(word[2],"ignore",3))
		{
			crash_action = 1;
			write_user(user,"~FG You change crash_action to~CB:~CR IGNORE\n");
			return;
		}
		if (!strncasecmp(word[2],"sreboot",4))
		{
			crash_action = 3;
			write_user(user,"~FG You change crash_action to~CB:~CR SREBOOT\n");
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-crash ~CB[~CRreboot~CW|~CRignore~CW|~CRsreboot~CW|~CRnone~CB]");
		save_config();
		return;
	}
	if (!strncasecmp(word[1],"-timeout",5))
	{
		temp = time_out_maxlevel;
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-time ~CB[~CRlevel~CB]");
			return;
		}
		if ((lev = get_level(word[2])) == -1)
		{
			write_user(user,"~FG You have specified an invalid level.\n");
			return;
		}
		time_out_maxlevel = lev;
		writeus(user,"~FG You change the time_out_maxlevel from~CB: [~CR%s~CB]~FG to ~CB[~CR%s~CB]\n",level[temp].name,level[time_out_maxlevel].name);
		write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: wizport_level - [%s] to [%s]\n",user->name,level[temp].name,level[time_out_maxlevel].name);
		save_config();
		return;
	}
	if (!strncasecmp(word[1],"-backup",5))
	{
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-back ~CB[~CRoff~CW|~CRon~CB]");
			return;
		}
		if (!strcasecmp(word[2],"on"))
		{
			if (backup_talker)
			{
				write_user(user,"~FG Talker backup is already set to ON.\n");
				return;
			}
			backup_talker = 1;
			write_user(user,"~FG You change backup_talker from~CB: ~CROFF~FG to ~CRON\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: backup_talker - OFF to ON\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (backup_talker == 0)
			{
				write_user(user,"~FG Talker backup is already set to OFF.\n");
				return;
			}
			backup_talker = 0;
			write_user(user,"~FG You change backup_talker from~CB: ~CRON~FG to ~CROFF\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: backup_talker - ON to OFF\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-back ~CB[~CRoff~CW|~CRon~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-debug",4))
	{
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-deb ~CB[~CRoff~CW|~CRon~CB]");
			return;
		}
		if (!strcasecmp(word[2],"on"))
		{
			if (debug_talker)
			{
				write_user(user,"~FG Talker debugging is already set to ON.\n");
				return;
			}
			debug_talker = 1;
			write_user(user,"~FG You change debug_talker from~CB: ~CROFF~FG to ~CRON\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: debug_talker - OFF to ON\n",user->name);
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (debug_talker == 0)
			{
				write_user(user,"~FG Talker debugging is already set to OFF.\n");
				return;
			}
			debug_talker = 0;
			write_user(user,"~FG You change debug_talker from~CB: ~CRON~FG to ~CROFF\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: debug_talker - ON to OFF\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-deb ~CB[~CRoff~CW|~CRon~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-webpage",4))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-web ~CB[~CRoff~CW|~CRon~CB]");
			return;
		}
		if (!strcasecmp(word[2],"on"))
		{
			if (webpage_on)
			{
				write_user(user,"~FG The webpage generator is already set to ON.\n");
				return;
			}
			webpage_on = 1;
			write_user(user,"~FG You change webpage_on from~CB: ~CROFF~FG to ~CRON\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: webpage_on - OFF to ON\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (webpage_on == 0)
			{
				write_user(user,"~FG The webpage generator is already set to OFF.\n");
				return;
			}
			webpage_on = 0;
			write_user(user,"~FG You change webpage_on from~CB: ~CRON~FG to ~CROFF\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: webpage_on - ON to OFF\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-web ~CB[~CRoff~CW|~CRon~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-autoauth",5))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-auto ~CB[~CRoff~CW|~CRon~CB]");
			return;
		}
		if (!strcasecmp(word[2],"on"))
		{
			if (auto_auth)
			{
				write_user(user,"~FG Auto auth is already set to ON.\n");
				return;
			}
			auto_auth = 1;
			write_user(user,"~FG You change auto_auth from~CB: ~CROFF~FG to ~CRON\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: auto_auth - OFF to ON\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"off"))
		{
			if (auto_auth == 0)
			{
				write_user(user,"~FG Auto auth is already set to OFF.\n");
				return;
			}
			auto_auth = 0;
			write_user(user,"~FG You change auto_auth from~CB: ~CRON~FG to ~CROFF\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: auto_auth - ON to OFF\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-auto ~CB[~CRoff~CW|~CRon~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-apromote",4))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-apr ~CB[~CRyes~CW|~CRno~CB]");
			return;
		}
		if (!strcasecmp(word[2],"no"))
		{
			if (auto_promote == 0)
			{
				write_user(user,"~FG Auto promote is already set to NO.\n");
				return;
			}
			auto_promote = 0;
			write_user(user,"~FG You change auto_promote from~CB: ~CRYES~FG to ~CRNO\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: auto_promote - YES to NO\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"yes"))
		{
			if (auto_promote)
			{
				write_user(user,"~FG Auto promote is already set to YES.\n");
				return;
			}
			auto_promote = 1;
			write_user(user,"~FG You change auto_promote from~CB: ~CRNO~FG to ~CRYES\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: auto_promote - NO to YES\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-apr ~CB[~CRyes~CW|~CRno~CB]");
		return;
	}
	if (!strncasecmp(word[1],"-freerooms",5))
	{
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-free ~CB[~CRyes~CW|~CRno~CB]");
			return;
		}
		if (!strcasecmp(word[2],"no"))
		{
			if (free_rooms == 0)
			{
				write_user(user,"~FG Free rooms are already set to NO.\n");
				return;
			}
			free_rooms = 0;
			write_user(user,"~FG You change free_rooms from~CB: ~CRYES~FG to ~CRNO\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: free_rooms - YES to NO\n",user->name);
			save_config();
			return;
		}
		if (!strcasecmp(word[2],"yes"))
		{
			if (free_rooms)
			{
				write_user(user,"~FG Free rooms are already set to YES.\n");
				return;
			}
			free_rooms = 1;
			write_user(user,"~FG You change free_rooms from~CB: ~CRNO~FG to ~CRYES\n");
			write_log(1,LOG1,"[CHANGE] - User: [%s] Changed: free_rooms - NO to YES\n",user->name);
			save_config();
			return;
		}
		writeus(user,usage,command[com_num].name,"~CR-free ~CB[~CRyes~CW|~CRno~CB]");
		return;
	}
	sprintf(cusage,"~CW - ~FGCommand usage~CB: ~CR.change ");
	writeus(user,"%s-clones  ~CB[~CR1~CW-~CR6~CB]       ~FTCurrently~CB: [~CR%2d~CB]\n",cusage,max_clones);
	writeus(user,"%s-life    ~CB[~CR1~CW-~CR15~CB]      ~FTCurrently~CB: [~CR%2d~CB]\n",cusage,mesg_life);
	writeus(user,"%s-users   ~CB[~CR10~CW-~CR99~CB]     ~FTCurrently~CB: [~CR%2d~CB]\n",cusage,max_users);
	writeus(user,"%s-wizplev ~CB[~CRlevel~CB]     ~FTCurrently~CB: [~CR%5s~CB]\n",cusage,level[wizport_level].name);
	writeus(user,"%s-timeout ~CB[~CRlevel~CB]     ~FTCurrently~CB: [~CR%5s~CB]\n",cusage,level[time_out_maxlevel].name);
	writeus(user,"%s-char    ~CB[~CRoff~CW|~CRon~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,offon[charecho_def]);
	writeus(user,"%s-prompt  ~CB[~CRoff~CW|~CRon~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,offon[prompt_def]);
	writeus(user,"%s-backup  ~CB[~CRoff~CW|~CRon~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,offon[backup_talker]);
	writeus(user,"%s-debug   ~CB[~CRoff~CW|~CRon~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,offon[debug_talker]);
	writeus(user,"%s-webpage ~CB[~CRoff~CW|~CRon~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,offon[webpage_on]);
	writeus(user,"%s-auto    ~CB[~CRoff~CW|~CRon~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,offon[auto_auth]);
	writeus(user,"%s-afks    ~CB[~CRyes~CW|~CRno~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,noyes1[time_out_afks]);
	writeus(user,"%s-apromo  ~CB[~CRyes~CW|~CRno~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,noyes1[auto_promote]);
	writeus(user,"%s-free    ~CB[~CRyes~CW|~CRno~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,noyes1[free_rooms]);
	writeus(user,"%s-passwd  ~CB[~CRyes~CW|~CRno~CB]    ~FTCurrently~CB: [~CR%3s~CB]\n",cusage,noyes1[password_echo]);
	writeus(user,"%s-mesg    ~CB[~CRnew time~CB]  ~FTCurrently~CB: [~CR%02d:%02d~CB]\n",cusage,mesg_check_hour,mesg_check_min);
	writeus(user,"%s-crash   ~CB[~CRreb~CW|~CRnone~CW|~CRign~CW|~CRsreb~CB]  ~FTCurrently~CB: [~CR%s~CB]\n",cusage,ca[crash_action]);
	return;
}

/*
   This loads up room details from the roomfiles directory.. I'm choosing to
   do it this way because of the fact that rooms are a huge hog on memory
   and I prefer not to have a memory hog talker code.
*/
int LoadRoom(Room room,char *name)
{
	FILE *fp;
	char filename[FSL];
	int version,err = 0,i;

	if (room == NULL) return(0);
	sprintf(filename,"%s/%s.dat",RoomDir,name);
	if (!file_exists(filename))
	{
		filename[0] = '\0';
		strtolower(name);
		name[0] = toupper(name[0]);
		sprintf(filename,"%s/%s.dat",RoomDir,name);
	}
	fp = fopen(filename,"r");
	if (fp == NULL) return(0);
	strcpy(room->name,name);
	version = '1';
	rval(version);
	if (err) return(-1);
	for (i = 0 ; i < MaxLinks ; ++i)
	{
		rbuff(room->link[i]);
	} i = 0;
	for (i = 0; i < MaxLinks; ++i)
	{
		if (!strcasecmp(room->link[i],"#NOTSET#"))
			room->link[i][0] = '\0';
	} i = 0;
	for (i = 0; i < MaxKeys; ++i)
	{
		rbuff(room->key[i]);
	} i = 0;
	for (i = 0; i < MaxKeys; ++i)
	{
		if (!strcasecmp(room->key[i],"#NOTSET#"))
			room->key[i][0] = '\0';
	} i = 0;
	rbuff(room->owner);
	if (!strcmp(room->owner,"#NOTSET#")) room->owner[0] = '\0';
	rbuff(room->recap);
	if (!strcmp(room->recap,"#NOTSET#"))
		strcpy(room->recap,room->name);
	rbuff(room->map);
	if (!strcmp(room->map,"#NS#"))
		strcpy(room->map,"main");
	rbuff(room->topic);
	if (!strcmp(room->topic,"#NOTSET#"))
		strcpy(room->topic,"Welcome to this room.");
	rval(room->access);
	rval(room->no_buff);
	rval(room->topic_lock);
	rval(room->lock);
	rval(room->temp_access);
	rval(room->hidden);
	rval(room->level);
	rval(room->mesg_cnt);
	FCLOSE(fp);
	if (err) return(-1);
	if (!(load_atmos(room)))
	{
		for (i = 0 ; i < MaxAtmos ; ++i) room->atmos[i][0] = '\0';
	}
	return(1);
}

/* This saves the room's details */
int SaveRoom(Room room)
{
	FILE *fp;
	char filename[FSL];
	int err = 0,version,i=0;

	if (room == NULL) return(0);
	version = '1';
	room->name[0] = toupper(room->name[0]);
	sprintf(filename,"%s/%s.dat",RoomDir,room->name);
	fp = fopen("tempfile","w");
	if (fp == NULL)
	{
		write_log(1,LOG1,"[ERROR] - Failed to save room file for room: [%s]\n",room->name);
		return(0);
	}
	wval(version);
	for (i = 0; i < MaxLinks; ++i)
	{
		if (room->link[i][0] == '\0') strcpy(room->link[i],"#NOTSET#");
	} i = 0;
	for (i = 0; i < MaxLinks; ++i)
	{
		wbuff(room->link[i]);
	} i = 0;
	for (i = 0; i < MaxKeys; ++i)
	{
		if (room->key[i][0] == '\0') strcpy(room->key[i],"#NOTSET#");
	} i = 0;
	for (i = 0; i < MaxKeys; ++i)
	{
		wbuff(room->key[i]);
	} i = 0;
	if (room->owner[0] == '\0') strcpy(room->owner,"#NOTSET#");
	wbuff(room->owner);
	if (room->recap[0] == '\0') strcpy(room->recap,room->name);
	wbuff(room->recap);
	if (room->map[0] =='\0') strcpy(room->map,"main");
	wbuff(room->map);
	if (room->topic[0] == '\0')
		strcpy(room->topic,"Welcome to this room.");
	wbuff(room->topic);
	wval(room->access);
	wval(room->no_buff);
	wval(room->topic_lock);
	wval(room->lock);
	wval(room->temp_access);
	wval(room->hidden);
	wval(room->level);
	wval(room->mesg_cnt);
	FCLOSE(fp);
	if (err)
	{
		remove("tempfile");
		write_log(1,LOG1,"[ERROR] - Failed to save room file for room: [%s]\n",room->name);
		return(0);
	}
	else if (rename("tempfile",filename))
	{
		write_log(1,LOG1,"[ERROR] - Unable to rename 'tempfile' to '%s.dat'. Reason: [%s]\n",room->name,strerror(errno));
		return(0);
	}
	for (i = 0; i < MaxLinks; ++i)
	{
		if (!strcasecmp(room->link[i],"#NOTSET#"))
			room->link[i][0] = '\0';
	} i = 0;
	for (i = 0; i < MaxKeys; ++i)
	{
		if (!strcasecmp(room->key[i],"#NOTSET#"))
			room->key[i][0] = '\0';
	} i = 0;
	return(1);
}

/* Load up the atmospherics for the room. */
int load_atmos(Room room)
{
	FILE *fp;
	char filename[FSL],line[ARR_SIZE];
	int i=0;

	sprintf(filename,"%s/%s/%s.A",RoomDir,AtmosFiles,room->name);
	if ((fp = fopen(filename,"r")) == NULL) return(0);
	fgets(line,ARR_SIZE,fp);
	while (!(feof(fp)))
	{
		if (i >= MaxAtmos)
		{
			write_log(1,LOG1,"[ERROR] - Room: [%s] has to many atmos defined.\n",room->name);
			FCLOSE(fp);
			return(1);  /* We were able to load up to MaxAtmos */
		}
		line[strlen(line)-1] = '\0';
		strncpy(room->atmos[i],line,AtmosLen);
		line[0] = '\0'; ++i;
		fgets(line,ARR_SIZE,fp);
	}
	FCLOSE(fp);
	return(1);
}

/* Does the room exist? */
int room_exists(char *name)
{
	Rmlobj rm;

	for_rmlist(rm)
	{
		if (!strcasecmp(rm->name,name)) return(1);
	}
	return(0);
}

/* Does a user have a key to this room? */
int has_room_key(User user,Room room)
{
	int i;

	for (i = 0 ; i < MaxKeys ; ++i)
	{
		if (!strcasecmp(room->key[i],user->name)) return(1);
	}
	return(0);
}

/* This counts the amount of users in the room. */
int count_room_users(Room room)
{
	User u;
	int cnt=0;

	for_users(u)
	{
		if (u->room == room) ++cnt;
	}
	return(cnt);
}

/*
   This allows you to add a room.... I'm not really quite sure how well this
   is going to work out.. here's hoping it works out quite well...
*/
void room_admin(User user,char *inpstr,int rt)
{
	User u = NULL;
	Room room;
	int access;
	char accesstype[50],filename[FSL],tempname[RoomNameLen+2],*rmname;
	size_t i=0;

	access = 0;  accesstype[0] = tempname[0] = '\0';
	if (!strcasecmp(word[1],"-add"))
	{
		if (word_count<4)
		{
			writeus(user,usage,command[com_num].name,"~CR-add ~CB[~CRroom access~CB] [~CRroom name~CB]");
			return;
		}
		if (!strcasecmp(word[2],"PUBLIC"))
		{
			access = FIXED_PUBLIC;
			strcpy(accesstype,"FIXED_PUBLIC");
		}
		else if (!strcasecmp(word[2],"PRIVATE"))
		{
			access = FIXED_PRIVATE;
			strcpy(accesstype,"FIXED_PRIVATE");
		}
		else if (!strcasecmp(word[2],"BOTH"))
		{
			access = PUBLIC;
			strcpy(accesstype,"TOGGLEABLE");
		}
		else if (!strcasecmp(word[2],"HIDE"))
		{
			access = 69;
			strcpy(accesstype,"PUBLIC");
		}
		else
		{
			write_user(user,"~FG You specified an invalid access.\n~FG Possible types are~CB: <~CRpublic~CW/~CRprivate~CW/~CRboth~CW/~CRhide~CB>\n");
			return;
		}
		inpstr = remove_first(inpstr);
		inpstr = remove_first(inpstr);
		inpstr = ColourStrip(inpstr);
		strtolower(inpstr);
		if (room_exists(inpstr))
		{
			write_user(user,"~FG There is already a room with that name.\n");
			return;
		}
		for (i = 0 ; i < strlen(inpstr) ; ++i)
		{
			if (!isalpha(inpstr[i]) && !isspace(inpstr[i]))
			{
				write_user(user,"~FG You can only have alphabetic letters in the room name.\n");
				return;
			}
		}
		if (strlen(inpstr)>RoomNameLen)
		{
			write_user(user,"~FG That room name would be too long.\n");
			return;
		}
		if ((room = create_room()) == NULL)
		{
			write_user(user,"~FG Sorry, I was unable to create a room at this time..\n");
			return;
		}
		strcpy(room->name,inpstr);
		strcpy(room->recap,room->name);
		if (access == 69) room->hidden = 1;  else room->hidden = 0;
		switch (access)
		{
			case FIXED_PUBLIC:
			  room->access = FIXED_PUBLIC;  break;
			case FIXED_PRIVATE:
			  room->access = FIXED_PRIVATE; break;
			case PUBLIC:
			  room->access = PUBLIC;	break;
			default:
			  room->access = PUBLIC;        break;
		}
		write_user(user,"~FG You must choose which rooms to link this room via .rmadmin -link\n");
		strcpy(room->map,"main");
		strcpy(room->owner,"NULL");
		strcpy(room->topic,def_topic);
		if (!(SaveRoom(room)))
		{
			write_user(user,"~FG There was an error trying to save the rooms details...\n");
			destruct_room(room);
			return;
		}
		writeus(user,"~FT Room~CB: [~CR%s~CB] ~FTwas created. Access~CB: [~CR%s~CB] ~FTHidden~CB: [~CR%s~CB]\n",room->name,accesstype,noyes1[room->hidden]);
		writeus(user,"~FT To enter it, type~CB: ~CW'~CR.go %s~CW'\n",room->name);
		write_log(1,LOG1,"[RMADMIN] - User: [%s] Created room: [%s]\n",user->name,room->name);
		add_roomlist(room);
		destruct_room(room);
		return;
	}
	if (!strncasecmp(word[1],"-del",2))
	{
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-del ~CB[~CRroom name~CB]");
			return;
		}
		inpstr = remove_first(inpstr);
		inpstr = ColourStrip(inpstr);
		strtolower(inpstr);
		strncpy(tempname,inpstr,sizeof(tempname)-1);
		if (!strcasecmp(tempname,main_room)
		  || !strcasecmp(tempname,idle_room)
		  || !strcasecmp(tempname,jail_room))
		{
			write_user(user,"~FG You can't delete the main room, idle room, or the jail room.\n");
			return;
		}
		if ((rmname = get_room_full(user,tempname)) == NULL)
		{
			write_user(user,"~FG That room doesn't even exist.\n");
			return;
		}
		sprintf(filename,"%s/%s.dat",RoomDir,rmname);
		unlink(filename);
		sprintf(filename,"%s/%s/%s.R",RoomDir,Descript,rmname);
		unlink(filename);
		sprintf(filename,"%s/%s/%s.B",RoomDir,MesgBoard,rmname);
		unlink(filename);
		sprintf(filename,"%s/%s/%s.A",RoomDir,AtmosFiles,rmname);
		unlink(filename);
		sprintf(filename,"%s/%s.key",RoomDir,rmname);
		unlink(filename);
		if ((room = get_room(rmname)) != NULL)
		{
			for_users(u)
			{
				if (u->room == room)
				{
					write_room(room,"~CB[~CRRaMTITS~CB] - ~CRThis room is being deleted.\n");
					if ((get_room(main_room)) == NULL)
					{
						write_log(1,LOG1,"[ERROR] - Unable to find the main room.\n");
						return;
					}
					/* Move the users back to the main room. */
					u->room = get_room(main_room);
				}
			}
			destruct_room(room);
		}
		writeus(user,"~FT Room ~CW'~CR%s~CW' ~FThas been deleted.\n",rmname);
		write_log(1,LOG1,"[RMADMIN] - User: [%s] Deleted room: [%s]\n",user->name,rmname);
		clean_roomlist(rmname);
		return;
	}
	if (!strcasecmp(word[1],"-link"))
	{
		room = user->room;
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-link ~CB[~CRname of room to link to~CB]");
			return;
		}
		inpstr = remove_first(inpstr);
		inpstr = ColourStrip(inpstr);
		strtolower(inpstr);
		strncpy(tempname,inpstr,sizeof(tempname)-1);
		if (!room_exists(tempname))
		{
			writeus(user,"~FT Room~CB '~CR%s~CB'~FT doesn't even exist.\n",tempname);
			return;
		}
		if (add_link(user,room,tempname))
		{
			writeus(user,"~FG You create a link to room ~FT%s~FG from this one.\n",tempname);
			write_log(1,LOG1,"[RMADMIN] - User: [%s] Linked room: [%s] to room: [%s]\n",user->name,tempname,room->name);
			SaveRoom(room);
		}
		else write_user(user,"~FG An error occurred while trying to add the link.\n");
		return;
	}
	if (!strncasecmp(word[1],"-recap",4))
	{
		room = user->room;
		if (word_count < 3)
		{
			writeus(user,usage,command[com_num].name,"~CR-rec ~CB[~CRrecap of room's name~CB]");
			return;
		}
		inpstr = remove_first(inpstr);
		strcpy(tempname,ColourStrip(inpstr));
		if (strcasecmp(tempname,room->name))
		{
			write_user(user,"~FG The recapped name doesn't match the room name.\n");
			return;
		}
		if (strlen(inpstr) > 80)
		{
			write_user(user,"~FG That room recapped name would be too long.\n");
			return;
		}
		strcpy(room->recap,inpstr);
		writeus(user,"~FT You recapped this rooms name to appear as ~CW'~CR%s~CW'\n",room->recap);
		SaveRoom(room);
		return;
	}
	if (!strcasecmp(word[1],"-map"))
	{
		room = user->room;
		if (word_count<3)
		{
			writeus(user,usage,command[com_num].name,"~CR-map ~CB[~CRthe map the room is in~CB]");
			return;
		}
		if (strlen(word[2]) > 5)
		{
			write_user(user,"~FG That map length is too long.\n");
			return;
		}
		strcpy(room->map,word[2]);
		writeus(user,"~FG This room is now in the '%s' map.\n",room->map);
		write_log(1,LOG1,"[RMADMIN] - User: [%s] Mapped room: [%s] to [%s]\n",user->name,room->name,room->map);
		SaveRoom(room);
		clean_roomlist(room->name);
		add_roomlist(room);
		return;
	}
	writeus(user,usage,command[com_num].name,"~CR-add ~CB[~CRroom access~CB] [~CRroom name~CB]");
	writeus(user,usage,command[com_num].name,"~CR-del ~CB[~CRroom name~CB]");
	writeus(user,usage,command[com_num].name,"~CR-link ~CB[~CRname of room to link to~CB]");
	writeus(user,usage,command[com_num].name,"~CR-rec ~CB[~CRrecap of room's name~CB]");
	writeus(user,usage,command[com_num].name,"~CR-map ~CB[~CRthe map the room is in~CB]");
	return;
}

/*
   Adds a link to the room->name.link file, and then re-loads the links for
   that room.. provided it's already in memory.
*/
int add_link(User user,Room room,char *rmname)
{
	Room rm2 = NULL;
	int cnt1 = 0,i,err = 0;

	if ((rm2 = get_room(rmname)) == NULL)
	{
		if ((rm2 = create_room()) == NULL) return(0);
		strcpy(rm2->name,rmname);
		err = LoadRoom(rm2,rmname);
		if (err == 0 || err == -1)
		{
			writeus(user,"~FT Unable to load room '%s' details..\n",rmname);
			destruct_room(rm2);
			return(0);
		}
	}
	for (i = 0 ; i <MaxLinks ; ++i)
	{
		if (room->link[i][0] != '\0') ++cnt1;
	}  i = 0;
	if (rm2 == room)
	{
		write_user(user,"~CW-~FG You can't link a room to itself.\n");
		destruct_room(rm2);
		return(0);
	}
	if (cnt1 >= MaxLinks)
	{
		write_user(user,"~FG This room already has the max. amount of links.\n");
		destruct_room(rm2);
		return(0);
	}
	for (i = 0 ; i < MaxLinks ; ++i)
	{
		if (!strcmp(room->link[i],rm2->name))
		{
			writeus(user,"~FG Room ~FT%s~FG already has a link to room %s\n",room->name,rm2->name);
			destruct_room(rm2);
			return(0);
		}
		if (room->link[i][0] == '\0')
		{
			strcpy(room->link[i],rm2->name);
			destruct_room(rm2);
			break;
		}
	}
	return(1);
}

/* Saves the config file */
void save_config(void)
{
	FILE *fp;
	char filename[FSL],name1[RoomNameLen+1],name2[RoomNameLen+1];
	char name3[RoomNameLen+1];
	char *ca[]={ "NONE","IGNORE","REBOOT","SREBOOT" };
	size_t i = 0;

	sprintf(filename,"%s/config.talker",DataDir);
	if (file_exists(filename)) unlink(filename); /* Delete the old one. */
	fp = fopen(filename,"w");
	if (fp == NULL)
	{
		write_log(0,LOG1,"[ERROR] - Unable to create config file in save_config().\n");
		return;
	}
	strcpy(name1,main_room);		strcpy(name2,idle_room);
	strcpy(name3,jail_room);
	for (i = 0 ; i < strlen(name1) ; ++i)
	{
		if (name1[i] == ' ') name1[i] = '_';
	}  i = 0;
	for (i = 0 ; i < strlen(name2) ; ++i)
	{
		if (name2[i] == ' ') name2[i] = '_';
	}  i = 0;
	for (i = 0 ; i < strlen(name3) ; ++i)
	{
		if (name3[i] == ' ') name3[i] = '_';
	}  i = 0;
	fprintf(fp,"# This is a generated config file that was created via the .change command.\n\n");
	fprintf(fp,"mainport           %d\n",port[0]);
	fprintf(fp,"wizport            %d\n",port[1]);
	fprintf(fp,"minlogin_level     %s\n",minlogin_level != -1?level[minlogin_level].name:"NONE");
	fprintf(fp,"mesg_life          %d\n",mesg_life);
	fprintf(fp,"wizport_level      %s\n",level[wizport_level].name);
	fprintf(fp,"prompt_def         %s\n",offon[prompt_def]);
	fprintf(fp,"gatecrash_level    %s\n",level[gatecrash_level].name);
	fprintf(fp,"min_private        2\n");
	fprintf(fp,"ignore_mp_level    %s\n",level[ignore_mp_level].name);
	fprintf(fp,"max_users          %d\n",max_users);
	fprintf(fp,"heartbeat          2\n");
	fprintf(fp,"mesg_check_time    %02d:%02d\n",mesg_check_hour,mesg_check_min);
	fprintf(fp,"login_idle_time    180\n");
	fprintf(fp,"user_idle_time     3600\n");
	fprintf(fp,"password_echo      %s\n",noyes1[password_echo]);
	fprintf(fp,"ignore_sigterm     %s\n",noyes1[ignore_sigterm]);
	fprintf(fp,"max_clones         %d\n",max_clones);
	fprintf(fp,"ban_swearing       %s\n",noyes1[ban_swearing]);
	fprintf(fp,"crash_action       %s\n",ca[crash_action]);
	fprintf(fp,"time_out_afks      %s\n",noyes1[time_out_afks]);
	fprintf(fp,"charecho_def       %s\n",offon[charecho_def]);
	fprintf(fp,"time_out_maxlevel  %s\n",level[time_out_maxlevel].name);
	fprintf(fp,"backup_talker      %s\n",offon[backup_talker]);
	fprintf(fp,"debug_talker       %s\n",offon[debug_talker]);
	fprintf(fp,"webpage_on         %s\n",offon[webpage_on]);
	fprintf(fp,"arrest_swearers    %s\n",noyes1[arrest_swearers]);
	fprintf(fp,"free_rooms         %s\n",noyes1[free_rooms]);
	fprintf(fp,"auto_promote       %s\n",noyes1[auto_promote]);
	fprintf(fp,"auto_auth          %s\n",offon[auto_auth]);
	fprintf(fp,"main_room          %s\n",name1);
	fprintf(fp,"idle_room          %s\n",name2);
	fprintf(fp,"jail_room          %s\n",name3);
	fprintf(fp,"# End of generated config file.\n");
	FCLOSE(fp);
	write_log(1,LOG1,"[CONFIG] - Config file saved as config.talker.\n");
	confile[0] = '\0';
	strcpy(confile,"config.talker");
	return;
}
/* END OF CONFIG.C */
