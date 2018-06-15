#include <deht.hpp>

DEHT::DEHT(Address local_address, Address remote_address){
    this->local = Local(local_address, remote_address);
    this->shutdown = false;

    std::thread h(this->distribute_data);
    this->daemons["distribute_data"] = h;

    this->daemons["distribute_data"].join();
    this->local.start();
};

void DEHT::shutdown(){
    this->lcoal.shutdown();
    this->shutdown = true;
}

std::string DEHT::_get(std::string request){
    return this->data[request];
}

std::string DEHT::_set(std::string key, std::string value){
    this->data[key] = value;
}

std::string DEHT::get(std::string key){
    try
    {
        return this->data[key];
    }
    catch(const std::exception& e)
    {
        // We don't have it
        Remote suc = this->local.find_successor(std::hash<std::string>{}(key));
        if(this->local.id() == suc.id()){
            // I said we don't have it already!
            return "";
        }
        
        try
        {
            Remote response = suc.command("get " + key);
            return resposne;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            reaturn "";
        }
        
    }
    return "";
}

void DEHT::set(std::string key, std::string value){
    this->data[key] = value;
}

bool DEHT::distribute_data(){
    this->removeKeys.clear();
    
    for(auto const& key : this->removeKeys){
        if(this->local.predecessor().data.ip && !inrange(std::hash<std::string>{}(key),
            this->local.predecessor().id(1), this->local.id(1))){
           try
           {
               // Edit the parsing later
               Remote node = this->local.find_successor(hash<std::string>{}(key));
               node.command("set " + "key " + key + "value " + this->data[key]);
               this->removeKeys.push_back(key);
           }
           catch(const std::exception& e)
           {
               std::cerr << e.what() << '\n';
               continue;
           }
                    
        }
    }
    std::map<std::string,std::string>::iterator it;
    for(auto const&key : this->removeKeys){
        it = this->data.find(key);
        this->data->erase(it);
    }
    return true;
}

/*void DEHT::create_dht(std::string addr, std::string port){
    Address laddress(addr, port);
    std::vector<DEHT> r;

    //....
}*/

