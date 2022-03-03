#pragma once

#include "routingTable.hpp"
#include "inc.hpp"

// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"
// #include <stdio.h>
// #if defined(IMGUI_IMPL_OPENGL_ES2)
// #include <GLES2/gl2.h>
// #endif
// #include <GLFW/glfw3.h> // Will drag system OpenGL headers

#define PORT 8080
#define ADDRESS "0.0.0.0"
// #define ASIO_STANDALONE

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

// Cli_handler is spawned for each new client that connects to the router
class cli_handler : public boost::enable_shared_from_this<cli_handler>, public routingTable
{
    public:
    

    private:

        tcp::socket sock;
        sockaddr_in sock_info;
        // Clients stores shared pointers to cli_handler objects
        enum{ max_length = 2048};
        net::Packet::packet data;
        net::Packet::packet *new_pkt;
        std::vector<net::Packet::packet> *cache;
        routingTable *rTable_ptr;

    public: 
        
        cli_handler(tcp::socket& io_service, std::vector<net::Packet::packet> &cache_f, routingTable &rTable, net::Packet::packet &pkt): sock(std::move(io_service)){
            //copy address to cache
            cache = &cache_f;
            //copy address to routingTable
            rTable_ptr = &rTable;
            //copy address to packet
            new_pkt = &pkt;

            inet_pton(AF_INET, sockaddr_tostring(sock.remote_endpoint().address()).c_str(), &(sock_info.sin_addr));
            sock_info.sin_port = sock.remote_endpoint().port();
        }

        // creating the pointer
        static pointer create(tcp::socket& io_service, std::vector<net::Packet::packet> &cache, routingTable &rTable, net::Packet::packet &pkt)
        {
            return pointer(new cli_handler(io_service, cache, rTable, pkt));
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
                
            }catch(const std::exception &err){
                std::cerr << "Failed to async_read: " << err.what() << std::endl;
                rTable_ptr->dequeue_clients(sock, sock_info);
                return;
            }
        }

        void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
        {
                     

            if(!err){
                //cache packet
                cache_packet();
                string client = sockaddr_tostring(socket().remote_endpoint());

                *new_pkt = data;

                printf("Router received %ld bytes from [Client %s]\n", bytes_transferred, client.c_str());

                // cout << "Router received data from [Client " << socket().remote_endpoint() << "] " << endl;

                forward_packet();
                
            }
            else{
                std::cerr << "handle_read error: " << err.message() << endl;

                // Remove client from map
                rTable_ptr->dequeue_clients(sock, sock_info);
                return;
            }
            start();
        }

        void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
        {
            if(!err){
                printf("[21e8::router] Router sent %ld bytes.\n\n", bytes_transferred);
                // cout << "[21e8::router] Router sent  message!" << endl;
            }else{
                std::cerr << "handle_write error: " << err.message() << endl;
                // Remove socket from map
                rTable_ptr->dequeue_clients(sock, sock_info);
                return;
            }
            start();
        }

        void forward_packet()
        {

            try{
                //read header src address check first 8 char of table and match to id
                unsigned char dstaddress[INET6_ADDRSTRLEN];
                pointer dest;

                /* 
                Get pointer to client from map
                Router attempts to route the data to the specified destination address
                If the specified address is not in the routing table then send the data to the first client in the routing table
                */
                try{
                    inet_ntop(AF_INET6, data.header.daddr, (char*)dstaddress, INET6_ADDRSTRLEN);
                    string table_query ((char*)dstaddress);

                    //query connected piers for destination address
                    dest = rTable_ptr->query_clients(rTable_ptr->query_table(table_query));

                }catch(const std::exception& e){
                    std::cerr << "Client not connected to router. Routing to first client in routing table: " << e.what() << endl;
                    //if query fails then rout to first connected pier in router list
                    auto clients = rTable_ptr->get_clients();
                    auto it = clients.begin();
                    dest = it->second;
                }
                cout << "[21e8::router] Routing to " << dest->socket().remote_endpoint() << endl; 

                /*TODO build new packet to forward with source address set to routers address*/

                //Write to specified face
                dest->socket().async_write_some(
                    boost::asio::buffer(&data, sizeof(data)),
                    boost::bind(&cli_handler::handle_write,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));

                // boost::asio::write(dest->socket(), boost::asio::buffer(&data, sizeof(data)));

            }catch(const std::exception &err){
                std::cerr << "failed to forward packet: " << err.what() << std::endl;
                rTable_ptr->dequeue_clients(sock, sock_info);
                return;
            }


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
    routingTable rTable;
    std::vector<net::Packet::packet> cache;
    string cli_id;
    net::Packet::packet pkt;

    void start_accept()
    {

        // asynchronous accept operation and wait for a new connection.
        acceptor_.async_accept(
            [this](boost::system::error_code er, tcp::socket socket)
            {
                if(!er){

                    std::lock_guard<std::mutex> guard(cache_mutex);
                    pointer connection = cli_handler::create(socket, cache, rTable, pkt);
                    handle_accept(connection);

                }
            }
        );   

    }

    public:
    
    //constructor for accepting connection from client
    Router(boost::asio::io_service& io_service, int port, std::string address, int port_con, boost::asio::io_service& router_ioservice): acceptor_(io_service, tcp::endpoint(boost::asio::ip::address::from_string("10.147.20.40"), port)), router_sock(router_ioservice)
    {

        cout << "Router LISTENING on " << acceptor_.local_endpoint() << endl;

        //  Connect to router 
        try{
            if(acceptor_.local_endpoint().port() != 8080){
                std::cout << address << port_con << std::endl;
                router_connect(address, port_con);
            }
        }catch(const std::exception &er){
            std::cerr << "Unable to connect to router: " << er.what() << endl;
        }

        start_accept();
    }

    void handle_accept(pointer connection)
    {

        try{
            cout << "[21e8::router] Client connected on port: " << connection->socket().remote_endpoint() << "\n" << endl;   

        }catch(const std::exception& e)
        {
            std::cerr << e.what() << endl;
        }

        connection->start();

        //store client in map
        rTable.cli_insert(connection, connection->socket().remote_endpoint().port(), connection->socket().remote_endpoint().address());
        print_clients(rTable.get_clients());
                    
        start_accept();
    }

    std::map<string, boost::shared_ptr<cli_handler>> ShowClients(){

        try{
            return rTable.get_clients();
        }
        catch(const std::exception& e){
            std::cout<< e.what()<< std::endl;

        }
        
    }

    bool ShowCache(std::vector<net::Packet::packet> &cache1){
        if(!cache.empty()){
            std::lock_guard<std::mutex> guard(cache_mutex);
            if(std::equal(cache1.begin(), cache1.end(), cache.begin())){
                cache1 = cache;
                return false;
            }
            cache1 = cache;
        }
        return false;
        
    }

    bool ShowPacket(net::Packet::packet &packet){
        packet = pkt;
        if (packet == pkt){
            return false;
        }
        return true;
    }

    static void print_clients(std::map<string, boost::shared_ptr<cli_handler>> cli){
        cout << "Connected Clients: " << endl;
        for(auto const& [key, val] : cli){
            cout << key << endl;
        }
    }

    void router_connect(std::string address, int port){
        
        std::lock_guard<std::mutex> guard(cache_mutex);

        //connect socket to endpoint
        router_sock.connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port));
        //create cli_handler pointer
        pointer routerCon = cli_handler::create(router_sock, cache, rTable, pkt);
        // boost::asio::ip::tcp::acceptor::reuse_address option(true); 
        // // routerCon->socket().set_option(option);
        // routerCon->socket().bind(tcp::endpoint(tcp::v4(), port));
        //start the client handler
        routerCon->start();
        //insert the pointer to the 
        rTable.cli_insert(routerCon, routerCon->socket().remote_endpoint().port(), routerCon->socket().remote_endpoint().address());
        print_clients(rTable.get_clients());
        
       
    } 

    routingTable getRoutingTable(void)
    {
        return rTable;
    }

};