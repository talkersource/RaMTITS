/*
  string.c
    Source code needed for the manipulation of strings.

    The source within this file is Copyright 1998 - 2001 by
        Rob Melhuish, Andrew Collington, Arnaud Abelard, and
	Tim van der Leeuw

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
#define _STRING_C
  #include "include/needed.h"
#undef _STRING_C

/* strip ctrl-chars out of the string. */
void remove_ctrl_chars(User user,char *str)
{
	char *src,*dest;
	int bell=7,tab=9;

	/* Only allow ctrl-g from user->level>=WIZ */
	if (user->level<WIZ) bell = '*';
	dest=src=str;
	while (*src)
	{
		if (((unsigned char)*src >= ' ' && (unsigned char)*src<127)
		  || *src == bell || *src == tab)
		{
			*dest++ = *src;
		}
		++src;
	}
	*dest = 0;
}

/* Put string terminate char at first char < 32 */
void terminate(char *str)
{
	int i,bell=7,tab=9;

	for (i = 0 ; i<ARR_SIZE ; ++i)
	{
		if ((*(str+i)<32 && *(str+i) != bell && *(str+i) != tab)
		  || *(str+i)>126)
		{
			*(str+i) = 0;
			i = ARR_SIZE;
		}
	}
}

/* Return pos. of second word in inpstr */
char *remove_first(char *inpstr)
{
	char *pos=inpstr;

	while (*pos<33 && *pos) ++pos;
	while (*pos>32) ++pos;
	while (*pos<33 && *pos) ++pos;
	return(pos);
}

/* See if string contains any swearing. */
int contains_swearing(char *str)
{
	char *s;
	int i;

	s = (char *)malloc(strlen(str)+1);
	if (s == NULL)
	{
		write_log(0,LOG1,"[ERROR] - malloc FAILED in contains_swearing().\n");
		return(0);
	}
	strcpy(s,ColourStrip(str));
	strtolower(s);
	i = 0;
	while (swear_words[i][0] != '*')
	{
		if (strstr(s,swear_words[i]))
		{
			free(s);
			s = NULL;
			return(1);
		}
		++i;
	}
	free(s);
	s = NULL;
	return(0);
}

/* Count the number of colour commands in a string */
int colour_com_count(char *str)
{
	char *s;
	int i,cnt=0;

	s = str;
	while (*s)
	{
		if (*s == '~')
		{
			++s;
			for (i = 0 ; i<NumCols ; ++i)
			{
				if (!strncmp(s,colcom[i],2))
				{
					cnt++;
					s++;
					break;;
				}
			}
			if (!strncmp(s,"RD",2))
			{
				cnt++;
				s++;
				continue;
			}
			continue;
		}
		++s;
	}
	return(cnt);
}

/* Strip out colour commands from a string. */
char *ColourStrip(char *str)
{
	char *s,*t;
	static char text2[ARR_SIZE*2];
	int i;

	if (str == NULL) return(NULL);
	s = str;
	t = text2;
	while (*s)
	{
		if (*s == '~')
		{
			++s;
			for (i = 0 ; i<NumCols ; ++i)
			{
				if (!strncmp(s,colcom[i],2))
				{
					s++;
					goto CONT;
				}
			}
			if (!strncmp(s,"RD",2))
			{
				s++;
				goto CONT;
			}
			--s;
			*t++ = *s;
		}
		else *t++ = *s;
		CONT:
		  s++;
	}
	*t = '\0';
	return(text2);
}

/* Centers the text, derived from Moenuts (Mike Irving) */
char *center(char *str,int clen)
{
	static char text2[ARR_SIZE*2];
	char text3[ARR_SIZE*2];
	int len=0,spc=0;

	strcpy(text3,str);
	len=strlen(ColourStrip(text3));
	if (len>clen)
	{
		strcpy(text2,text3);
		return(text2);
	}
	spc = (clen/2)-(len/2);
	sprintf(text2,"%*.*s",spc,spc," ");
	strcat(text2,text3);
	return(text2);
}

/*
   Searches ss for sf, derived from Amnuts (Andrew Collington) modded by me
   Rob Melhuish.
*/
int ainstr(char *ss,char *sf)
{
	register int f,g;

	for (f = 0 ; *(ss+f) ; ++f)
	{
		for (g=0;;++g)
		{
			if (*(sf+g) == '\0' && g>0) return(f);
			if (*(sf+g) != *(ss+f+g)) break;
		}
	}
	return(-1);
}

/*
   Returns a pointer to the first occurence of a pattern in the string,
   regardless of case. From Amnuts (Andrew Collington)
*/
char *istrstr(char *str,char *pat)
{
	char *pptr,*sptr,*start;
	uint slen,plen;

	for (start = (char *)str,pptr = (char *)pat,slen = strlen(str),
	  plen = strlen(pat) ; slen >= plen ; start++,slen--)
	{
		/* Find start of pattern in string */
		while (toupper(*start) != toupper(*pat))
		{
			start++;
			slen--;
			if (slen<plen) return(NULL);
		}
		sptr = start;
		pptr = (char *)pat;
		while (toupper(*sptr) == toupper(*pptr))
		{
			sptr++;
			pptr++;
			if (*pptr == '\0') return(start);
		}
	}
	return(NULL);
}

/*
   Searches a string for 'find' and replaces it with 'replace'. From
   Amnuts (Andrew Collington).
*/
char *andys_replace_string(char *str,char *find,char *replace)
{
	int flen,rlen;
	char *x,*y;

	if (NULL == (x = (char *)istrstr(str,ColourStrip(find)))) return(x);
	flen = strlen(find);	rlen = strlen(replace);
	memmove(y = x+rlen,x+flen,strlen(x+flen)+1);
	memcpy(x,replace,rlen);
	return(str);
}

/* copies chunk from string strf to string strt */
void midcpy(char *strf,char *strt,int fr,int to)
{
	int f;

	for (f = fr ; f <= to ; ++f)
	{
		if (!strf[f])
		{
			strt[f-fr] = '\0';
			return;
		}
		strt[f-fr] = strf[f];
	}
	strt[f-fr] = '\0';
}

/* strip all trailing tabs/spaces of input string and return new length. */
int rtrim(char *inp)
{
	int i;

	i = strlen(inp);
	if (i == 0) return(0);
	while (i>0 && (inp[--i] == ' ' || inp[i] == '\t')) inp[i] = 0;
	return(inp[0] == 0?0:i+1);
}

/* Figures out which variable to is to return what. */
char *which_var(User user,char *which_one)
{
	static char temp[ARR_SIZE];

	temp[0] = '\0';
	if (!strcasecmp(which_one,"$USER"))
	  sprintf(temp,"%s",user->recap);
	else if (!strcasecmp(which_one,"$SITE"))
	  sprintf(temp,"%s",user->site);
	else if (!strcasecmp(which_one,"$HANGCH"))
	  sprintf(temp,"%s",user->hangman_word);
	else if (!strcasecmp(which_one,"$GENDER"))
	  sprintf(temp,"%s",gendert[user->gender]);
	else if (!strcasecmp(which_one,"$EMAIL"))
	  sprintf(temp,"%s",user->email);
	else if (!strcasecmp(which_one,"$URL"))
	  sprintf(temp,"%s",user->webpage);
	else if (!strcasecmp(which_one,"$LALIAS"))
	  sprintf(temp,"%s",user->lev_rank);
	else if (!strcasecmp(which_one,"$BDAY"))
	  sprintf(temp,"%s",user->bday);
	else if (!strcasecmp(which_one,"$RLEVEL"))
	  sprintf(temp,"%s",user->gender == Female?level[user->level].fname:level[user->level].mname);
	else if (!strcasecmp(which_one,"$RNAME"))
	  sprintf(temp,"%s",user->name);
	else if (!strcasecmp(which_one,"$TIME"))
	  sprintf(temp,"%s",long_date(0));
	else if (!strcasecmp(which_one,"$ICQ"))
	  sprintf(temp,"%ld",user->icq);
	else if (!strcasecmp(which_one,"$MONEY"))
	  sprintf(temp,"%ld",user->money);
	else if (!strcasecmp(which_one,"$DMONEY"))
	  sprintf(temp,"%ld",user->dep_money);
	else if (!strcasecmp(which_one,"$CHIPS"))
	  sprintf(temp,"%ld",user->chips);
	else if (!strcasecmp(which_one,"$DCHIPS"))
	  sprintf(temp,"%ld",user->dep_chips);
	else if (!strcasecmp(which_one,"$AGE"))
	  sprintf(temp,"%d",user->age);
	else if (!strcasecmp(which_one,"$PDESC"))
	  sprintf(temp,"%s",user->predesc);
	else if (!strcasecmp(which_one,"$DESC"))
	  sprintf(temp,"%s",user->desc);
	else sprintf(temp,"Unknown variable");
	return(temp);
}

/*
   Go through the string, and replace any of the $VARS with the correct var.
   which is found in the which_var() function.
*/
char *parse_string(User user,char *str)
{
	static char text2[ARR_SIZE*2];
	char *s,*t,*str2;
	int x;

	x = 0;  t = text2;  s = str;  str2 = NULL;
	if (user->name[0] == '\0') return(str);
	while (*s)
	{
		for (x = 0 ; x<NumVars ; ++x)
		{
			if (!strcasecmp(s,"$HTML")) continue;
			if (!strncasecmp(s,user_vars[x],strlen(user_vars[x])))
			{
				memcpy(t,which_var(user,user_vars[x]),strlen(which_var(user,user_vars[x])));
				t += strlen(which_var(user,user_vars[x]))-1;
				s += strlen(user_vars[x])-1;
				goto CONT;
			}
		}
		*t = *s;
		CONT:
		  ++s; ++t;
	}
	*t = '\0';
	if ((user->ignore & Swears) && contains_swearing(text2))
	{
		str2 = replace_swear_words(text2);
		return(str2);
	}
	return(text2);
}

/*
   This goes through the string, and replaces any instance of user->fmacros[i]
   with user->rmacros[i].
*/
char *replace_macros(User user,char *str)
{
	char *s,*t;
	static char text2[ARR_SIZE*2];
	int i,cnt = 0;

	/*
	   Check the length of the string, if greater then array size, just
	   return the string, other wise we could crash big time.
	*/
	if (strlen(str) >= ARR_SIZE*2) return(str);
	if (user->has_macros == 0) return(str);
	/*
	   Go through, if the string doesn't contain any
	   macros, just return the string.
	*/
	s = str;
	cnt = 0;
	while (*s)
	{
		for (i = 0; i < 16; ++i)
		{
			if (user->fmacros[i][0] == '\0') continue;
			if (!strncmp(s,user->fmacros[i],strlen(user->fmacros[i])))
			{
				++cnt;
				s += strlen(user->fmacros[i])-1;
			}
		}
		s++;
	}
	if (cnt == 0 || cnt > 20) return(str);
	/* Actually go through now and replace the macros. */
	text2[0] = '\0';
	i = cnt = 0;
	s = str;
	t = text2;
	while (*s)
	{
		for (i = 0; i < 16; ++i)
		{
			if (user->fmacros[i][0] == '\0') continue;
			if (!strncmp(s,user->fmacros[i],strlen(user->fmacros[i])))
			{
				memcpy(t,user->rmacros[i],strlen(user->rmacros[i]));
				t += strlen(user->rmacros[i])-1;
				s += strlen(user->fmacros[i])-1;
				/*
				   If the user has a hat.. go add it after the
				   macro has been replaced...
				*/
				if (user->hat)
				{
					++t;
					memcpy(t,hatcolor[user->hat],strlen(hatcolor[user->hat]));
					t += strlen(hatcolor[user->hat])-1;
				}
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

/* replaces the swear words with a random censored word. */
char *replace_swear_words(char *str)
{
	int x,y;
	char *str2=NULL;

	x = y = 0;
	y = ran(NumCensors);
	while (swear_words[x][0] != '*')
	{
		while (str!=NULL)
		{
			str2 = str;
			str = andys_replace_string(str2,swear_words[x],censored[y]);
		}
		++x;
		str = str2;
	}
	return(str2);
}

/* Convert string to upper case */
void strtoupper(char *str)
{
	while(*str)
	{
		*str = toupper(*str);
		str++;
	}
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

/* Returns 1 if string is a positive number */
int isnumber(char *str)
{
	while(*str) if (!isdigit(*str++)) return(0);
	return(1);
}

/* This should return the type of smiley that is in a string. */
char *smiley_type(char *str,int com)
{
	int len=0;
	static char type1[15],type2[15],type3[15];

	type1[0] = '\0';
	type2[0] = '\0';
	type3[0] = '\0';
	len = strlen(str);
	switch (str[len-1])
	{
		case '?':
		  if (str[len-2] == '!' && str[len-3] == '?'
		    && str[len-4] == '!')
		  {
			strcpy(type1,"wtf");	strcpy(type2,"wtf to");
			strcpy(type3,"wtfs at");
		  }
		  else
		  {
			strcpy(type1,"ask");	strcpy(type2,"ask");
			strcpy(type3,"asks");
		  }
		  break;
		case '!':
		  if (str[len-2] == '?' && str[len-3] == '!'
		    && str[len-4] == '?')
		  {
			strcpy(type1,"wtf");	strcpy(type2,"wtf to");
			strcpy(type3,"wtfs at");
		  }
		  else
		  {
			strcpy(type1,"exclaim");
			strcpy(type2,"exclaim to");
			strcpy(type3,"exclaims to");
		  }
		  break;
		case '(':
		  if ((str[len-2] == ':' || str[len-2] == '=')
		    || ((str[len-2] == '-' || str[len-2] == 'o')
		    && (str[len-3] == ':' || str[len-3] == '=')))
		  {
			strcpy(type1,"frown");	strcpy(type2,"frown at");
			strcpy(type3,"frown to");
		  }
		  else if ((str[len-2] == '~' || str[len-2] == ',')
		    && str[len-3] == ':')
		  {
			strcpy(type1,"crie");	strcpy(type2,"cry to");
			strcpy(type3,"cries to");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case 'P':
		case 'p':
		  if ((str[len-2] == ':' || str[len-2] == '='
		    || str[len-2]==';') || ((str[len-2] == '-'
		    || str[len-2] == 'o') && (str[len-3] == ':'
		    || str[len-3]=='=' || str[len-3]==';')))
		  {
			strcpy(type1,"pfffft");	strcpy(type2,"pfffft at");
			strcpy(type3,"pffffts at");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case ')':
		  if ((str[len-2] == ':' || str[len-2] == '=')
		    || ((str[len-2] == '-' || str[len-2] == 'o')
		    && (str[len-3] == ':' || str[len-3] == '=')))
		  {
			strcpy(type1,"smile");	strcpy(type2,"smile at");
			strcpy(type3,"smiles at");
		  }
		  else if ((str[len-2] == ';') || ((str[len-2] == '-'
		    || str[len-2] == 'o') && str[len-3] == ';'))
		  {
			strcpy(type1,"wink");	strcpy(type2,"wink at");
			strcpy(type3,"winks at");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case '/':
		  if (str[len-2] == ':' || str[len-2] == '='
		    || str[len-2] == ';')
		  {
			strcpy(type1,"grimace"); strcpy(type2,"grimace at");
			strcpy(type3,"grimaces at");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case 'D':
		  if ((str[len-2] == ':' || str[len-2] == '='
		    || str[len-2] == ';') || ((str[len-2] == '-'
		    || str[len-2] == 'o') && (str[len-3] == ':'
		    || str[len-3] == '=' || str[len-3] == ';')))
		  {
			strcpy(type1,"laugh");	strcpy(type2,"laugh at");
			strcpy(type3,"laughs at");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case 'I':
		  if ((str[len-2] == ':' || str[len-2] == '='
		    || str[len-2] == ';') || ((str[len-2] == '-'
		    || str[len-2] == 'o') && (str[len-3] == ':'
		    || str[len-3] == '=' || str[len-3] == ';')))
		  {
			strcpy(type1,"hmmmm");	strcpy(type2,"hmmmm at");
			strcpy(type3,"hmmmms at");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case 'O':
		  if ((str[len-2] == ':' || str[len-2] == '='
		    || str[len-2] == ';') || ((str[len-2] == '-'
		    || str[len-2] == 'o') && (str[len-3] == ':'
		    || str[len-3] == '=' || str[len-3] == ';')))
		  {
			strcpy(type1,"yell");	strcpy(type2,"yell at");
			strcpy(type3,"yells at");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case '9':
		  if ((str[len-2] == ':' || str[len-2] == '='
		    || str[len-2] == ';') || ((str[len-2] == '-'
		    || str[len-2] == 'o') && (str[len-3] == ':'
		    || str[len-3] == '=' || str[len-3] == ';')))
		  {
			strcpy(type1,"slurp");	strcpy(type2,"slurp");
			strcpy(type3,"slurps");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		case '~':
		  if ((str[len-2] == 'P' || str[len-2] == 'p')
		    && (str[len-3] == ':' || str[len-3] == '='
		    || str[len-3] == ';'))
		  {
			strcpy(type1,"drool");	strcpy(type2,"drool on");
			strcpy(type3,"drools on");
		  }
		  else if ((str[len-2] == 'P' || str[len-2] == 'p')
		    && (str[len-3] == '-' || str[len-3] == 'o')
		    && (str[len-4] == ':' || str[len-4] == '='
		    || str[len-4]==';'))
		  {
			strcpy(type1,"drool");	strcpy(type2,"drool on");
			strcpy(type3,"drools on");
		  }
		  else
		  {
			strcpy(type1,"say");	strcpy(type2,"tell");
			strcpy(type3,"tells");
		  }
		  break;
		default :
		  strcpy(type1,"say");
		  strcpy(type2,"tell");
		  strcpy(type3,"tells");
	}
	switch (com)
	{
		case 1 : return(type1);
		case 2 : return(type2);
		case 3 : return(type3);
		default: return(type1);
	}
}

/* END OF STRING.C */
