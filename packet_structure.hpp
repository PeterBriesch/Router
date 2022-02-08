#pragma once
#include <iostream>
#include <cstring>
#include "boost/lexical_cast.hpp"
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