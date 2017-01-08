#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define PID_FILE "runtime/ramtits.pid"
int main(void)
{
	FILE *fp;
	char filename[256];
	unsigned pid;

	sprintf(filename,"%s",PID_FILE);
	if ((fp=fopen(filename,"r")))
	{
		fscanf(fp,"%u\n",&pid);
		printf("[RaMTITS] - Shutting down the talker...\n");
		kill(pid,SIGKILL);
		fclose(fp);
		unlink(filename);
		unlink("runtime/dns_server.pid");
	}
	else printf("[RaMTITS] - Failed to locate: [%s]\n",PID_FILE);
	exit(0);
}
