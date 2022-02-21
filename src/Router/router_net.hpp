#include <iostream>
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "packet_structure.hpp"
#define PORT 8080
// #define ASIO_STANDALONE

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

namespace core
{
    namespace net
    {
        template<typename T>
        class router_interface
        {
            private:
            map<string, tcp::socket&> sockets;
            void router_interface(){

            }

            virtual ~router_interface(){
                Disconnect();
            }
            public:

        };
    }
}
