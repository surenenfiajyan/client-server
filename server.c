#include "util.h"

struct sockaddr_in address;
int socketId = -1;
const int MAX_CLIENTS = 5;
_Atomic int clients;

void *connectionHandler(void *input)
{
	long long clientSocketId = (long long)input;
	char *strBuffer = NULL;

	printf("Client socket %lli connected, %i clients total\n", clientSocketId, clients);

	if (!configureInputSocket(clientSocketId))
	{
		printf("Failed to configure the socket: %s\n", strerror(errno));
		close(clientSocketId);
		--clients;
		free(strBuffer);
		return NULL;
	}

	if (!sendMessage("", clientSocketId))
	{
		printf("Failed to send to the client: %s\n", strerror(errno));
		close(clientSocketId);
		--clients;
		free(strBuffer);
		return NULL;
	}

	bool status;

	do
	{
		status = getMessage(clientSocketId, &strBuffer, "timeout -s SIGKILL 10 ", " 2>&1");

		if (status)
		{
			printf("Executing command for socket %lli:\n%s\n", clientSocketId, strBuffer);
			FILE *fpipe = popen(strBuffer, "r");

			if (fpipe)
			{
				size_t readBufferTotalSize = 128;
				size_t readBufferOccupiedSpace = 0;
				char *readBuffer = malloc(readBufferTotalSize);
				readBuffer[0] = 0;

				while (fgets(readBuffer + readBufferOccupiedSpace, readBufferTotalSize - readBufferOccupiedSpace, fpipe) != NULL)
				{
					size_t bytesRead = strlen(readBuffer + readBufferOccupiedSpace);

					if (!bytesRead)
					{
						printf("Before: %li\n", readBufferTotalSize);
						readBufferTotalSize += readBufferTotalSize / 2;
						printf("After: %li\n", readBufferTotalSize);
						readBuffer = realloc(readBuffer, readBufferTotalSize);
						continue;
					}

					readBufferOccupiedSpace += bytesRead;
				}

				bool success = sendMessage(readBuffer, clientSocketId);

				free(readBuffer);
				pclose(fpipe);

				if (!success)
				{
					--clients;
					printf("Failed to respond to client: %s\n", strerror(errno));
					free(strBuffer);
					close(clientSocketId);
					return NULL;
				}
			}
			else
			{
				puts(strerror(errno));
				sendMessage("Error occurred", clientSocketId);
			}
		}
		else if (strBuffer[0])
		{
			--clients;
			printf("Failed to get client request: %s\n", strBuffer);
			free(strBuffer);
			close(clientSocketId);
			return NULL;
		}
	} while (status);

	--clients;
	printf("Client socket %lli disconnected, %i clients left\n", clientSocketId, clients);
	free(strBuffer);
	close(clientSocketId);
	return NULL;
}

void handleRequests()
{
	if (clients >= MAX_CLIENTS)
	{
		usleep(500);
		return;
	}

	socklen_t addressLength = sizeof(address);
	long long clientSocketId = accept(socketId, (struct sockaddr *)&address, &addressLength);
	pthread_t tid;

	++clients;
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

	int opt = 1;

	if (setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
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