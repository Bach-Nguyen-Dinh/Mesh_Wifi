#include <stdio.h>
#include <winsock2.h>

#pragma commnet(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char *msg;
    char server_rep[2000];
    int recv_size;

    printf("Initialising Socket ... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return 1;
	}
    else {
	    printf("Initialised.\n");
    }

    printf("Creating Socket ... ");
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        return 1;
	}
    else {
	    printf("Socket created.\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8888);

    printf("Connecting to server ... ");
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        printf("Connected.\n");
        msg = "Hello server, this is client\n";
        send(s, msg, strlen(msg), 0);
    }

    if ((recv_size = recv(s, server_rep, sizeof(server_rep), 0)) == SOCKET_ERROR) {
        printf("Receive failed. Error code: %d\n", WSAGetLastError());
    }
    else {
        printf("Response received.\n");
        server_rep[recv_size] = '\0';
        puts(server_rep);
    }

    printf("Closing socket ... ");
    if (closesocket(s) < 0) {
        printf("Could not close socket. Error code: %d\n", WSAGetLastError());
    }
    else {
        printf("Socket closed.\n");
    }
	WSACleanup();

    return 0;
}