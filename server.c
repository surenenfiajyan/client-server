#include "util.h"

struct sockaddr_in address;
int socketId = -1;

void handleRequests()
{
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		puts("Please provide a port number");
		return 0;
	}

	int portNumber = atoi(argv[1]);

	if (portNumber < 0 || portNumber > 65535)
	{
		puts("Please provide a port number within range 0 - 65535");
		return 0;
	}

	socketId = socket(AF_INET, SOCK_STREAM, 0);

	if (socketId < 0)
	{
		printf("Failed to create a socket: %s\n", strerror(errno));
		return 0;
	}

	int opt = true;

	if (setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
	{
		printf("Failed to configure the socket: %s\n", strerror(errno));
		return 0;
	}

	bzero(&address, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_port = htons(portNumber);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(socketId, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		printf("Failed to bind the socket: %s\n", strerror(errno));
		return 0;
	}

	if (listen(socketId, 5) < 0)
	{
		printf("Failed to listen to the socket: %s\n", strerror(errno));
		return 0;
	}

	puts("Listening to requests...");

	do
	{
		handleRequests();
	} while (true);

	return 0;
}