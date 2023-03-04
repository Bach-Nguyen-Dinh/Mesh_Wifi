#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 8888
#define DEFAULT_IP_ADDRESS "127.0.0.1"

int main()
{
    WSADATA wsaData;
    int iResult;
    SOCKET connectSocket = INVALID_SOCKET;
    struct sockaddr_in server = {0};
    u_long iMode = 0;

    //-------------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //-------------------------
    // Create a SOCKET object.
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.
    iMode = 1;    
    iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        printf("ioctlsocket failed with error: %ld\n", iResult);
        closesocket(connectSocket);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(DEFAULT_IP_ADDRESS);
    server.sin_port = htons(DEFAULT_PORT);
    
    printf("Connecting to server . . . ");
    if (connect(connectSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        fd_set writefds;

        FD_ZERO(&writefds);
        FD_SET(connectSocket, &writefds);

        timeval timeOut = {0};
        timeOut.tv_sec = 3;
        timeOut.tv_usec = 0;

        iResult = select(0, NULL, &writefds, NULL, &timeOut);
        if (iResult <= 0) {
            closesocket(connectSocket);
            printf("Time out.\n");
            return 1;
        }
        // else {
        //     printf("Connected\n");
        // } 
    }
    printf("Connected.\n");

    iMode = 0;
    iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        printf("ioctlsocket failed with error: %ld\n", iResult);
        closesocket(connectSocket);
        return 1;
    }

    return 0;
}