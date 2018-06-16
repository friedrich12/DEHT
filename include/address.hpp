#pragma once

#include <functional>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_set>
#include <utility>
#include <vars.hpp>

bool inrange(std::size_t c, std::size_t a, std::size_t b);

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