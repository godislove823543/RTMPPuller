//
//  rtmp_fmt.hpp
//  rtmp_server
//
//  Created by scarlett on 2023/10/26.
//

#ifndef rtmp_fmt_hpp
#define rtmp_fmt_hpp

#include <stdio.h>
#include <string>
#   define RTMP_PACKED __attribute__((__packed__))

struct uint24_t
{
    uint8_t low;
    uint8_t mid;
    uint8_t high;
};
union RTMP_PACKED RTMPChunkMessageHeader
{
    struct RTMP_PACKED Type0
    {
        uint8_t  timestamp[3];
        uint8_t message_length[3];
        uint8_t message_type_id;
        uint32_t message_stream_id;
    } _0;

    struct RTMP_PACKED Type1
    {
        uint24_t timestamp_delta; //與上一個chunk的時間戳差值 delta(增量)
        uint24_t message_length;
        uint8_t message_type_id;
    } _1;

    struct RTMP_PACKED Type2
    {
        uint24_t timestamp_delta;
    } _2;
};


template <class T>
union RTMP_PACKED RTMPChunkBasicHeader
{
    struct RTMP_PACKED One
    {
        uint8_t cs_id:6;
        uint8_t fmt:2;
        T message;
    } _1;

    struct RTMP_PACKED Two
    {
        uint8_t zero:6;
        uint8_t fmt:2;
        uint8_t cs_id;
        T message;
    } _2;

    struct RTMP_PACKED Three
    {
        uint8_t one:6;
        uint8_t fmt:2;
        uint8_t cs_id[2];
        T message;
    } _3;

};

typedef RTMPChunkBasicHeader<RTMPChunkMessageHeader> RTMPChunkHeader;




#endif /* rtmp_fmt_hpp */
