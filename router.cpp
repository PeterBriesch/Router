#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/lexical_cast.hpp"
#include "packet_structure.hpp"
#include "Packet.cpp"
#include <fstream>
#include <stdio.h>

#include "Node/helper.hpp"
#define PORT 8080
// #define ASIO_STANDALONE

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;


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
        std::map<string, string> routingTable;

    public: 
        
        cli_handler(io_service& io_service, std::map<string, cli_handler::pointer>& cli, string* id): sock(io_service){
            //copy address to cli_handler pointer 
            clients = &cli;
            //copy address to cli_id
            cli_id = *id;
            // std::ifstream inFile;
            // inFile.open("./table.txt");
            // if(!inFile){
            //     std::cerr << "unable to open file" << endl;
            //     exit(1);
            // }
            // string key;
            // string value;
            // while(inFile >> key){
            //     inFile >> value;
            //     routingTable.emplace(key, value);
            // }
        }
        ~cli_handler(){};

        // creating the pointer
        static pointer create(io_service& io_service, std::map<string, cli_handler::pointer>& clients, string* cli_id)
        {
            return pointer(new cli_handler(io_service, clients, cli_id));
        }

        //socket creation
        tcp::socket& socket()
        {
            return sock;
        }

        void dequeue_clients(std::map<string, boost::shared_ptr<cli_handler>> &cli, tcp::socket &sock){

            cli.erase(cli_id);
        }

        // MAIN LOOP for cli_handler
        void start()
        {
            
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
            
        }

        void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
        {
                     

            if(!err){

                cout << "Router received data from [Client " << socket().remote_endpoint() << "] " << endl;

                //read header src address check first 8 char of table and match to id
                string dstaddress( reinterpret_cast<char*>(data.header.daddr), 16);
                cout << dstaddress << "\n" << data.header.saddr << endl;

                // string id = routingTable.at(dstaddress);

                //Get pointer to client from map
                cli_handler::pointer dest;
                try{
                    dest = clients->at(dstaddress);
                }catch(const std::exception& e){
                    auto it = clients->begin();
                    dest = it->second;
                }
                cout << "Routing to " << dest->socket().remote_endpoint() << endl; 

                //Get the socket
                boost::asio::write(dest->socket(), boost::asio::buffer(&data, sizeof(data)));

                // cout << "block_interest: " << block_interest << endl;
                // cout << "prefix_interest: " << prefix_interest << endl;

                cout << "Packet successfully routed\n" << endl;
            }
            else{
            std::cerr << "error: " << err.message() << endl;

            // Remove client from map
            dequeue_clients(*clients, sock);

            // disconnect socket
            sock.shutdown(boost::asio::socket_base::shutdown_both);
            sock.close();
            return;
            }
            start();
        }

        void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
        {
            if(!err){
                cout << "[21e8::router] Router routed message!" << endl;
            }else{
                std::cerr << "error: " << err.message() << endl;
                // Remove socket from map
                cout << "handle_write dequeue" << endl;
                dequeue_clients(*clients, sock);
                // Diconnect socket
                sock.shutdown(boost::asio::socket_base::shutdown_both);
                sock.close();
                return;
            }
            start();
        }


};

class Router 
{
    private:

    tcp::acceptor acceptor_;
    // Map stores pointer to client objects
    std::map<string, boost::shared_ptr<cli_handler>> clients;
    string cli_id;

    void start_accept()
    {

        // socket
        cli_handler::pointer connection = cli_handler::create(acceptor_.get_io_service(), clients, &cli_id);

        // asynchronous accept operation and wait for a new connection.
        acceptor_.async_accept(connection->socket(),
            boost::bind(&Router::handle_accept, this, connection,
            boost::asio::placeholders::error));   
    }

    public:

    static void print_clients(std::map<string, boost::shared_ptr<cli_handler>> &cli){
        cout << "Connected Clients: " << endl;
        for(auto const& [key, val] : cli){
            cout << key << endl;
        }
    }
    
    //constructor for accepting connection from client
    Router(boost::asio::io_service& io_service, int port): acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }

    void handle_accept(cli_handler::pointer connection, const boost::system::error_code& err)
    {

        try{
            cout << "[21e8::router] Client connected on port: " << connection->socket().remote_endpoint() << "\n" << endl;   

        }catch(const std::exception& e)
        {
            std::cerr << e.what() << endl;
        }
        if (!err) {
            connection->start();
            //store client in map
            cli_insert(connection);
            print_clients(clients);
            
        }
        start_accept();
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

        cli_id = hashFunction(ip+":"+port).substr(0,16);
        clients.emplace(cli_id,cli);
    }
};

int main(int argc, char const *argv[]){
    try
    {
        
        boost::asio::io_service io_service;
        cout << "Start Router! choose port: " << endl;
        int port;
        std::cin >> port;
        Router router(io_service, port);    
        cout << "------------ Router Started ------------" << endl;
        io_service.run();
        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << endl;
    }

    return 0;

}