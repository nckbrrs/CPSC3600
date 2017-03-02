//Header file for caseInverter
#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>

// Constant variable names
enum sizeConstants {
	MAXSTRINGLENGTH = 128,
	BUFSIZE = 512
};

// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);

// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);

#endif
