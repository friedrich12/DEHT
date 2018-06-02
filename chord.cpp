#include <chord.hpp>

Local::Local(Address local_address, Address remote_address = NULL){
    this->address = local_address;
    std::cout << "Self id " << this->id();
    this->shutdown = false;
    //this->join(remote_address);
}

inline size_t Local::id(int offset = 0){
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

    this->log("Started Protocol");
}

bool Local::ping(){
    return true;
}

Remote Local::successor(){
    for(auto const& remote : this->finger){
        if(remote.ping()){
            this->finger[0] = remote;
            return remote;
        }
    }
    // Lets try this one
    for(auto const& remote : this->successors){
        if(remote.ping()){
            this->finger[0] = remote;
            return remote;
        }
    }
    std::cerr << "No successor found, terminating";
    this->shutdown = true;
    exit(1);
}

void Local::join(Address remote_address){
    //this->predecessor
    // My way of telling if its empty
    if(remote_address.data.ip != ""){
        Remote remote = Remote(remote_address);
        this->finger[0] = remote.find_successor(this->id()); 
    }else{
        this->finger[0] = Remote(this->address);
    }

    this->log("Node Joined");
}

bool Local::stabilize(){
    this->log("stabilize");

    Remote suc = this->successor();
    if(suc.id() != this->finger[0].id()){
        this->finger[0] = suc;
    }
    Remote r = suc.predecessor();
    if ((r.address.data.ip != "") &&
        (inrange(r.id(), this->id(1), suc.id())) && (this->id(1) != suc.id()) && (r.ping())) {
            this->finger[0] = r;
    }

    // Notify the new sucsessor
    this->successor().notify(this);

    return true;
}