#pragma once
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstring>
#include "Miner/mine.hpp"
#include <inttypes.h>
namespace net
{
    namespace Packet
    {
        struct header
        {
            uint8_t version=0;
            uint32_t flowLabel=0;
            uint16_t length=0;
            uint8_t nextHeader=0;
            uint8_t hopLimit=0;
            uint32_t saddr = 0;
            unsigned char daddr[16] = "";
            std::time_t timestamp = 0;

            bool operator ==(const net::Packet::header& a)
            {
                return version == a.version && flowLabel == a.flowLabel && length == a.length && nextHeader == a.nextHeader &&
                        hopLimit == a.hopLimit && saddr == a.saddr && daddr == a.daddr && timestamp == a.timestamp;
            }

        };

        struct payload
        {
            uint64_t payload;
            bool operator ==(const net::Packet::payload& a)
            {
                return payload == a.payload;
            }
        };

        struct packet
        {
            net::Packet::header header;
            net::Packet::payload payload;
            bool operator ==(const net::Packet::packet& a){
                return header == a.header && payload == a.payload;
            }
        };
    }
}

using namespace std;

class Packet
{

    private:

        net::Packet::packet pkt;

    public:

        Packet(uint32_t saddr, unsigned char daddr[16], uint64_t data){packet_builder(saddr, daddr, data);}

        Packet(net::Packet::packet packet){pkt = packet;}

        void dump(){

            char dest_address[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, pkt.header.daddr, dest_address, INET6_ADDRSTRLEN);
            std::cout << pkt.header.version << " | " << pkt.header.flowLabel << std::endl;
            std::cout << pkt.header.length << " | " << pkt.header.hopLimit << std::endl;
            std::cout << pkt.header.saddr << std::endl;
            std::cout << dest_address << std::endl;
            std::cout << pkt.payload.payload << std::endl;
        };

        std::string get_dstAddress(){
            char dest_address[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, pkt.header.daddr, dest_address, INET6_ADDRSTRLEN);
            std::string s (dest_address);
            return s;

        };

        int get_srcAddress(){

            return pkt.header.saddr;

        };

        size_t size(){
            return sizeof(pkt);

        }

        void packet_builder(uint32_t saddr, unsigned char daddr[INET6_ADDRSTRLEN], uint64_t data){
            net::Packet::header header;
            net::Packet::payload payload;


            //Build header
            payload = buildPayload(data);
            pkt.payload = payload;
            header = buildHeader(saddr, daddr);
            
            

            pkt.header = header;
            

            return;
        }

        net::Packet::packet get_packet(){
            return pkt;
        }
        
    private:
        net::Packet::header buildHeader(uint32_t saddr, unsigned char daddr[INET6_ADDRSTRLEN]){

            uint8_t version=0;
            uint32_t flowLabel=0;
            uint16_t length=sizeof(pkt.payload);
            uint8_t nextHeader=0;
            uint8_t hopLimit=0;
            std::time_t timestamp = std::time(nullptr);
    
            net::Packet::header header = {version, flowLabel, length, nextHeader, hopLimit, saddr};
            header.timestamp = timestamp;
            inet_pton(AF_INET6, (char*)daddr, header.daddr);

            return header;
        }

        net::Packet::payload buildPayload(uint64_t pl){

            net::Packet::payload payload;
            payload.payload = pl;

            return payload;

        }

};