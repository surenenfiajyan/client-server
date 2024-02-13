#include "util.h"

struct sockaddr_in address;

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		puts("Please provide a port number");
		return 0;
	}

	int portNumber = atoi(argv[1]);

	if (portNumber < 0 || portNumber > 65535 )
	{
		puts("Please provide a port number within range 0 - 65535");
		return 0;
	}

	bzero(&address, sizeof(address));

	return 0;
}