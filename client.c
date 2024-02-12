#include "util.h"

char *executeShell(const char *command)
{
	printf("executeShell '%s'\n", command);
	return NULL;
}

void executeConnect(const char *addr)
{
	printf("executeConnect '%s'\n", addr);
}

void executeDisonnect()
{
	printf("executeDisonnect\n");
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
	return 0;
}