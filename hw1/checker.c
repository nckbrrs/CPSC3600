/*
UDP client to send message to server then resend the received
message to see if the final message is the same as the first
to check the functionality of caseInverter.c
*/

#include "caseInverter.h"

int main(int argc, char *argv[]) {

	// Test for correct number of arguments
	if (argc != 7)	{
		printf("Syntax: ./checker -s <serverIP> -p <serverPort> -m <message>\n");
		exit(1);
	}
	
	// Declare variables
	char *server;		// server IP address
 	char *servPort;	// port number
	char *message;		// message to send

	// Parse command line arguments and intialize variables
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "s:p:m:")) != -1) {
		switch (c) {
			case 's':
				server = optarg;
				break;
			case 'p':
				servPort = optarg;
				break;
			case 'm':
				message = optarg;
				break;
			case '?':
				if (optopt == 'c')
					fprintf (stderr, "Option -%c needs an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				abort ();
		}
	}
	
	// Declare and initialize length of initial message
	size_t messageLen = strlen(message);
	if (messageLen > MAXSTRINGLENGTH) {
		printf("string too long\n");
		exit(1);
	} 

	// Tell the system what kind(s) of address info we want
	struct addrinfo addrCriteria;                   // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out the structure
	addrCriteria.ai_family = AF_UNSPEC;             // Any address family

	// For the following fields, a zero value means "don't care"
	addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
	addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

	// Get address(es)
	struct addrinfo *servAddr; // List of server addresses
	int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		printf("getaddrinfo() failed: %s", gai_strerror(rtnVal));
		exit(1);
	}

	// Create a datagram/UDP socket
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
					servAddr->ai_protocol);
 
	if (sock < 0) {
		perror("socket() failed");
		exit(1);
	}

	// Declare new variables for sending/receiving and clock time
	_Bool send_success = false, recv_success = false;
	ssize_t numBytes;
	int attempts = 0;
	clock_t begin, end;

	// Send the initial string to the server
	begin = clock(); //start the clock
	while (!send_success) {
		attempts++;
		numBytes = sendto(sock, message, messageLen, 0,
						servAddr->ai_addr, servAddr->ai_addrlen);
		if (numBytes == messageLen)
			send_success = true;
		else if (numBytes != messageLen) {
			printf("sendto() error: sent unexpected number of bytes; ");
			printf("sending message again...\n");
		}
		else
			printf("sendto() failed; sending message again...\n");
	}

	// Receive inverted message
	struct sockaddr_storage fromAddr; // Source address of server
	socklen_t fromAddrLen = sizeof(fromAddr);
	char inverted[MAXSTRINGLENGTH + 1];
	bzero(inverted, sizeof(inverted));

	while (!recv_success) {
		numBytes = recvfrom(sock, inverted, MAXSTRINGLENGTH, 0,
				(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (numBytes == messageLen)
			recv_success = true;
		else if (numBytes != messageLen) {
			printf("recvfrom() error: received unexpected number of bytes; ");
			printf("attempting to receive again...\n");
		}
		else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr*)&fromAddr))
			printf("recvfrom() error: received a packet from unknown source\n");
		else
			printf("recvfrom() failed; attempting to receive again...\n");
	}
	
	// Send inverted string back to caseInverter
	size_t invertedLen = strlen(inverted);
	send_success = false;
	while (!send_success) {
		attempts++;
		numBytes = sendto(sock, inverted, invertedLen, 0,
						servAddr->ai_addr, servAddr->ai_addrlen);
		if (numBytes == invertedLen)
			send_success = true;
		else if (numBytes != invertedLen) {
			printf("sendto() error: sent unexpected number of bytes;\n");
			printf("sending message again...\n");
		}
		else
			printf("sendto() failed; sending message again...\n");
	}

	// Receive final re-inverted string back from caseInverter
	char final[MAXSTRINGLENGTH + 1];
	bzero(final, sizeof(final));
	recv_success = false;
	while (!recv_success) {
		numBytes = recvfrom(sock, final, MAXSTRINGLENGTH, 0,
						(struct sockaddr *) &fromAddr, &fromAddrLen);
		if (numBytes == invertedLen) 
			recv_success = true;
		else if (numBytes != invertedLen) {
			printf("recvfrom() error: received unexpected number of bytes;\n");
			printf("attempting to receive again...\n");
		}
		else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr*)&fromAddr))
			printf("recvfrom() error: received a packet from unknown source\n");
		else
			printf("recvfrom() failed; attempting to receive again...\n");
	}

	// Verify that initial msg and final doubly-inverted msg are identical
	inverted[messageLen] = '\0';
	final[messageLen] = '\0';	
	
	_Bool verified = !strcmp(message, final);
	freeaddrinfo(servAddr);
	
	end = clock();
	double time_spent = ((double)(end - begin) / CLOCKS_PER_SEC);
	printf("%d	%.6f	%s	%s	%s\n", attempts, 
				time_spent, message, inverted, 
				verified ? "Verified" : "Not Verified");
	close(sock);
	exit(0);
}
