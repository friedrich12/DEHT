#include <network.hpp>

std::string read_from_socket(int socket){
   /* int bytesRead = 0;
    int result;
    while(bytesRead < x){
        result = read(socket, buffer + bytesRead, x - bytesRead);
        if(result < 1){
            std::cout << "Error" << std::endl;
        }
        bytesRead += result;
    }*/
    char buffer[4096];
    read(socket, buffer, 4096);
    std::string ret(buffer, strlen(buffer));
    return ret;
}
void send_to_socket(int socket, std::string msg){
    send(socket, msg, strlen(msg),0);
}