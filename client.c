#include "util.h"

struct sockaddr_in server;
int socketId = -1;

void executeShell(const char *command)
{
	if (socketId < 0)
	{
		puts("Please, connect to the server first.");
		return;
	}

	if (!sendMessage(command, socketId))
	{
		printf("Could not send to the server: %s\n", strerror(errno));
		close(socketId);
		socketId = -1;
		return;
	}

	char *strBuffer = NULL;

	if (getMessage(socketId, &strBuffer, NULL, NULL))
	{
		puts(strBuffer);
		printf("\nDone.\n");
	}
	else
	{
		printf("Could not receive from the server: %s\n", strBuffer);
		close(socketId);
		socketId = -1;
	}

	free(strBuffer);
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

	if (!error && !configureInputSocket(socketId))
	{
		error = true;
	}

	if (!error && connect(socketId, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		error = true;
	}

	struct timeval tv;
	tv.tv_sec = 20;
	tv.tv_usec = 0;

	if (!error && setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
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

	char *errorMessage = NULL;

	if (getMessage(socketId, &errorMessage, NULL, NULL))
	{
		puts("Done.");
	}
	else
	{
		puts(errorMessage);
		close(socketId);
		socketId = -1;
	}

	free(errorMessage);
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
	puts("Done.");
}

void executeHelp()
{
	puts("The list of commands:");
	puts("");
	puts("    connect -    Connects to the server");
	puts("        usage:   connect [HOST] [PORT]");
	puts("        example: connect 127.0.0.1 2500");
	puts("");
	puts("    shell -      executes a shell command on the server");
	puts("        usage:   shell [COMMAND]");
	puts("        example: shell uname -a");
	puts("");
	puts("    disconnect - Disconnects from the server it was connected to");
	puts("        usage:   disconnect");
	puts("");
	puts("    help -       Prints the list of commands");
	puts("        usage:   help");
	puts("");
	puts("");
}

int main()
{
	char *lineBuffer = NULL;
	size_t len = 0;

	executeHelp();

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
		else if (strncmp(trimmedFromStart, helpCommand.prefix, helpCommand.length) == 0)
		{
			if (strcmp(trimFragmentInPlace(trimmedFromStart), helpCommand.prefix) == 0)
			{
				executeHelp();
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