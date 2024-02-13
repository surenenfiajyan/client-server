#include "util.h"

struct sockaddr_in server;
int socketId = -1;
char *responseBufer = NULL;

const char *executeShell(const char *command)
{
	if (socketId < 0)
	{
		return "Please, connect to the server fisrst.";
	}

	return "Some result";
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
	server.sin_family = PF_INET;
	server.sin_addr.s_addr = inet_addr(ipAddr);
	server.sin_port = htons(atoi(port));

	socketId = socket(PF_INET, SOCK_STREAM, 0);
	bool error = false;

	if (socketId == -1)
	{
		error = true;
	}

	if (!error && bind(socketId, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		error = true;
	}

	if (!error && listen(socketId, 5) == -1)
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

	while (getline(&lineBuffer, &len, stdin) != -1)
	{
		char *trimmedFromStart = findFirstNonSpace(lineBuffer);
		bool unknownommand = false;

		if (strncmp(trimmedFromStart, shellCommand.prefix, shellCommand.length) == 0 &&
			isInlineSpace(trimmedFromStart[shellCommand.length]))
		{
			puts(executeShell(trimFragmentInPlace(trimmedFromStart + shellCommand.length + 1)));
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
	free(responseBufer);
	lineBuffer = NULL;
	responseBufer = NULL;

	if (socketId >= 0)
	{
		close(socketId);
	}

	return 0;
}