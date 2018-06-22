#include <address.hpp>


bool inrange(std::size_t c, std::size_t a, std::size_t b){
    a = a % SIZE;
    b = b % SIZE;
    c = c % SIZE;
    if(a < b){return(a <= c && c < b);}
    return (a <= c || c < b);
}

Address::Address(std::string ip, int port){
    this->data.ip = ip;
    this->data.port = port;
} 

size_t Address::Hash(){
    auto port = std::to_string(this->data.port);
    std::size_t h1 = std::hash<std::string>{}(this->data.ip);
    std::size_t h2 = std::hash<std::string>{}(port);
    auto hash = h1 ^ (h2 << h1);
    return hash % SIZE;
}

Data Address::getData(){
    return this->data;
}

std::string Address::ToString(){
    return (this->data.ip) + ":" + std::to_string(this->data.port);
}