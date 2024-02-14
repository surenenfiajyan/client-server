#include "util.h"

struct sockaddr_in address;
int socketId = -1;
const int MAX_CLIENTS = 5;
_Atomic int clients;

void *connectionHandler(void *input)
{
	long long clientSocketId = (long long)input;
	ssize_t bytesRead;
	char readBuffer[1025];
	size_t commandBufferSize = 64, commandBufferSizeUsed = 0;
	char *commandBuffer = malloc(commandBufferSize);
	send(clientSocketId, "", 1, MSG_NOSIGNAL);

	do
	{
		bytesRead = recv(clientSocketId, readBuffer, sizeof(readBuffer) - 1, 0);

		if (bytesRead < 0)
		{
			printf("Could not receive from the client: %s\n", strerror(errno));
			close(clientSocketId);
			return NULL;
		}

		readBuffer[bytesRead] = 0;

		if (commandBufferSizeUsed + bytesRead >= commandBufferSize)
		{
			commandBufferSize = commandBufferSizeUsed + bytesRead + commandBufferSize / 5;
			commandBuffer = realloc(commandBuffer, commandBufferSize);
		}

		strcpy(commandBuffer + commandBufferSizeUsed, readBuffer);
		commandBufferSizeUsed += bytesRead;

		if (commandBufferSizeUsed && (!bytesRead || !readBuffer[bytesRead - 1]))
		{
			commandBufferSizeUsed = 0;
			puts(commandBuffer);
			FILE *fpipe = popen(commandBuffer, "r");

			if (fpipe)
			{
				while (fgets(readBuffer, sizeof(readBuffer), fpipe) != NULL)
				{
					send(clientSocketId, readBuffer, strlen(readBuffer), MSG_NOSIGNAL);
				}

				send(clientSocketId, "", 1, MSG_NOSIGNAL);
				pclose(fpipe);
			}
			else
			{
				puts(strerror(errno));
				const char *message = "Error occurred";
				send(clientSocketId, message, strlen(message) + 1, MSG_NOSIGNAL);
			}
		}
	} while (bytesRead > 0);

	--clients;
	free(commandBuffer);
	close(clientSocketId);
	return NULL;
}

void handleRequests()
{
	socklen_t addressLength = sizeof(address);
	long long clientSocketId = accept(socketId, (struct sockaddr *)&address, &addressLength);

	if (clients >= MAX_CLIENTS)
	{
		const char *message = "Max client count exceeded";
		send(clientSocketId, message, strlen(message) + 1, MSG_NOSIGNAL);
		close(clientSocketId);
		return;
	}

	++clients;
	pthread_t tid;
	pthread_create(&tid, NULL, connectionHandler, (void *)clientSocketId);
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