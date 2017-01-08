/* dns.h								     *
 -  Header file for DNS lookup server code.				     -

 -  All of this code contained herein is Copyright 1997 by Neil Peter        -
 -  Charley. Permission to use this code authored by the above person(s) is  -
 -  given provided:						             -

 -  i) Full credit is given to the author(s).				     -

 -  ii) The code is not re-distributed in ANY form to persons other than     -
 -      yourself without the express permission of the author(s).            -
 -      (i.e. you can transfer it from account to account as long as it is   -
 -      to yourself)							     -

 -  iii) You make EVERY effort to forward bugfixes to the author(s)          -
 -       NB: Neil is co-ordinating any bugfixes that go into this program,   -
 *           so please try and send them to him first.			     */
/* $Revision 1.2 $ */

#if !defined(DNSH)
#define DNSH

#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define NONBLOCKING

#if defined(__sun__)
  /* SunOS of some description */
  #if defined(__svr4__)
    /* Solaris, i.e. SunOS 5.x */
    #if !defined(SOLARIS)
      #define SOLARIS
    #endif /* !SOLARIS */
    #define NOALARM
    /* Signal configuration */
    #define USE_SIGACTION
    #define USE_SIGEMPTYSET
    #define HAVE_SA_SIGACTION
    #define HAVE_SA_SIGACTION
    #define SIGTYPE int sigtype
  #else
    /* __sun__ && !__svr4__ (SunOS 4.x presumeably */
    #define BSD
    #define SIGTYPE int sigtype, int code, struct sigcontext *scp char *addr
    #define HAVE_BCOPY
    #define HAVE_BZERO
  #endif /* __svr4__ */
#endif /* __sun__ */

#if defined(__linux__)
  #define HAVE_UNISTDH
  #define SIGTYPE int it
  #define USE_SIGACTION
  #if defined(__GLIBC__) && (__GLIBC__ >= 2)
    #define USE_SIGEMPTYSET
  #endif /* GLIBC >= 2 */
#endif /* __linux__ */

#if defined(__FreeBSD__) || defined(__NetBSD__)
  #define SIGTYPE int i
  #define USE_SIGACTION
  #define USE_SIGEMPTYSET
  #define HAVE_UNISTDH
  #define HAVE_BZERO
#endif /* __FreeBSD__ || __NetBSD__ */

#define AWAITING_DNS		-1
#define DNS_SERVER_PID_PATH	"runtime/dns_server.pid"
#define DNS_LOOKUP_ATTEMPTS	3

/* the various fd's for the two pipes between client and server */
#define DNS_SERVER_READ		dns_toserver_fds[0]
#define DNS_SERVER_WRITE	dns_toclient_fds[1]
#define DNS_CLIENT_READ		dns_toclient_fds[0]
#define DNS_CLIENT_WRITE	dns_toserver_fds[1]

/* And the various fd's for between the server and the lookup children */
#define DNS_PARENT_READ		from_child_fds[0]
#define DNS_PARENT_WRITE	to_child_fds[1]
#define DNS_CHILD_READ		to_child_fds[0]
#define DNS_CHILD_WRITE		from_child_fds[1]

/* messages between client and server */
#define DNS_SERVER_CONNECT_MSG		"Server Connect:"
#define DNS_CLIENT_CONNECT_MSG		"Client Connect:"
#define DNS_CLIENT_SEND_REQUEST		'A'
#define DNS_CLIENT_CANCEL_REQUEST	'B'
#define DNS_SERVER_SEND_REPLY		':'

/* Misc #defines */
#define MAX_DNS_IN_PROGRESS	5 /* Unused atm */
#define TIMER_CLICK		5

typedef int	dns_identifier;

/* struct for a dns request */
struct s_dns_request {
	dns_identifier		dns_id;
	struct in_addr		ip_addr;
	time_t			request_time;
	int			dusing;		/* dns child slot number */
	struct s_dns_request	*next;
	};
typedef struct s_dns_request dns_request;

struct s_dns_child {
	int			to_child_fds[2];
	int			from_child_fds[2];
	pid_t			pid;
	struct s_dns_request	*request;
	};
typedef struct s_dns_child dns_child;

#endif /* DNSH */
