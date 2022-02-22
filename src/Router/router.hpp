#pragma once
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/lexical_cast.hpp"
#include "packet_structure.hpp"
#include <fstream>
#include <chrono>
#include <stdio.h>

// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"
// #include <stdio.h>
// #if defined(IMGUI_IMPL_OPENGL_ES2)
// #include <GLES2/gl2.h>
// #endif
// #include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "Miner/mine.hpp"
#define PORT 8080
#define ADDRESS "0.0.0.0"
// #define ASIO_STANDALONE

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

std::mutex cache_mutex;

// Cli_handler is spawned for each new client that connects to the router
class cli_handler : public boost::enable_shared_from_this<cli_handler>
{
    public:
    // Shared pointer used to access client handler object
    typedef boost::shared_ptr<cli_handler> pointer;

    private:

        tcp::socket sock;
        string cli_id;
        // Clients stores shared pointers to cli_handler objects
        std::map<string, cli_handler::pointer> *clients;
        enum{ max_length = 2048};
        net::Packet::packet data;
        std::vector<net::Packet::packet> *cache;
        std::map<string, string> routingTable;

    public: 
        
        cli_handler(tcp::socket& io_service, std::map<string, cli_handler::pointer>& cli, string* id, std::vector<net::Packet::packet> &cache_f): sock(std::move(io_service)){
            //copy address to cli_handler pointer 
            clients = &cli;
            //copy address to cache
            cache = &cache_f;
            //copy address to cli_id
            cli_id = *id;

            //Read routing table (CURRENTLY NOT BEING USED)
            std::ifstream inFile;
            inFile.open("./src/Router/table.txt");
            if(!inFile){
                std::cerr << "unable to open file" << endl;
                dequeue_clients(clients, sock);
                exit(1);
            }
            string key;
            string value;
            while(inFile >> key){
                inFile >> value;
                routingTable.emplace(key, value);
            }

        }
        ~cli_handler(){};

        // creating the pointer
        static pointer create(tcp::socket& io_service, std::map<string, cli_handler::pointer>& clients, string* cli_id, std::vector<net::Packet::packet> &cache)
        {
            return pointer(new cli_handler(io_service, clients, cli_id, cache));
        }

        //socket creation
        tcp::socket& socket()
        {
            return sock;
        }

        // MAIN LOOP for cli_handler
        void start()
        {
            try{
                sock.async_read_some(
                    boost::asio::buffer(&data, sizeof(data)),
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
            }catch(const std::exception &err){
                std::cerr << "Failed to async_read: " << err.what() << std::endl;
                dequeue_clients(clients, sock);
                return;
            }
        }

        void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
        {
                     

            if(!err){
                //cache packet
                cache_packet();

                cout << "Router received data from [Client " << socket().remote_endpoint() << "] " << endl;

                forward_packet();
                
            }
            else{
            std::cerr << "handle_read error: " << err.message() << endl;

            // Remove client from map
            dequeue_clients(clients, sock);


            return;
            }
            start();
        }

        void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
        {
            if(!err){
                cout << "[21e8::router] Router routed message!" << endl;
            }else{
                std::cerr << "handle_write error: " << err.message() << endl;
                // Remove socket from map
                dequeue_clients(clients, sock);
                return;
            }
            start();
        }

        void forward_packet()
        {

            try{
                //read header src address check first 8 char of table and match to id
                string dstaddress( reinterpret_cast<char*>(data.header.daddr), 16);
                cout << dstaddress << "\n" << data.header.saddr << endl;

                //Get pointer to client from map
                // Router attempts to route the data to the specified destination address
                // If the specified address is not in the routing table then send the data to the first client in the routing table
                cli_handler::pointer dest;
                try{
                    //query connected piers for destination address
                    dest = clients->at(dstaddress);
                }catch(const std::exception& e){
                    //if query fails then rout to first connected pier in router list
                    auto it = clients->begin();
                    dest = it->second;
                }
                cout << "Routing to " << dest->socket().remote_endpoint() << endl; 

                //Write to specified face
                boost::asio::write(dest->socket(), boost::asio::buffer(&data, sizeof(data)));

                cout << "Packet successfully routed\n" << endl;
            }catch(const std::exception &err){
                std::cerr << "failed to forward packet: " << err.what() << std::endl;
                dequeue_clients(clients, sock);
            }

            return;

        }

        void dequeue_clients(std::map<string, boost::shared_ptr<cli_handler>> *cli, tcp::socket &sock){

            //parse boost::asio::ip::address as string and port to string
            std::ostringstream os;
            os << sock.remote_endpoint().address();

            string port_s = to_string(sock.remote_endpoint().port());
            string address_s = os.str();
            string ip = address_s + ":" + port_s;
            
            //First 16 char of the hash
            string id = hashFunction(ip).substr(0, 16);
            
            //remove cli from the client map
            cli->erase(id);

            sock.cancel();
            sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            sock.close();
        }

        void cache_packet(){
            std::lock_guard<std::mutex> guard(cache_mutex);
            //check if cache is full
            if(cache->size() >= 2000){
                cache->pop_back();
            }    

            //add data to cache
            cache->emplace(cache->begin(), data);            
        }


};

class Router 
{
    private:

    tcp::acceptor acceptor_;
    tcp::socket router_sock;
    // Map stores pointer to client objects
    std::map<string, boost::shared_ptr<cli_handler>> clients;
    std::vector<net::Packet::packet> cache;
    string cli_id;

    void start_accept()
    {

        // asynchronous accept operation and wait for a new connection.
        acceptor_.async_accept(
            [this](boost::system::error_code er, tcp::socket socket)
            {
                if(!er){

                    std::lock_guard<std::mutex> guard(cache_mutex);
                    cli_handler::pointer connection = cli_handler::create(socket, clients, &cli_id, cache);
                    handle_accept(connection);

                }
            }
        );   

    }

    public:
    
    //constructor for accepting connection from client
    Router(boost::asio::io_service& io_service, int port, std::string address, int port_con, boost::asio::io_service& router_ioservice): acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), router_sock(io_service)
    {
        cout << "Router LISTENING on " << acceptor_.local_endpoint() << endl;

        //  Connect to router 
        try{
            if(acceptor_.local_endpoint().port() != 8080){
                router_connect(address, port_con);
            }
        }catch(const std::exception &er){
            std::cerr << "Unable to connect to router: " << er.what() << endl;
        }

        start_accept();
    }

    void handle_accept(cli_handler::pointer connection)
    {

        try{
            cout << "[21e8::router] Client connected on port: " << connection->socket().remote_endpoint() << "\n" << endl;   

        }catch(const std::exception& e)
        {
            std::cerr << e.what() << endl;
        }
        
        connection->start();
        //store client in map
        cli_insert(connection);
        print_clients(clients);
            
        start_accept();
    }

    std::map<string, boost::shared_ptr<cli_handler>> ShowClients(){

        try{
            return clients;
        }
        catch(const std::exception& e){
            std::cout<< e.what()<< std::endl;

        }
        
    }

    bool ShowCache(std::vector<net::Packet::packet> &cache1){
        if(!cache.empty()){
            std::lock_guard<std::mutex> guard(cache_mutex);
            if(cache1[0].header.daddr == cache[0].header.daddr &&
                cache1[0].payload.payload == cache[0].payload.payload){
                return false;
            }

            cache1 = cache;
            return true;
        }
        return true;
        
    }

    static void print_clients(std::map<string, boost::shared_ptr<cli_handler>> &cli){
        cout << "Connected Clients: " << endl;
        for(auto const& [key, val] : cli){
            cout << key << endl;
        }
    }

    void router_connect(std::string address, int port){
        
        std::lock_guard<std::mutex> guard(cache_mutex);

        //create cli_handler pointer
        cli_handler::pointer routerCon = cli_handler::create(router_sock, clients, &cli_id, cache);
        // boost::asio::ip::tcp::acceptor::reuse_address option(true); 
        // // routerCon->socket().set_option(option);
        // routerCon->socket().bind(tcp::endpoint(tcp::v4(), port));
        //connect socket to endpoint
        routerCon->socket().connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port));
        //start the client handler
        routerCon->start();
        //insert the pointer to the 
        cli_insert(routerCon);
        print_clients(clients);
    }


    cli_handler::pointer get_cli(string key)
    {
        return clients.at(key);
    }

    void cli_insert(cli_handler::pointer cli)
    {
        // parse ip::address to string
        std::ostringstream os;
        os << cli->socket().remote_endpoint().address();

        string port = to_string(cli->socket().remote_endpoint().port());
        string ip = os.str();

        //compute hashfunction of ip:port
        cli_id = hashFunction(ip+":"+port).substr(0,16);
        //add client to map
        clients.emplace(cli_id,cli);
    }
};
