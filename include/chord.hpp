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
#include <random>
#include <iterator>
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
        vector<Remote> get_successors();
        inline std::size_t id(int offset = 0);
        Remote successor();
        Remote predecessor();
        Remote find_successor(std::size_t id);
        Remote find_predessor(std::size_t id);
        Remote closest_preceding_finger(std::size_t id);
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

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}