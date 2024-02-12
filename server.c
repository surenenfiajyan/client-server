#include "util.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		puts("Please provide port number");
	}

	int portNumber = atoi(argv[1]);

	if (portNumber < 0 || portNumber > 65535 )
	{
		puts("Please provide port number within range 0 - 65535");
	}

	return 0;
}