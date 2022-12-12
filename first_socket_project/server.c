#include <stdio.h>
#include <winsock2.h> // header file for enabling socket function on windows

#pragma commnet(lib, "ws2_32.lib") // library file need to be linked with the program to be able to use socket function

int main() {
	WSADATA wsa; // create a structure to hold additionnal information after socket has been initialized
	SOCKET s;

	// =============================================== Initialize socket ============================================================
	// using WSAStartup() function to initialize socket
	// first parameter is version of winsock and the the other is where to store data after initializing
	// ==============================================================================================================================
	printf("Initialising Socket...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	printf("Initialised.");

	// ================================================ Create a socket =============================================================
	// socket(int af,int type,int protocol)
	// af (address family): use AF_INET for IPv4 or AF_INET6 for IPv6
	// type: use SOCK_STREAM for TCP(reliable, connection oriented) or SOCK_DGRAM for UDP(unreliable, connectionless)
	// protocol: use 0 for Internet Protocol (IP)
	// ==============================================================================================================================
	printf("Creating Socket...\n");
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}
	printf("Socket created.\n");

	return 0;
}