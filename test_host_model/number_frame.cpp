#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <chrono>
#include <mutex>
// #include <condition_variable>

#pragma commnet(lib, "ws2_32.lib")

#define FUNC_SEND 80
// #define FUNC_RECV 81
// #define FUNC_TNFR 82
// #define FUNC_SHDW 83
#define FUNC_FIND 84
#define FUNC_FOUND 85

#define NODE_A_ID 90
#define NODE_A_ADDR "192.168.24.190"
#define NODE_A_PORT 8080

#define NODE_B_ID 91
#define NODE_B_ADDR "192.168.24.191"
#define NODE_B_PORT 8080

// #define NODE_A_ID 90
// #define NODE_A_ADDR "192.168.55.103"
// #define NODE_A_PORT 8080

// #define NODE_B_ID 91
// #define NODE_B_ADDR "192.168.55.114"
// #define NODE_B_PORT 8080

#define NODE_C_ID 92
#define NODE_C_ADDR "192.168.55.106"
#define NODE_C_PORT 8080

#define NODE_D_ID 93
#define NODE_D_ADDR "127.0.0.1"
#define NODE_D_PORT 8080

#define NODE_ID NODE_B_ID
#define NODE_ADDR NODE_B_ADDR
#define NODE_PORT NODE_B_PORT
#define HOP_SIZE ((NODE_ID != NODE_B_ID)?1:2)

// =================================================== Define Structure ==================================================
typedef struct FRAME{
    int function;
    int buffer;
    int source;
    int destination;
} frame_t;

typedef struct HOP_LIST{
    int id;
    const char *ip_addr;
    int port;
} hop_list_t;

// =================================================== Global Variable ====================================================
WSADATA wsaDATA;

hop_list_t hop[HOP_SIZE];

int flag_recv = 0;
// int flag_found = 0;

// =================================================== Define Function ====================================================
void create_hop() {
    if (NODE_ID == NODE_B_ID) {
        hop[0].id = NODE_A_ID;
        hop[0].ip_addr = NODE_A_ADDR;
        hop[0].port = NODE_A_PORT;

        hop[1].id = NODE_C_ID;
        hop[1].ip_addr = NODE_C_ADDR;
        hop[1].port = NODE_C_PORT;
    }
    if (NODE_ID == NODE_A_ID) {
        hop[0].id = NODE_B_ID;
        hop[0].ip_addr = NODE_B_ADDR;
        hop[0].port = NODE_B_PORT;
    }
    if (NODE_ID == NODE_C_ID) {
        hop[0].id = NODE_B_ID;
        hop[0].ip_addr = NODE_B_ADDR;
        hop[0].port = NODE_B_PORT;
    }
}

void create_buffer(frame_t data, char *buffer, int buffsize) {
    char char_arr[4];
    char temp[1];

    itoa(data.function, temp, 10);
    char_arr[0] = temp[0];

    itoa(data.buffer, temp, 10);
    char_arr[1] = temp[0];

    itoa(data.source, temp, 10);
    char_arr[2] = temp[0];

    itoa(data.destination, temp, 10);
    char_arr[3] = temp[0];

    char_arr[sizeof(char_arr)] = '\0';
}

frame_t read_buffer(char *buffer) {
    frame_t frame;
    frame.function = buffer[0];
    frame.buffer = buffer[1];
    frame.source = buffer[2];
    frame.destination = buffer[3];
    return frame;
}

void send_to_node(hop_list_t dst, char *buffer, int buffsize, int *flag) {
    struct sockaddr_in server;
    SOCKET connectSocket;

    connectSocket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(dst.ip_addr);
    server.sin_port = htons(dst.port);

    printf("Connecting to NODE_ID:%d . . . ", dst.id);
    if (connect(connectSocket, (const struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connect faile. Error code : %d\n", WSAGetLastError());
        closesocket(connectSocket);
    }
    else {
        printf("Connected.\n");

        printf("Sending request to NODE_ID:%d . . . ", dst.id);
        if ((send(connectSocket, buffer, buffsize, 0)) == SOCKET_ERROR) {
            printf("Failed.\n");
            closesocket(connectSocket);
        }
        else {
            printf("Sent.\n");

            // Waiting for the response message
            int result;
            int out_of_time = 0;
            auto start = std::chrono::high_resolution_clock::now();

            do {
                result = recv(connectSocket, buffer, buffsize, 0);
                if (result > 0) {
                    printf("Bytes received: %d\n", result);
                }
                else if (result == 0) {
                    printf("Connection closed.\n");
                }
                else {
                    printf("Receive failed. Error code: %d.\n", WSAGetLastError());
                }

                auto interval = std::chrono::high_resolution_clock::now() -  start;
                if (interval >= std::chrono::seconds(3)) {
                    out_of_time = 1;
                    printf("Out of time.\n");
                    break;
                }                
            } while (result > 0);

            if (!out_of_time) {
                frame_t data_recv = read_buffer(buffer);

                if (data_recv.function == FUNC_FOUND) {
                    printf("Found.\n");
                    *flag = 1;
                }
            }
        }
        closesocket(connectSocket);
    }
}

// =================================================== Thread Function ====================================================
void p1() {
    frame_t data_input;
    data_input.source = NODE_ID;

    int temp;

    while(1) {
        int flag_found = 0;
        printf("Select function:\n(1) SEND\n(2) SHUTDOWN\n");
        scanf("%d", &temp);
        if (temp == 1) {
            data_input.function = FUNC_SEND;

            printf("\nEnter a number: ");
            scanf("%d", &(data_input.buffer));

            if (NODE_ID == NODE_A_ID) {
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
            }
            if (NODE_ID == NODE_B_ID) {
                printf("\nSelect destination:\n(1) A\n(2) C \n(3) D\n");
                scanf("%d", &temp);
                if (temp == 1) {
                    data_input.destination = NODE_A_ID;
                }
                if (temp == 2) {
                    data_input.destination = NODE_C_ID;
                }
                if (temp == 3) {
                    data_input.destination = NODE_D_ID;
                }
            }
            if (NODE_ID == NODE_C_ID) {
                printf("\nSelect destination:\n(1) A\n(2) C \n(3) D\n");
                scanf("%d", &temp);
                if (temp == 1) {
                    data_input.destination = NODE_A_ID;
                }
                if (temp == 2) {
                    data_input.destination = NODE_B_ID;
                }
                if (temp == 3) {
                    data_input.destination = NODE_D_ID;
                }
            }

            // check if the destination node is in hop
            for (int i=0; i<HOP_SIZE; i++) {
                // if the destination node is in hop, send the message
                if (data_input.destination == hop[i].id) {
                    printf("Node is in hop.\n");
                    flag_found = 1;
                    break;
                }
            }
            // if the destination node is not in hop, find a route to it
            if (flag_found == 0) {
                for (int i=0; i<HOP_SIZE; i++) {
                    int buffsize = 4;
                    char buffer[buffsize];

                    frame_t data_find_route = data_input;
                    data_find_route.function = FUNC_FIND;

                    printf("\nAsking NODE_ID:%i.\n", hop[i].id);
                    create_buffer(data_find_route, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, &flag_found);

                    if (flag_found == 1) {
                        printf("\nFound.\n\n");
                        break;
                    }
                }
                printf("\nCan not send to NODE_ID:%d.\n\n", data_input.destination);
            }
        }
        if (temp == 2) {
            exit(0);
        }
    }
}

void p3() {
    struct sockaddr_in server, client;
    
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    int buffsize = 4;
    char buffer[buffsize];

    int flag_found = 0;
    int result;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(NODE_ADDR);
    server.sin_port = htons(NODE_PORT);

    if (bind(listenSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("\t\t\t\t\t\t\t");
        printf("Bind failed. Error code : %d\n", WSAGetLastError());
    }
    else {
        listen(listenSocket, SOMAXCONN);
        printf("\t\t\t\t\t\t\t");
        printf("Server is running . . .\n");

        while((clientSocket = accept(listenSocket , NULL, NULL)) != INVALID_SOCKET)
        {
            printf("\t\t\t\t\t\t\t");
            printf("Connection accepted\n");
            do {
                result = recv(clientSocket, buffer, buffsize, 0);
                if (result > 0) {
                    printf("\t\t\t\t\t\t\t");
                    printf("Server received a message: %s\n", buffer);
                    frame_t data_recv = read_buffer(buffer);

                    if (data_recv.destination == NODE_ID) {
                        if (data_recv.function == FUNC_FIND) {
                            frame_t data_rep;

                            data_rep.function = FUNC_FOUND;
                            data_rep.buffer = data_recv.buffer;
                            data_rep.source = NODE_ID;
                            data_rep.destination = data_recv.source;

                            create_buffer(data_rep, buffer, buffsize);
                            send(clientSocket, buffer, buffsize, 0);
                            closesocket(clientSocket);
                            printf("\t\t\t\t\t\t\t");
                            printf("Server responsed the message.\n");
                        }
                    }
                    else {
                        for (int i=0; i<HOP_SIZE; i++) {
                            if (hop[i].id == data_recv.source) {
                                continue;
                            }
                            else {
                                frame_t data_find_route = data_recv;
                                data_find_route.function = FUNC_FIND;

                                create_buffer(data_find_route, buffer, buffsize);
                                send_to_node(hop[i], buffer, buffsize, &flag_found);

                                if (flag_found) {
                                    frame_t data_rep;

                                    data_rep.function = FUNC_FOUND;
                                    data_rep.buffer = data_recv.buffer;
                                    data_rep.source = NODE_ID;
                                    data_rep.destination = data_recv.source;

                                    create_buffer(data_rep, buffer, buffsize);
                                    send(clientSocket, buffer, buffsize, 0);
                                    closesocket(clientSocket);
                                    printf("\t\t\t\t\t\t\t");
                                    printf("Server responsed the message.\n");
                                        
                                    flag_found = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (result == 0) {
                    printf("\t\t\t\t\t\t\t");
                    printf("Connection closed\n");
                }
                else {
                    printf("\t\t\t\t\t\t\t");
                    printf("Receive failed. Error code : %d\n", WSAGetLastError());
                }
            } while (result > 0);
            closesocket(clientSocket);
        }
    }
}

// ===================================================== Main Program =====================================================
int main() {
    create_hop();

    if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}

    std::thread t1 = std::thread(p1);
    std::thread t3 = std::thread(p3);

    t1.join();
    t3.join();

    return 0;
}