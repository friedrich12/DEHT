#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

class Daemon{
    public:
        Daemon();
        Run();
};