#include "nlohmann/json.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <utility>

using json = nlohmann::json;


int main(){
    using namespace std;

    vector<pair<string, int>> myvec;
    pair<string,int> c;
    c.first = "Friedrich";
    c.second = 16;

    pair<string,int> a;
    a.first = "Juliet";
    a.second = 16;

    myvec.push_back(c);
    myvec.push_back(a);

    json j = myvec;
    cout << j << endl;

    string str = j.dump();
    cout << str << endl;

    json d;
    d.push_back(c);
    for (json::iterator it = d.begin(); it != d.end(); ++it) {
        std::cout << it.key() << " : " << it.value() << "\n";
    }
}