#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Cmd {
	size_t length;
	const char * prefix;
} Command;

extern const Command shellCommand;
extern const Command connectCommand;
extern const Command disconnectCommand;

extern char *findFirstNonSpace(char *);
extern char *trimFragmentInPlace(char *);

#endif