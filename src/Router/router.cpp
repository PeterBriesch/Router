#include <router.hpp>

//To Start router use input arguments:
/* 
1. ip address to start router on
2. port for router to listen on
3. ip address for router to connect to
4. port for router to connect to
*/ 
int main(int argc, char const *argv[]){
    try
    {
        // variables for connecting to other router
        std::string ip = argv[1];  
        int port = stoi(argv[2]);    
        std::string address = argv[3];
        int port_con = stoi(argv[4]);  

        boost::asio::io_service io_service;
        boost::asio::io_service router_ioservice;

        //Start router
        Router router(io_service, ip, port, address, port_con, router_ioservice);    
        cout << "------------ Router Started ------------" << endl;
        io_service.run();
        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << endl;
    }

    return 0;

}