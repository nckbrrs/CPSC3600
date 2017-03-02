// frank.c -- verifies whether the server, caseConverter, performs its desired
// task of inverting the case of the letters of a message it receives from a 
// network client. frank does this by sending a message to caseConverter,
// receiving the modified message, and then sending the modified message back
// and comparing the ultimate response with the original message. if it is the
// same as the original message, the server is "verified" and this program
// terminates displaying an appropriate message to the user.
#include "caseConverter.h"

int main(int argc, char *argv[]) {

	// Ensure that user ran with correct syntax and # of arguments. If not, exit
	if (argc != 7) {
		printf("Syntax: ./frank -s <server address> -p <port> -m <message>\n");
		exit(1);
	}

	// Declare variables
	char *servIP;		// server IP address 
	char *servPort;	// server port number
	char *message;		// message to send to server

	// Parse command line arguments with flags and initialize variables
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "s:p:m:")) != -1) {
		switch (c) {
			case 's':
				servIP = optarg;
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

	// Tell the system what kind of address info we want
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;
	addrCriteria.ai_socktype = SOCK_STREAM;
	addrCriteria.ai_protocol = IPPROTO_TCP;

	// Get address
	struct addrinfo *servAddr;
	int rtnVal = getaddrinfo(servIP, servPort, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		printf("getaddrinfo() failed: %s\n", gai_strerror(rtnVal));
		exit(1);
	}

	// Create a reliable stream socket using TCP
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
							servAddr->ai_protocol);
	if (sock < 0) {
		perror("socket() failed");
		exit(1);
	}

	// Establish the connection to the server
	if (connect(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		perror("connect() failed");
		exit(1);
	}

	// Declare new variables for sending/receiving and clock time
	_Bool send_success = false, recv_success = false;
	int attempts = 0;
	clock_t begin,end;
	begin = clock();

	// Send the string to the server
	size_t messageLen = strlen(message);
	ssize_t numBytes = send(sock, message, messageLen, 0);

	if (numBytes < 0) {
		perror("send() failed");
		exit(1);
	}
	else if (numBytes != messageLen) {
		printf("send(): sent unexpected number of bytes\n");
		exit(1);
	}

	// Receive the same string back from the server
	struct sockaddr_storage fromAddr; // Source address of server
	socklen_t fromAddrLen = sizeof(fromAddr);
	char inverted[MAXSTRINGLENGTH + 1];
	bzero(inverted, sizeof(inverted));

	while (!recv_success) {
		numBytes = recv(sock, inverted, MAXSTRINGLENGTH, 0);
		if (numBytes == messageLen)
			recv_success = true;
		else if (numBytes != messageLen) {
			printf("recv() error: received unexpected number of bytes; ");
			printf("attempting to receive again...\n");
		}
		else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr*)&fromAddr))
			printf("recv() error: received a packet from unknown source\n");
		else
			printf("recv() failed; attempting to receive again...\n");
  }

	// Send inverted string back to caseConverter
	size_t invertedLen = strlen(inverted);
	send_success = false;

	while (!send_success) {
		close(sock);
		freeaddrinfo(servAddr);		
		// Get address
		struct addrinfo *servAddr;
		int rtnVal = getaddrinfo(servIP, servPort, &addrCriteria, &servAddr);
		if (rtnVal != 0) {
			printf("getaddrinfo() failed: %s\n", gai_strerror(rtnVal));
			exit(1);
		}
		
		// Create a reliable stream socket using TCP
		sock = socket(servAddr->ai_family, servAddr->ai_socktype,
							servAddr->ai_protocol);
		if (sock < 0) {
			perror("socket() failed");
			exit(1);
		}

		// Establish the connection to the server
		if (connect(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
			perror("connect() failed");
			exit(1);
		}
		
			attempts++;

		// Send the string to the server
		size_t messageLen = strlen(message);
		ssize_t numBytes = send(sock, inverted, invertedLen, 0);
		//numBytes = send(sock, inverted, invertedLen, 0);
		if (numBytes == invertedLen)
			send_success = true;
		else if (numBytes != invertedLen) {
			printf("send() error: sent unexpected number of bytes;\n");
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
		numBytes = recv(sock, final, BUFSIZE - 1, 0);
		if (numBytes == invertedLen) 
			recv_success = true;
		else if (numBytes != invertedLen) {
			printf("recv() error: received unexpected number of bytes;\n");
			printf("attempting to receive again...\n");
		}
		else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr*)&fromAddr))
			printf("recv() error: received a packet from unknown source\n");
		else
			printf("recvfrom() failed; attempting to receive again...\n");
	}

	// Null-terminate inverted and final message
	inverted[messageLen] = '\0';
	final[messageLen] = '\0';

	// Verify that initial msg and final doubly-inverted msg are identical
	_Bool verified = !strcmp(message, final);

	// End clock and print communication stats
	end = clock();
	double time_spent = ((double)(end - begin)) / CLOCKS_PER_SEC;
	printf(" %d	%.6f	%s	%s	%s\n", attempts, time_spent, message, inverted,
			 verified ? "Verified" : "Not Verified");
	
	// Close socket and free addrinfo allocated in getaddrinfo()
	close(sock);
	freeaddrinfo(servAddr);
	return 0;
}

