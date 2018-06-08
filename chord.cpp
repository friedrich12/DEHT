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

void Local::notify(Remote r){
    // Someone will be out predecessor iff
    // we don't have one or
    // the new node r is in the range of (pred(n), n)
    // out previous predecessor is dead
    if((this->predecessor.RemoteAddrStr() == "") ||
        (inrange(r.id(),this->predecessor().id(1), this->id())) || (!this->predecessor().ping())){
        this->predecessor = r;
    }
}

bool Local::fix_fingers(){
    // Randomly selected entries and update its value
    this->log("fix_fingers");
    vector<Remote>::iterator i = 
        *select_randomly(this->finger.begin(),this->finger.end());
    
    auto x = std::distance(foo.begin(), i);
    this->finger[x] = this->find_successor(this-id(1 << x));

    return true;
}

bool Local::update_successors(){
    this->log("update successor");
    Remote suc = this->successor();
    vector<Remote> sucs;
    vector<Remote> suc_list;
    // Do this if we are not alone
    if(suc.id() != this->id()){
        sucs.push_back(suc);
        suc_list = suc.get_successors();
        if(!suc_list.empty()){
            sucs.push_back(suc_list);
        }
        this->successors = sucs;
    }
    return true;
}

json Local::get_successors(){
    this->log("get_successors");
    json enc;
    vector<Remote> remote_nodes = this->successors;
    for(auto const& node : remote_nodes){
        Address a = node.address;
        Data d = a.data;
        json tmp = d;
        enc.push_back(tmp);
    }
    return enc;
}

inline std::size_t Local::id(int offset = 0){
    return(this->address.Hash() + offset) % SIZE;
}

Remote Local::closest_preceding_finger(std::size_t id){
    this->log("closest_preceding_finger");
    vector<Remote> data = this->finger;
    data.insert(data.end(), this->successors.begin(), this->successors.end());
    
    for(auto const& node : data){
        if((node.address.data.ip != "") && inrange(node.id(), this->id(1), id) 
            && (node.ping())){
            return node;
        }
    }
    return Remote(this->address);
}

Remote Local::predecessor(){
    return this->predecessor;
}

Remote Local::find_predessor(std::size_t id){
    this->log("find_predecessor");
    Remote node(this->address);

    if(node.successor().id() == node.id()){
        // We are alone in the ring
        return node;
    }
    while(!inrange(id, node.id(1), node.successor().id(1))){
        node =closest_preceding_finger(id);
    }
    return node;
}

Remote Local::find_successor(std::size_t id){
    this->log("find_successor")
    if(this->predecessor() && 
        inrange(id,this->predecessor().id(1), this->id(1))){
        return Remote(this->address);
    }
}