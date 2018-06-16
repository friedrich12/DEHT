#include <remote.hpp>

Remote::Remote(Address remoteAddress) noexcept{
    this->address = remoteAddress;
}

void Remote::open_connection(){
    this->socket = socket(AF_INET, SOCK_STREAM, 0);
    if(this->socket == 0){
        perror("Failed to create socket\n");
    }
    struct sockaddr_in serv_addr;

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->address.data.port);

    if(inet_pton(AF_INET,this->address.data.ip, &serv_addr.sin_addr) <= 0){
        perror("Invalid address\n");
    }

    if(connect(this->socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <= 0){
        perror("Connection failed\n");
    }
    this->connected = true;
}

void Remote::close_connection(){
    close(this->socket);
    this->socket = NULL;
    this->connected = false;
}

std::string Remote::RemoteAddrStr(){
    std::string r = "Remote " + this->address.ToString();
    return r;
}

std::size_t Remote::id(std::size_t offset = 0){
    return(this->address.Hash() + offset) % SIZE;
}

void Remote::send(std::string msg){
    send(this->socket, msg, strlen(msg.c_str()), 0);
    this->lastMessageSent = msg;
}

std::string Remote::recv(){
    return read_from_socket(this->socket);
}

bool Remote::ping(){
    try
    {
        int p = socket(AF_INET, SOCK_STREAM, 0);
        if(p == 0){
            throw "Failed to create socket";
        }
        struct sockaddr_in serv_addr;

        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(this->address.data.port);

        if(inet_pton(AF_INET,this->address.data.ip, &serv_addr.sin_addr) <= 0){
            throw "invalid address";
        }

        if(connect(p, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <= 0){
            throw "connection failed";
        }

        //carriage return-newline pair is both needed for newline in a network virtual terminal session.
        send(p, "\r\n" , strlen("\r\n") , 0);
        return true;

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

std::string Remote::command(std::string msg){
    if(this->connected){
        this->send(msg);
        std::string response = this->recv();
        return response;
    }
    return "";
}

std::vector<Remote> Remote::get_successors(){
    std::vector<Remote> myvec;
    if(this->connected){
        this->send("get_successors");
        std::string response = this->recv();

        if(response == ""){
            return myvec;
        }
    
        auto j = json::parse(response);
        
        for(json::iterator it = j.begin(); it != j.end(); ++it){
            json tmp = *it;
            Data d = tmp;
            Address addr(d.ip, d.port);
            Remote r(addr);
            myvec.push_back(r);
        }
        return myvec;
    }
    return myvec;
}

Remote Remote::successor(){
    Address d("",NULL);
    if(this->connected){
        this->send("get_successor");

        auto j = json::parse(this->recv());
        Data d = j;
        return Remote(Address(d.ip,d.port));
    }
    return Remote(d);
}

Remote Remote::predecessor(){
    Address d("",NULL);
    if(this->connected){
        this->send("get_predecessor");

        std::string reponse = this->recv();
        if(reponse == ""){
            return d;
        }
        auto j = json::parse(reponse);
        Data d = j;
        return Remote(Address(d.ip,d.port));
    }
    return Remote(d);
}

Remote Remote::find_successor(std::size_t id){
    Address d("", NULL);
    if(this->connected){
        this->send("find_successor " + id);
        auto j = json::parse(this->recv());
        Data d = j;
        return Remote(Address(d.ip,d.port));
    }
    return Remote(d);
}

Remote Remote::closest_preceding_finger(std::size_t id){
    Address d("", NULL);
    if(this->connected){
        this->send("closest_preceding_finger " + id);

        std::string response = this->recv();
        auto j = json::parse(response);
        Data d = j;
        return Remote(Address(d.ip,d.port));
    }
    return Remote(d);
}

void Remote::notify(Remote node){
    this->send("notify " + node.address.data.ip + " " + std::to_string(node.address.data.port));
}