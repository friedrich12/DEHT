#include <chord.hpp>

using namespace deht;

deht::Local::Local(Address local_address, Address remote_address){
    this->address = local_address;
    std::cout << "Self id " << this->id();
    this->shutdown = false;
    this->join(remote_address);
}

inline size_t deht::Local::id(int offset) noexcept{
    return (this->address.Hash() + offset) % SIZE;
}

bool deht::Local::is_ours(std::size_t id) noexcept{
    assert(id >= 0 && id < SIZE);
    return inrange(id, this->predecessor.id(1), this->id(1));
}

void deht::Local::shutdownConnection(){
    this->shutdown = true;
    close(this->socket);
}

void deht::Local::log(std::string info){
    using namespace std;
    ofstream out;
    out.open("/tmp/chord.log");
    if(out.fail()){
        cout << "Failed to open file" << endl;
    }
    out << this->id() << " : " << info << endl;
    out.close();
}

void deht::Local::start(){
    // Start threads
    std::thread h(&deht::Local::run, this);
    std::thread h1(&deht::Local::fix_fingers, this);
    std::thread h2(&deht::Local::stabilize, this);
    std::thread h3(&deht::Local::update_successors, this);

    this->daemons["run"] = std::ref(h);
    this->daemons["fix_fingers"] = std::ref(h1);
    this->daemons["stabilize"] = std::ref(h2);
    this->daemons["update_successors"] = std::ref(h3);

    for(auto thread : this->daemons){
        thread.second.get().join();
    }

    this->log("Started Protocol");
}

bool deht::Local::ping(){
    return true;
}

Remote deht::Local::successor(){

    std::for_each(this->finger.begin(), this->finger.end(),
        [&](Remote rnode)
        {
            if(rnode.ping()){
                this->finger[0] = rnode;
                return rnode;
            }
        }
    );
    // Lets try this one
    std::for_each(this->successors.begin(), this->successors.end(),
        [&](Remote rnode)
        {
            if(rnode.ping()){
                this->finger[0] = rnode;
                return rnode;
            }
        }
    );

    std::cerr << "No successor found, terminating";
    this->shutdown = true;
    exit(1);
}

void deht::Local::join(Address remote_address){
    //this->predecessor
    // My way of telling if its empty
    if(remote_address.data.ip != ""){
        Remote remote(remote_address);
        this->finger[0] = remote.find_successor(this->id()); 
    }else{
        Remote self(this->address);
        this->finger[0] = self;
    }

    this->log("Node Joined");
}

bool deht::Local::stabilize(){
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
    Address tmp = this->address;
    Remote notify(tmp);
    this->successor().notify(notify);

    return true;
}

void deht::Local::notify(Remote r){
    // Someone will be out predecessor iff
    // we don't have one or
    // the new node r is in the range of (pred(n), n)
    // out previous predecessor is dead
    if((this->predecessor.RemoteAddrStr() == "") ||
        (inrange(r.id(),this->getPredecessor().id(1), this->id())) || (!this->getPredecessor().ping())){
        this->predecessor = r;
    }
}

bool deht::Local::fix_fingers(){
    // Randomly selected entries and update its value
    this->log("fix_fingers");
    std::vector<Remote>::iterator i = 
        select_randomly(this->finger.begin(),this->finger.end());
    
    auto x = std::distance(this->finger.begin(), i);
    // AKA 1 * 2^x just a performance upgrade
    int offset = 1 << x;
    this->finger[x] = this->find_successor(this->id(offset));

    return true;
}

bool deht::Local::update_successors(){
    this->log("update successor");
    Remote suc = this->successor();
    std::vector<Remote> sucs;
    std::vector<Remote> suc_list;
    // Do this if we are not alone
    if(suc.id() != this->id()){
        sucs.push_back(suc);
        suc_list = suc.get_successors();
        if(!suc_list.empty()){
            sucs.insert(std::end(suc_list),std::begin(suc_list), std::end(suc_list));
        }
        this->successors = sucs;
    }
    return true;
}

json deht::Local::get_successors(){
    this->log("get_successors");
    json enc;
    std::vector<Remote> remote_nodes = this->successors;
    for(auto const& node : remote_nodes){
        // TODO Fix with new to_json serializer
        Address a = std::move(node.address);
        Data d = std::move(a.data);
        json tmp = std::move(d);
        enc.push_back(tmp);
    }
    return enc;
}

Remote deht::Local::closest_preceding_finger(std::size_t id){
    this->log("closest_preceding_finger");
    std::vector<Remote> data = this->finger;
    data.insert(std::end(data), std::begin(this->successors), std::end(this->successors));
    
      std::for_each(data.begin(), data.end(),
        [&](Remote node)
        {
            if((node.address.data.ip != "") && inrange(node.id(), this->id(1), id) 
                && (node.ping())){
                return node;
            }
        }
    );
    Remote self(this->address);
    return self;
}

Remote deht::Local::getPredecessor(){
    return this->predecessor;
}

Remote deht::Local::find_predessor(std::size_t id){
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

Remote deht::Local::find_successor(std::size_t id){
    this->log("find_successor");
    /*
        We are the successor if both statements hold true
        1.If we don't have a predecessor
        2.The id is in our range
    */
    if((this->getPredecessor().address.data.ip != "") && 
        inrange(id,this->getPredecessor().id(1), this->id(1))){
        Remote r(this->address);
        return r;
    }
    Remote node = this->find_predessor(id);
    return node.successor();
}

void deht::Local::run(){
    this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if(this->socket == 0){
        std::cerr << "Failed to create socket" << std::endl;
    }

    struct sockaddr_in serv_addr;
    int len = sizeof(serv_addr);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->address.data.port);

    if(bind(this->socket, (struct sockaddr *) &serv_addr, sizeof(address)) < 0){
        std::cerr << "Failed to bind" << std::endl;
    }

    // Allow 3 pending connections
    listen(this->socket, 3);

    for(;;){
        int new_socket = accept(this->socket, (struct sockaddr*) &serv_addr,(socklen_t*) &len);
        if(new_socket < 0){
            std::cerr << "Failed to accept socket" << std::endl;
        }

        std::string request = read_from_socket(new_socket);

        // Parse commands
        std::vector<std::string> vec;
        std::stringstream parse(request);
        std::string p;
        while(std::getline(parse, p, ' ')){
            vec.push_back(p);
        }
        
        std::map<std::string, int> mymap;
        mymap["get_successor"] = 0;
        mymap["get_predecessor"] = 1;
        mymap["find_successor"] = 2;
        mymap["closest_preceding_finger"] = 3;
        mymap["notify"] = 4;
        mymap["get_successors"] = 5;

        std::string result = "";
        json j;

        auto command = mymap[vec[0]];
        std::size_t node = std::stoi(vec[1]);

        switch(command){
            // TODO: Finish This
            case 0:{
                Remote successor = this->successor();
                j = successor;
                result = j.dump();
            }
                break;
            case 1:{
                if(this->predecessor.connected != false){
                    Remote predecessor = this->predecessor;
                    j = predecessor;
                    result = j.dump();
                }
            }
                break;
            case 2:{
                Remote successor = this->find_successor(node);
                j = successor;
                result = j.dump();
            }
                break;
            case 3:{
                Remote closest_finger = this->closest_preceding_finger(node);
                j = closest_finger;
                result = j.dump();
            }
                break;
            case 4:{
                std::string ip = vec[1];
                std::string port = vec[2];
                Address a(ip, std::stoi(port));
                Remote r(a);
                this->notify(r);
            }
                break;
            case 5:{
                j = this->get_successors();
                result = j.dump();
            }
                break;
        }

        send(new_socket, result.c_str() , strlen(result.c_str()) , 0);
        
        // Close the socket
        close(new_socket);
    }
}