#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <mutex>

#pragma commnet(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_FUNCLEN 20
#define DEFAULT_DESTLEN 10
#define DEFAULT_SRCLEN 10
#define frame_size (DEFAULT_FUNCLEN + DEFAULT_BUFLEN + DEFAULT_SRCLEN + DEFAULT_DESTLEN)

#define HOP_SIZE 2
#define SOURCE "A"

#define host_A_addr "127.0.0.1"
#define host_A_port 8080

#define host_B_addr "127.0.0.1"
#define host_B_port 8080

#define host_D_addr "127.0.0.1"
#define host_D_port 8080

struct msg {
    char function[DEFAULT_FUNCLEN];
    char buffer[DEFAULT_BUFLEN];
    char source[DEFAULT_SRCLEN];
    char destination[DEFAULT_DESTLEN];
    int size = frame_size;
    char frame[frame_size];
};

struct hop_list {
    const char *name;
    const char *ip_addr;
    int port;
};

// void add_to_hop(struct hop *hop, char *ip_addr, int port) {
//     struct hop *temp;
//     temp->ip_addr = ip_addr;
//     temp->port = port;
//     hop->next_hop = temp;
// }

std::mutex mt;
struct hop_list hop[HOP_SIZE];

void create_hop() {
    hop[0].name = "B";
    hop[0].ip_addr = host_B_addr;
    hop[0].port = host_B_port;

    hop[1].name = "D";
    hop[1].ip_addr = host_D_addr;
    hop[1].port = host_D_port;
}

void insert_to_frame(char *frame, char *seg, int pos, int size) {
    for (int i=0; i<size; i++) {
        *(frame + pos + i) = *(seg + i);
    }
}


struct msg extract_from_frame(char *frame) {
    struct msg data;
    int pos = 0;

    for (int i=0; i<DEFAULT_FUNCLEN; i++) {
        data.function[i] = *(frame + pos + i);
    }
    pos += DEFAULT_FUNCLEN;

    for (int i=0; i<DEFAULT_BUFLEN; i++) {
        data.buffer[i] = *(frame + pos + i);
    }
    pos += DEFAULT_BUFLEN;

    for (int i=0; i<DEFAULT_SRCLEN; i++) {
        data.source[i] = *(frame + pos + i);
    }
    pos += DEFAULT_SRCLEN;

    for (int i=0; i<DEFAULT_DESTLEN; i++) {
        data.destination[i] = *(frame + pos + i);
    }

    return data;
}

void create_frame(struct msg *input_data) {
    int pos = 0;
    insert_to_frame(input_data->frame, input_data->function, pos, DEFAULT_FUNCLEN);
    pos += DEFAULT_FUNCLEN;
    insert_to_frame(input_data->frame, input_data->buffer, pos, DEFAULT_BUFLEN);
    pos += DEFAULT_BUFLEN;
    insert_to_frame(input_data->frame, input_data->source, pos, DEFAULT_SRCLEN);
    pos += DEFAULT_SRCLEN;
    insert_to_frame(input_data->frame, input_data->destination, pos, DEFAULT_DESTLEN);
}

void send_data(struct msg *data) {
    WSADATA wsaDATA;
    SOCKET ConnectSocket;
    struct sockaddr_in server;

    create_frame(data);

    printf("Sending data . . .\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}

	if ((ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
	}

    for(int i=0; i<HOP_SIZE; i++) {
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

    closesocket(ConnectSocket);
    WSACleanup();
}

void server_function() {
    mt.lock();
    WSADATA wsaDATA;

    SOCKET ListenSocket = INVALID_SOCKET;

    struct sockaddr_in server;
    struct msg data;
    
    char recvbuf[frame_size];

    printf("\t\t\t\t\t\tStarting server . . .\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0) {
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}

	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket. Error code : %d\n", WSAGetLastError());
        WSACleanup();
	}

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host_A_addr);
    server.sin_port = htons(host_A_port);

    if (bind(ListenSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error code : %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
    }

    if ((listen(ListenSocket, SOMAXCONN))== SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
    }

    printf("\t\t\t\t\t\tServer is running\n");

    mt.unlock();

    if (recv(ListenSocket, recvbuf, frame_size, 0) == SOCKET_ERROR) {
        printf("Receive failed. Error code: %d\n", WSAGetLastError());
    }
    else {
        printf("Message received.\n");
        data = extract_from_frame(recvbuf);

        if (strcmp(data.function, "send") == 0) {
            
        }
    }
}

// void get_input(struct data_t *input) {
//     char user_input[DEFAULT_BUFLEN];
//     printf("Select function [send]: ");
//     scanf("%s", user_input);
//     input->function = user_input;
    
//     if (strcmp(input->function, "send") == 0) {
//         printf("Enter message: ");
//         scanf("%s", user_input);
//         input->data = user_input;

//         printf("Select destination [B/C]: ");
//         scanf("%s", user_input);
//         input->destination = user_input;
//     }
// }

// void handle_input(struct data_t *input) {
//     if ((strcmp(input->function, "send")) == 0) {
//         send_data(input);
//     }
// }

void client_function() {
    struct msg data;

    strcpy(data.source, SOURCE);

    while(1) {
        mt.lock();
        printf("Select function [send / shut down]: ");
        fgets(data.function, sizeof(data.function), stdin);
        
        if (strcmp(data.function, "send\n") == 0) {
            printf("Enter message: ");
            scanf("%s", data.buffer);

            printf("Select destination [B / C / D]: ");
            scanf("%s", data.destination);

            mt.unlock();

            send_data(&data);
            continue;
        }

        if (strcmp(data.function, "shut down\n") == 0) {
            mt.unlock();

            for (int i=0; i<HOP_SIZE; i++) {
                strcpy(data.destination, hop[i].name);
                send_data(&data);
            }
        }
        mt.unlock();  
    }
}

int main() {
    create_hop();

    std::thread thread_server = std::thread(server_function);
    std::thread thread_client = std::thread(client_function);    

    thread_server.join();
    thread_client.join();

    return 0;
}