/*
 * idclient.c
 *
 */

/*
 The majority of the code contained herein is Copyright 1995-2000 by
 Neil Peter Charley.
 Portions of the code (notably that for Solaris 2.x and BSD compatibility)
 are Copyright 1996 James William Hawtin. Thanks also goes to James
 for pointing out ways to make the code more robust.
 ALL code contained herein is covered by one or the other of the above
 Copyright's.

 Permission to use this code authored by the above persons is given
 provided:

   i) Full credit is given to the author(s).

  ii) The code is not re-distributed in ANY form to persons other than
     yourself without the express permission of the author(s).
     (i.e. you can transfer it from account to account as long as it is
      to yourself)
       NB: Special permission has been given to the maintainer(s) of the
	  package known as 'SensiSummink' AND to the maintainer(s) of
	  the package known as 'PG+' (later version of PG96) AND to the
	  maintainer(s) of the package known as 'RaMTITS' to make use of
	  this code in that.  Anyone using this package automatically
	  gets the right to USE this code.  You are *NOT*, however,
	  given permission to redistribute it in other forms other
	  than passing on the package that you yourself recieved.

 iii) You make reasonable effort to forward bugfixes to the author(s)
     NB: Neil is co-ordinating any bugfixes that go into this program, so
         please try and send them to him first.  The current best email
         address for contacting him is:

		    ident_daemon@miggy.org
 */

char identclientc_rcsid[] = "$Revision: 1.5 $";


#if defined(__STRICT_ANSI__)
 #include <features.h>
 #define __USE_POSIX
 #define __USE_BSD
 #define __USE_MISC 
#endif /* __STRICT_ANSI__ */

#include "include/ident.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#if defined(HAVE_UNISTDH)
 #include <unistd.h>
#endif /* HAVE_UNISTDH */
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h> 
#include <sys/socket.h>
#if defined(HAVE_FILIOH)
 #include <sys/filio.h>
#endif /* HAVE_FILIOH */
#if !defined(OSF)
 #include <sys/wait.h>
#endif /* !OSF */

#define IDCLIENT_SOURCE
  #include "include/needed.h"
#undef IDCLIENT_SOURCE

#if defined(ANSI)
 extern int kill __P ((__pid_t __pid, int __sig));
 extern void bzero __P ((__ptr_t __s, size_t __n));
#endif

/* This is an UGLY hack, 'cos the gcc setup on rabbit precludes the
 * #include'ing of both <sys/wait.h> and <sys/time.h>
 * The lines below (inside the #if defined) are from <sys/wait.h> on rabbit
 */
#if defined(OSF)
/* WNOHANG causes the wait to not hang if there are no stopped or terminated */
#define WNOHANG         0x1     /* dont hang in wait                     */
#endif /* OSF */

/* Extern functions */

/* Extern variables */

#ifndef errno
extern int		 errno;
#endif

/* Local functions */

int			 init_ident_server(void);
void			 kill_ident_server(void);
void			 ident_process_reply(int msg_size);
void			 read_ident_reply(void);
void			 send_ident_request(User u,
	 struct sockaddr_in *sadd);
void			 setup_itimer(void);
void			 stdarg_log(char *str, ...);
void			 log(char *,char *, ...);

/* Local Variables */

#define BUFFER_SIZE 2048      /* Significantly bigger than the
			       * equivalent in ident_server.c
			       */
int			 ident_toclient_fds[2];
int			 ident_toserver_fds[2];
int			 ident_server_pid = 0;
ident_identifier	 ident_id = 0;
char			 ident_buf_input[BUFFER_SIZE];
char			 ident_buf_output[BUFFER_SIZE];
char			 reply_buf[BUFFER_SIZE];

/*
 * Start up the ident server
 */

int init_ident_server(void)
{
	int ret;
#if defined(FIONBIO)
	int dummy;
#endif /* FIONBIO */
	fd_set fds;
	int i,cnt = 0;
	struct timeval timeout;

	if (-1 == pipe(ident_toclient_fds))
	{
		switch (errno)
		{
			case EMFILE:
				log("boot", "init_ident_server: Too many fd's in use by"
		 		" process");
				exit(1);
			case ENFILE:
				log("boot", "init_ident_server: Too many fd's in use in"
		 		" system");
				exit(1);
			case EFAULT:
				log("boot", "init_ident_server: ident_toclient_fds invalid!");
				exit(1);
		}
	}
	if (-1 == pipe(ident_toserver_fds))
	{
		switch (errno)
		{
			case EMFILE:
				log("boot", "init_ident_server: Too many fd's in use by"
		 		" process");
				exit(1);
			case ENFILE:
				log("boot", "init_ident_server: Too many fd's in use in"
		 		" system");
				exit(1);
			case EFAULT:
				log("boot", "init_ident_server: ident_toserver_fds invalid!");
				exit(1);
		}
	}

	ret = fork();
	switch (ret)
	{
		case -1: /* Error */
			log("boot", "init_ident_server couldn't fork!");
			exit(1);
		case 0:   /* Child */
			close(IDENT_CLIENT_READ);
			close(IDENT_CLIENT_WRITE);
			close(0);
			dup(IDENT_SERVER_READ);
			close(IDENT_SERVER_READ);
			close(1);
			dup(IDENT_SERVER_WRITE);
			close(IDENT_SERVER_WRITE);
			IDENT_SERVER_READ=IDENT_SERVER_WRITE=-1;
			for (i=3;i<(1<<12);i++)
			{
				if (i==IDENT_SERVER_READ) continue;
				if (i==IDENT_SERVER_WRITE) continue;
				if (close(i) == 0) ++cnt;
			}
			execlp("./ident_server", "ident_server", 0);
			log("boot", "init_ident_server failed to exec ident_server");
			exit(1);
		default: /* Parent */
			ident_server_pid = ret;
			close(IDENT_SERVER_READ);
			close(IDENT_SERVER_WRITE);
			IDENT_SERVER_READ = IDENT_SERVER_WRITE = -1;
#if defined(FIONBIO)
	 if (ioctl(IDENT_CLIENT_READ, FIONBIO, &dummy) < 0)
	 {
		log("error", "Ack! Can't set non-blocking to ident read.");
	 }
	 if (ioctl(IDENT_CLIENT_WRITE, FIONBIO, &dummy) < 0)
	 {
		log("error", "Ack! Can't set non-blocking to ident write.");
	 }
#endif /* FIONBIO */
	}

	FD_ZERO(&fds);
	FD_SET(IDENT_CLIENT_READ, &fds);
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	while (-1 == (ret = select(FD_SETSIZE, &fds, 0, 0, &timeout)))
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			continue;
		}
		log("boot", "init_ident_server: Timed out waiting for server"
				  " connect");
		kill_ident_server();
		return 0;
	}

	ioctl(IDENT_CLIENT_READ, FIONREAD, &ret);
	while ((size_t)ret != strlen(IDENT_SERVER_CONNECT_MSG))
	{
		sleep(1);
		ioctl(IDENT_CLIENT_READ, FIONREAD, &ret);
	}
	ret = read(IDENT_CLIENT_READ, ident_buf_input, ret);
	ident_buf_input[ret] = '\0';
	if (strcmp(ident_buf_input, IDENT_SERVER_CONNECT_MSG))
	{
		fprintf(stderr, "From Ident: '%s'\n", ident_buf_input);
		log("boot", "init_ident_server: Bad connect from server, killing");
		kill_ident_server();
		return 0;
	}
	log("boot", "Ident Server Up and Running");

	setup_itimer();
	return 1;
}

/* Shutdown the ident server */

void kill_ident_server(void)
{
	int status;

	close(IDENT_CLIENT_READ);
	close(IDENT_CLIENT_WRITE);
	IDENT_CLIENT_READ = -1;
	IDENT_CLIENT_WRITE = -1;
	kill(ident_server_pid, SIGTERM);
	/* We can't use sleep(3) in case we interfere with other
	 * timers... */
	waitpid(-1, &status, WNOHANG);
	/* Make sure the bugger dies, we're assuming it has at this
	 * point anyway, and it's not as if it holds any state info
	 * across different instances of itself */
	kill(ident_server_pid, SIGKILL);
}

void send_ident_request(User u, struct sockaddr_in *sadd)
{
	char *s;
	int bwritten;
	struct sockaddr_in sname;
#if defined(__GLIBC__) || (__GLIBC__ >= 2)
	socklen_t l;
#else
	int l;
#endif

	l = sizeof(struct sockaddr_in);
	getsockname(u->socket, (struct sockaddr *)&sname, &l);
#if defined(DEBUG_IDENT_TOO)
fprintf(stderr, "send_ident_request: sockname = '%s'\n",
	inet_ntoa(sname.sin_addr));
#endif /* DEBUG_IDENT_TOO */

	s = ident_buf_output;
	*s++ = IDENT_CLIENT_SEND_REQUEST;
	memcpy(s, &ident_id, sizeof(ident_id));
	s += sizeof(ident_id);
	memcpy(s, &(sname.sin_addr.s_addr), sizeof(sname.sin_addr.s_addr));
	s += sizeof(sname.sin_addr.s_addr);
	memcpy(s, &(sname.sin_port), sizeof(sname.sin_port));
	s += sizeof(sname.sin_port);
	memcpy(s, &(sadd->sin_family), sizeof(sadd->sin_family));
	s += sizeof(sadd->sin_family);
	memcpy(s, &(sadd->sin_addr.s_addr), sizeof(sadd->sin_addr.s_addr));
	s += sizeof(sadd->sin_addr.s_addr);
	memcpy(s, &(sadd->sin_port), sizeof(sadd->sin_port));
	s += sizeof(sadd->sin_port);
	bwritten = write(IDENT_CLIENT_WRITE, ident_buf_output,
						  (s - ident_buf_output));
	u->ident_id = ident_id++;
	if (bwritten < (s - ident_buf_output))
	{
		log("ident", "Client failed to write request, killing and restarting"
					" Server");
		kill_ident_server();
		/* FIXME: Maybe test for presence of PID file? */
		sleep(3);
		init_ident_server();
		bwritten = write(IDENT_CLIENT_WRITE, ident_buf_output,
							  (s - ident_buf_output));
		if (bwritten < (s - ident_buf_output))
		{
			log("ident", "Restart failed");
		}
	} else
	{
#if defined(DEBUG_IDENT)
		stdarg_log("ident_ids", "Player '%s', fd %d, ident_id %d",
			 u->name[0] ? u->name : "<NOT-ENTERED>",
	  u->socket,
	  u->ident_id);
#endif /* DEBUG_IDENT */
	}
	
#if defined(DEBUG_IDENT)
	fprintf(stderr, "Bytes Written %d, Should have sent %d\n",
		bwritten, (s - ident_buf_output)); 
	fprintf(stderr, "Client: %08X:%d\n",
			  (int) ntohl(sadd->sin_addr.s_addr),
			  ntohs(sadd->sin_port));
	fflush(stderr);
#endif
}

void read_ident_reply(void)
{
	int bread;
	int toread;
	int i;
	static int bufpos = 0;

	ioctl(IDENT_CLIENT_READ, FIONREAD, &toread);
	if (toread <= 0)
	{
		return;
	}
	bread = read(IDENT_CLIENT_READ, ident_buf_input, BUFFER_SIZE - 20);
	ident_buf_input[bread] = '\0';

	for (i = 0 ; i < bread ; )
	{
		reply_buf[bufpos++] = ident_buf_input[i++];
		if (((size_t)bufpos > (sizeof(char) + sizeof(ident_identifier)))
			 && (reply_buf[bufpos - 1] == '\n'))
		{
	 ident_process_reply(bufpos);
			bufpos = 0;
#if defined(HAVE_BZERO)
	 bzero(reply_buf, BUFFER_SIZE);
#else
	 memset(reply_buf, 0, BUFFER_SIZE);
#endif /* HAVE_BZERO */
		}
	}
}

void ident_process_reply(int msg_size)
{
	char *s, *t;
	char reply[41 + 1];
	int i;
	ident_identifier id;
	User u;

	for (i = 0 ; i < msg_size ;)
	{
		switch (reply_buf[i++])
		{
			case IDENT_SERVER_SEND_REPLY:
				memcpy(&id, &reply_buf[i], sizeof(ident_identifier));
				i += sizeof(ident_identifier);
#if defined(DEBUG_IDENT)
		 stdarg_log("ident_ids", "Got reply for ident_id %d", 
			  id);
#endif /* DEBUG_IDENT */
				for_users(u)
				{
					if (u->ident_id == id)
					{
#if defined(DEBUG_IDENT)
		  stdarg_log("ident_ids", "Matched ident_id %d to Player '%s',"
		  		   " fd %d",
				id,
				u->name[0] ? u->name : "<NOT-ENTERED>",
				u->socket);
#endif /* DEBUG_IDENT */
						break;
					}
				}
#if defined(DEBUG_IDENT)
		 fprintf(stderr, "Ident Client: Got reply '%s'\n", &reply_buf[i]);
#endif /* DEBUG_IDENT */
				s = strchr(&reply_buf[i], '\n');
				if (s)
				{
					*s++ = '\0';
				} else
				{
			 s = strchr(reply_buf, '\0');
			 *s++ = '\n';
			 *s = '\0';
				}
		 if (u)
		 {
				  /* *sigh*, some git might set up an identd that sends crap
					* stuff back, so we need to filter the reply
					* RFC1413 does state that the 'result' should consist
					* of printable characters:

						  "Returned user identifiers are expected to be printable
								 in the character set indicated."

					* Default character set is US-ASCII, and we're not catering
					* for anything else, at least not yet.
					*/
			 s = &reply_buf[i];
			 t = reply;
			 while (*s && t < &reply[51])
			 {
						if (isprint(*s))
						{
								*t++ = *s++;
						} else
						{
								s++;
						}
			 }
			 *t = '\0';
			strncpy(u->realname, reply, sizeof(u->realname)-1);
#if defined(DEBUG_IDENT)
			 stdarg_log("ident_ids", "Write ident_id %d, Reply '%s' to player '%s'"
			 		        " fd %d",
			id,
			u->realname,
			u->name[0] ? u->name : "<NOT-ENTERED>",
			u->socket);
#endif /* DEBUG_IDENT */
				} else
				{
					/* Can only assume connection dropped from here and we still
					 * somehow got a reply, throw it away
					 */
#if defined(DEBUG_IDENT)
			 stdarg_log("ident_ids", "Threw away response for ident_id %d",
				  id);
#endif /* DEBUG_IDENT */
				}
		 while (reply_buf[i] != '\n')
		 {
			 i++;
		 }
				break;
			default:
#if defined(DEBUG_IDENT_TOO)
		 stdarg_log("ident", "Bad reply from server '%d'", reply_buf[i]);
#endif /* DEBUG_IDENT_TOO */
				i++;
		}
	}
}


/* log using stdarg variable length arguments */

void stdarg_log(char *str, ...)
{
	va_list args;
	FILE *fp;
	char *fmt;
	struct tm *tim;
	char the_time[256];
	char fname[21];
	time_t current_time;

	va_start(args, str);

	current_time = time(0);
	tim = localtime(&current_time);
	strftime((char *)&the_time, 255, "%H:%M:%S %Z - %d/%m/%Y - ", tim);
	sprintf(fname, "logs/%s.log", str);

	fp = fopen(fname, "a");
	if (!fp)
	{
		fprintf(stderr, "Eek! Can't open file to log: %s\n", fname);
		return;
	}
	fprintf(fp, "%s", the_time);
	fmt = va_arg(args, char *);
	vfprintf(fp, fmt, args);
	va_end(args);
	fprintf(fp, "\n");
	fclose(fp);
}

/* Log using stdarg variable length arguments */
void log(char *str1,char *str,...)
{
va_list args;
FILE *fp;
struct tm *tim;    
char the_time[256];
char fname[256],buffer[2000];
time_t current_time;

buffer[0]='\0';
va_start(args,str);
vsprintf(buffer,str,args);
va_end(args);
current_time=time(0);
tim=localtime(&current_time);
strftime((char *)&the_time,255,"[%m/%d %I:%M:%S %p]",tim);
sprintf(fname,"../systemlogs/identlog");
fp=fopen(fname,"a");   
if (fp==NULL) {
        fprintf(stderr,"Unable to open 'identlog'. Reason: %s\n",strerror(errno));
        return;
        }
fprintf(fp,"%s %s\n",the_time,buffer);
fclose(fp);
}

void setup_itimer(void)
{
struct itimerval inew,iold;

inew.it_interval.tv_sec=0;
inew.it_interval.tv_usec=(1000000/TIMER_CLICK);
inew.it_value.tv_sec=0;
inew.it_value.tv_usec=inew.it_interval.tv_usec;
if (setitimer(ITIMER_REAL,&inew,&iold)<0)
  log("error","[IDENT] - Can't set timer.\n");
}

