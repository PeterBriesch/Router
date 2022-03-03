#pragma once
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "packet_structure.hpp"
#include <fstream>
#include <chrono>
#include <stdio.h>
#include <algorithm>
#include <functional>
#include "Miner/mine.hpp"

class cli_handler;
class routingTable;
class Router;

// Shared pointer used to access client handler object
typedef boost::shared_ptr<cli_handler> pointer;


/* HELPER FUNCTIONS AND STRUCTS*/

std::mutex cache_mutex;


string iptohash(string port, string address)
{

    string ip = address + ":" + port;   
    //First 16 char of the hash
    string id = hashFunction(ip);

    return id;
}

string hashtoIPv6(string hash)
{
    char ipv6[INET6_ADDRSTRLEN];

    if(hash.size() <= 32){
        //represents size of ipv6 char array
        int count = 0;
        int n = 0;

        //loop through hash string and add colon as needed 
        for(char const &c: hash)
        {
            ipv6[count] = c;

            count++;

            //check if its reached 4th position
            if((count-n)%4 == 0 && c != hash.back()){
                ipv6[count] = ':';
                count++;
                n++;
            }
        }

        while((count-n)%4 == 1)
        {
            ipv6[count] = '0';
            count++;
        }
        if(count < 39)
        {
            ipv6[count] = ':';
            ipv6[count+1] = ':';
        }
        ipv6[count+2] = '\0';
        
    }
    else{
        std::cerr << "Hash is to long for IPv6 address" << std::endl;
    }

    return ipv6;
}

template<class T>
string sockaddr_tostring(T adr)
{
    std::ostringstream os;
    os << adr;
    return os.str();
}

/*----------------------------------------------------------*/