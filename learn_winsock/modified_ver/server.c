#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8888"
#define DEFAULT_BUFLEN 512

int main() {
    WSADATA wsaData;
    int iResult;

    // struct addrinfo structure is used by the getaddrinfo() function to hold host address information
    struct addrinfo *result = NULL, hints;
    struct sockaddr_in server;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    printf("Initializing Winsock ... ");
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    else {
        printf("Initialized.\n");
    }

    // Resolve the local address and port to be used by the server by using getaddrinfo() function
    // hints provides informations about the type of socket
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // AI_PASSIVE flag indicates the caller intends to use the returned socket address structure in a call to the bind function

    // getaddrinfo() function returns server informations in result variable
    // When the AI_PASSIVE flag is set and nodename parameter to the getaddrinfo() function is a NULL pointer,
    // the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses
    // which means localhost
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo() failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections
    // printf("Creating socket ... ");
    // ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    // if (ListenSocket == INVALID_SOCKET) {
    //     printf("Error at socket. Error code: %d\n", WSAGetLastError());
    //     freeaddrinfo(result);
    //     WSACleanup();
    //     return 1;
    // }
    // else {
    //     printf("Socket created.\n");
    // }

    printf("Creating socket ... ");
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Could not create socket. Error code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    else {
        printf("Socket created.\n");
    }

    // A socket must be binded to an IP address and port in order to accecpt client connections
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8888);

    // printf("Binding ... ");
    // iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    // if (iResult == SOCKET_ERROR) {
    //     printf("Bind failed. Error code: %d\n", WSAGetLastError());
    //     freeaddrinfo(result);
    //     closesocket(ListenSocket);
    //     WSACleanup();
    //     return 1;
    // }
    // else {
    //     printf("Bind done.\n");
    //     freeaddrinfo(result);
    // }

    printf("Binding ... ");
    iResult = bind(ListenSocket, (struct sockaddr *)&server, sizeof(server));
    if (iResult == SOCKET_ERROR) {
        printf("Bind failed. Error code: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
        printf("Bind done.\n");
    }

    // listen() function has a parameter names backlog which is used to detemind maximum length of the queue of pending connections to accept
    // SOMAXCONN is a special constant that allows a maximum reasonable number of pending connections in the queue
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed. Error code: %d\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    printf("Waiting for incoming connections ... ");
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("Accept failed. Error code: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
        printf("Connected.\n");
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("Send failed. Error code: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            else {
                printf("Bytes sent: %d\n", iSendResult);
            }
        }
        else if (iResult == 0) {
            printf("Connection closing ... ");
        }
        else {
            printf("Receive failed. Error code: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("Shutdown failed. Error code: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    printf("Closed.\n");

    return 0;
}