#include "util.h"

struct sockaddr_in server;
int socketId = -1;

void executeShell(const char *command)
{
	char buffer[1025];

	if (socketId < 0)
	{
		puts("Please, connect to the server first.");
		return;
	}

	ssize_t byteSent = send(socketId, command, strlen(command) + 1, MSG_NOSIGNAL);

	if (byteSent < 0)
	{
		printf("Could not send to the server: %s\n", strerror(errno));
	}

	ssize_t bytesRead;

	do
	{
		bytesRead = recv(socketId, buffer, sizeof(buffer) - 1, 0);

		if (bytesRead < 0)
		{
			printf("Could not receive from the server: %s\n", strerror(errno));
			break;
		}

		buffer[bytesRead] = 0;
		printf("%s", buffer);
		
		if (!buffer[bytesRead - 1]) {
			break;
		}
	} while (bytesRead > 0);

	if (bytesRead < 1) {
		close(socketId);
		socketId = -1;
	}

	printf("\n");
}

void executeConnect(char *addr)
{
	if (socketId >= 0)
	{
		puts("Please, disconnect from the already opened connection first.");
		return;
	}

	char ipAddr[64] = {0};
	int i;

	for (i = 0; i < 63 && !isSpace(addr[i]); ++i)
	{
		ipAddr[i] = addr[i];
	}

	const char *port = trimFragmentInPlace(addr + i);

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ipAddr);
	server.sin_port = htons(atoi(port));

	socketId = socket(AF_INET, SOCK_STREAM, 0);
	bool error = false;

	if (socketId < 0)
	{
		error = true;
	}

	if (!error && connect(socketId, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		error = true;
	}

	if (error)
	{
		printf("Could not connect to the server: %s\n", strerror(errno));

		if (socketId >= 0)
		{
			close(socketId);
			socketId = -1;
		}

		return;
	}
}

void executeDisonnect()
{
	if (socketId < 0)
	{
		puts("Already disconnected.");
		return;
	}

	close(socketId);
	socketId = -1;
}

int main()
{
	char *lineBuffer = NULL;
	size_t len = 0;

	while (getline(&lineBuffer, &len, stdin) >= 0)
	{
		char *trimmedFromStart = findFirstNonSpace(lineBuffer);
		bool unknownommand = false;

		if (strncmp(trimmedFromStart, shellCommand.prefix, shellCommand.length) == 0 &&
			isInlineSpace(trimmedFromStart[shellCommand.length]))
		{
			executeShell(trimFragmentInPlace(trimmedFromStart + shellCommand.length + 1));
		}
		else if (strncmp(trimmedFromStart, connectCommand.prefix, connectCommand.length) == 0 &&
				 isInlineSpace(trimmedFromStart[connectCommand.length]))
		{
			executeConnect(trimFragmentInPlace(trimmedFromStart + connectCommand.length + 1));
		}
		else if (strncmp(trimmedFromStart, disconnectCommand.prefix, disconnectCommand.length) == 0)
		{
			if (strcmp(trimFragmentInPlace(trimmedFromStart), disconnectCommand.prefix) == 0)
			{
				executeDisonnect();
			}
			else
			{
				unknownommand = true;
			}
		}
		else
		{
			unknownommand = true;
		}

		if (unknownommand)
		{
			printf("Unrecognized or incomplete command: %s\n", trimmedFromStart);
		}
	}

	free(lineBuffer);
	lineBuffer = NULL;

	if (socketId >= 0)
	{
		close(socketId);
	}

	return 0;
}