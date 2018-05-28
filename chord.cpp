#include <chord.hpp>

Local::Local(Address local_address, Address remote_address = NULL){
    this->address = local_address;
    std::cout << "Self id " << this->id();
    this->shutdown = false;
    //this->join(remote_address);
}

size_t Local::id(int offset = 0){
    return (this->addr.Hash() + offset) % SIZE;
}

bool Local::is_ours(std::size_t id){
    assert(id >= 0 && id < SIZE);
    return inrange(id, this->predecessor.id(1), this->id(1));
}

void Local::shutdown(){
    this->shutdown = true;
    close(this->socket);
}

void Local::log(std::string info){
    using namespace std;
    ofstream out;
    out.open("/tmp/chord.log");
    if(out.fail()){
        cout << "Failed to open file" << endl;
    }
    out << this->id() << " : " << info << endl;
    out.close();
}

void Local::start(){
    // Start threads
    std::thread h(Local::run);
    std::thread h1(Local::fix_fingers);
    std::thread h2(Local::stabilize);
    std::thread h3(Local::update_successors);

    this->daemons["run"] = h;
    this->daemons["fix_fingers"] = h1;
    this->daemons["stabilize"] = h2;
    this->daemons["update_successors"] = h3;

    for(auto const& thread : this->daemons){
        thread.join();
    }
}

bool Local::ping(){
    return true;
}

void Local::join(Address remote_address){
    //this->predecessor
    if(remote_address.data.ip == ""){
        
    }
}

