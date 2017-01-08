#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	char ch,fname[256];

	if (argc < 2)
	{
		printf("Usage: %s [filename]\n",argv[0]);
		exit(1);
	}
	strcpy(fname,argv[1]);
	if ((fp = fopen(fname,"r")) == NULL)
	{
		printf("[Error] Couldn't open file \"%s\".\n"
		       "        Reason: %s\n",fname,strerror(errno));
		exit(1);
	}
	printf("\033(U");
	printf("\n\n");
	while(!feof(fp))
	{
		ch = getc(fp);
		printf("%c",ch);
	}
	fclose(fp);
	printf("\n");
	exit(0);
}
