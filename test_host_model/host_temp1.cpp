#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <mutex>

#pragma commnet(lib, "ws2_32.lib")

#define host_A_addr "192.168.55.112"
#define host_A_port 8080

#define host_B_addr "192.168.55.110"
#define host_B_port 8080

std::mutex mt;

SOCKET s, recv_s, connect_s;
struct sockaddr_in host_A, host_B, host;
char other_host_msg[2000];
int recv_size;

// void accept_host() {
//     printf("Waiting for incoming connections ... ");
//     while(1) {
//         mt.lock();
//         int c = sizeof(struct sockaddr_in);
//         if ((recv_s = accept(s, (struct sockaddr *)&host, &c)) == INVALID_SOCKET) {
//             printf("Accept failed. Error code : %d\n", WSAGetLastError());
//         }
//         else {
//             printf("Connection accepted.\n");
//         }

//         if ((recv_size = recv(recv_s, other_host_msg, sizeof(other_host_msg), 0)) == SOCKET_ERROR) {
//             printf("Receive failed. Error code: %d\n", WSAGetLastError());
//         }
//         else {
//             printf("Message received.\n");
//             other_host_msg[recv_size] = '\0';
//             puts(other_host_msg);
//         }
//         mt.unlock();
//     }
// }
// void connect_host() {
//     printf("Connecting to server ... ");
//     mt.lock();
//     if (connect(connect_s, (struct sockaddr *)&host_B, sizeof(host_B)) < 0) {
//         printf("Connect failed. Error code : %d\n", WSAGetLastError());
//     }
//     else {
//         printf("Connected.\n");
//         const char *msg = "Hello, this is host A";
//         send(connect_s, msg, strlen(msg), 0);
//     }

//     if ((recv_size = recv(connect_s, other_host_msg, sizeof(other_host_msg), 0)) == SOCKET_ERROR) {
//             printf("Receive failed. Error code: %d\n", WSAGetLastError());
//         }
//         else {
//             printf("Message received.\n");
//             other_host_msg[recv_size] = '\0';
//             puts(other_host_msg);
//         }
//     mt.unlock();
// }

int main() {
    WSADATA wsa;

    host_A.sin_family = AF_INET;
    host_A.sin_addr.s_addr = inet_addr(host_A_addr);
    host_A.sin_port = htons(host_A_port);

    host_B.sin_family = AF_INET;
    host_B.sin_addr.s_addr = inet_addr(host_B_addr);
    host_B.sin_port = htons(host_B_port);

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
    
    printf("Binding ... ");
    if (bind(s, (struct sockaddr *)&host_A, sizeof(host_A)) == SOCKET_ERROR) {
        printf("Bind failed. Error code : %d\n", WSAGetLastError());
        return 1;
    }
    else {
        printf("Bind done.\n");
    }

    listen(s, 3);

    printf("Waiting for incoming connections ... ");
    int c = sizeof(struct sockaddr_in);
    if ((recv_s = accept(s, (struct sockaddr *)&host, &c)) == INVALID_SOCKET) {
        printf("Accept failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        printf("Connection accepted.\n");
    }
    
    if ((recv_size = recv(recv_s, other_host_msg, sizeof(other_host_msg), 0)) == SOCKET_ERROR) {
        printf("Receive failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }
    else {
        printf("Message received.\n");
        other_host_msg[recv_size] = '\0';
        puts(other_host_msg);
    }

    printf("Connecting to server ... ");
    if (connect(connect_s, (struct sockaddr *)&host_B, sizeof(host_B)) < 0) {
        printf("Connect failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        printf("Connected.\n");
        const char *msg = "Hello, this is host A";
        send(connect_s, msg, strlen(msg), 0);
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