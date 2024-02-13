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

void executeConnect(const char *addr)
{
	if (socketId >= 0)
	{
		puts("Please, disconnect from the already opened connection first.");
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
	bzero(&server, sizeof(server));
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