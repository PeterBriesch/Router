#include "Prosumer.hpp"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

int main(int argc, char const *argv[]){

    int node_port = 2180; // port to start the node on
    int bindPort; // port to bind prosumer to
    int newport; // port used to connect to router
    boost::asio::io_service io_service;
    boost::asio::io_service client_service;
    string address; // address that the router is on

    if(argc >= 2){
        node_port = stoi(argv[1]);
        bindPort = stoi(argv[2]);
        address = argv[3];
        newport = stoi(argv[4]);
        
        while(1){
            try{
                std::thread srv(StartServer, node_port);
                Client client(client_service, newport, address, bindPort);
                client_service.run();
            }catch(const std::exception& e){
                cout << "couldn't connect to prosumer retrying" << endl;
            }
        }

    }
    else{
        cout << node_port << endl;
            
        // Start node for handling mining requests
        std::thread srv(StartServer, node_port);

        //start server and client io service

        // Start server
        cout << "[21e8::Prosumer] Start Prosumer! choose port: ";
        std::cin >> bindPort;

        cout << "-------------------------- Server Started on port "<< bindPort << " --------------------------" << endl;

        // start clients
        cout << "[21e8::Prosumer] Make Connection? [Y]" << endl;
        string operation;
        std::cin >> operation;
        while(1){
            if(operation == "Y"){

                cout << "[21e8::Prosumer] Choose a address: ";
                std:: cin >> address;
                // address = "10.147.20.40";
                cout << "[21e8::Prosumer] Choose a port: ";

                std::cin >> newport;
                // newport = 8080;
                Client client(client_service, newport, address, bindPort);
                client_service.run();
                std::cin >> operation;

            }

        }    
    }     

    return 0;

}