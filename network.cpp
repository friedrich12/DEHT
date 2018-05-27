#include <network.h>

void read_from_socket(int socket, unsigned int x, std::string* buffer){
    int bytesRead = 0;
    int result;
    while(bytesRead < x){
        result = read(socket, buffer + bytesRead, x - bytesRead);
        if(result < 1){
            std::cout << "Error" << std::endl;
        }
        bytesRead += result;
    }
}
void send_to_socket(int socket, std::string msg){
    send(socket, msg, strlen(msg),0);
}