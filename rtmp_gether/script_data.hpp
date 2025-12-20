//
//  script_data.hpp
//  rtmp_gether
//
//  Created by scarlett on 2023/12/11.
//

#ifndef script_data_hpp
#define script_data_hpp

#include <stdio.h>
#include <stdio.h>
#include "PacketBuffer.hpp"
#include <vector>
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
    T value; //let every type have their own diff type var to storage data
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

#endif /* script_data_hpp */
