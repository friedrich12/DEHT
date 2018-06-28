#include <deht.hpp>
#include <cstdlib>

using namespace std;
using namespace deht;

int main(int argc, char *argv[]){
    // Start the hash table
    Address d;
    /*for(int i = 0; i < argc; ++i){
        cout << argv[i] << endl;
    }*/
    DEHT(Address(argv[1], atoi(argv[2])), d);
    return 0;
}