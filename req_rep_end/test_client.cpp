#include <stdio.h>
#include <winsock2.h>

#pragma commnet(lib, "ws2_32.lib")

#define FUNC_SEND 80
#define FUNC_SHDW 83

#define NODE_A_ID 90
#define NODE_B_ID 91
#define NODE_C_ID 92
#define NODE_D_ID 93

typedef struct FRAME{
    int function;
    int buffer;
    int source;
    int destination;
} frame_t;

void create_buffer(frame_t data, char buffer[], int buffsize) {
    buffer[0] = data.function;
    buffer[1] = data.buffer;
    buffer[2] = data.source;
    buffer[3] = data.destination;
}

int main() {
    WSADATA wsa;
    SOCKET connectSocket;
    struct sockaddr_in server;
    
    int buffsize = 4;
    char buffer[buffsize];

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
	if ((connectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        return 1;
	}
    else {
	    printf("Socket created.\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.55.114");
    server.sin_port = htons(8080);

    printf("Connecting to server ... ");
    if (connect(connectSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        printf("Connected.\n");
        int temp;
        frame_t data_input;
        data_input.source = NODE_A_ID;

        printf("Select function:\n(1) SEND\n(2) SHUTDOWN\n");
        scanf("%d", &temp);
        if (temp == 1) {
            data_input.function = FUNC_SEND;

            printf("\nEnter a number: ");
            scanf("%d", &(data_input.buffer));

            printf("\nSelect destination:\n(1) B\n(2) C \n(3) D\n");
            scanf("%d", &temp);
            if (temp == 1) {
                data_input.destination = NODE_B_ID;
            }
            if (temp == 2) {
                data_input.destination = NODE_C_ID;
            }
            if (temp == 3) {
                data_input.destination = NODE_D_ID;
            }
            printf("Your input: %c%c%c%c\n", data_input.function, data_input.buffer, data_input.source, data_input.destination);
            create_buffer(data_input, buffer, buffsize);
            send(connectSocket, buffer, buffsize, 0);
        }
        if (temp == 2) {
            exit(0);
        }
    }

    // if ((recv_size = recv(connectSocket, server_rep, sizeof(server_rep), 0)) == SOCKET_ERROR) {
    //     printf("Receive failed. Error code: %d\n", WSAGetLastError());
    // }
    // else {
    //     // printf("Response received.\n");
    //     server_rep[recv_size] = '\0';
    //     puts(server_rep);
    // }

    printf("Closing socket ... ");
    if (closesocket(connectSocket) < 0) {
        printf("Could not close socket. Error code: %d\n", WSAGetLastError());
    }
    else {
        printf("Socket closed.\n");
    }
	WSACleanup();

    return 0;
}