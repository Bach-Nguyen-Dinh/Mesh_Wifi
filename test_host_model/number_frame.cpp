#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#pragma commnet(lib, "ws2_32.lib")

#define FUNC_SEND 80
#define FUNC_RECV 81
#define FUNC_TNFR 82
#define FUNC_SHDW 83
#define FUNC_FIND 84
#define FUNC_FOUND 85
#define FUNC_STBY 86

#define NODE_A_ID 90
#define NODE_A_ADDR "192.168.55.106"
#define NODE_A_PORT 8080

#define NODE_B_ID 91
#define NODE_B_ADDR "192.168.55.114"
#define NODE_B_PORT 8080

#define NODE_C_ID 92
#define NODE_C_ADDR "192.168.55.111"
#define NODE_C_PORT 8080

#define NODE_D_ID 93
#define NODE_D_ADDR "127.0.0.1"
#define NODE_D_PORT 8080

#define NODE_ID NODE_B_ID
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
int flag_found = 0;

char quit_standby = 0;

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

    connect(connectSocket, (const struct sockaddr *)&server, sizeof(server));
    send(connectSocket, buffer, buffsize, 0);

    int out_of_time = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (recv(connectSocket, buffer, buffsize, 0) == SOCKET_ERROR) {
        auto interval = std::chrono::high_resolution_clock::now() -  start;
        if (interval >= std::chrono::seconds(3)) {
            out_of_time = 1;
            break;
        }
    }
    if (!out_of_time) {
        frame_t data_recv = read_buffer(buffer);

        if (data_recv.function == FUNC_FOUND) {
            printf("Found.\n");
            *flag = 1;
        }
    }

}

// =================================================== Thread Function ====================================================
void p1() {
    std::condition_variable cv;
    std::mutex mt;
    std::unique_lock<std::mutex> lck(mt);

    frame_t data_input;
    data_input.source = NODE_ID;

    int temp;

    while(1) {
        printf("Select function:\n(1) SEND\n(2) SHUTDOWN\n(3) STANDBY\n");
        scanf("%d", &temp);
        if (temp == 1) {
            data_input.function = FUNC_SEND;

            printf("Enter a number: ");
            scanf("%d", &(data_input.buffer));

            printf("Select destination:\n(1) B\n(2) C \n(3) D\n");
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

                    create_buffer(data_find_route, buffer, buffsize);
                    send_to_node(hop[i], buffer, buffsize, 0);
                }
            }
        }
        if (temp == 2) {
            data_input.function = FUNC_SHDW;
        }
        if (temp == 3) {
            cv.wait(lck, [](){
                while (quit_standby != 'q') {
                    scanf("%c", &quit_standby);
                }
                quit_standby = 0;
                return true;
            });
        }
    }
}

void p3() {
    struct sockaddr_in server;
    
    SOCKET listenSocket, clientSocket;

    int buffsize = 4;
    char buffer[buffsize];

    int flag_found = 0;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(NODE_A_ADDR);
    server.sin_port = htons(NODE_A_PORT);

    bind(listenSocket, (struct sockaddr *)&server, sizeof(server));
    listen(listenSocket, SOMAXCONN);

    while(1) {
        if (recv(clientSocket, buffer, buffsize, 0) == SOCKET_ERROR) {
            continue;
        }
        else {
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
                }
            }
            else {
                for (int i=0; i<HOP_SIZE; i++) {
                    if (data_recv.source == hop[i].id) {
                        continue;
                    }

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
                        
                        break;
                    }
                }
            }
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