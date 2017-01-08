#ifndef _SYSTEM_C
  #define EXTERN extern
#else
  #define EXTERN
  enum meminfo_row {
	meminfo_main=0,
	meminfo_swap
	};
  enum meminfo_col {
	meminfo_total=0,meminfo_used,meminfo_free,
	meminfo_shared,meminfo_buffers,meminfo_cached
	};
#endif
EXTERN void servinfo(User,char *,int);
EXTERN int uptime(double *uptime_secs,double *idle_secs);
EXTERN int loadavg(double *av1,double *av5,double *av15);
EXTERN unsigned** meminfo(void);
EXTERN unsigned read_total_main(void);
EXTERN void set_linux_version(void);
#undef EXTERN
