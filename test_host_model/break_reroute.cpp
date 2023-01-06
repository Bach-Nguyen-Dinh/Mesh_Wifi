/* =============================================== Demo Scenario =====================================================
    normaly, A is able to connect and communicate with C
    when not, A must find a new route to C and save this route
    later, A sends message to C throught the newly founded route
*/

/* ============================================== Quy trình xử lí =====================================================
    A muốn gửi đến D nhưng không kết nối được

    A hỏi những node xung quanh xem node nào có thể kết nối được đến D hay không
    (giả sử A đã biết trước A có thế kết nối đến được những node nào)
    (khi hỏi mỗi node sẽ có thời gian timeout để chuyển sang hỏi node khác nếu node đó quá lâu không phản hồi)

    A đợi hỏi hết rồi mới xử lí gửi đi

    A đợi E phản hồi rồi mới chuyển sang hỏi B
    A hỏi E có thể kết nối đến D hay không
        E nhận được yêu cầu từ A sẽ
            kiểm tra có node nào xung quanh không (trừ A)
            xung quanh E không có node nào
        E phản hồi lại cho A là không kết nối đến D được

    A hỏi B có thể kết nối đến D hay không
        B nhận được yêu cầu từ A sẽ
            kiểm tra có node nào xung quanh không (trừ A)
            hỏi xung quanh xem có D hay không (trừ A)
            xung quanh B không có D nên B sẽ hỏi xung quanh xem có node nào có thể kết nối được đến D hay không
            
            B đợi C phản hồi rồi mới phản hồi lại cho A
            B hỏi C có thế kết nối đến D hay không
                C nhận được yêu cầu từ B sẽ
                    kiểm tra có node nào xung quanh không (trừ B)
                    hỏi xung quanh xem có D hay không (trừ B)
                C phản hồi lại cho B là có thể kết nối đến D được

        B phản hồi lại cho A là có thể kết nối đến D được
    
    Mỗi node phản hồi sẽ lưu đường đi vào bản tin phản hồi của nó, qua mỗi lần phản hồi sẽ bổ sung đường đi 
    vào bản tin phản hồi

    A lưu đường đi vừa tìm được vào bản tin (ngoài nội dung bản tin và các thông tin khác)
    A gửi bản tin đi dựa vào thông tin đường đi có trong bản tin cần gửi
*/
#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <chrono>

#pragma commnet(lib, "ws2_32.lib")

#define LEN_FUNC 20
#define LEN_BUFF 255
#define LEN_NAME 1
#define LEN_FRAME (LEN_FUNC + LEN_BUFF + 3*LEN_NAME)

#define HOP_SIZE 2
#define NODE_ID "A"

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
    char function[LEN_FUNC];
    char buffer[LEN_BUFF];
    char source[LEN_NAME];
    char destination[LEN_NAME];
    char previous[LEN_NAME];
    char frame[LEN_FRAME];
} frame_t;

typedef struct HOP_LIST{
    const char *name;
    const char *ip_addr;
    int port;
} hop_list_t;
