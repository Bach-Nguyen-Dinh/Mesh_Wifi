#include <stdio.h>
#include <winsock2.h> // header file for enabling socket function on windows

#pragma commnet(lib, "ws2_32.lib") // library file need to be linked with the program to be able to use socket function

int main() {
	WSADATA wsa; // create a structure to hold additionnal information after socket has been initialized
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	char *msg;

	// =============================================== Initialize socket ============================================================
	// using WSAStartup() function to initialize socket
	// first parameter is version of winsock and the the other is where to store data after initializing
	// ==============================================================================================================================
	printf("Initialising Socket...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return 1;
	}
	printf("Initialised.\n");

	// ================================================ Create a socket =============================================================
	// socket(int af,int type,int protocol)
	// af (address family): use AF_INET for IPv4 or AF_INET6 for IPv6
	// type: use SOCK_STREAM for TCP(reliable, connection oriented) or SOCK_DGRAM for UDP(unreliable, connectionless)
	// protocol: use 0 for Internet Protocol (IP)
	// ==============================================================================================================================
	printf("Creating Socket...");
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n", WSAGetLastError());
	}
	printf("Socket created.\n");

	// ==================================================== Binding =================================================================
	// specify parameters for binding a socket to a particular IP address and port
	server.sin_family = AF_INET; // using IPv4
	server.sin_addr.s_addr = INADDR_ANY; // localhost IP address
	server.sin_port = htons(8888);
	
	printf("Binding...");
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d\n", WSAGetLastError());
	}
	else {
		printf("Bind done.\n");
	}
	
	// =========================================== Listen to incoming connections ====================================================
	// backlog parameter specifies the number of incoming connections that can be queued for acceptance
	listen(s, 3);
	
	// =========================================== Accept an incoming connection =====================================================
	printf("Waiting for incoming connections...");
	
	int c = sizeof(struct sockaddr_in);

	while ((new_socket = accept(s, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
		printf("Connection accepted.\n");

		//Reply to the client
		msg = "Hello Client , I have received your connection. But I have to go now, bye\n";
		send(new_socket , msg , strlen(msg) , 0);
	}

	if (new_socket == INVALID_SOCKET) {
		printf("Accept failed with error code : %d\n" , WSAGetLastError());
		return 1;
	}

	// ================================================== Terminate socket ============================================================
	printf("Closing socket...");
	if (closesocket(s) < 0) {
		printf("Could not close socket : %d\n", WSAGetLastError());
	}
	printf("Socket is closed\n");
	WSACleanup();

	return 0;
}