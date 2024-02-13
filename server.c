#include "util.h"

struct sockaddr_in address;
int socketId = -1;
const int MAX_CLIENTS = 5;
_Atomic int clients;

void *connectionHandler(void *input)
{
	long long clientSocketId = (long long)input;
	ssize_t bytesRead;
	char buffer[1025];

	do
	{
		bytesRead = recv(clientSocketId, buffer, sizeof(buffer) - 1, 0);
		if (bytesRead < 0)
		{
			printf("Could not receive from the client: %s\n", strerror(errno));
			close(clientSocketId);
			return NULL;
		}

		buffer[bytesRead] = 0;
		printf("%s\n", buffer);

		if (!buffer[bytesRead - 1]) {
			send(clientSocketId, "hello", strlen("hello") + 1, 0);
		}
	} while (bytesRead > 0);

	--clients;
	close(clientSocketId);
	return NULL;
}

void handleRequests()
{
	socklen_t addressLength = sizeof(address);
	long long clientSocketId = accept(socketId, (struct sockaddr *)&address, &addressLength);

	if (clients >= 5)
	{
		send(clientSocketId, "Max client count exceeded", strlen("Max client count exceeded") + 1, 0);
		close(clientSocketId);
		return;
	}

	++clients;
	pthread_t tid;
	pthread_create(&tid, NULL, connectionHandler, (void*)clientSocketId);
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