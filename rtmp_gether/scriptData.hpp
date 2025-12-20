//
//  decode_pkg.hpp
//  rewrite_rtmp
//
//  Created by scarlett on 2023/11/16.
//

#ifndef decode_pkg_hpp
#define decode_pkg_hpp

#include <stdio.h>
#include "PacketBuffer.hpp"
using namespace std;

enum VaryType
{
    MSG_NUM=0,
    MSG_BOOL,
    MSG_STRING,
    MSG_OBJ,
    MSG_NULL=5,
    MSG_MIXARR=8,
};

class amf_all
{
public:
    virtual bool decode(PacketBuffer &packet)=0;
public:
    VaryType vary_type;
};

template <class T>
class Interface_amf: public amf_all
{
protected:
    T value; //read 8 byte
};

struct ele_num:Interface_amf<double>
{
public:
    bool decode(PacketBuffer &packet);
};
struct ele_bool:Interface_amf<uint8_t>
{
    
    bool decode(PacketBuffer &packet);
};
struct ele_str:Interface_amf<string>
{
    uint16_t len;
    bool decode(PacketBuffer &packet);
};
struct ele_null:Interface_amf<uint8_t>
{
    bool decode(PacketBuffer &packet);
};
struct ele_mixarr:Interface_amf<uint32_t>
{
    bool decode(PacketBuffer &packet);
};
struct element
{
    ele_str key;
    VaryType type;
    void* value;
};
struct ele_obj:Interface_amf<vector<element *>>
{
    bool decode(PacketBuffer &packet);
};

#endif /* decode_pkg_hpp */
