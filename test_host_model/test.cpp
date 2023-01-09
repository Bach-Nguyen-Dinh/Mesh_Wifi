#include <string>
#include <iostream>

#define LEN_FUNC 20
#define LEN_BUFF 255
#define LEN_NAME 1
#define LEN_FRAME (LEN_FUNC + LEN_BUFF + 3*LEN_NAME)
#define LEN_ROUTE 10

#define NODE_ID "A"

#define FUNC_SEND "send"
#define FUNC_RECV "receive"
#define FUNC_TNFR "transfer"
#define FUNC_SHDW "shutdown"

using namespace std;

typedef struct FRAME{
    string function;
    string buffer;
    string source;
    string destination;
    string frame;
    int size_route;
    string route;
} frame_t;

int main() {
    frame_t data_input;
    frame_t data_output;
    
    data_input.source = NODE_ID;

    cout << "Select function [send / shutdown]: ";
    cin >> data_input.function;
        
    if (data_input.function == FUNC_SEND) {
        cout << "Enter message: ";
        cin >> data_input.buffer;

        cout << "Select destination [B / C / D]: ";
        cin >> data_input.destination;
    }

    string frame = data_input.function + "|" + data_input.buffer + "|" + data_input.destination + "#";
    cout << frame << endl;

    int i=0;
    int seg_num=0;
    while(frame[i] != '#') {
        if (frame[i] == '|') {
            seg_num++;
        }
        else {
            if (seg_num == 0) {
                data_output.function += frame[i];
            }

            if (seg_num == 1) {
                data_output.buffer += frame[i];
            }

            if (seg_num == 2) {
                data_output.destination += frame[i];
            }
        }
        i++;
    }

    cout << "Data entered: " << data_output.function << " | " << data_output.buffer << " | " << data_output.destination << " # ";

    // create_frame(&data_input);
    
    // init_frame(&data_input);

    // printf("%s\n", data_input.function);
    // strcpy(data_input.function, FUNC_SEND);
    // printf("%s, size = %d\n", data_input.function, sizeof(data_input.function));
    // printf("%c\n", data_input.function[18]);

    // printf("%s\n", data_input.buffer);
    // strcpy(data_input.buffer, "HELLO");
    // printf("%s, size = %d\n", data_input.buffer, sizeof(data_input.buffer));
    // printf("%c\n", data_input.buffer[253]);

    // printf("Frame initialized: \n%s", data_input.frame);
    // printf("_END\n");

    // for(int i=0; i<20; i++) {
    //     data_input.frame[i] = data_input.function[i];
    // }
    // for(int i=0; i<255; i++) {
    //     data_input.frame[20+i] = data_input.buffer[i];
    // }

    // printf("Frame created: \n%s", data_input.frame);
    // printf("_END\n");


    // char test_frame[LEN_FRAME];

    // init_frame(test_frame);

    // char seg[] = "BachNguyen";
    // int pos = 0;
    // insert_to_arr(test_frame, seg, pos, sizeof(seg));

    // char next_seg[] = "VietNam";
    // pos += sizeof(seg)-1;
    // insert_to_arr(test_frame, next_seg, pos, sizeof(next_seg));

    // printf("%s", test_frame);
    // printf("_END\n");

    return 0;
}