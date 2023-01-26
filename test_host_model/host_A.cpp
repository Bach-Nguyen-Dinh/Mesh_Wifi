#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <chrono>

#pragma commnet(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512

#define host_A_addr "192.168.55.112"
#define host_A_port 8080

#define host_B_addr "192.168.55.109"
#define host_B_port 8888

std::mutex mt;

void delay_ms(int t) {
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();

    while ((stop - start) < static_cast<std::chrono::milliseconds>(t)) {
        stop = std::chrono::high_resolution_clock::now();
    }
}

void p_server() {
    struct sockaddr_in thisHost;

    SOCKET listenSocket, clientSocket;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    printf("Creating Socket ... ");
	if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
	}
    else {
	    printf("Socket created.\n");
    }

    thisHost.sin_family = AF_INET;
    thisHost.sin_addr.s_addr = inet_addr(host_A_addr);
    thisHost.sin_port = htons(host_A_port);

    printf("Binding ... ");
    if (bind(listenSocket, (struct sockaddr *)&thisHost, sizeof(thisHost)) == SOCKET_ERROR) {
        printf("Bind failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        printf("Bind done.\n");
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed. Error code : %d\n", WSAGetLastError());
    }

    while(1) {
        clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("Listen failed. Error code: %d\n", WSAGetLastError());
        }
        else {
            printf("Accepted.\n");
            const char *msg = "Hello, this is host A";
            send(clientSocket, msg, strlen(msg), 0);
        }
    }
}

void p_client() {
    struct sockaddr_in connectHost;
    SOCKET connectSocket;

    printf("Creating Socket ... ");
	if ((connectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
	}
    else {
	    printf("Socket created.\n");
    }

    connectHost.sin_family = AF_INET;
    connectHost.sin_addr.s_addr = inet_addr(host_B_addr);
    connectHost.sin_port = htons(host_B_port);

    printf("Connecting to server ... ");
    if (connect(connectSocket, (struct sockaddr *)&connectHost, sizeof(connectHost)) < 0) {
        printf("Connect failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        printf("Connected.\n");
    }

    const char *msg = "Hello server, this is client\n";
    while(1) {
        send(connectSocket, msg, strlen(msg), 0);
        // delay_ms(500);
    }
}

int main() {
    std::thread t_server, t_client;
    WSADATA wsaData;

    printf("Initialising Socket ... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return 1;
	}
    else {
	    printf("Initialised.\n");
    }

    t_server = std::thread(p_server);
    t_client = std::thread(p_client);

    t_server.join();
    t_client.join();

    return 0;
}