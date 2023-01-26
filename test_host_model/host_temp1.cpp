#include <stdio.h>
#include <winsock2.h>
#include <p2p.h>

#pragma commnet(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512

#define host_A_addr "192.168.55.112"
#define host_A_port 8080

#define host_B_addr "192.168.55.109"
#define host_B_port 8888

int main() {
    WSADATA wsa;
    SOCKET ListenSocket, ClientSocket, ConnectSocket;
    struct sockaddr_in server, client;
    const char *msg;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    printf("Initialising Socket ... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return 1;
	}
    else {
	    printf("Initialised.\n");
    }

    printf("Creating Listen Socket ... ");
	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
	}
    else {
	    printf("Listen Socket created.\n");
    }

    printf("Creating Connect Socket ... ");
	if ((ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
	}
    else {
	    printf("Connect Socket created.\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host_A_addr);
    server.sin_port = htons(host_A_port);

    printf("Binding ... ");
    if (bind(ListenSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error code : %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
        printf("Bind done.\n");
    }

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed. Error code : %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(host_B_addr);
    client.sin_port = htons(host_B_port);

    printf("Connecting to server ... ");
    if (connect(ConnectSocket, (struct sockaddr *)&client, sizeof(client)) == SOCKET_ERROR) {
        printf("Connect failed. Error code : %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    else {
        printf("Connected.\n");
        msg = "Hello, this is client A\n";
        send(ConnectSocket, msg, strlen(msg), 0);
    }

    printf("Waiting for incoming connections ... ");

    while(1) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("Listen failed. Error code: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        else {
            printf("Accepted.\n");
            msg = "Hello, this is server A";
            send(ClientSocket, msg, strlen(msg), 0);
        }

        if (recv(ClientSocket, recvbuf, sizeof(recvbuf), 0) == SOCKET_ERROR) {
            printf("Receive failed. Error code: %d\n", WSAGetLastError());
        }
        else {
            printf("Request received.\n");
        }

        if (recv(ConnectSocket, recvbuf, sizeof(recvbuf), 0) == SOCKET_ERROR) {
            printf("Receive failed. Error code: %d\n", WSAGetLastError());
        }
        else {
            printf("Response received.\n");
        }
    }

    return 0;
}