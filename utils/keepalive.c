#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define M_PS		"ps -p"
#define FCLOSE(fp) 	if (fp != NULL) fclose(fp)

int main(int argc, char *argv[])
{
	FILE *fp;
	int running,active;
	char exec[80], text[150], check[81], run[80];

	active = 0;
	running = 1;
	if (argc < 2)
	{
		printf("Usage: %s [program to keep alive]\n",argv[0]);
		exit(1);
	}
	printf("\nRaMTITS Keepalive v2.0\n");
	printf("\nExecuting keepalive script for %s.\n");

	switch (fork())
	{
		case -1:
		  running = 0;
		  printf("Unable to execute program. Fork failed.\n");
		  exit(1);
		case 0 :
		  break;
		default:
		  sleep(1);
		  printf("Running in background....\n");
		  exit(0);
	}
	sprintf(exec,"./%s",argv[1]);
	sprintf(run,"%s",argv[1]);
	while (running == 1)
	{
		sprintf(file,"runtime/ramtits.pid");
		fp = fopen(filename,"r");
		if (fp == NULL)
		{
			system(exec);
			remove("ps_file");
			sleep(60);
		}
		fscanf(fp,"%d",&rt_pid);
		FCLOSE(fp);

		sprintf(text,"ps -p%d > ps.file",rt_pid);
		system(text);
		fp = fopen("ps_file","r");
		if (fp == NULL)
		{
			system(exec);
			remove("ps_file");
			sleep(60);
		}
		while (!feof(fp))
		{
			fgets(check,80,fp);
			if (strstr(check,run) && !strstr(check,argv[0]))
				active = 1;
		}
		FCLOSE(fp);
		if (active == 0)
		{
			system(exec);
			unlink("ps_file");
			sleep(60);
		}
		unlink("ps_file");
		sleep(60);
	}
	running = 0;
	exit(0);
}
