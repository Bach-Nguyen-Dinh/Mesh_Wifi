#include <stdio.h>
#include <winsock2.h> // header file for enabling socket function on windows

#pragma commnet(lib, "ws2_32.lib") // library file need to be linked with the program to be able to use socket function

int main() {
	WSADATA wsa; // create a structure to hold additionnal information after socket has been initialized
	SOCKET s;
	struct sockaddr_in server;
	char *msg;
	char server_reply[2000];
	int recv_size;

	// =============================================== Initialize socket ============================================================
	// using WSAStartup() function to initialize socket
	// first parameter is version of winsock and the the other is where to store data after initializing
	// ==============================================================================================================================
	printf("Initialising Socket...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	printf("Initialised.\n");

	// ================================================ Create a socket =============================================================
	// socket(int af,int type,int protocol)
	// af (address family): use AF_INET for IPv4 or AF_INET6 for IPv6
	// type: use SOCK_STREAM for TCP(reliable, connection oriented) or SOCK_DGRAM for UDP(unreliable, connectionless)
	// protocol: use 0 for Internet Protocol (IP)
	// ==============================================================================================================================
	printf("Creating Socket...\n");
	if ((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) {
		printf("Could not create socket : %d" , WSAGetLastError());
	}
	printf("Socket is created.\n");

	// ============================================= Connect to a remote server =====================================================
	// specify parameters of the remote server
	server.sin_addr.s_addr = inet_addr("172.217.25.14"); // IP address of the remote server
	server.sin_family = AF_INET;
	server.sin_port = htons( 80 );

	printf("Connecting to a remote server...\n");
	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0) {
		printf("Connect error\n");
		return 1;
	}
	printf("Connected.\n");

	// ========================================== Send some data to the remote server ================================================
	msg = "GET / HTTP/1.1\r\n\r\n"; // a http command to fetch the mainpage of a website
	if (send(s, msg, strlen(msg), 0) < 0) {
		printf("Send failed\n");
		return 1;
	}
	printf("Data sent.\n");

	// ============================================= Receive data from the server ====================================================
	if ((recv_size = recv(s, server_reply, sizeof(server_reply), 0)) == SOCKET_ERROR) {
		printf("Receive failed\n");
	}
	printf("Reply received\n");

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);

	return 0;
}