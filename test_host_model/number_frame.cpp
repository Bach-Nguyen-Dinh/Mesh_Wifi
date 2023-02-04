#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <chrono>
#include <mutex>
// #include <condition_variable>

#pragma commnet(lib, "ws2_32.lib")

#define FUNC_SEND 80
#define FUNC_RECV 81
// #define FUNC_TNFR 82
// #define FUNC_SHDW 83
#define FUNC_FIND 84
#define FUNC_FOUND 85

#define NODE_A_ID 90
#define NODE_A_ADDR "192.168.55.103"
#define NODE_A_PORT 8080

#define NODE_B_ID 91
#define NODE_B_ADDR "192.168.55.114"
#define NODE_B_PORT 8080

#define NODE_C_ID 92
#define NODE_C_ADDR "192.168.55.107"
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
    char function;
    char buffer;
    char source;
    char destination;
} frame_t;

typedef struct HOP_LIST{
    char id;
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

void create_buffer(frame_t data, char buffer[], int buffsize) {
    buffer[0] = data.function;
    buffer[1] = data.buffer;
    buffer[2] = data.source;
    buffer[3] = data.destination;
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

    printf("Asking NODE_ID:%i.\n", dst.id);
    printf("Connecting to NODE_ID:%d . . . ", dst.id);
    if (connect(connectSocket, (const struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connect faile. Error code : %d\n", WSAGetLastError());
        closesocket(connectSocket);
    }
    else {
        printf("Connected.\n");

        printf("Sending to NODE_ID:%d . . . ", dst.id);
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
                    if (!out_of_time) {
                        printf("Received from NODE_ID:%d. Bytes received: %d\n", dst.id, result);
                        frame_t data_recv = read_buffer(buffer);

                        if ((data_recv.function == FUNC_FOUND) || (data_recv.function == FUNC_RECV)) {
                            *flag = 1;
                        }
                        break;
                    }
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
        }
        closesocket(connectSocket);
    }
}

// =================================================== Thread Function ====================================================
void p1() {
    frame_t data_input;

    while(1) {
        int temp;
        int flag_found = 0;
        int flag_recv = 0;

        printf("Select function: (1)SEND (2)SHUTDOWN\n");
        scanf("%d", &temp);
        if (temp == 1) {
            data_input.function = FUNC_SEND;

            printf("Enter a number: ");
            scanf("%d", &(data_input.buffer));

            if (NODE_ID == NODE_A_ID) {
                printf("Select destination: (1)B (2)C (3)D\n");
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
                printf("Select destination: (1)A (2)C (3)D\n");
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
                printf("Select destination: (1)A (2)C (3)D\n");
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
            data_input.source = NODE_ID; 
            printf("Your input: %c%c%c%c\n", data_input.function, data_input.buffer, data_input.source, data_input.destination);

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
                    printf("Find route frame: %c%c%c%c\n", data_find_route.function, data_find_route.buffer, data_find_route.source, data_find_route.destination);

                    create_buffer(data_find_route, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, &flag_found);

                    if (flag_found == 1) {
                        printf("Found.\n");
                        printf("Delivering to NODE_ID:%d\n", data_input.destination);
                        create_buffer(data_input, buffer, buffsize);
                        send_to_node(hop[i], buffer, buffsize, &flag_recv);
                        break;
                    }
                }
            }
            if (flag_found == 0) {
                printf("Can not send to NODE_ID:%d.\n", data_input.destination);
            }
            if (flag_recv == 1) {
                printf("NODE_ID:%d received.\n", data_input.destination);
            }
        }
        else if (temp == 2) {
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
    int flag_transfer = 0;
    int flag_recv = 0;
    int result;
    int index_node_transfer;

    frame_t data_recv;
    frame_t data_rep;
    frame_t data_find_route;

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
                    printf("Server received a message: %s. Bytes received: %d\n", buffer, result);
                    data_recv = read_buffer(buffer);

                    if (data_recv.destination == NODE_ID) {
                        if (data_recv.function == FUNC_FIND) {
                            data_rep.function = FUNC_FOUND;
                            data_rep.buffer = data_recv.buffer;
                            data_rep.source = NODE_ID;
                            data_rep.destination = data_recv.source;
                        }
                        if (data_recv.function == FUNC_SEND) {
                            printf("\t\t\t\t\t\t\t");
                            printf("Message reached destination: %s. Byte received: %d\n", buffer, result);

                            data_rep.function = FUNC_RECV;
                            data_rep.buffer = data_recv.buffer;
                            data_rep.source = NODE_ID;
                            data_rep.destination = data_recv.source;
                        }
                        create_buffer(data_rep, buffer, buffsize);
                        send(clientSocket, buffer, buffsize, 0);
                        closesocket(clientSocket);
                        printf("\t\t\t\t\t\t\t");
                        printf("Server responsed the message.\n");
                    }
                    else {
                        if (flag_transfer == 0) {
                            for (int i=0; i<HOP_SIZE; i++) {
                                if (hop[i].id == data_recv.source) {
                                    continue;
                                }
                                else {
                                    data_find_route = data_recv;
                                    data_find_route.function = FUNC_FIND;
                                    
                                    create_buffer(data_find_route, buffer, buffsize);
                                    send_to_node(hop[i], buffer, buffsize, &flag_found);

                                    if (flag_found) {
                                        data_rep = data_recv;
                                        data_rep.function = FUNC_FOUND;

                                        create_buffer(data_rep, buffer, buffsize);
                                        send(clientSocket, buffer, buffsize, 0);

                                        printf("\t\t\t\t\t\t\t");
                                        printf("Server responsed the message.\n");
                                        closesocket(clientSocket);

                                        index_node_transfer = i;
                                        flag_transfer = 1;                                        
                                        flag_found = 0;
                                        break;
                                    }
                                }
                            }
                        }
                        else {
                            create_buffer(data_recv, buffer, buffsize);
                            send_to_node(hop[index_node_transfer], buffer, buffsize, &flag_recv);

                            if (flag_recv == 1) {
                                data_rep = data_recv;

                                create_buffer(data_rep, buffer, buffsize);
                                send(clientSocket, buffer, buffsize, 0);

                                printf("\t\t\t\t\t\t\t");
                                printf("Server responsed the message.\n");
                                closesocket(clientSocket);
                                flag_recv = 0;
                                flag_transfer = 0;
                            }
                        }
                    }
                }
                if (result == 0) {
                    printf("\t\t\t\t\t\t\t");
                    printf("Connection closed\n");
                }
                // else {
                //     printf("\t\t\t\t\t\t\t");
                //     printf("Receive failed. Error code : %d\n", WSAGetLastError());
                // }
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