#include <stdio.h>

unsigned char x;

int main(void)
{
	int cnt = 0;

	printf("\033(U");
	for (x = 179; x < 218; x++)
	{
		printf("Numeric value: %d  Printed Char: %c ",x,x);
		if (++cnt%2 == 0) printf("\n");
	}
	printf("\n");
	exit(0);
}
