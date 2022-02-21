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
            uint32_t saddr;
            unsigned char daddr[16];
            std::time_t timestamp;


        };

        struct payload
        {
            uint64_t payload;
        };

        struct packet
        {
            net::Packet::header header;
            net::Packet::payload payload;
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
            std::cout << pkt.header.version << " | " << pkt.header.flowLabel << std::endl;
            std::cout << pkt.header.length << " | " << pkt.header.hopLimit << std::endl;
            std::cout << pkt.header.saddr << std::endl;
            std::cout << pkt.header.daddr << std::endl;
            std::cout << pkt.payload.payload << std::endl;
        };

        std::string get_dstAddress(){
            std::string s (pkt.header.daddr, pkt.header.daddr + sizeof(pkt.header.daddr)/sizeof(pkt.header.daddr[0]));
            return s;

        };

        int get_srcAddress(){

            return pkt.header.saddr;

        };

        size_t size(){
            return sizeof(pkt);

        }

        void packet_builder(uint32_t saddr, unsigned char daddr[16], uint64_t data){
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
        net::Packet::header buildHeader(uint32_t saddr, unsigned char daddr[16]){

            uint8_t version=0;
            uint32_t flowLabel=0;
            uint16_t length=sizeof(pkt.payload);
            uint8_t nextHeader=0;
            uint8_t hopLimit=0;
            std::time_t timestamp = std::time(nullptr);
    
            net::Packet::header header = {version, flowLabel, length, nextHeader, hopLimit, saddr};
            header.timestamp = timestamp;
            memcpy(header.daddr, daddr, sizeof(unsigned char[16]));

            return header;
        }

        net::Packet::payload buildPayload(uint64_t pl){

            net::Packet::payload payload;
            payload.payload = pl;

            return payload;

        }

};