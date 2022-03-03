#include "inc.hpp"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;


class routingTable
{
    private:
        std::map<string, string> table; //Key: ip address and port Value: hash of key
        std::map<string, pointer> client;//Key: hash of key same as table Value: cli_handler::pointer to client_handler object

    public:

        routingTable(void)
        {
            std::ifstream inFile;
            try{
                inFile.open("./src/Router/table.txt");
            }catch(const std::exception &err){
                std::cerr << "unable to open file: " << err.what() << std::endl;
                exit(1);
            }

            string key;
            string value;
            while(inFile >> key){
                inFile >> value;
                table.emplace(key, value);
            }
            inFile.close();
        }

        void operator=(const routingTable &r){
            table = r.table;
            client = r.client;
        }

        void dequeue_clients(tcp::socket &sock, sockaddr_in sock_info){

            //parse boost::asio::ip::address as string and port to string
            char address_s[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sock_info.sin_addr), address_s, INET_ADDRSTRLEN);
            string port_s = to_string(sock_info.sin_port);
            string ip (address_s);
            string ip_port = ip + ":" + port_s;

            std::cout << ip_port << std::endl;


            //remove cli from the client map
            this->client.erase(ip_port);
            sock.close();

        }

        void cli_insert(pointer cli, int port_in, boost::asio::ip::address addr_in)
        {

            std::ofstream inFile;
            try{
                inFile.open("./src/Router/table.txt", std::ios_base::app);
            }catch(const std::exception &err){
                std::cerr << "unable to open file: " << err.what() << std::endl;
                exit(1);
            }


            // parse ip::address to string
            string ip = sockaddr_tostring(addr_in);
            string port = to_string(port_in);
            string ip_port = ip + ":" + port;

            string ip_hash = iptohash(port, ip).substr(0, 16);

            string cli_id = hashtoIPv6(ip_hash);

            //add ip and hash of ip to routingTable
            if(!(this->table.count(cli_id))){
                this->table.emplace(cli_id, ip_port);
                inFile << cli_id << " " << ip_port << "\n";
            }
            
            inFile.close();

            //add hash of ip and pointer to clientTable
            this->client.emplace(ip_port, cli);
        }

        string query_table(string q)
        {
            return table.at(q);
        }

        pointer query_clients(string q)
        {

            return client.at(q);
        }

        std::map<string, pointer> get_clients()
        {
            return this->client;
        }

        std::map<string, string> get_rTable()
        {
            return this->table;
        }

};