#include <stdio.h>
#include <stdlib.h>

int main()
{
	int addr = 111;
	printf("%lu\n", addr);
	addr = addr >> 1;
	addr = addr << 1;
	printf("%lu\n", addr);
	return 0;
}