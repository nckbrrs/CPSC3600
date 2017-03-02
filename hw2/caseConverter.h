#ifndef CASECONVERTER_H_
#define CASECONVERTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);

// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);

enum sizeConstants {
  MAXSTRINGLENGTH = 128,
  BUFSIZE = 512,
};

static const int MAXPENDING = 5;

#endif

