// caseConverter.c -- this server receives a string from a client and inverts
// all lowercase characters to uppercase, and uppercase characters to lowercase
// for the characters where this can be done. all other characters are to
// remain unchanged. caseConverter then sends the inverted message back to the
// client. caseConverter runs forever until CTRL+C'd. when this happens,
// the number of messages received and a list of IP addresses of unique
// clients that sent messages to caseConverter is printed.
#include "caseConverter.h"

int main(int argc, char *argv[]) {
	
	// Array to hold IP of each unique client from which a message is received
	char *ipAddrs[MAXSTRINGLENGTH];
	int ipCounter = 0;	

	// Ensure that user ran with correct syntax and # of arguments. If not, exit
	if (argc != 3) {
		printf("Syntax: ./caseConverter -p <port>\n");
		exit(1);
	}

	// Parse command line and initialize variables
	in_port_t servPort;
	int c;
	while ((c = getopt(argc, argv, "p:")) != -1) {
		switch (c) {
			case 'p':
				servPort = atoi(optarg);
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

	// Create socket for incoming connections
	int servSock;
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket() failed");
		exit(1);
	}
  
	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(servPort);          // Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind() failed");
		exit(1);
	}

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0) {
		perror("listen() failed");
		exit(1);
	}
  
	// Print number of messages and the unique client ips after CTRL+C
	int msgs_recvd = 0;
	void ctrlHandle(int sig) {
		char c;
		signal(sig, SIG_IGN);
		printf("\n%d	", msgs_recvd-1);
		int i;		
		for(i = 0; i < ipCounter; i++){
			printf("%s, ",ipAddrs[i]);
		}
		printf("\n");
		exit(1);
	}
	signal(SIGINT, ctrlHandle);

	// Actual receiving/inverting/sending portion of program
	while(true) { // Run forever (until CTRL+C)
		msgs_recvd++;

		// Declare and initialize variable used for receiving/inverting/sending
		int unique = 0, x, y; // Unique IP checker and variables for loops
		struct sockaddr_in clntAddr; // Client address structure
		socklen_t clntAddrLen = sizeof(clntAddr); // Length of client address
		char buffer[BUFSIZE], inverted[BUFSIZE]; // Strings to hold messages
	
		// Clear strings used for receiving and storing inverted message
		bzero(inverted, sizeof(inverted));
		bzero(buffer, sizeof(buffer));
    
		// Wait for a client to connect
		int clntSock = accept(servSock,(struct sockaddr*)&clntAddr, &clntAddrLen);
		if (clntSock < 0) {
			perror("accept() failed");
			exit(1);
		}

		// clntSock is connected to a client
		char clntName[INET_ADDRSTRLEN]; // String to contain client address
		if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
							sizeof(clntName)) != NULL)
			printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
		else
			printf("Unable to get client address\n");
    
		// Save first client's IP in ipAddrs
		if (ipCounter == 0) {			
			ipAddrs[ipCounter] = clntName;
			ipCounter++;
		}
		
		// Check if new client's IP is unique. If unique, save in ipAddrs
		else {
			for(x = 0; x < ipCounter; x++) {
				if (strcmp(ipAddrs[x], clntName) == 0)
					unique = 1;
				else
					unique = 0;
	   	}
			if(unique == 0) {
				ipAddrs[ipCounter] = clntName;
				ipCounter++;
			}
		}	
	
		// Receive message from client
		ssize_t numBytesRcvd = recv(clntSock, buffer, BUFSIZE, 0);
		if (numBytesRcvd < 0) {
			perror("recv() failed");
			exit(1);
		}

		// Iterate through received message in buffer char array, 
		// swap case of each letter, and store new message in inverted char array 
		for (y=0; y<numBytesRcvd; y++) {
			if ((int)buffer[y] >= 65 && (int)buffer[y] <= 90)
				inverted[y] = buffer[y]+32;
			else if ((int)buffer[y] >= 97 && (int)buffer[y] <= 122)
				inverted[y] = buffer[y]-32;
			else
				inverted[y] = buffer[y];
		}
 
		// Send inverted message back to client
		ssize_t numBytesSent = send(clntSock, inverted, numBytesRcvd, 0);
		if (numBytesSent < 0) {
			perror("send() failed");
			exit(1);
		}
		else if (numBytesSent != numBytesRcvd) {
			printf("send(): sent unexpected number of bytes\n");
			exit(1);
		}
	}	
}
