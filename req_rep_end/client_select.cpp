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
    u_long iMode = 0;

    printf("Initialising Socket . . . ");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return 1;
	}
    else {
	    printf("Initialised.\n");
    }

    printf("Creating Socket . . . ");
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        return 1;
	}
    else {
	    printf("Socket created.\n");
    }

    iMode = 1;
    if (ioctlsocket(s, FIONBIO, &iMode) == SOCKET_ERROR) {
        closesocket(s);
        printf("Set non blocking mode failed.\n");
        return 1;
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8888);

    printf("Connecting to server . . . ");
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            printf("Failed.\n");
            closesocket(s);
            return 1;
        }

        fd_set writefds;

        FD_ZERO(&writefds);
        FD_SET(s, &writefds);

        timeval timeOut = {0};
        timeOut.tv_sec = 3;
        timeOut.tv_usec = 0;

        int iResult = select(0, NULL, &writefds, NULL, &timeOut);
        if (iResult == SOCKET_ERROR) {
            closesocket(s);
            printf("Connect time out.\n");
            return 1;
        }
    }
    else {
        printf("Connected.\n");
        // msg = "Hello server, this is client\n";
        // send(s, msg, strlen(msg), 0);
    }

    // if ((recv_size = recv(s, server_rep, sizeof(server_rep), 0)) == SOCKET_ERROR) {
    //     printf("Receive failed. Error code: %d\n", WSAGetLastError());
    // }
    // else {
    //     printf("Response received.\n");
    //     server_rep[recv_size] = '\0';
    //     puts(server_rep);
    // }

    iMode = 0;
    if (ioctlsocket(s, FIONBIO, &iMode) == SOCKET_ERROR) {
        closesocket(s);
        printf("Set blocking mode failed.\n");
        return 1;
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