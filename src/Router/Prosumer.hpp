#pragma once

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

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;
condition_variable cv;
mutex mtx;


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