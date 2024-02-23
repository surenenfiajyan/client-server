#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

typedef struct Cmd
{
	size_t length;
	const char *prefix;
} Command;

extern const Command shellCommand;
extern const Command connectCommand;
extern const Command disconnectCommand;

extern char *findFirstNonSpace(char *);
extern char *trimFragmentInPlace(char *);

inline bool isInlineSpace(char c)
{
	return c == ' ' || c == '\t';
}

inline bool isSpace(char c)
{
	return isInlineSpace(c) || c == '\n';
}

#endif
