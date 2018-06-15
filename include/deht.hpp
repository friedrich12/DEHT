#pragma once

// Core Things
#include <address.hpp>
#include <chord.hpp>
#include <daemon.hpp>
#include <network.hpp>
#include <remote.hpp>
#include <vars.hpp>

#include <map>
#include <iostream>
#include <thread>
#include <chrono>

class DEHT{
    public:
        DEHT(Address local_address, Address remote_address);
        void shutdown();

        std::string get(std::string key);
        void set(std::string key, std::string value);

        std::string _get(std::string request);
        std::string _set(std::string key, std::string value);

        bool distribute_data();
        //void create_dht(std::string addr, std::string port);

        map<std::string, std::string> data;
        vector<std::string> removeKeys;
        bool shutdown;
        map<std::string, std::thread> daemons;
        Local local;
};