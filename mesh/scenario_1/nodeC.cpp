#include <stdio.h>
#include <winsock2.h>
#include <thread>

#pragma commnet(lib, "ws2_32.lib")

#define FUNC_SEND 80
#define FUNC_RECV 81
#define FUNC_ACTV 82
#define FUNC_ON 83
#define FUNC_FIND 84
#define FUNC_FOUND 85
#define FUNC_ERROR 86

#define NODE_A_ID 90
#define NODE_A_ADDR "127.0.0.1"
#define NODE_A_PORT 8080

#define NODE_B_ID 91
#define NODE_B_ADDR "127.0.0.1"
#define NODE_B_PORT 8081

#define NODE_C_ID 92
#define NODE_C_ADDR "127.0.0.1"
#define NODE_C_PORT 8082

#define NODE_D_ID 93
#define NODE_D_ADDR "127.0.0.1"
#define NODE_D_PORT 8083

#define NODE_ID NODE_C_ID
#define NODE_ADDR NODE_C_ADDR
#define NODE_PORT NODE_C_PORT
#define HOP_SIZE 3

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

// =================================================== Define Function ====================================================
void create_hop() {
    if (NODE_ID == NODE_A_ID) {
        hop[0].id = NODE_D_ID;
        hop[0].ip_addr = NODE_D_ADDR;
        hop[0].port = NODE_D_PORT;
        
        hop[1].id = NODE_C_ID;
        hop[1].ip_addr = NODE_C_ADDR;
        hop[1].port = NODE_C_PORT;

        hop[2].id = NODE_B_ID;
        hop[2].ip_addr = NODE_B_ADDR;
        hop[2].port = NODE_B_PORT;
    }
    if (NODE_ID == NODE_B_ID) {
        hop[0].id = NODE_A_ID;
        hop[0].ip_addr = NODE_A_ADDR;
        hop[0].port = NODE_A_PORT;

        hop[1].id = NODE_C_ID;
        hop[1].ip_addr = NODE_C_ADDR;
        hop[1].port = NODE_C_PORT;

        hop[2].id = NODE_D_ID;
        hop[2].ip_addr = NODE_D_ADDR;
        hop[2].port = NODE_D_PORT;
    }
    if (NODE_ID == NODE_C_ID) {
        hop[0].id = NODE_B_ID;
        hop[0].ip_addr = NODE_B_ADDR;
        hop[0].port = NODE_B_PORT;
        
        hop[1].id = NODE_D_ID;
        hop[1].ip_addr = NODE_D_ADDR;
        hop[1].port = NODE_D_PORT;

        hop[2].id = NODE_D_ID;
        hop[2].ip_addr = NODE_D_ADDR;
        hop[2].port = NODE_D_PORT;
    }
    if (NODE_ID == NODE_D_ID) {
        hop[0].id = NODE_B_ID;
        hop[0].ip_addr = NODE_B_ADDR;
        hop[0].port = NODE_B_PORT;
        
        hop[1].id = NODE_C_ID;
        hop[1].ip_addr = NODE_C_ADDR;
        hop[1].port = NODE_C_PORT;

        hop[2].id = NODE_A_ID;
        hop[2].ip_addr = NODE_A_ADDR;
        hop[2].port = NODE_A_PORT;
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
    u_long unBlockingMode = 0;
    frame_t data_temp = read_buffer(buffer);

    connectSocket = socket(AF_INET, SOCK_STREAM, 0);

    unBlockingMode = 1;
    if (ioctlsocket(connectSocket, FIONBIO, &unBlockingMode) != NO_ERROR) {
        printf("ioctlsocket() failed. Error code: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(dst.ip_addr);
    server.sin_port = htons(dst.port);

    create_buffer(data_temp, buffer, buffsize);

    // printf("Connecting to NODE_ID:%d . . . ", dst.id);
    if (connect(connectSocket, (const struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        fd_set writefds;

        FD_ZERO(&writefds);
        FD_SET(connectSocket, &writefds);

        timeval timeOut = {0};
        timeOut.tv_sec = 3;
        timeOut.tv_usec = 0;

        if (select(0, NULL, &writefds, NULL, &timeOut) <= 0) {
            // printf("Time out.\n");
            closesocket(connectSocket);
            return;
        }
    }
    // printf("Connected.\n");
	unBlockingMode = 0;
    if (ioctlsocket(connectSocket, FIONBIO, &unBlockingMode) != NO_ERROR) {
        printf("ioctlsocket() failed. Error code: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        exit(1);
    }

    // printf("Sending to NODE_ID:%d . . . ", dst.id);
    if ((send(connectSocket, buffer, buffsize, 0)) == SOCKET_ERROR) {
        printf("Failed.\n");
        closesocket(connectSocket);
        exit(1);
    }
    // printf("Sent.\n");

    // Waiting for the response message
    int result;
    do {
        result = recv(connectSocket, buffer, buffsize, 0);
        if (result > 0) {
            // printf("Received from NODE_ID:%d: %s. Bytes received: %d\n", dst.id, buffer, result);
            frame_t data_recv = read_buffer(buffer);

            if ((data_recv.function == FUNC_ON) || (data_recv.function == FUNC_FOUND) || (data_recv.function == FUNC_RECV)) {
                *flag = 1;
            }
            if (data_recv.function == FUNC_ERROR) {
                if (dst.id == NODE_A_ID) {
                    printf("Receive \"ERROR\" from NODE_A.\n");
                }
                if (dst.id == NODE_B_ID) {
                    printf("Receive \"ERROR\" from NODE_B.\n");
                }
                if (dst.id == NODE_C_ID) {
                    printf("Receive \"ERROR\" from NODE_C.\n");
                }
                if (dst.id == NODE_D_ID) {
                    printf("Receive \"ERROR\" from NODE_D.\n");
                }
            }
            break;
        }
        else if (result == 0) {
            printf("Connection closed.\n");
        }
        else {
            printf("Receive failed. Error code: %d.\n", WSAGetLastError());
        }              
    } while (result > 0);

    closesocket(connectSocket);
}

// =================================================== Thread Function ====================================================
void p1() {
    frame_t data_input;
    int flag_disconnect_c = 0;

    while(1) {
        int temp;
        int flag_actv = 0;
        int flag_found = 0;
        int flag_recv = 0;

        int buffsize = 4;
        char buffer[buffsize];

        if (NODE_ID == NODE_A_ID) {
            if (flag_disconnect_c == 0) {
                printf("Select function: (1)SEND (2)SHUTDOWN (3)DISCONNECT_C\n");
                scanf("%d", &temp);
            }
            else {
                printf("Select function: (1)SEND (2)SHUTDOWN (3)RECONNECT_C\n");
                scanf("%d", &temp);
            }
        }
        else {
            printf("Select function: (1)SEND (2)SHUTDOWN\n");
            scanf("%d", &temp);
        }
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
                printf("Select destination: (1)A (2)B (3)D\n");
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
            if (NODE_ID == NODE_D_ID) {
                printf("Select destination: (1)A (2)B (3)C\n");
                scanf("%d", &temp);
                if (temp == 1) {
                    data_input.destination = NODE_A_ID;
                }
                if (temp == 2) {
                    data_input.destination = NODE_B_ID;
                }
                if (temp == 3) {
                    data_input.destination = NODE_C_ID;
                }
            }
            data_input.source = NODE_ID;

            // check if the destination node is in hop
            for (int i=0; i<HOP_SIZE; i++) {
                // if the destination node is in hop, send the message
                if (data_input.destination == hop[i].id) {
                    if (data_input.destination == NODE_A_ID) {
                        printf("NODE_A is in reach.\n");
                    }
                    if (data_input.destination == NODE_B_ID) {
                        printf("NODE_B is in reach.\n");
                    }
                    if (data_input.destination == NODE_C_ID) {
                        printf("NODE_C is in reach.\n");
                    }
                    if (data_input.destination == NODE_D_ID) {
                        printf("NODE_D is in reach.\n");
                    }
                    flag_found = 1;

                    if (hop[i].id == NODE_A_ID) {
                        printf("Send \"ACT\" to NODE_A.\n");
                    }
                    if (hop[i].id == NODE_B_ID) {
                        printf("Send \"ACT\" to NODE_B.\n");
                    }
                    if (hop[i].id == NODE_C_ID) {
                        printf("Send \"ACT\" to NODE_C.\n");
                    }
                    if (hop[i].id == NODE_D_ID) {
                        printf("Send \"ACT\" to NODE_D.\n");
                    }
                    frame_t data_check_avtive = data_input;
                    data_check_avtive.function = FUNC_ACTV;
                    data_check_avtive.destination = hop[i].id;

                    create_buffer(data_check_avtive, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, &flag_actv);

                    if (flag_actv == 1) {
                        // if node is available, find new route by it
                        if (hop[i].id == NODE_A_ID) {
                            printf("Receive \"ON\" from NODE_A.\n");
                        }
                        if (hop[i].id == NODE_B_ID) {
                            printf("Receive \"ON\" from NODE_B.\n");
                        }
                        if (hop[i].id == NODE_C_ID) {
                            printf("Receive \"ON\" from NODE_C.\n");
                        }
                        if (hop[i].id == NODE_D_ID) {
                            printf("Receive \"ON\" from NODE_D.\n");
                        }

                        if (data_input.destination == NODE_A_ID) {
                            printf("Deliver message to NODE_A.\n");
                        }
                        if (data_input.destination == NODE_B_ID) {
                            printf("Deliver message to NODE_B.\n");
                        }
                        if (data_input.destination == NODE_C_ID) {
                            printf("Deliver message to NODE_C.\n");
                        }
                        if (data_input.destination == NODE_D_ID) {
                            printf("Deliver message to NODE_D.\n");
                        }
                        create_buffer(data_input, buffer, buffsize);
                        send_to_node(hop[i], buffer, buffsize, &flag_recv);
                        break;
                    }
                    else {
                        if (hop[i].id == NODE_A_ID) {
                            printf("NODE_A not reply.\n");
                        }
                        if (hop[i].id == NODE_B_ID) {
                            printf("NODE_B not reply.\n");
                        }
                        if (hop[i].id == NODE_C_ID) {
                            printf("NODE_C not reply.\n");
                        }
                        if (hop[i].id == NODE_D_ID) {
                            printf("NODE_D not reply.\n");
                        }
                    }
                }
            }
            // if the destination node is not in hop, find a route to it
            if (flag_found == 0) {
                if (data_input.destination == NODE_A_ID) {
                    printf("NODE_A is not in reach.\n");
                }
                if (data_input.destination == NODE_B_ID) {
                    printf("NODE_B is not in reach.\n");
                }
                if (data_input.destination == NODE_C_ID) {
                    printf("NODE_C is not in reach.\n");
                }
                if (data_input.destination == NODE_D_ID) {
                    printf("NODE_D is not in reach.\n");
                }
                printf("Find new route.\n");
                for (int i=0; i<HOP_SIZE; i++) {
                    // Check availability of surrounding node
                    if (hop[i].id == NODE_A_ID) {
                        printf("Send \"ACT\" to NODE_A.\n");
                    }
                    if (hop[i].id == NODE_B_ID) {
                        printf("Send \"ACT\" to NODE_B.\n");
                    }
                    if (hop[i].id == NODE_C_ID) {
                        printf("Send \"ACT\" to NODE_C.\n");
                    }
                    if (hop[i].id == NODE_D_ID) {
                        printf("Send \"ACT\" to NODE_D.\n");
                    }
                    frame_t data_check_avtive = data_input;
                    data_check_avtive.function = FUNC_ACTV;
                    data_check_avtive.destination = hop[i].id;

                    create_buffer(data_check_avtive, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, &flag_actv);

                    if (flag_actv == 1) {
                        // if node is available, find new route by it
                        if (hop[i].id == NODE_A_ID) {
                            printf("Receive \"ON\" from NODE_A.\nSend \"FIND\" to NODE_A.\n");
                        }
                        if (hop[i].id == NODE_B_ID) {
                            printf("Receive \"ON\" from NODE_B.\nSend \"FIND\" to NODE_B.\n");
                        }
                        if (hop[i].id == NODE_C_ID) {
                            printf("Receive \"ON\" from NODE_C.\nSend \"FIND\" to NODE_C.\n");
                        }
                        if (hop[i].id == NODE_D_ID) {
                            printf("Receive \"ON\" from NODE_D.\nSend \"FIND\" to NODE_D.\n");
                        }
                        frame_t data_find_route = data_input;
                        data_find_route.function = FUNC_FIND;
                        data_find_route.buffer = hop[i].id;
                        // printf("Find route frame: %c%c%c%c\n", data_find_route.function, data_find_route.buffer, data_find_route.source, data_find_route.destination);

                        create_buffer(data_find_route, buffer, buffsize);
                        send_to_node(hop[i], buffer, buffsize, &flag_found);

                        if (flag_found == 1) {
                            if (hop[i].id == NODE_A_ID) {
                                printf("Receive \"FOUND\" from NODE_A.\n");
                            }
                            if (hop[i].id == NODE_B_ID) {
                                printf("Receive \"FOUND\" from NODE_B.\n");
                            }
                            if (hop[i].id == NODE_C_ID) {
                                printf("Receive \"FOUND\" from NODE_C.\n");
                            }
                            if (hop[i].id == NODE_D_ID) {
                                printf("Receive \"FOUND\" from NODE_D.\n");
                            }
                            
                            if (data_input.destination == NODE_A_ID) {
                                printf("Deliver message to NODE_A.\n");
                            }
                            if (data_input.destination == NODE_B_ID) {
                                printf("Deliver message to NODE_B.\n");
                            }
                            if (data_input.destination == NODE_C_ID) {
                                printf("Deliver message to NODE_C.\n");
                            }
                            if (data_input.destination == NODE_D_ID) {
                                printf("Deliver message to NODE_D.\n");
                            }
                            create_buffer(data_input, buffer, buffsize);
                            send_to_node(hop[i], buffer, buffsize, &flag_recv);
                            break;
                        }
                        // else {
                        //     if (hop[i].id == NODE_A_ID) {
                        //         printf("NODE_A can not find a route.\n");
                        //     }
                        //     if (hop[i].id == NODE_B_ID) {
                        //         printf("NODE_B can not find a route.\n");
                        //     }
                        //     if (hop[i].id == NODE_C_ID) {
                        //         printf("NODE_C can not find a route.\n");
                        //     }
                        //     if (hop[i].id == NODE_D_ID) {
                        //         printf("NODE_D can not find a route.\n");
                        //     }
                        // }
                    }
                    else {
                        if (hop[i].id == NODE_A_ID) {
                            printf("NODE_A not reply.\n");
                        }
                        if (hop[i].id == NODE_B_ID) {
                            printf("NODE_B not reply.\n");
                        }
                        if (hop[i].id == NODE_C_ID) {
                            printf("NODE_C not reply.\n");
                        }
                        if (hop[i].id == NODE_D_ID) {
                            printf("NODE_D not reply.\n");
                        }
                    }
                }
            }
            if (flag_found == 0) {
                if (data_input.destination == NODE_A_ID) {
                    printf("---------------- Can not find NODE_A ----------------\n");
                }
                if (data_input.destination == NODE_B_ID) {
                    printf("---------------- Can not find NODE_B ----------------\n");
                }
                if (data_input.destination == NODE_C_ID) {
                    printf("---------------- Can not find NODE_C ----------------\n");
                }
                if (data_input.destination == NODE_D_ID) {
                    printf("---------------- Can not find NODE_D ----------------\n");
                }
            }
            else {
                if (flag_recv == 1) {
                    if (data_input.destination == NODE_A_ID) {
                        printf("---------------- NODE_A received the message ----------------\n");
                    }
                    if (data_input.destination == NODE_B_ID) {
                        printf("---------------- NODE_B received the message ----------------\n");
                    }
                    if (data_input.destination == NODE_C_ID) {
                        printf("---------------- NODE_C received the message ----------------\n");
                    }
                    if (data_input.destination == NODE_D_ID) {
                        printf("---------------- NODE_D received the message ----------------\n");
                    }
                }
                else {
                    if (data_input.destination == NODE_A_ID) {
                        printf("---------------- Can not send to NODE_A ----------------\n");
                    }
                    if (data_input.destination == NODE_B_ID) {
                        printf("---------------- Can not send to NODE_B ----------------\n");
                    }
                    if (data_input.destination == NODE_C_ID) {
                        printf("---------------- Can not send to NODE_C ----------------\n");
                    }
                    if (data_input.destination == NODE_D_ID) {
                        printf("---------------- Can not send to NODE_D ----------------\n");
                    }
                }
            }
        }
        else if (temp == 2) {
            exit(0);
        }
        else if (temp == 3) {
            if (flag_disconnect_c == 0) {
                hop[1] = {0};
                flag_disconnect_c = 1;
            }
            else {
                hop[1].id = NODE_C_ID;
                hop[1].ip_addr = NODE_C_ADDR;
                hop[1].port = NODE_C_PORT;
                flag_disconnect_c = 0;
            }
        }
    }
}

void p3() {
    struct sockaddr_in server, client;
    
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    int buffsize = 4;
    char buffer[buffsize];

    int flag_actv = 0;
    int flag_found = 0;
    int flag_transfer = 0;
    int flag_recv = 0;
    int result;
    int index_node_transfer = 0;
    int index_node_find = 0;

    frame_t data_recv;
    frame_t data_rep;
    frame_t data_check_avtive;
    frame_t data_find_route;
    frame_t data_send;

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
        // printf("\t\t\t\t\t\t\t");
        // printf("Server is running . . .\n");

        while((clientSocket = accept(listenSocket , NULL, NULL)) != INVALID_SOCKET)
        {
            // printf("\t\t\t\t\t\t\t");
            // printf("Connection accepted\n");
            do {
                result = recv(clientSocket, buffer, buffsize, 0);
                if (result > 0) {
                    // printf("\t\t\t\t\t\t\t");
                    // printf("Server received a message: %s. Bytes received: %d\n", buffer, result);
                    data_recv = read_buffer(buffer);
                    // message function
                    if (data_recv.function == FUNC_ACTV) {
                        printf("Receive \"ACT\" ");
                    }
                    if (data_recv.function == FUNC_FIND) {
                        printf("Receive \"FIND\" ");
                        if (data_recv.destination == NODE_A_ID) {
                            printf("NODE_A ");
                        }
                        if (data_recv.destination == NODE_B_ID) {
                            printf("NODE_B ");
                        }
                        if (data_recv.destination == NODE_C_ID) {
                            printf("NODE_C ");
                        }
                        if (data_recv.destination == NODE_D_ID) {
                            printf("NODE_D ");
                        }
                    }
                    if (data_recv.function == FUNC_ON) {
                        printf("Receive \"ON\" ");
                    }
                    if (data_recv.function == FUNC_SEND) {
                        printf("Got message to ");
                        if (data_recv.destination == NODE_A_ID) {
                            printf("NODE_A ");
                        }
                        if (data_recv.destination == NODE_B_ID) {
                            printf("NODE_B ");
                        }
                        if (data_recv.destination == NODE_C_ID) {
                            printf("NODE_C ");
                        }
                        if (data_recv.destination == NODE_D_ID) {
                            printf("(NODE_D) ");
                        }
                    }

                    // message source
                    if (data_recv.source == NODE_A_ID) {
                        printf("from NODE_A.\n");
                    }
                    if (data_recv.source == NODE_B_ID) {
                        printf("from NODE_B.\n");
                    }
                    if (data_recv.source == NODE_C_ID) {
                        printf("from NODE_C.\n");
                    }
                    if (data_recv.source == NODE_D_ID) {
                        printf("from NODE_D.\n");
                    }

                    // message destination
                    if (data_recv.destination == NODE_ID) {
                        if (data_recv.function == FUNC_ACTV) {
                            data_rep.function = FUNC_ON;
                            data_rep.buffer = data_recv.buffer;
                            data_rep.source = NODE_ID;
                            data_rep.destination = data_recv.source;

                            create_buffer(data_rep, buffer, buffsize);
                            send(clientSocket, buffer, buffsize, 0);
                            closesocket(clientSocket);

                            if (data_recv.source == NODE_A_ID) {
                                printf("Send \"ON\" to NODE_A.\n");
                            }
                            if (data_recv.source == NODE_B_ID) {
                                printf("Send \"ON\" to NODE_B.\n");
                            }
                            if (data_recv.source == NODE_C_ID) {
                                printf("Send \"ON\" to NODE_C.\n");
                            }
                            if (data_recv.source == NODE_D_ID) {
                                printf("Send \"ON\" to NODE_D.\n");
                            }
                        }
                        if (data_recv.function == FUNC_FIND) {
                            data_rep.function = FUNC_FOUND;
                            data_rep.buffer = data_recv.buffer;
                            data_rep.source = NODE_ID;
                            data_rep.destination = data_recv.source;

                            create_buffer(data_rep, buffer, buffsize);
                            send(clientSocket, buffer, buffsize, 0);
                            closesocket(clientSocket);
                            
                            if (data_recv.source == NODE_A_ID) {
                                printf("Send \"FOUND\" to NODE_A.\n");
                            }
                            if (data_recv.source == NODE_B_ID) {
                                printf("Send \"FOUND\" to NODE_B.\n");
                            }
                            if (data_recv.source == NODE_C_ID) {
                                printf("Send \"FOUND\" to NODE_C.\n");
                            }
                            if (data_recv.source == NODE_D_ID) {
                                printf("Send \"FOUND\" to NODE_D.\n");
                            }
                        }
                        if (data_recv.function == FUNC_SEND) {
                            data_rep.function = FUNC_RECV;
                            data_rep.buffer = data_recv.buffer;
                            data_rep.source = NODE_ID;
                            data_rep.destination = data_recv.source;

                            create_buffer(data_rep, buffer, buffsize);
                            send(clientSocket, buffer, buffsize, 0);
                            closesocket(clientSocket);

                            if (data_recv.source == NODE_A_ID) {
                                printf("Send \"RECV\" to NODE_A.\n");
                            }
                            if (data_recv.source == NODE_B_ID) {
                                printf("Send \"RECV\" to NODE_B.\n");
                            }
                            if (data_recv.source == NODE_C_ID) {
                                printf("Send \"RECV\" to NODE_C.\n");
                            }
                            if (data_recv.source == NODE_D_ID) {
                                printf("Send \"RECV\" to NODE_D.\n");
                            }
                            
                            // printf("\t\t\t\t\t\t\t");
                            printf("---------------- Message reached destination: %d ----------------\n", data_recv.buffer);
                        }
                    }
                    else {
                        if (flag_transfer == 0) {
                            for (int i=0; i<HOP_SIZE; i++) {
                                if (hop[i].id == data_recv.destination) {
                                    flag_found = 1;
                                    index_node_find = i;
                                    break;
                                }
                            }
                            if (flag_found == 1) {
                                if (hop[index_node_find].id == NODE_A_ID) {
                                    printf("Send \"ACT\" to NODE_A.\n");    
                                }
                                if (hop[index_node_find].id == NODE_B_ID) {
                                    printf("Send \"ACT\" to NODE_B.\n");    
                                }
                                if (hop[index_node_find].id == NODE_C_ID) {
                                    printf("Send \"ACT\" to NODE_C.\n");    
                                }
                                if (hop[index_node_find].id == NODE_D_ID) {
                                    printf("Send \"ACT\" to NODE_D.\n");    
                                }
                                
                                data_check_avtive.function = FUNC_ACTV;
                                data_check_avtive.buffer = data_recv.buffer;
                                data_check_avtive.source = NODE_ID;
                                data_check_avtive.destination = hop[index_node_find].id;

                                create_buffer(data_check_avtive, buffer, buffsize);
                                send_to_node(hop[index_node_find], buffer, buffsize, &flag_actv);

                                if (flag_actv == 1) {
                                    if (hop[index_node_find].id == NODE_A_ID) {
                                        printf("Receive \"ON\" from NODE_A.\n");
                                    }
                                    if (hop[index_node_find].id == NODE_B_ID) {
                                        printf("Receive \"ON\" from NODE_B.\n");
                                    }
                                    if (hop[index_node_find].id == NODE_C_ID) {
                                        printf("Receive \"ON\" from NODE_C.\n");
                                    }
                                    if (hop[index_node_find].id == NODE_D_ID) {
                                        printf("Receive \"ON\" from NODE_D.\n");
                                    }
                                    
                                    data_rep.function = FUNC_FOUND;
                                    data_rep.buffer = data_recv.destination;
                                    data_rep.source = NODE_ID;
                                    data_rep.destination = data_recv.source;

                                    create_buffer(data_rep, buffer, buffsize);
                                    send(clientSocket, buffer, buffsize, 0);
                                    closesocket(clientSocket);

                                    if (data_recv.source == NODE_A_ID) {
                                        printf("Send \"FOUND\" to NODE_A.\n");
                                    }
                                    if (data_recv.source == NODE_B_ID) {
                                        printf("Send \"FOUND\" to NODE_B.\n");
                                    }
                                    if (data_recv.source == NODE_C_ID) {
                                        printf("Send \"FOUND\" to NODE_C.\n");
                                    }
                                    if (data_recv.source == NODE_D_ID) {
                                        printf("Send \"FOUND\" to NODE_D.\n");
                                    }

                                    flag_transfer = 1;
                                    index_node_transfer = index_node_find;
                                }
                                else {
                                    if (hop[index_node_find].id == NODE_A_ID) {
                                        printf("NODE_A not reply.\n");
                                    }
                                    if (hop[index_node_find].id == NODE_B_ID) {
                                        printf("NODE_B not reply.\n");
                                    }
                                    if (hop[index_node_find].id == NODE_C_ID) {
                                        printf("NODE_C not reply.\n");
                                    }
                                    if (hop[index_node_find].id == NODE_D_ID) {
                                        printf("NODE_D not reply.\n");
                                    }
                                    data_rep.function = FUNC_ERROR;
                                    data_rep.buffer = data_recv.buffer;
                                    data_rep.source = NODE_ID;
                                    data_rep.destination = data_recv.source;

                                    create_buffer(data_rep, buffer, buffsize);
                                    send(clientSocket, buffer, buffsize, 0);
                                    closesocket(clientSocket);
                                }
                            }
                            else {
                                for (int i=0; i<HOP_SIZE; i++) {
                                    if (hop[i].id == data_recv.source) {
                                        continue;
                                    }
                                    else {
                                        // Check availability of surrounding node
                                        if (hop[i].id == NODE_A_ID) {
                                            printf("Send \"ACT\" to NODE_A.\n");
                                        }
                                        if (hop[i].id == NODE_B_ID) {
                                            printf("Send \"ACT\" to NODE_B.\n");
                                        }
                                        if (hop[i].id == NODE_C_ID) {
                                            printf("Send \"ACT\" to NODE_C.\n");
                                        }
                                        if (hop[i].id == NODE_D_ID) {
                                            printf("Send \"ACT\" to NODE_D.\n");
                                        }
                                        data_check_avtive.function = FUNC_ACTV;
                                        data_check_avtive.buffer = data_recv.buffer;
                                        data_check_avtive.source = NODE_ID;
                                        data_check_avtive.destination = hop[i].id;

                                        create_buffer(data_check_avtive, buffer, buffsize);
                                        send_to_node(hop[i], buffer, buffsize, &flag_actv);

                                        if (flag_actv == 1) {
                                            // if node is available, find new route by it
                                            if (hop[i].id == NODE_A_ID) {
                                                printf("Receive \"ON\" from NODE_A.\nSend \"FIND\" to NODE_A.\n");
                                            }
                                            if (hop[i].id == NODE_B_ID) {
                                                printf("Receive \"ON\" from NODE_B.\nSend \"FIND\" to NODE_B.\n");
                                            }
                                            if (hop[i].id == NODE_C_ID) {
                                                printf("Receive \"ON\" from NODE_C.\nSend \"FIND\" to NODE_C.\n");
                                            }
                                            if (hop[i].id == NODE_D_ID) {
                                                printf("Receive \"ON\" from NODE_D.\nSend \"FIND\" to NODE_D.\n");
                                            }

                                            data_find_route.function = FUNC_FIND;
                                            data_find_route.buffer = data_recv.source;
                                            data_find_route.source = NODE_ID;
                                            data_find_route.destination = data_recv.destination;
                                            
                                            create_buffer(data_find_route, buffer, buffsize);
                                            send_to_node(hop[i], buffer, buffsize, &flag_found);

                                            if (flag_found == 1) {
                                                if (hop[i].id == NODE_A_ID) {
                                                    printf("Receive \"FOUND\" from NODE_A.\n");
                                                }
                                                if (hop[i].id == NODE_B_ID) {
                                                    printf("Receive \"FOUND\" from NODE_B.\n");
                                                }
                                                if (hop[i].id == NODE_C_ID) {
                                                    printf("Receive \"FOUND\" from NODE_C.\n");
                                                }
                                                if (hop[i].id == NODE_D_ID) {
                                                    printf("Receive \"FOUND\" from NODE_D.\n");
                                                }
                                                data_rep.function = FUNC_FOUND;
                                                data_rep.buffer = data_recv.destination;
                                                data_rep.source = NODE_ID;
                                                data_rep.destination = data_recv.source;

                                                create_buffer(data_rep, buffer, buffsize);
                                                send(clientSocket, buffer, buffsize, 0);
                                                closesocket(clientSocket);

                                                if (data_recv.source == NODE_A_ID) {
                                                    printf("Send \"FOUND\" to NODE_A.\n");
                                                }
                                                if (data_recv.source == NODE_B_ID) {
                                                    printf("Send \"FOUND\" to NODE_B.\n");
                                                }
                                                if (data_recv.source == NODE_C_ID) {
                                                    printf("Send \"FOUND\" to NODE_C.\n");
                                                }
                                                if (data_recv.source == NODE_D_ID) {
                                                    printf("Send \"FOUND\" to NODE_D.\n");
                                                }

                                                index_node_transfer = i;
                                                flag_transfer = 1;                                        
                                                flag_found = 0;
                                                break;
                                            }
                                            else {
                                                data_rep.function = FUNC_ERROR;
                                                data_rep.buffer = data_recv.buffer;
                                                data_rep.source = NODE_ID;
                                                data_rep.destination = data_recv.source;

                                                create_buffer(data_rep, buffer, buffsize);
                                                send(clientSocket, buffer, buffsize, 0);
                                                closesocket(clientSocket);

                                                if (data_recv.source == NODE_A_ID) {
                                                    printf("Send \"ERROR\" to NODE_A.\n");
                                                }
                                                if (data_recv.source == NODE_B_ID) {
                                                    printf("Send \"ERROR\" to NODE_B.\n");
                                                }
                                                if (data_recv.source == NODE_C_ID) {
                                                    printf("Send \"ERROR\" to NODE_C.\n");
                                                }
                                                if (data_recv.source == NODE_D_ID) {
                                                    printf("Send \"ERROR\" to NODE_D.\n");
                                                }
                                            }
                                        }
                                        else {
                                            if (hop[i].id == NODE_A_ID) {
                                                printf("NODE_A not reply.\n");
                                            }
                                            if (hop[i].id == NODE_B_ID) {
                                                printf("NODE_B not reply.\n");
                                            }
                                            if (hop[i].id == NODE_C_ID) {
                                                printf("NODE_C not reply.\n");
                                            }
                                            if (hop[i].id == NODE_D_ID) {
                                                printf("NODE_D not reply.\n");
                                            }
                                            data_rep.function = FUNC_ERROR;
                                            data_rep.buffer = data_recv.buffer;
                                            data_rep.source = NODE_ID;
                                            data_rep.destination = data_recv.source;

                                            create_buffer(data_rep, buffer, buffsize);
                                            send(clientSocket, buffer, buffsize, 0);
                                            closesocket(clientSocket);
                                        }
                                    }
                                }
                            }
                        }
                        else {
                            if (hop[index_node_transfer].id == NODE_A_ID) {
                                printf("Transfer message to NODE_A.\n");
                            }
                            if (hop[index_node_transfer].id == NODE_B_ID) {
                                printf("Transfer message to NODE_B.\n");
                            }
                            if (hop[index_node_transfer].id == NODE_C_ID) {
                                printf("Transfer message to NODE_C.\n");
                            }
                            if (hop[index_node_transfer].id == NODE_D_ID) {
                                printf("Transfer message to NODE_D.\n");
                            }
                            data_send = data_recv;
                            data_send.source = NODE_ID;
                            create_buffer(data_send, buffer, buffsize);
                            send_to_node(hop[index_node_transfer], buffer, buffsize, &flag_recv);

                            if (flag_recv == 1) {
                                if (hop[index_node_transfer].id == NODE_A_ID) {
                                    printf("Receive \"RECV\" from NODE_A.\n");
                                }
                                if (hop[index_node_transfer].id == NODE_B_ID) {
                                    printf("Receive \"RECV\" from NODE_B.\n");
                                }
                                if (hop[index_node_transfer].id == NODE_C_ID) {
                                    printf("Receive \"RECV\" from NODE_C.\n");
                                }
                                if (hop[index_node_transfer].id == NODE_D_ID) {
                                    printf("Receive \"RECV\" from NODE_D.\n");
                                }
                                data_rep.function = FUNC_RECV;
                                data_rep.buffer = data_recv.destination;
                                data_rep.source = NODE_ID;
                                data_rep.destination = data_recv.source;

                                create_buffer(data_rep, buffer, buffsize);
                                send(clientSocket, buffer, buffsize, 0);
                                closesocket(clientSocket);

                                flag_recv = 0;
                                flag_transfer = 0;

                                if (data_recv.source == NODE_A_ID) {
                                    printf("Send \"RECV\" to NODE_A.\n");
                                }
                                if (data_recv.source == NODE_B_ID) {
                                    printf("Send \"RECV\" to NODE_B.\n");
                                }
                                if (data_recv.source == NODE_C_ID) {
                                    printf("Send \"RECV\" to NODE_C.\n");
                                }
                                if (data_recv.source == NODE_D_ID) {
                                    printf("Send \"RECV\" to NODE_D.\n");
                                }
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