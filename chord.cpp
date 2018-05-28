#include <chord.hpp>

Local::Local(Address local_address, Address remote_address = NULL){
    this->addr = local_address;
    std::cout << "Self id " << this->id();
    this->shutdown = false;
    //this->join(remote_address);
}

size_t Local::id(int offset = 0){
    return (this->addr.Hash() + offset) % SIZE;
}

bool is_ours(std::size_t id){
    assert(id >= 0 && id < SIZE);
    return inrange(id, )
}

