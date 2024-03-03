#include "util.h"

const Command shellCommand = {5, "shell"};
const Command connectCommand = {7, "connect"};
const Command disconnectCommand = {10, "disconnect"};
const Command helpCommand = {4, "help"};

unsigned int calculateCrcHash(const char *buffer, size_t length)
{
	int i, j;
	unsigned int byte, crc, mask;

	i = 0;
	crc = 0xFFFFFFFF;
	while (i < length)
	{
		byte = buffer[i];
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--)
		{
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i = i + 1;
	}

	return ~crc;
}

char *findFirstNonSpace(char *str)
{
	if (!str)
	{
		return NULL;
	}

	while (isSpace(*str))
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

	while (p >= str && isSpace(*p))
	{
		*p = 0;
		--p;
	}

	return str;
}

bool configureInputSocket(const int socketId)
{
	int opt = 1;

	if (setsockopt(socketId, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
	{
		return false;
	}

	return true;
}

bool sendMessage(const char *str, const int socketId)
{
	uint64_t length = strlen(str);

	if (send(socketId, &length, sizeof(length), MSG_NOSIGNAL) < 0)
	{
		return false;
	}

	if (send(socketId, str, length, MSG_NOSIGNAL) < 0)
	{
		return false;
	}

	unsigned int crc = calculateCrcHash(str, length);

	if (send(socketId, &crc, sizeof(crc), MSG_NOSIGNAL) < 0)
	{
		return false;
	}

	return true;
}

static char *writeMessageInString(char *buffer, const char *message)
{
	size_t messageSize = strlen(message);
	buffer = buffer ? realloc(buffer, messageSize + 1) : malloc(messageSize + 1);
	strcpy(buffer, message);
	return buffer;
}

bool getMessage(const int socketId, char **bufferPtr, const char *prefix, const char *postfix)
{
	char *buffer = *bufferPtr;

	uint64_t textSize;
	ssize_t bytesRead = recv(socketId, &textSize, sizeof(textSize), 0);

	if (bytesRead == 0)
	{
		*bufferPtr = writeMessageInString(buffer, "");
		return false;
	}

	if (bytesRead < 0)
	{
		*bufferPtr = writeMessageInString(buffer, strerror(errno));
		return false;
	}

	if (bytesRead < sizeof(textSize))
	{
		*bufferPtr = writeMessageInString(buffer, "Could not read size");
		return false;
	}

	size_t prefixSize = 0;
	size_t postfixSize = 0;

	if (prefix)
	{
		prefixSize = strlen(prefix);
	}

	if (postfix)
	{
		postfixSize = strlen(postfix);
	}

	size_t totalStringLength = textSize + prefixSize + postfixSize;
	buffer = buffer ? realloc(buffer, totalStringLength + 1) : malloc(totalStringLength + 1);

	if (prefix)
	{
		strcpy(buffer, prefix);
	}

	bytesRead = recv(socketId, buffer + prefixSize, textSize, 0);

	if (bytesRead < 0)
	{
		*bufferPtr = writeMessageInString(buffer, strerror(errno));
		return false;
	}

	if (bytesRead < textSize)
	{
		*bufferPtr = writeMessageInString(buffer, "Size is smaller than expected");
		return false;
	}

	unsigned int expectedCrc;

	bytesRead = recv(socketId, &expectedCrc, sizeof(expectedCrc), 0);

	if (bytesRead < 0)
	{
		*bufferPtr = writeMessageInString(buffer, strerror(errno));
		return false;
	}

	if (bytesRead < sizeof(expectedCrc))
	{
		*bufferPtr = writeMessageInString(buffer, "Could not read CRC");
		return false;
	}

	if (calculateCrcHash(buffer + prefixSize, textSize) != expectedCrc)
	{
		*bufferPtr = writeMessageInString(buffer, "CRC mismatch");
		return false;
	}

	strcpy(buffer + prefixSize + textSize, postfix ? postfix : "");

	*bufferPtr = buffer;
	return true;
}