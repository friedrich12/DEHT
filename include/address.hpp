#pragma once

#include <functional>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_set>
#include <utility>
#include <vars.hpp>

bool inrange(std::size_t c, std::size_t a, std::size_t b);

namespace deht{

    void to_json(json& j, const Address& p) {
        j = json{{"ip", p.data.ip}, {"port", p.data.port};
    }
    
    void from_json(const json& j, Address& p) {
        p.data.ip = j.at("ip").get<std::string>();
        p.data.port = j.at("port").get<int>();
    }

    struct Data{
        std::string ip;
        int port;
    };

    typedef struct Data Data;

    class Address{
        public:
            Data data;
            Address(std::string ip, int port);
            Address();
            std::size_t Hash();
            bool operator == (Address a){
                return (this->Hash() == a.Hash());
            }
            bool operator < (Address a){
                return (this->Hash() < a.Hash());
            }
            Data getData();
            std::string ToString();
    };

}