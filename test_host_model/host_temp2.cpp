#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <mutex>

#pragma commnet(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 255
#define DEFAULT_FUNCLEN 20
#define DEFAULT_NAMELEN 1
#define DEFAULT_FRAMELEN (DEFAULT_FUNCLEN + DEFAULT_BUFLEN + 2*DEFAULT_NAMELEN)

#define HOP_SIZE 2
#define NODE_NAME "A"

#define SEND_FUNC "send"
#define RECV_FUNC "receive"
#define TNFR_FUNC "transfer"
#define SHDW_FUNC "shutdown"

#define HOST_A_ADDR "127.0.0.1"
#define HOST_A_PORT 8080

#define HOST_B_ADDR "127.0.0.1"
#define HOST_B_PORT 8080

#define HOST_D_ADDR "127.0.0.1"
#define HOST_D_PORT 8080

// =================================================== Define Structure ==================================================
struct frame {
    char function[DEFAULT_FUNCLEN];
    char buffer[DEFAULT_BUFLEN];
    char source[DEFAULT_NAMELEN];
    char destination[DEFAULT_NAMELEN];
    char frame[DEFAULT_FRAMELEN];
    int size = DEFAULT_FRAMELEN;
};

struct hop_list {
    const char *name;
    const char *ip_addr;
    int port;
};

// =================================================== Global Variable ====================================================
std::mutex mt;

struct hop_list hop[HOP_SIZE];
struct frame data_transfer;

SOCKET ConnectSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

int flag_recv = 0;
int flag_reps = 0;
int flag_trfr = 0;

// =================================================== Define Function ====================================================
void create_hop() {
    hop[0].name = "B";
    hop[0].ip_addr = HOST_B_ADDR;
    hop[0].port = HOST_B_PORT;

    hop[1].name = "D";
    hop[1].ip_addr = HOST_D_ADDR;
    hop[1].port = HOST_D_PORT;
}

void insert_to_arr(char *frame, char *seg, int pos, int size) {
    for (int i=0; i<size; i++) {
        *(frame + pos + i) = *(seg + i);
    }
}


struct frame extract_from_frame(char *frame) {
    struct frame data;
    int pos = 0;

    for (int i=0; i<DEFAULT_FUNCLEN; i++) {
        data.function[i] = *(frame + pos + i);
    }
    pos += DEFAULT_FUNCLEN;

    for (int i=0; i<DEFAULT_BUFLEN; i++) {
        data.buffer[i] = *(frame + pos + i);
    }
    pos += DEFAULT_BUFLEN;

    for (int i=0; i<DEFAULT_NAMELEN; i++) {
        data.source[i] = *(frame + pos + i);
    }
    pos += DEFAULT_NAMELEN;

    for (int i=0; i<DEFAULT_NAMELEN; i++) {
        data.destination[i] = *(frame + pos + i);
    }

    return data;
}

void create_frame(struct frame *input_data) {
    int pos = 0;
    insert_to_arr(input_data->frame, input_data->function, pos, DEFAULT_FUNCLEN);
    pos += DEFAULT_FUNCLEN;
    insert_to_arr(input_data->frame, input_data->buffer, pos, DEFAULT_BUFLEN);
    pos += DEFAULT_BUFLEN;
    insert_to_arr(input_data->frame, input_data->source, pos, DEFAULT_NAMELEN);
    pos += DEFAULT_NAMELEN;
    insert_to_arr(input_data->frame, input_data->destination, pos, DEFAULT_NAMELEN);
}

void send_to_hop(SOCKET ConnectSocket, struct frame *data) {
    
    struct sockaddr_in server;

    create_frame(data);

    printf("Sending data . . .\n");

	if ((ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
	}

    for(int i=0; i<HOP_SIZE; i++) {
        if(strcmp(data->source, hop[i].name) == 0) {
            continue;
        }

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(hop[i].ip_addr);
        server.sin_port = htons(hop[i].port);

        if (connect(ConnectSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
            printf("Connect failed. Error code : %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
        }

        if (send(ConnectSocket, data->frame, data->size, 0) == SOCKET_ERROR) {
            printf("Send failed. Error code: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
        }
        else {
            printf("Sent\n");
        }
    }
}

// =================================================== Thread Function ====================================================
void server_function() {
    mt.lock();
    WSADATA wsaDATA;

    struct sockaddr_in server;
    struct frame data_recv;
    struct frame data_reps;
    
    char recvframe[DEFAULT_FRAMELEN];

    printf("\t\t\t\t\t\tStarting server . . .\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0) {
		printf("\t\t\t\t\t\tFailed. Error Code : %d\n", WSAGetLastError());
	}

	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("\t\t\t\t\t\tCould not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
	}

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(HOST_A_ADDR);
    server.sin_port = htons(HOST_A_PORT);

    if (bind(ListenSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("\t\t\t\t\t\tBind failed. Error code : %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
    }

    if ((listen(ListenSocket, SOMAXCONN))== SOCKET_ERROR) {
        printf("\t\t\t\t\t\tListen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
    }

    printf("\t\t\t\t\t\tServer is running\n");

    mt.unlock();

    while(1) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == SOCKET_ERROR) {
            printf("\t\t\t\t\t\tAccept failed. Error code: %d\n", WSAGetLastError());
        }
        else {
            if (recv(ClientSocket, recvframe, DEFAULT_FRAMELEN, 0) == SOCKET_ERROR) {
                printf("\t\t\t\t\t\tReceive failed. Error code: %d\n", WSAGetLastError());
            }
            else {
                data_recv = extract_from_frame(recvframe);

                if (strcmp(data_recv.function, SEND_FUNC) == 0) {
                    if (strcmp(data_recv.destination, NODE_NAME) == 0) {
                        printf("\t\t\t\t\t\tMessage received from %s: %s", data_recv.source, data_recv.buffer);
                        strcpy(data_reps.function, RECV_FUNC);
                        strcpy(data_reps.source, data_recv.destination);
                        strcpy(data_reps.destination, data_recv.source);

                        create_frame(&data_reps);
                        send(ClientSocket, data_reps.frame, DEFAULT_FRAMELEN, 0);
                    }
                    else {
                        strcpy(data_reps.function, TNFR_FUNC);
                        strcpy(data_reps.source, data_recv.destination);
                        strcpy(data_reps.destination, data_recv.source);

                        create_frame(&data_reps);
                        send(ClientSocket, data_reps.frame, DEFAULT_FRAMELEN, 0);
                        
                        strcpy(data_transfer.function, TNFR_FUNC);
                        strcpy(data_transfer.buffer, data_recv.buffer);
                        strcpy(data_transfer.source, data_recv.source);
                        strcpy(data_transfer.destination, data_recv.destination);
                        send_to_hop(ConnectSocket, &data_transfer);
                        closesocket(ConnectSocket);
                    }                    
                }

                if (strcmp(data_recv.function, TNFR_FUNC) == 0) {

                }
            }
        }
    }
}

void client_function() {
    WSADATA wsaDATA;

    struct frame data;

    char recvframe[DEFAULT_FRAMELEN];

    strcpy(data.source, NODE_NAME);

	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}

    while(1) {
        mt.lock();
        printf("Select function [send / shutdown]: ");
        scanf("%s", data.function);
        
        if (strcmp(data.function, SEND_FUNC) == 0) {
            printf("Enter message: ");
            scanf("%s", data.buffer);

            printf("Select destination [B / C / D]: ");
            scanf("%s", data.destination);
        }

        mt.unlock();  
        send_to_hop(ConnectSocket, &data);

        while(flag_recv == 0) {
            if (recv(ConnectSocket, recvframe, DEFAULT_FRAMELEN, 0) == SOCKET_ERROR) {
                printf("Receive failed. Error code: %d\n", WSAGetLastError());
            }
            else {
                data = extract_from_frame(recvframe);
                
                if (strcmp(data.function, RECV_FUNC) == 0) {
                    flag_recv = 1;
                    printf("Node %s received the message.\n", data.source);
                }

                if (strcmp(data.function, TNFR_FUNC) == 0) {
                    if (flag_reps == 0) {
                        flag_reps == 1;
                        printf("Node %s transfered the message.\n", data.source);
                    }
                }
            }
        }
        closesocket(ConnectSocket);
        WSACleanup();
    }
}

// =================================================== Main Program ====================================================
int main() {
    create_hop();

    std::thread thread_server = std::thread(server_function);
    std::thread thread_client = std::thread(client_function);    

    thread_server.join();
    thread_client.join();

    return 0;
}