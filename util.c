#include "util.h"

const Command shellCommand = {6, "shell "};
const Command connectCommand = {8, "connect "};
const Command disconnectCommand = {10, "disconnect"};

char *findFirstNonSpace(char *str)
{
	if (!str)
	{
		return NULL;
	}

	while (*str && *str == ' ')
	{
		++str;
	}

	return str;
}

char *trimFragmentInPlace(char *str)
{
	if (!str)
	{
		return NULL;
	}

	str = findFirstNonSpace(str);
	size_t length = strlen(str);

	char *p = str + length - 1;

	while (p >= str && *p == ' ')
	{
		*p = 0;
		--p;
	}

	return str;
}