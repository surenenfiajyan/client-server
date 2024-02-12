#include "util.h"

char *executeShell(const char *command)
{
	return NULL;
}

void executeConnect(const char *addr)
{
}

void executeDisonnect()
{
}

int main()
{
	char *lineBuffer = NULL;
	size_t len = 0;

	while (getline(&lineBuffer, &len, stdin) != -1)
	{
		char *trimmedFromStart = findFirstNonSpace(lineBuffer);
		bool unknownommand = false;

		if (strncmp(trimmedFromStart, shellCommand.prefix, shellCommand.length) == 0)
		{
			executeShell(trimmedFromStart + shellCommand.length);
		}
		else if (strncmp(trimmedFromStart, connectCommand.prefix, connectCommand.length) == 0)
		{
			executeConnect(trimFragmentInPlace(trimmedFromStart + connectCommand.length));
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
	return 0;
}