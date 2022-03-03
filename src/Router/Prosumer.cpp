#include <iostream>
#include <thread>
#include <chrono>
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/lexical_cast.hpp"
#include "packet_structure.hpp"
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>

#include "Node/Server.hpp"
#include "Miner/mine.hpp"
// #include "dump.hpp"
#define PORT 8080
// #define ASIO_STANDALONE

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;
condition_variable cv;
mutex mtx;

unsigned char antelope;

/* // FOR HANDLING CLIENT CONNECTIONS TO PROSUMER
class cli_handler : public boost::enable_shared_from_this<cli_handler>
{
    private:

        tcp::socket sock;
        enum{ max_length = 1024};
        net::Packet::packet data;

    public:

        typedef boost::shared_ptr<cli_handler> pointer;

        //Constructor
        cli_handler(tcp::socket& io_service): sock(std::move(io_service)){
            
        };

        // creating the pointer
        static pointer create(tcp::socket& io_service)
        {
            return pointer(new cli_handler(io_service));
        };

        //socket creation
        tcp::socket& socket()
        {
            return sock;
        };

        //Main loop
        void start()
        {
            sock.async_read_some(
                boost::asio::buffer(&data.header, sizeof(data.header)),
                boost::bind(&cli_handler::handle_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            
            // sock.async_write_some(
            //     boost::asio::buffer(message, max_length),
            //     boost::bind(&cli_handler::handle_write,
            //                 shared_from_this(),
            //                 boost::asio::placeholders::error,
            //                 boost::asio::placeholders::bytes_transferred));
            
        };

        void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
        {
        
            if(!err){
                // cout << "[Client " << socket().remote_endpoint() << "] " << data << endl;
            }else{
                std::cerr << "error: " << err.message() << endl;
                sock.close();
                return;
            }
        }

        void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
        {
            if(!err){
                cout << "[21e8::Prosumer] Server sent Hello message!" << endl;
            }else{
                std::cerr << "error: " << err.message() << endl;
                sock.close();
                return;
            }
        };

        


};

class Server 
{
    private:
    tcp::acceptor acceptor_;
    
    void start_accept()
    {
        // socket
        // cli_handler::pointer connection = cli_handler::create(acceptor_.get_io_service());

        // asynchronous accept operation and wait for a new connection.
        // acceptor_.async_accept(connection->socket(),
        //     boost::bind(&Server::handle_accept, this, connection,
        //     boost::asio::placeholders::error));
        acceptor_.async_accept(
            [this](boost::system::error_code er, tcp::socket socket)
            {
                if(!er){
                    cli_handler::pointer connection = cli_handler::create(socket);
                    handle_accept(connection);

                }
            }
        );
        
    }
    public:
    //constructor for accepting connection from client
    Server(boost::asio::io_service& io_service, int port): acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }
    void handle_accept(cli_handler::pointer connection)
    {

        try{
            cout << "[21e8::Prosumer] Client connected on port: " << connection->socket().remote_endpoint() << endl;
        }catch(const std::exception& e)
        {
            std::cerr << e.what() << endl;
        }
        
        connection->start();
        
        start_accept();
    }
}; */


// FOR connecting to router
class Client :public boost::enable_shared_from_this<Client>
{
    private:
        tcp::socket socket;
        net::Packet::packet pkt;
        net::Packet::packet rcv_pkt;
        int hash = 0;
        int count = 0;
        

    public:

    Client(boost::asio::io_service &io_service, int port, string address, int bindPort): socket(io_service){

        socket.open(boost::asio::ip::tcp::v4());
        socket.bind(tcp::endpoint(socket.local_endpoint().address(), bindPort));
        bool connected = false;

        while(!connected){
                
            connected = cliConnect(address, port);
                
        }

    }

    bool cliConnect(string address, int port){

        try{
            socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port));
            cout << "[21e8::Prosumer] Connection on : " << address << ":" << port << endl;;
            cout << "[21e8::Prosumer] Started on port: " << socket.local_endpoint().port() << endl;
            run();
            return true;
        }catch(const std::exception& e){
            std::cout<< "coultdn't connect to prosumer: RETRYING" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }
    }

    void cliDisconnect(){
        try{
            boost::system::error_code errorcode;
            socket.cancel();

            if (socket.is_open()){

                socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorcode);
                if (errorcode)
                    std::cerr << "socket.shutdown error: " << errorcode.message() << std::endl;

                socket.close(errorcode);
                if (errorcode)
                    std::cerr << "socket.close error: " << errorcode.message() << std::endl;

            }
        }catch(const std::exception &err){
            std::cerr << "error: " << err.what() << std::endl;
        }

            return;
    }

    void streamIndex(){

        

    }

    void start(int port, string address){
        
    }

    // Main loop
    void run(){

        try{
            
            uint32_t ip_int = socket.local_endpoint().port();
            if(ip_int != 8081 && ip_int != 8085){
                //iterate counter
                count++;

                uint64_t hash;
                char* end;

                // string temp1 = "bcaf:48cb:ef7c:9453:bcaf:48cb:ef7c:9453";
                // inet_pton(AF_INET6, temp1.c_str(), &(antelope));
                // unsigned char temp[INET6_ADDRSTRLEN];
                // inet_ntop(AF_INET6, &(antelope), (char*)temp, INET6_ADDRSTRLEN);
                // std::cout << "THE IPV6 address is :" <<  temp << std::endl;

                static unsigned char dest[][INET6_ADDRSTRLEN] = {"bcaf:48cb:ef7c:9453::", "c076:6f12:85c1:f25a:bcaf:48cb:ef7c:9453", "541a:208f:f1eb:4d61:bcaf:48cb:ef7c:9453"};
                int rand_index = rand() % 3;
                pkt.header.saddr = ip_int;
                //convert counter to string
                stringstream ss;
                ss << count;

                //create data to put into packet
                string data_out = hashFunction(ss.str()).substr(0, 16)+"\n";
                hash = strtoul(data_out.c_str(), &end, 16);

                Packet packet (pkt);
                packet.packet_builder(ip_int, dest[rand_index], hash);

                net::Packet::packet snd_pkt = packet.get_packet();

                socket.async_write_some(
                boost::asio::buffer(&snd_pkt, packet.size()),
                boost::bind(&Client::handle_write,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)
                );
                
                std::this_thread::sleep_for(std::chrono::seconds(1));

            }
            // string in;        
        
            /* // // --------------------- get new input from client -------------------
            // cout << "Send packet 1 or 2: " << endl;

            // std::cin >> in;

            // uint32_t ip_int = socket.local_endpoint().port();
            // pkt.header.saddr = ip_int;
            // Packet packet (pkt);
        
            // if(in == "1"){
            //     packet.packet_builder(ip_int, "bcaf48cbef7c9453", 0);
            //     pkt = packet.get_packet();

            //     boost::asio::write(socket, boost::asio::buffer(&pkt, packet.size()));
            //     socket.async_read_some(
            //     boost::asio::buffer(&rcv_pkt, sizeof(rcv_pkt)),
            //     boost::bind(&Client::handle_response,
            //                 this,
            //                 boost::asio::placeholders::error,
            //                 boost::asio::placeholders::bytes_transferred));
            //     return;
                
                
            // }else if (in == "2"){
            //     packet.packet_builder(ip_int, "c0766f1285c1f25a", 0);
            //     pkt = packet.get_packet();

            //     boost::asio::write(socket, boost::asio::buffer(&pkt, packet.size()));
            //     socket.async_read_some(
            //     boost::asio::buffer(&rcv_pkt, sizeof(rcv_pkt)),
            //     boost::bind(&Client::handle_response,
            //                 this,
            //                 boost::asio::placeholders::error,
            //                 boost::asio::placeholders::bytes_transferred));
            //     return;

            // }
    */
            socket.async_read_some(
            boost::asio::buffer(&rcv_pkt, sizeof(rcv_pkt)),
            boost::bind(&Client::handle_response,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }
        catch(const std::exception &err){
            std::cerr << "error: " << err.what() << std::endl;
            cliDisconnect();
            return;
        }

        return;        
    }
    
    void handle_response(const boost::system::error_code& err, size_t bytes_transferred){

        Packet temp(rcv_pkt);
        cout << "\nREADING Response" << endl;

        if(!err){

            temp.dump();
            // socket.cancel();
            
        }
        else{
            cout << "receive failed: " << err.message() << endl;
            cliDisconnect();
            return;
        }
        run();
    }

    void handle_read(const boost::system::error_code& err, size_t bytes_transferred){

        Packet temp(rcv_pkt);
        cout << "READING" << endl;
        
        
        
        if(!err){

            // Swap dest and src address
            cout << "PACKET RECEIVED:" << endl; 
            temp.dump();

            cout << "\nBuilding response packet" << endl;
            //convert src address of rcv_packet to string and hash
            string address = "10.147.20.40:" + to_string(temp.get_srcAddress());
            address = hashFunction(address).substr(0,16);
            unsigned char res_dstaddr[16];
            memcpy(res_dstaddr, address.c_str(), sizeof(unsigned char[16]));

            //set src address to local endpoint
            uint32_t res_srcaddr = socket.local_endpoint().port();
            //set data 
            uint64_t data = 9999;
            //Build packet
            Packet res(res_srcaddr, res_dstaddr, data);
            auto res_packet = res.get_packet();
            //send response
            boost::asio::write(socket, boost::asio::buffer(&res_packet, res.size()));
            cout << "Response Sent\n" << endl;

            
        }
        else{
            cout << "receive failed: " << err.message() << endl;
            cliDisconnect();
            return;
            
        }
        run();
    }
    void handle_write(const boost::system::error_code& err, size_t bytes_transferred){

        
    
        if(!err){
            cout << "[21e8::Prosumer] Data sent" << endl;
            
        }
        else{
            cout << "send failed: " << err.message() << endl;
            cliDisconnect();
            return;
        }
        run();
    }

};



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