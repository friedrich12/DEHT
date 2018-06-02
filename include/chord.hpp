#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>
//#include <functional>
#include <string>
#include <assert.h>
//Core
#include <address.hpp>
#include <fstream>
#include <thread>
#include <map>
#include <remote.hpp>
#include <vars.hpp>
#include <network.hpp>    

class Local{
    public:
        Local(Address local_address, Address remote_address = NULL);
        bool is_ours(std::size_t id);
        void shutdown();
        std::string log(std::string info);
        void start();
        bool ping();
        void join();
        bool stabilize();
        void notify();
        bool fix_fingers();
        bool update_successors();
        void get_successors();
        inline std::size_t id(int offset = 0);
        Remote successor();
        void predecessor();
        void find_successor(int id);
        void find_predessor(int id);
        void closest_preceding_finger(int id);
        void run();
    private:
        Address address;
        Remote predecessor;
        bool shutdown;
        int socket;
        std::vector<Remote> successors;
        std::map<std::string,std::thread> daemons;
        std::vector<std::string> commands;
        std::vector<Remote> finger;
};