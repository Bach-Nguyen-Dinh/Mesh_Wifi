#include <stdio.h>
#include <winsock2.h>

#pragma commnet(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET s, new_s;
    struct sockaddr_in server, client;
    int c;
    char *msg;
    char client_req[2000];
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
    server.sin_addr.s_addr = INADDR_ANY; // localhost IP address
    // server.sin_addr.s_addr = inet_addr("192.168.55.112");
    server.sin_port = htons(8888);

    printf("Binding ... ");
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error code : %d\n", WSAGetLastError());
        return 1;
    }
    else {
        printf("Bind done.\n");
    }

    listen(s, 3);

    printf("Waiting for incoming connections ... ");
    c = sizeof(struct sockaddr_in);
    if ((new_s = accept(s, (struct sockaddr *)&client, &c)) == INVALID_SOCKET) {
        printf("Accept failed. Error code : %d\n", WSAGetLastError());
        return 1;
    }
    else {
        printf("Accepted.\n");
        // msg = "Hello client, this is server\n";
        // send(new_s, msg, strlen(msg), 0);
    }

    // if ((recv_size = recv(new_s, client_req, sizeof(client_req), 0)) == SOCKET_ERROR) {
    //     printf("Receive failed. Error code: %d\n", WSAGetLastError());
    //     return 1;
    // }
    // else {
    //     printf("Request received.\n");
    //     client_req[recv_size] = '\0';
    //     puts(client_req);
    // }

    // msg = "Hello client, this is server\n";
    // send(new_s, msg, strlen(msg), 0);

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