/* ==================================================== Demo Scenario =====================================================
    normaly, A is able to connect and communicate with C
    when not, A must find a new route to C and save this route
    later, A sends message to C throught the newly founded route
*/

/* ================================================= Operating Procedure ==================================================
    A muốn gửi đến D nhưng không kết nối được

    A hỏi những node xung quanh xem node nào có thể kết nối được đến D hay không
    (giả sử A đã biết trước A có thế kết nối đến được những node nào)
    (khi hỏi mỗi node sẽ có thời gian timeout để chuyển sang hỏi node khác nếu node đó quá lâu không phản hồi)

    A đợi hỏi hết rồi mới xử lí gửi đi

    A đợi E phản hồi rồi mới chuyển sang hỏi B
    A hỏi E có thể kết nối đến D hay không
        E nhận được yêu cầu từ A sẽ
            kiểm tra có node nào xung quanh không (không tính A)
            xung quanh E không có node nào
        E phản hồi lại cho A là không kết nối đến D được

    A hỏi B có thể kết nối đến D hay không
        B nhận được yêu cầu từ A sẽ
            kiểm tra có node nào xung quanh không (không tính A)
            hỏi xung quanh xem có D hay không (không hỏi A)
            xung quanh B không có D nên B sẽ hỏi xung quanh xem có node nào có thể kết nối được đến D hay không
            (không hỏi A)
            
            B đợi C phản hồi rồi mới phản hồi lại cho A

            B hỏi C có thế kết nối đến D hay không
                C nhận được yêu cầu từ B sẽ
                    kiểm tra có node nào xung quanh không (không tính B)
                    hỏi xung quanh xem có D hay không (không hỏi B)
                C phản hồi lại cho B là có thể kết nối đến D được

        B phản hồi lại cho A là có thể kết nối đến D được
    
    Mỗi node phản hồi sẽ lưu đường đi vào bản tin phản hồi của nó, qua mỗi lần phản hồi sẽ bổ sung đường đi 
    vào bản tin phản hồi

    A lưu đường đi vừa tìm được vào bản tin gửi đi (ngoài nội dung bản tin và các thông tin khác)
    A gửi bản tin đi dựa vào thông tin đường đi có trong bản tin cần gửi
*/

/* ============================================== Frame Structure =====================================================
    - function
    - buffer
    - source
    - destination
    - route_size
    - route
*/
#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

#pragma commnet(lib, "ws2_32.lib")

#define LEN_FUNC 20
#define LEN_BUFF 255
#define LEN_NAME 1
#define LEN_FRAME (LEN_FUNC + LEN_BUFF + 3*LEN_NAME)
#define LEN_ROUTE 10

#define HOP_SIZE 2
#define NODE_ID "A"

#define FUNC_SEND "send"
#define FUNC_RECV "receive"
#define FUNC_TNFR "transfer"
#define FUNC_SHDW "shutdown"
#define FUNC_FIND "find"

#define HOST_A_ADDR "127.0.0.1"
#define HOST_A_PORT 8080

#define HOST_B_ADDR "127.0.0.1"
#define HOST_B_PORT 8081

#define HOST_D_ADDR "127.0.0.1"
#define HOST_D_PORT 8082

// =================================================== Define Structure ==================================================
typedef struct FRAME{
    std::string function;
    std::string buffer;
    std::string source;
    std::string destination;
    std::string frame;
    int size_route;
    std::string route;
} frame_t;

typedef struct HOP_LIST{
    std::string name;
    const char *ip_addr;
    int port;
} hop_list_t;

// =================================================== Global Variable ====================================================
WSADATA wsaDATA;

hop_list_t hop[HOP_SIZE];

frame_t data_input;

int flag_check_hop = 0;
int flag_in_hop = 0;
int flag_recv = 0;

// =================================================== Define Function ====================================================
void create_hop() {
    hop[0].name = "B";
    hop[0].ip_addr = HOST_B_ADDR;
    hop[0].port = HOST_B_PORT;

    hop[1].name = "D";
    hop[1].ip_addr = HOST_D_ADDR;
    hop[1].port = HOST_D_PORT;
}

void create_frame(frame_t *data) {
    (*data).frame = (*data).function + "|" + (*data).buffer + "|" + (*data).source + "|" + (*data).destination + "#";
}

frame_t read_frame(char frame[]) {
    frame_t data;

    int i = 0;
    int seg_num = 0;

    while(frame[i] != '#') {
        if (frame[i] == '|') {
            seg_num++;
        }
        else {
            if (seg_num == 0) {
                data.function += frame[i];
            }

            if (seg_num == 1) {
                data.buffer += frame[i];
            }

            if (seg_num == 2) {
                data.source += frame[i];
            }

            if (seg_num == 3) {
                data.destination += frame[i];
            }
        }
        i++;
    }
    return data;
}

int find_route(std::string source, std::string destination) {
    struct sockaddr_in server;
    SOCKET ConnectSocket;

    int sizebuff = 100;
    char recvbuff[sizebuff];

	if ((ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		std::cout << "Could not create socket. Error code: " << WSAGetLastError() << std::endl;
        exit(1);
	}
        
    for (int i=0; i<HOP_SIZE; i++) {
        if (source == hop[i].name) {
            continue;
        }

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(hop[i].ip_addr);
        server.sin_port = htons(hop[i].port);
    
        if (connect(ConnectSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
            continue;
        }
        else {
            std::string func = FUNC_FIND;
            std::string buff = FUNC_FIND;
            std::string src = source;
            std::string dst = destination;
            std::string frame = func + "|" + buff + "|" + src + "|" + dst + "#";

            const int length = frame.length();
        
            // declaring character array (+1 for null terminator)
            char *char_array = new char[length + 1];
            
            // copying the contents of the
            // string to char array
            strcpy(char_array, frame.c_str());
            
            send(ConnectSocket, char_array, sizeof(char_array), 0);
            delete[] char_array;

            
            auto start = std::chrono::high_resolution_clock::now();
            while(recv(ConnectSocket, recvbuff, sizebuff, 0) < 0) {
                auto stop = std::chrono::high_resolution_clock::now();
                if ((stop - start) >= std::chrono::seconds(5)) {
                    break;
                }
            }

            if (recvbuff) {
                std::cout << hop[i].name << " found " << destination << std::endl;
                return 1;
            }
        }
    }
    return 0;
}

// =================================================== Thread Function ====================================================
void p1() {
    data_input.source = NODE_ID;

    while(1) {
        std::cout << "Select function [send / shutdown]: ";
        std::cin >> data_input.function;
            
        if (data_input.function == FUNC_SEND) {
            std::cout << "Enter message: ";
            std::cin >> data_input.buffer;

            std::cout << "Select destination [B / C / D]: ";
            std::cin >> data_input.destination;
        }

        flag_check_hop = 1;
        while(flag_recv == 0) {};
    }
}

void p2() {
    while(1) {
        if (flag_check_hop) {
            for (int i=0; i<HOP_SIZE; i++) {
                if (data_input.destination == hop[i].name) {
                    flag_in_hop = 1;
                    break;
                }
            }
            flag_check_hop = 0;
        }
        if (flag_in_hop) {
            std::cout << "Found." << std::endl;
            flag_in_hop = 0;
        }
        else {
            find_route(data_input.source, data_input.destination);
        }
    }
}

void p3() {
    struct sockaddr_in server;
    
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        WSACleanup();
        exit(1);
	}

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(HOST_A_ADDR);
    server.sin_port = htons(HOST_A_PORT);

    if (bind(ListenSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    if ((listen(ListenSocket, SOMAXCONN))== SOCKET_ERROR) {
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    while(1) {
        frame_t data_recv;
        int sizebuff = 500;
        char recvbuff[sizebuff];

        if (recv(ClientSocket, recvbuff, sizebuff, 0) == SOCKET_ERROR) {
            continue;
        }
        else {
            data_recv = read_frame(recvbuff);

            if (data_recv.destination == NODE_ID) {
                if (data_recv.function == FUNC_FIND) {
                    frame_t data_rep;

                    data_rep.function = FUNC_FIND;
                    data_rep.buffer = data_recv.buffer;
                    data_rep.source = NODE_ID;
                    data_rep.destination = data_recv.source;

                    create_frame(&data_rep);
                    const int length = data_rep.frame.length();
                
                    // declaring character array (+1 for null terminator)
                    char *char_array = new char[length + 1];
                    
                    // copying the contents of the
                    // string to char array
                    strcpy(char_array, data_rep.frame.c_str());

                    send(ClientSocket, char_array, sizeof(char_array), 0);
                    delete[] char_array;
                }
            }
            else {
                find_route(NODE_ID, data_recv.destination);
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
    std::thread t2 = std::thread(p2);
    std::thread t3 = std::thread(p3);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}