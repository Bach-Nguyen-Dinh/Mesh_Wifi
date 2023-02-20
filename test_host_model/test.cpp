#include <stdio.h>
#include <winsock2.h>
#include <thread>
// #include <chrono>
// #include <mutex>
// #include <condition_variable>

#pragma commnet(lib, "ws2_32.lib")

#define FUNC_SEND 80
#define FUNC_RECV 81
#define FUNC_ACTV 82
#define FUNC_ON 83
#define FUNC_FIND 84
#define FUNC_FOUND 85
#define FUNC_ERROR 86

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
#define HOP_SIZE 2

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
    if (NODE_ID == NODE_B_ID) {
        hop[0].id = NODE_A_ID;
        hop[0].ip_addr = NODE_A_ADDR;
        hop[0].port = NODE_A_PORT;

        hop[1].id = NODE_C_ID;
        hop[1].ip_addr = NODE_C_ADDR;
        hop[1].port = NODE_C_PORT;
    }
    if (NODE_ID == NODE_A_ID) {
        hop[0].id = NODE_D_ID;
        hop[0].ip_addr = NODE_D_ADDR;
        hop[0].port = NODE_D_PORT;
        
        hop[1].id = NODE_B_ID;
        hop[1].ip_addr = NODE_B_ADDR;
        hop[1].port = NODE_B_PORT;
    }
    if (NODE_ID == NODE_C_ID) {
        hop[0].id = NODE_B_ID;
        hop[0].ip_addr = NODE_B_ADDR;
        hop[0].port = NODE_B_PORT;
        
        hop[1].id = NODE_D_ID;
        hop[1].ip_addr = NODE_D_ADDR;
        hop[1].port = NODE_D_PORT;
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
    char *tempBuff = buffer;

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

    // printf("Connecting to NODE_ID:%d . . . ", dst.id);
    if (connect(connectSocket, (const struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        fd_set writefds;

        FD_ZERO(&writefds);
        FD_SET(connectSocket, &writefds);

        timeval timeOut = {0};
        timeOut.tv_sec = 3;
        timeOut.tv_usec = 0;

        if (select(0, NULL, &writefds, NULL, &timeOut) <= 0) {
            printf("Time out.\n");
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
                if (data_recv.source == NODE_A_ID) {
                    printf("Receive \"ERROR\" from node A.\n");
                }
                if (data_recv.source == NODE_B_ID) {
                    printf("Receive \"ERROR\" from node B.\n");
                }
                if (data_recv.source == NODE_C_ID) {
                    printf("Receive \"ERROR\" from node C.\n");
                }
                if (data_recv.source == NODE_D_ID) {
                    printf("Receive \"ERROR\" from node D.\n");
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

    while(1) {
        int temp;
        int flag_actv = 0;
        int flag_found = 0;
        int flag_recv = 0;

        int buffsize = 4;
        char buffer[buffsize];

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
            // printf("Your input: %c%c%c%c\n", data_input.function, data_input.buffer, data_input.source, data_input.destination);

            // check if the destination node is in hop
            for (int i=0; i<HOP_SIZE; i++) {
                // if the destination node is in hop, send the message
                if (data_input.destination == hop[i].id) {
                    if (data_input.destination == NODE_A_ID) {
                        printf("Node A is in reach.\n");
                    }
                    if (data_input.destination == NODE_B_ID) {
                        printf("Node B is in reach.\n");
                    }
                    if (data_input.destination == NODE_C_ID) {
                        printf("Node C is in reach.\n");
                    }
                    if (data_input.destination == NODE_D_ID) {
                        printf("Node D is in reach.\n");
                    }
                    flag_found = 1;
                    if (data_input.destination == NODE_A_ID) {
                        printf("Deliver message to node A.\n");
                    }
                    if (data_input.destination == NODE_B_ID) {
                        printf("Deliver message to node B.\n");
                    }
                    if (data_input.destination == NODE_C_ID) {
                        printf("Deliver message to node C.\n");
                    }
                    if (data_input.destination == NODE_D_ID) {
                        printf("Deliver message to node D.\n");
                    }
                    create_buffer(data_input, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, &flag_recv);
                    break;
                }
            }
            // if the destination node is not in hop, find a route to it
            if (flag_found == 0) {
                if (data_input.destination == NODE_A_ID) {
                    printf("Node A is not in reach.\n");
                }
                if (data_input.destination == NODE_B_ID) {
                    printf("Node B is not in reach.\n");
                }
                if (data_input.destination == NODE_C_ID) {
                    printf("Node C is not in reach.\n");
                }
                if (data_input.destination == NODE_D_ID) {
                    printf("Node D is not in reach.\n");
                }
                printf("Check surrounding node for new route.\n");
                for (int i=0; i<HOP_SIZE; i++) {
                    // Check availability of surrounding node
                    if (hop[i].id == NODE_A_ID) {
                        printf("Send \"ACT\" to node A.\n");
                    }
                    if (hop[i].id == NODE_B_ID) {
                        printf("Send \"ACT\" to node B.\n");
                    }
                    if (hop[i].id == NODE_C_ID) {
                        printf("Send \"ACT\" to node C.\n");
                    }
                    if (hop[i].id == NODE_D_ID) {
                        printf("Send \"ACT\" to node D.\n");
                    }
                    frame_t data_check_avtive = data_input;
                    data_check_avtive.function = FUNC_ACTV;
                    data_check_avtive.destination = hop[i].id;

                    create_buffer(data_check_avtive, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, &flag_actv);

                    if (flag_actv == 1) {
                        // if node is available, find new route by it
                        if (hop[i].id == NODE_A_ID) {
                            printf("Receive \"ON\" from node A.\nSend \"FIND\" to node A.\n");
                        }
                        if (hop[i].id == NODE_B_ID) {
                            printf("Receive \"ON\" from node B.\nSend \"FIND\" to node A.\n");
                        }
                        if (hop[i].id == NODE_C_ID) {
                            printf("Receive \"ON\" from node C.\nSend \"FIND\" to node A.\n");
                        }
                        if (hop[i].id == NODE_D_ID) {
                            printf("Receive \"ON\" from node D.\nSend \"FIND\" to node A.\n");
                        }
                        frame_t data_find_route = data_input;
                        data_find_route.function = FUNC_FIND;
                        // printf("Find route frame: %c%c%c%c\n", data_find_route.function, data_find_route.buffer, data_find_route.source, data_find_route.destination);

                        create_buffer(data_find_route, buffer, buffsize);
                        send_to_node(hop[i], buffer, buffsize, &flag_found);

                        if (flag_found == 1) {
                            if (hop[i].id == NODE_A_ID) {
                                printf("Receive \"FOUND\" from node A.\n");
                            }
                            if (hop[i].id == NODE_B_ID) {
                                printf("Receive \"FOUND\" from node B.\n");
                            }
                            if (hop[i].id == NODE_C_ID) {
                                printf("Receive \"FOUND\" from node C.\n");
                            }
                            if (hop[i].id == NODE_D_ID) {
                                printf("Receive \"FOUND\" from node D.\n");
                            }
                            
                            if (data_input.destination == NODE_A_ID) {
                                printf("Deliver message to node A.\n");
                            }
                            if (data_input.destination == NODE_B_ID) {
                                printf("Deliver message to node B.\n");
                            }
                            if (data_input.destination == NODE_C_ID) {
                                printf("Deliver message to node C.\n");
                            }
                            if (data_input.destination == NODE_D_ID) {
                                printf("Deliver message to node D.\n");
                            }
                            create_buffer(data_input, buffer, buffsize);
                            send_to_node(hop[i], buffer, buffsize, &flag_recv);
                            break;
                        }
                        else {
                            if (hop[i].id == NODE_A_ID) {
                                printf("Node A can not find a route.\n");
                            }
                            if (hop[i].id == NODE_B_ID) {
                                printf("Node B can not find a route.\n");
                            }
                            if (hop[i].id == NODE_C_ID) {
                                printf("Node C can not find a route.\n");
                            }
                            if (hop[i].id == NODE_D_ID) {
                                printf("Node D can not find a route.\n");
                            }
                        }
                    }
                    else {
                        if (hop[i].id == NODE_A_ID) {
                            printf("Node A not reply.\n");
                        }
                        if (hop[i].id == NODE_B_ID) {
                            printf("Node B not reply.\n");
                        }
                        if (hop[i].id == NODE_C_ID) {
                            printf("Node C not reply.\n");
                        }
                        if (hop[i].id == NODE_D_ID) {
                            printf("Node D not reply.\n");
                        }
                    }
                }
            }
            if (flag_found == 0) {
                printf("---------------- Can not find NODE_ID:%d ----------------\n", data_input.destination);
            }
            else {
                if (flag_recv == 1) {
                    printf("---------------- NODE_ID:%d received the message ----------------\n", data_input.destination);
                }
                else {
                    printf("---------------- Can not send to NODE_ID:%d ----------------\n", data_input.destination);
                }
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

    int flag_actv = 0;
    int flag_found = 0;
    int flag_transfer = 0;
    int flag_recv = 0;
    int result;
    int index_node_transfer;

    frame_t data_recv;
    frame_t data_rep;
    frame_t data_check_avtive;
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
                    if (data_recv.function == FUNC_ACTV) {
                        printf("Receive \"ACT\" ");
                    }
                    if (data_recv.function == FUNC_FIND) {
                        printf("Receive \"FIND\" ");
                        if (data_recv.destination == NODE_A_ID) {
                            printf("(node A) ");
                        }
                        if (data_recv.destination == NODE_B_ID) {
                            printf("(node B) ");
                        }
                        if (data_recv.destination == NODE_C_ID) {
                            printf("(node C) ");
                        }
                        if (data_recv.destination == NODE_D_ID) {
                            printf("(node D) ");
                        }
                    }
                    if (data_recv.function == FUNC_ON) {
                        printf("Receive \"ON\" ");
                    }
                    if (data_recv.function == FUNC_SEND) {
                        printf("Got message to ");
                        if (data_recv.destination == NODE_A_ID) {
                            printf("(node A) ");
                        }
                        if (data_recv.destination == NODE_B_ID) {
                            printf("(node B) ");
                        }
                        if (data_recv.destination == NODE_C_ID) {
                            printf("(node C) ");
                        }
                        if (data_recv.destination == NODE_D_ID) {
                            printf("(node D) ");
                        }
                    }

                    if (data_recv.source == NODE_A_ID) {
                        printf("from node A.\n");
                    }
                    if (data_recv.source == NODE_B_ID) {
                        printf("from node B.\n");
                    }
                    if (data_recv.source == NODE_C_ID) {
                        printf("from node C.\n");
                    }
                    if (data_recv.source == NODE_D_ID) {
                        printf("from node D.\n");
                    }

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
                                printf("Send \"ON\" to node A.\n");
                            }
                            if (data_recv.source == NODE_B_ID) {
                                printf("Send \"ON\" to node B.\n");
                            }
                            if (data_recv.source == NODE_C_ID) {
                                printf("Send \"ON\" to node C.\n");
                            }
                            if (data_recv.source == NODE_D_ID) {
                                printf("Send \"ON\" to node D.\n");
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
                            
                            // if (data_recv.source == NODE_A_ID) {
                            //     printf("Send \"FOUND\" to node A.\n");
                            // }
                            // if (data_recv.source == NODE_B_ID) {
                            //     printf("Send \"FOUND\" to node B.\n");
                            // }
                            // if (data_recv.source == NODE_C_ID) {
                            //     printf("Send \"FOUND\" to node C.\n");
                            // }
                            // if (data_recv.source == NODE_D_ID) {
                            //     printf("Send \"FOUND\" to node D.\n");
                            // }
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
                                printf("Send \"RECV\" to node A.\n");
                            }
                            if (data_recv.source == NODE_B_ID) {
                                printf("Send \"RECV\" to node B.\n");
                            }
                            if (data_recv.source == NODE_C_ID) {
                                printf("Send \"RECV\" to node C.\n");
                            }
                            if (data_recv.source == NODE_D_ID) {
                                printf("Send \"RECV\" to node D.\n");
                            }
                            
                            // printf("\t\t\t\t\t\t\t");
                            printf("---------------- Message reached destination: %C%c%c%c ----------------\n",
                                    data_recv.function, data_recv.buffer, data_recv.source, data_recv.destination);
                        }
                    }
                    else {
                        if (flag_transfer == 0) {
                            for (int i=0; i<HOP_SIZE; i++) {
                                if (hop[i].id == data_recv.source) {
                                    continue;
                                }
                                else {
                                    // Check availability of surrounding node
                                    if (hop[i].id == NODE_A_ID) {
                                        printf("Send \"ACT\" to node A.\n");
                                    }
                                    if (hop[i].id == NODE_B_ID) {
                                        printf("Send \"ACT\" to node B.\n");
                                    }
                                    if (hop[i].id == NODE_C_ID) {
                                        printf("Send \"ACT\" to node C.\n");
                                    }
                                    if (hop[i].id == NODE_D_ID) {
                                        printf("Send \"ACT\" to node D.\n");
                                    }
                                    data_check_avtive = data_recv;
                                    data_check_avtive.function = FUNC_ACTV;
                                    data_check_avtive.destination = hop[i].id;

                                    create_buffer(data_check_avtive, buffer, buffsize);
                                    send_to_node(hop[i], buffer, buffsize, &flag_actv);

                                    if (flag_actv == 1) {
                                         // if node is available, find new route by it
                                        if (hop[i].id == NODE_A_ID) {
                                            printf("Receive \"ON\" from node A.\nSend \"FIND\" to node A.\n");
                                        }
                                        if (hop[i].id == NODE_B_ID) {
                                            printf("Receive \"ON\" from node B.\nSend \"FIND\" to node A.\n");
                                        }
                                        if (hop[i].id == NODE_C_ID) {
                                            printf("Receive \"ON\" from node C.\nSend \"FIND\" to node A.\n");
                                        }
                                        if (hop[i].id == NODE_D_ID) {
                                            printf("Receive \"ON\" from node D.\nSend \"FIND\" to node A.\n");
                                        }

                                        data_find_route = data_recv;
                                        data_find_route.function = FUNC_FIND;
                                        
                                        create_buffer(data_find_route, buffer, buffsize);
                                        send_to_node(hop[i], buffer, buffsize, &flag_found);

                                        if (flag_found == 1) {
                                            // if (hop[i].id == NODE_A_ID) {
                                            //     printf("Receive \"FOUND\" from node A.\n");
                                            // }
                                            // if (hop[i].id == NODE_B_ID) {
                                            //     printf("Receive \"FOUND\" from node B.\n");
                                            // }
                                            // if (hop[i].id == NODE_C_ID) {
                                            //     printf("Receive \"FOUND\" from node C.\n");
                                            // }
                                            // if (hop[i].id == NODE_D_ID) {
                                            //     printf("Receive \"FOUND\" from node D.\n");
                                            // }
                                            data_rep = data_recv;
                                            data_rep.function = FUNC_FOUND;

                                            create_buffer(data_rep, buffer, buffsize);
                                            send(clientSocket, buffer, buffsize, 0);
                                            closesocket(clientSocket);

                                            if (data_recv.source == NODE_A_ID) {
                                                printf("Send \"FOUND\" to node A.\n");
                                            }
                                            if (data_recv.source == NODE_B_ID) {
                                                printf("Send \"FOUND\" to node B.\n");
                                            }
                                            if (data_recv.source == NODE_C_ID) {
                                                printf("Send \"FOUND\" to node C.\n");
                                            }
                                            if (data_recv.source == NODE_D_ID) {
                                                printf("Send \"FOUND\" to node D.\n");
                                            }

                                            index_node_transfer = i;
                                            flag_transfer = 1;                                        
                                            flag_found = 0;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (flag_found == 0) {
                                data_rep = data_recv;
                                data_rep.function = FUNC_ERROR;

                                create_buffer(data_rep, buffer, buffsize);
                                send(clientSocket, buffer, buffsize, 0);
                                closesocket(clientSocket);

                                if (data_recv.source == NODE_A_ID) {
                                    printf("Send \"ERROR\" to node A.\n");
                                }
                                if (data_recv.source == NODE_B_ID) {
                                    printf("Send \"ERROR\" to node B.\n");
                                }
                                if (data_recv.source == NODE_C_ID) {
                                    printf("Send \"ERROR\" to node C.\n");
                                }
                                if (data_recv.source == NODE_D_ID) {
                                    printf("Send \"ERROR\" to node D.\n");
                                }
                            }
                        }
                        else {
                            if (hop[index_node_transfer].id == NODE_A_ID) {
                                printf("Deliver message to node A.\n");
                            }
                            if (hop[index_node_transfer].id == NODE_B_ID) {
                                printf("Deliver message to node B.\n");
                            }
                            if (hop[index_node_transfer].id == NODE_C_ID) {
                                printf("Deliver message to node C.\n");
                            }
                            if (hop[index_node_transfer].id == NODE_D_ID) {
                                printf("Deliver message to node D.\n");
                            }
                            create_buffer(data_recv, buffer, buffsize);
                            send_to_node(hop[index_node_transfer], buffer, buffsize, &flag_recv);

                            if (flag_recv == 1) {
                                if (hop[index_node_transfer].id == NODE_A_ID) {
                                    printf("Receive \"RECV\" from node A.\n");
                                }
                                if (hop[index_node_transfer].id == NODE_B_ID) {
                                    printf("Receive \"RECV\" from node B.\n");
                                }
                                if (hop[index_node_transfer].id == NODE_C_ID) {
                                    printf("Receive \"RECV\" from node C.\n");
                                }
                                if (hop[index_node_transfer].id == NODE_D_ID) {
                                    printf("Receive \"RECV\" from node D.\n");
                                }
                                data_rep = data_recv;
                                data_rep.function = FUNC_RECV;

                                create_buffer(data_rep, buffer, buffsize);
                                send(clientSocket, buffer, buffsize, 0);
                                closesocket(clientSocket);

                                flag_recv = 0;
                                flag_transfer = 0;

                                if (data_recv.source == NODE_A_ID) {
                                    printf("Send \"RECV\" to node A.\n");
                                }
                                if (data_recv.source == NODE_B_ID) {
                                    printf("Send \"RECV\" to node B.\n");
                                }
                                if (data_recv.source == NODE_C_ID) {
                                    printf("Send \"RECV\" to node C.\n");
                                }
                                if (data_recv.source == NODE_D_ID) {
                                    printf("Send \"RECV\" to node D.\n");
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