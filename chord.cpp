#include <chord.hpp>

Local::Local(Address local_address, Address remote_address = NULL) noexcept{
    this->address = local_address;
    std::cout << "Self id " << this->id();
    this->shutdown = false;
    this->join(remote_address);
}

inline size_t Local::id(int offset = 0) noexcept{
    return (this->addr.Hash() + offset) % SIZE;
}

bool Local::is_ours(std::size_t id) noexcept{
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
    std::thread h(this->run);
    std::thread h1(this->fix_fingers);
    std::thread h2(this->stabilize);
    std::thread h3(this->update_successors);

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
        Address a = std::move(node.address);
        Data d = std::move(a.data);
        json tmp = std::move(d);
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

void Local::run(){
    this->socket = socket(AF_INET, SOCK_STREAM, 0);
    if(this->socket == 0){
        std::cerr << "Failed to create socket" << endl;
    }

    struct sockaddr_in serv_addr;
    int len = sizeof(serv_addr);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->address.data.port);

    if(bind(this->socket, (struct sockaddr *) &serv_addr, sizeof(address)) < 0){
        std::cerr << "Failed to bind" << endl;
    }

    listen(this->socket, 3);

    for(;;){
        int new_socket = accept(this->socket, (struct sockaddr*) &serv_addr,(socklen_t*) &len);
        if(new_socket < 0){
            std::cerr << "Failed to accept socket" << endl;
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

        auto command = mymap[vec[0]];
        std::size_t node = std::stoi(vec[1]);

        switch(command){
            // TODO: Finish This
            case 0:
                Remote successor = this->successor();
                json j = successor;
                result = j.dump();
                break;
            case 1:
                if(this->predecessor.connected != false){
                    Remote predecessor = this->predecessor;
                    json j = predecessor;
                    result = j.dump();
                }
                break;
            case 2:
                Remote successor = this->find_successor(node);
                json j = successor;
                result = j.dump();
                break;
            case 3:
                Remote closest_finger = this->closest_preceding_finger(node);
                json j = closest_finger;
                result = j.dump();
            case 4:
                std::string ip = vec[1];
                std::string port = vec[2];
                Address a(ip, std::stoi(port));
                Remote r(a);
                this->notify(r);
                break;
            case 5:
                json j = this->get_successors();
                result = j.dump();
                break;
        }

        send(new_socket, result , strlen(hello) , 0);

        close(new_socket);
    }
}