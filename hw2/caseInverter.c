// caseInverter.c -- this server receives a string from a client and inverts
// all lowercase characters to uppercase, and uppercase characters to lowercase
// for the characters where this can be done. all other characters are to
// remain unchanged. caseInverter then sends the inverted message back to the
// client. caseInverter runs forever until CTRL+C'd. when this happens,
// the number of messages received and a list of IP addresses of unique
// clients that sent messages to caseInverter is printed.
#include "caseInverter.h"

int main(int argc, char *argv[]) {
	
	//Array to hold unique IPs
	char *ipAddrs[MAXSTRINGLENGTH];
	bzero(ipAddrs, sizeof(ipAddrs));
	int ipCounter = 0;
 	
	// Test for correct number of arguments
	if (argc != 3) {
   	printf("Syntax: ./caseInverter -p <Server Port/Service>\n");
		exit(1); 
	}

	// Parse command line and initialize variables
	char *portnum;
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "p:")) != -1) {
		switch (c) {
			case 'p':
				portnum = optarg;
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

	// Construct the server address structure
	struct addrinfo addrCriteria;                   // Criteria for address
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
	addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
	addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
	addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

	struct addrinfo *servAddr; // List of server addresses
	int rtnVal = getaddrinfo(NULL, portnum, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		printf("getaddrinfo() failed: %s", gai_strerror(rtnVal));
		exit(1);
	}
    
	// Create socket for incoming connections
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
					servAddr->ai_protocol);
	if (sock < 0) {
		perror("socket() failed");
		exit(1);
	}

	// Bind to the local address
	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		perror("bind() failed");
		exit(1);
	}

	// Free address list allocated by getaddrinfo()
	freeaddrinfo(servAddr);

	// Code necessary for printing output when ended with CTRL+C
	int msgs_recvd = 0;

	//Print number of messages and the unique client ips after CTRL+C
	void ctrlHandle(int sig) {
		char c;
		signal(sig, SIG_IGN);
		printf("\n%d	", msgs_recvd-1);
		int y;		
		for(y = 0; y < ipCounter; y++){
			printf("%s, ",ipAddrs[y]);
		}
		printf("\n");
		exit(1);
	}
	signal(SIGINT, ctrlHandle);
	
	for (;;) { // Run forever
		msgs_recvd++;
		int unique = 0; //0 means unique ip
		int y;
		struct sockaddr_storage clntAddr; // Client address
		socklen_t clntAddrLen = sizeof(clntAddr); // Length of client addr struct
		char buffer[MAXSTRINGLENGTH]; // message being received
		char inverted[MAXSTRINGLENGTH]; // inverted message 
		bzero(buffer, sizeof(buffer));
		bzero(inverted, sizeof(inverted));
		// Size of received message
		ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
					(struct sockaddr *) &clntAddr, &clntAddrLen);
		if (numBytesRcvd < 0) {
			perror("recvfrom() failed");
			exit(1);
		}

		// Print address of client that sent message
		printf("Handling client ");
		PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
		printf("\nMessage: %s\n\n", buffer);
		char ipString[INET6_ADDRSTRLEN];
		int err=getnameinfo((struct sockaddr*)&clntAddr,clntAddrLen,
			ipString,sizeof(ipString),0,0,NI_NUMERICHOST);
		
		//If it is the first clients connected save ip
		if (ipCounter == 0){			
			ipAddrs[ipCounter] = ipString;
			ipCounter++;
		}
		
		//Check if client ip is unique. If unique save in ipAddrs
		else {
			for(y = 0; y < ipCounter; y++){
				if (strcmp(ipAddrs[y], ipString) == 0){
					unique = 1;
				}
				else
					unique = 0;
			}
			if(unique == 0){
				ipAddrs[ipCounter] = ipString;
				ipCounter++;
			}
		}
	
		// Invert received message and store in inverted string
		int x;
		for (x=0; x<numBytesRcvd; x++) {
			if ((int)buffer[x] >= 65 && (int)buffer[x] <= 90)
				inverted[x] = buffer[x]+32;
			else if ((int)buffer[x] >= 97 && (int)buffer[x] <= 122)
				inverted[x] = buffer[x]-32;
			else
				inverted[x] = buffer[x];
		}

		// Send inverted message back to client
		ssize_t numBytesSent = sendto(sock, inverted, numBytesRcvd, 0,
      							  (struct sockaddr *) &clntAddr, sizeof(clntAddr));
		if (numBytesSent < 0) {
			perror("sendto() failed");
			exit(1);
		}
		else if (numBytesSent != numBytesRcvd) {
			printf("sendto(): sent unexpected number of bytes");
			exit(1);
		}
	}
}
