#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <address.h>
#include <thread>
#include <vector>
//#include <functional>
#include <string>
#include <assert.h>    

class Local{
    public:
        Local(Address local_address, Address remote_address = NULL);
        bool is_ours(std::size_t id);
        void shutdown();
        std::string log();
        void start();
        bool ping();
        void join();
        bool stabilize();
        void notify();
        bool fix_fingers();
        bool update_successors();
        void get_successors();
        std::size_t id(int offset = 0);
        void successor();
        void predecessor();
        void find_successor(int id);
        void find_predessor(int id);
        void closest_preceding_finger(int id);
        void run();
    private:
        Address addr;
        bool shutdown;
        std::vector<int> successors;
        std::vector<int> daemons;
        std::vector<int> commands;
};