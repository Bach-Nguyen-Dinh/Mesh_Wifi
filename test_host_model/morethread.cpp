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

#define FUNC_SEND "send"
#define FUNC_RECV "receive"
#define FUNC_TNFR "transfer"
#define FUNC_SHDW "shutdown"

#define HOST_A_ADDR "127.0.0.1"
#define HOST_A_PORT 8080

#define HOST_B_ADDR "127.0.0.1"
#define HOST_B_PORT 8080

#define HOST_D_ADDR "127.0.0.1"
#define HOST_D_PORT 8080

// =================================================== Define Structure ==================================================
typedef struct FRAME{
    char function[DEFAULT_FUNCLEN];
    char buffer[DEFAULT_BUFLEN];
    char source[DEFAULT_NAMELEN];
    char destination[DEFAULT_NAMELEN];
    char previous[DEFAULT_NAMELEN];
    char frame[DEFAULT_FRAMELEN];
} frame_t;

typedef struct HOP_LIST{
    const char *name;
    const char *ip_addr;
    int port;
} hop_list_t;

// =================================================== Global Variable ====================================================
std::mutex mt;

hop_list_t hop[HOP_SIZE];

frame_t data_input;
frame_t data_rep;
frame_t data_transfer;

WSADATA wsaDATA;

int flag_recv = 0;
int flag_rep = 0;
int flag_req = 0;
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

frame_t analyze_frame(char *frame) {
    frame_t data;
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

void create_frame(frame_t *input_data) {
    int pos = 0;
    insert_to_arr(input_data->frame, input_data->function, pos, DEFAULT_FUNCLEN);
    pos += DEFAULT_FUNCLEN;
    insert_to_arr(input_data->frame, input_data->buffer, pos, DEFAULT_BUFLEN);
    pos += DEFAULT_BUFLEN;
    insert_to_arr(input_data->frame, input_data->source, pos, DEFAULT_NAMELEN);
    pos += DEFAULT_NAMELEN;
    insert_to_arr(input_data->frame, input_data->destination, pos, DEFAULT_NAMELEN);
}

void send_to_hop(SOCKET ConnectSocket, frame_t *data) {
    struct sockaddr_in server;

    create_frame(data);

    printf("Sending data . . .\n");

	if ((ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
	}

    for(int i=0; i<HOP_SIZE; i++) {
        if ((strcmp(data->source, hop[i].name) == 0) || (strcmp(data->previous, hop[i].name) == 0)) {
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

        if (send(ConnectSocket, data->frame, DEFAULT_FRAMELEN, 0) == SOCKET_ERROR) {
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
void process_input() {
    while(1) {
        data_input = {};
        printf("Select function [send / shutdown]: ");
        scanf("%s", data_input.function);
        
        if (strcmp(data_input.function, FUNC_SEND) == 0) {
            printf("Enter message: ");
            scanf("%s", data_input.buffer);

            printf("Select destination [B / C / D]: ");
            scanf("%s", data_input.destination);
        }

        flag_req = 1;

        while(flag_recv == 0) {};
    }
}

void process_request() {
    while(1) {
        SOCKET ConnectSocket = INVALID_SOCKET;
        
        frame_t data_recv;

        char recvframe[DEFAULT_FRAMELEN];

        if(flag_req) {
            strcpy(data_input.source, NODE_NAME);
            strcpy(data_input.previous, NODE_NAME);
            send_to_hop(ConnectSocket, &data_input);
            flag_req = 0;
        }

        if (flag_trfr) {
            send_to_hop(ConnectSocket, &data_transfer);
            flag_trfr = 0;
        }

        if (flag_rep) {
            send_to_hop(ConnectSocket, &data_rep);
            flag_rep = 0;
        }

        if (recv(ConnectSocket, recvframe, DEFAULT_FRAMELEN, 0) == SOCKET_ERROR) {
            printf("Receive failed. Error code: %d\n", WSAGetLastError());
        }
        else {
            closesocket(ConnectSocket);
            WSACleanup();

            data_recv = analyze_frame(recvframe);

            if (strcmp(data_recv.function, FUNC_TNFR) == 0) {
                if (strcmp(data_recv.destination, NODE_NAME) == 0) {
                    printf("Node %s transfered.\n", data_recv.source);   
                }
                else {
                    send_to_hop(ConnectSocket, &data_recv);
                }
            }

            if (strcmp(data_recv.function, FUNC_RECV) == 0) {
                printf("Node %s received.\n", data_recv.source);
                flag_recv = 1;
            }
        }
    }
}

void process_repsonse() {
    struct sockaddr_in server;
    
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("\t\t\t\t\t\tCould not create listen socket. Error code : %d\n", WSAGetLastError());
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

    while(1) {
        frame_t data_recv;
        
        char recvframe[DEFAULT_FRAMELEN];

        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == SOCKET_ERROR) {
            printf("\t\t\t\t\t\tAccept failed. Error code: %d\n", WSAGetLastError());
        }
        else {
            if (recv(ClientSocket, recvframe, DEFAULT_FRAMELEN, 0) == SOCKET_ERROR) {
                printf("\t\t\t\t\t\tReceive failed. Error code: %d\n", WSAGetLastError());
            }
            else {
                data_recv = analyze_frame(recvframe);

                if (strcmp(data_recv.destination, NODE_NAME) == 0) {
                    if (strcmp(data_recv.function, FUNC_SEND) == 0) {
                        strcpy(data_rep.function, FUNC_RECV);
                        strcpy(data_rep.source, data_recv.destination);
                        strcpy(data_rep.destination, data_recv.source);
                        strcpy(data_rep.previous, data_recv.destination);

                        if (strcmp(data_recv.source, data_recv.previous) == 0) {
                            create_frame(&data_rep);
                            send(ClientSocket, data_rep.frame, DEFAULT_FRAMELEN, 0);
                        }
                        else {
                            flag_rep = 1;
                        }
                    }

                    if (strcmp(data_recv.function, FUNC_TNFR) == 0) {
                        printf("Node %s transfered.\n", data_recv.source);
                    }
                    
                    if (strcmp(data_recv.function, FUNC_RECV) == 0) {
                        printf("Node %s received.\n", data_recv.source);
                        flag_recv = 1;
                    }
                }
                else {
                    strcpy(data_rep.function, FUNC_TNFR);
                    strcpy(data_rep.source, NODE_NAME);
                    strcpy(data_rep.destination, data_recv.source);

                    create_frame(&data_rep);
                    if (send(ClientSocket, data_rep.frame, DEFAULT_FRAMELEN, 0)) {
                        data_transfer = {};

                        strcpy(data_transfer.function, data_recv.function);
                        strcpy(data_transfer.buffer, data_recv.buffer);
                        strcpy(data_transfer.source, data_recv.source);
                        strcpy(data_transfer.destination, data_recv.destination);
                        strcpy(data_transfer.previous, NODE_NAME);

                        flag_trfr = 1;
                    }
                }
            }
        }
    }
}

// =================================================== Main Program ====================================================
int main() {
    create_hop();

    if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}

    std::thread thread_input = std::thread(process_input);
    std::thread thread_resquest = std::thread(process_request); 
    std::thread thread_response = std::thread(process_repsonse);   

    thread_input.join();
    thread_resquest.join();
    thread_response.join();

    return 0;
}