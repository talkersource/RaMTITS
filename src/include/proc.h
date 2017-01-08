extern int uptime (double *uptime_secs,double *idle_secs);
static char buf2[128];

typedef struct proc_s {
	char
	  cmd[40],	/* basename of executable file in call to exec(2)  */
	  state,	/* single-char code for process state (S=sleeping) */
	  ttyc[5];	/* string representation of controlling tty device */
	int
	  uid,		/* User ID					*/
	  pid,		/* Process ID					*/
	  ppid,		/* PID of parent process.			*/
	  pgrp,		/* Process group ID				*/
	  session,	/* Session ID					*/
	  tty,		/* Full device number of controlling terminal	*/
	  tpgid,	/* Terminal process group ID			*/
	  priority,	/* Kernel scheduling priority.			*/
	  nice,		/* Standard unix nice level of process		*/
	  signal,	/* mask of pending signals.			*/
	  blocked,	/* mask of blocked signals.			*/
	  sigignore,	/* mask of ignored signals.			*/
	  sigcatch;	/* mask of caught  signals.			*/
	long
	  start_time,	/* Start time of process (Seconds since 1-1-70	*/
	  utime,	/* User-mode CPU time accumulated by process	*/
	  stime,	/* Kernel-mode CPU time accumulated by process	*/
	  cutime,	/* Cumulative utime of process and reaped children */
	  cstime,	/* Cumulative stime of process and reaped children */
			/* the next 7 members come from /proc/#/statm	*/
	  size,		/* Total # of pages of memory.			*/
	  resident,	/* Number of resident set (non-swapped) pages	*/
	  share,	/* Number of pages shared (mmap'd) memory	*/
	  trs,		/* Text resident set size.			*/
	  lrs,		/* Shared-lib resident set size.		*/
	  drs,		/* data resident set size.			*/
	  dt;		/* Dirty pages.					*/
	unsigned
	  pcpu;		/* %CPU usage (isn't filled in by readproc!!!)	*/
	unsigned long
	  vsize,	/* Number of pages of virtual memory ...	*/
	  rss,		/* Resident set size from /proc/#/stat		*/
	  rss_rlim,	/* Resident set size ... ?			*/
	  timeout,	/* ? */
	  it_real_value,/* ? */
	  flags,	/* Kernel flags for the process			*/
	  min_flt,	/* # of minor page faults since process start	*/
	  maj_flt,	/* # of major page faults since process start	*/
	  cmin_flt,	/* Cumulative min_flt of process and child processes */
	  cmaj_flt,	/* Cumulative maj_flt of process and child processes */
	  start_code,	/* Addy of beginning code segment.		*/
	  end_code,	/* Addy of end of code segment.			*/
	  start_stack,	/* Addy of the bottom of stack for the process.	*/
	  kstk_esp,	/* Kernel stack pointer.			*/
	  kstk_eip,	/* Kernel stack pointer.			*/
	  wchan;	/* Addy of kernel wait channel proc is sleeping in   */
	  /* struct proc_s *l, ptrs for building arbitrary linked structs.   */
	} proc_t;
void stat2proc(char *S,proc_t *P)
{
char *tmp=strrchr(S, ')');	  /* Split into "PID (cmd" and "<rest>" */

*tmp='\0';			  /* Replace trailing ')' with NULL     */
/* Parse these two strings seperately, skipping the leading "(". */
memset(P->cmd,0,sizeof(P->cmd));  /* clear even though *P xcallcc'd?    */
sscanf(S,"%d (%39c",&P->pid,P->cmd);
sscanf(tmp+2,
  "%c %d %d %d %d %d %lu %lu %lu %lu %lu %ld %ld %ld %ld %d "
  "%d %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %d %d %d %d %lu",
  &P->state,&P->ppid,&P->pgrp,&P->session,&P->tty,&P->tpgid,
  &P->flags,&P->min_flt,&P->cmin_flt,&P->maj_flt,&P->cmaj_flt,
  &P->utime,&P->stime,&P->cutime,&P->cstime,&P->priority,&P->nice,
  &P->timeout,&P->it_real_value,&P->start_time,&P->vsize,&P->rss,
  &P->rss_rlim,&P->start_code,&P->end_code,&P->start_stack,
  &P->kstk_esp,&P->kstk_eip,&P->signal,&P->blocked,&P->sigignore,
  &P->sigcatch,&P->wchan);
if (P->tty==0) P->tty=-1;
}

int file2str(char *directory,char *what,char *ret,int cap)
{
static char filename[FSL];
int fd,num_read;
sprintf(filename,"%s/%s",directory,what);
if ((fd=open(filename,O_RDONLY,0))==-1) {  perror("open");  return(-1);  }
if ((num_read=read(fd,ret,cap-1))<=0)   {  perror("read");  return(-1);  }
ret[num_read]=0;
close(fd);
return(num_read);
}

void freeproc(proc_t *p)
{

if (p==NULL) return;	/* in case P is null */
/* ptrs are after strings to avoid copying memory when building them. -
 - so free is called on the address of the address of strvec[0].      */
/*
if (p->cmdline) free((void *)*p->cmdline);
if (p->environ) free((void *)*p->environ);
*/
free((void *)p);
}

char *print_uptime(void)
{
int upminutes,uphours,updays,pos=0;
double uptime_secs,idle_secs;

uptime(&uptime_secs,&idle_secs);
updays=(int)uptime_secs/(60*60*24);
if (updays)
  pos+=sprintf(buf2+pos,"%d day%s, ",updays,(updays!=1)?"s":"");
upminutes=(int)uptime_secs/60;
uphours=upminutes/60;
uphours=uphours%24;
upminutes=upminutes%60;
sprintf(buf2,"%d days, %d hours, %d mins.",updays,uphours,upminutes);
return(buf2);
}

int num_user(void)
{
int numuser;
struct utmp *utmpstruct;

numuser=0;
setutent();
while ((utmpstruct=getutent())) {
	if ((utmpstruct->ut_type==USER_PROCESS) && (utmpstruct->ut_name[0]!='\0'))
	  numuser++;
	}
endutent();
return(numuser);
}

/* Return the device where the root system is mounted on. */
void get_root_dev(char *mount)
{
FILE *fstab;
char dev[100],root[100];

if (!(fstab=fopen("/etc/fstab","r"))) {
	printf("fopen: %s\n",strerror(errno));
	return;
	}
while (!(feof(fstab))) {
	fscanf(fstab,"%s %s\n",dev,root);
	if (root[0]=='/') {
		FCLOSE(fstab);
		strcpy(mount,dev);
		return;
		}
	}
}

float get_elapsed_time(void)
{
struct timeval time;
static struct timeval oldtime;
struct timezone timez;
float elapsed_time;

gettimeofday(&time,&timez);
elapsed_time=(time.tv_sec-oldtime.tv_sec)+(float)(time.tv_usec-oldtime.tv_usec)/1000000.0;
oldtime.tv_sec=time.tv_sec;
oldtime.tv_usec=time.tv_usec;
return(elapsed_time);
}
