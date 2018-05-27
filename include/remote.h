#include <address.h>
#include <vars.h>
#include <network.h>

/*template<typename Lambda>
bool Func1(int Arg1, Lambda Arg2){ // or Lambda&&, which is usually better
  if(Arg1 > 0){
    return Arg2(Arg1);
  } else {
    return false; // remember, all control paths must return a value
  }
}*/

class Remote{
    public:
        Remote::Remote(Address remoteAddress);
        void open_connection();
        void close_connection();
        std::string AddrStr();
        std::size_t id(std::size_t offset = 0);
        void send(std::string msg);
        void recv();
        void ping();

        // Depends on sucsessful connection
        
};