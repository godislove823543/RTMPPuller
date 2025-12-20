//
//  scriptData.cpp
//  rtmp_gether
//
//  Created by scarlett on 2023/11/13.
//
#include "scriptData.hpp"
#include "PacketBuffer.hpp"
//實作decode
bool ele_num::decode(PacketBuffer &packet)
{
    uint64_t v =packet.read_8bytes();
//    value= v; //value 是泛型
    memcpy(&value, &v, 8);
    vary_type = MSG_NUM; //讓不同的派生類別能夠覆寫這個虛擬函式
    printf("number: %lf \n", value);
    return true;
}
bool ele_bool::decode(PacketBuffer &packet)
{
    value = packet.read_1bytes();
    vary_type=MSG_BOOL;
    printf("bool: %d \n", value);
    return true;
}
bool ele_str::decode(PacketBuffer &packet)
{
    len = packet.read_2bytes(); //省略_string.length
    if(len==0)
    {
        return false;
    }
    value = packet.read_string(len);
    vary_type=MSG_STRING;
    printf("string: %s \n", value.c_str());
    return true;
}
bool ele_obj::decode(PacketBuffer &packet)
{
//    type = packet.read_1bytes();
//    len = packet.read_2bytes();
//    value = packet.read_string(len);
    element *object_element=nullptr; //initial 確保指標在第一時間不指向內存的任何位置
    value.clear();
    bool exit=false;
    while(!exit)
    {
        
        object_element=new element(); //key type void*
        printf("key: ");
        if(!object_element->key.decode(packet)) //key.decode是一段字串
        {
            packet.read_1bytes();
            exit=true;
            break;
        }
        printf("value: ");
        int amf_type=packet.read_1bytes();
        switch (amf_type)
        {
            case MSG_NUM:
            {
                ele_num *number=new ele_num(); //establish an obj
                number->decode(packet); //解碼number
                object_element->value=number; //把number的實例指標賦值給element結構的value成員
                object_element->type=MSG_NUM;
                value.push_back(object_element); //把所有類型的type物件放進amf_data的容器裡
                break;
            }
            case MSG_BOOL:
            {
                ele_bool *_bool = new ele_bool();
                _bool->decode(packet);
                object_element->value=_bool;
                object_element->type=MSG_BOOL;
                value.push_back(object_element);
                break;
            }
            case MSG_STRING:
            {
                ele_str *_string = new ele_str();
                _string->decode(packet);
                object_element->value=_string;
                object_element->type=MSG_STRING;
                value.push_back(object_element);
                break;
            }
            case MSG_MIXARR:
            {
                ele_mixarr *_mix = new ele_mixarr();
                _mix->decode(packet);
                object_element->value=_mix;
                object_element->type=MSG_MIXARR;
                value.push_back(object_element);
                printf("%d \n", packet.pos());
            }
            break;
        }
    }
    return true;
}

bool ele_null::decode(PacketBuffer &packet)
{
//value 是泛型
    vary_type=MSG_NULL; //type=enum
    printf("null");
    return true;
}
bool ele_mixarr::decode(PacketBuffer &packet)
{
    value = packet.read_4bytes();
    vary_type =  MSG_MIXARR;
    printf("Mix array");
    return true;
}
