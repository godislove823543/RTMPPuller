//
//  main.cpp
//  rtmp_gether
//
//  Created by scarlett on 2023/11/8.
//

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "loggg.hpp"
#include "generalSocket.hpp"
#include "rtmp_fmt.hpp"
#include "script_data.hpp"

//srs_app_rtmp_conn 172~261
PacketBuffer rtmp_packet(8192);
RTMPChunkHeader header;
generalSocket so(-1);
int fd=createSocket(TCP_TYPE);
uint32_t windows_size=0;
uint32_t chunk_size=0;
double value;
int64_t temp = 0x00;
uint32_t msg_length;
vector<amf_all *> amf_data; //amf_any(class) 裡面放所有類型的結構物件(含類型和解碼)

//-------------------------------------------------------------
uint32_t b2_24(const uint8_t* bytes) {
    return (uint32_t)(bytes[0] << 16 | bytes[1] << 8 | bytes[2]);
}
//--------------------------------------------------
void handshake()
{
    char packet[1537];
    char recv_packet[1537];
    
    so.set_fd(fd);
    so.connectTo("127.0.0.1",  1935);
    packet[0]=3;
    //c0, c1
    so.send_packet(packet, 1537);
    
    //s0, s1
    so.recv_packet(recv_packet, 1537, 0);
    //c2
    so.send_packet(recv_packet+1, 1536); //c2 送s1
    //s2
    so.recv_packet(recv_packet+1, 1536, 0); //s2 送c1
    
    log(__FILE__, __LINE__, "completely handshake!!"); //文件名 行數
}
//-------------------------------------------------------
void send_chunk_size()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x2);
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(4);
    rtmp_packet.write_1bytes(0x01);
    rtmp_packet.write_4bytes(0);
    
    rtmp_packet.write_4bytes(4096);
}
//--------------------------------------------------------
void send_connection_cmd()
    {
        //send_connection
        char conn[]={"connect"};
        
        rtmp_packet.reset_pointer();
        rtmp_packet.write_1bytes(0x3); //0:11bytes 3:high level(connect,create stream的消息)
        rtmp_packet.write_3bytes(0); //timestamp
        rtmp_packet.skip(3); //body size
        rtmp_packet.write_1bytes(0x14); //type ID
        rtmp_packet.write_4bytes(0); //Stream ID=0 -> 完成c&s的connect&control
        
        rtmp_packet.write_1bytes(0x02);
        rtmp_packet.write_2bytes(strlen(conn)); //len
        rtmp_packet.write_bytes(conn, strlen(conn)); //name
        rtmp_packet.write_1bytes(0x0);
        rtmp_packet.write_8bytes(0);
        
        
        //object
        rtmp_packet.write_1bytes(0x03);
        
        //key
         rtmp_packet.write_2bytes(strlen("app"));   //len
        rtmp_packet.write_string(string("app"));   //string
        //value
        rtmp_packet.write_1bytes(0x02); //type
        rtmp_packet.write_2bytes(strlen("live"));   //len
        rtmp_packet.write_string(string("live"));   //string
        
        //key
        rtmp_packet.write_2bytes(strlen("tcUrl"));
        rtmp_packet.write_string(string("tcUrl"));
        //value
        string tcUrl=string("rtmp://127.0.0.1/live");
        rtmp_packet.write_1bytes(0x02);
        rtmp_packet.write_2bytes(tcUrl.size());
        rtmp_packet.write_string(tcUrl);

        
        //key
        rtmp_packet.write_2bytes(strlen("fpad"));
        rtmp_packet.write_string("fpad");
        //value
        rtmp_packet.write_1bytes(0x01);
        rtmp_packet.write_1bytes(0x00);
        
        //key
        rtmp_packet.write_2bytes(strlen("capabilities"));
        rtmp_packet.write_string("capabilities");
        //value
        rtmp_packet.write_1bytes(0x00);
        value=15;
        memcpy(&temp, &value, 8);
        rtmp_packet.write_8bytes(temp); //這邊要放uint64_t
        
        rtmp_packet.write_2bytes(0);
        rtmp_packet.write_1bytes(9); //109
        
        //回去填上bodysize的長度
        int len=rtmp_packet.pos(); //109
        rtmp_packet.skip(-len+4); //-105
        rtmp_packet.write_3bytes(len-12); //109-12 current 7
        rtmp_packet.skip(len-7); //跳過109-7回到現在位置
    }

//------------------------------------------------------------
void amf12_14(uint32_t msg_length)
{
    rtmp_packet.reset_pointer();
    so.recv_packet((char *)rtmp_packet.data(), msg_length, 0);
    amf_data.clear();
    int len=msg_length;
    while(true)
    {
        uint8_t amf_type = 0;
//        printf("%d \n", rtmp_packet.pos());
        amf_type = rtmp_packet.read_1bytes();
        if(amf_type==MSG_NUM)
        {
            if(len-rtmp_packet.pos()<8)
                break;
            //在heap動態分配一個number物件，因為在程式執行時物件大小無法確定，且生命週期不止在當前函數中
            ele_num *number=new ele_num();
            number->decode(rtmp_packet); //number結構裡的decode函數實作
            //amf_any class裡有vfunction 和enum type
            amf_data.push_back((amf_all *)number); //儲存起來 vector<amf_any *> amf_data;
            printf("%d \n", rtmp_packet.pos());
        }
        if(amf_type==MSG_BOOL)
        {
            ele_bool *_bool = new ele_bool();
            _bool->decode(rtmp_packet);
            amf_data.push_back((amf_all *)_bool); //這是一種類型轉換（type casting）的語法，將 _bool 轉換為 amf_any 的指標型別，但_bool要是amf_any的子類別或實例
            printf("%d \n", rtmp_packet.pos());
        }
        if(amf_type==MSG_STRING)
        {
            ele_str *_string = new ele_str();
            _string->decode(rtmp_packet);
            amf_data.push_back((amf_all *)_string);
//            _string.length = rtmp_packet.read_2bytes();
//            _string.value = rtmp_packet.read_string(_string.length);
            printf("%d \n", rtmp_packet.pos());
        }
        if(amf_type==MSG_OBJ)
        {
            ele_obj *_obj=new ele_obj();
            _obj->decode(rtmp_packet);
            amf_data.push_back((amf_all *)_obj); //將 num 轉換成 amf_any* 型別，
            //然後將其加入到 amf_data 中。這樣的轉換可能是因為 amf_data 儲存的是指向 amf_any 類型的指標
            printf("%d \n", rtmp_packet.pos());
        }
        if(amf_type==MSG_NULL)
        {
            ele_null *_null = new ele_null();
            _null->decode(rtmp_packet);
            amf_data.push_back((amf_all*)_null);
            printf("%d \n", rtmp_packet.pos());
        }
        if(amf_type==MSG_MIXARR)
        {
            ele_mixarr *_mix = new ele_mixarr();
            _mix->decode(rtmp_packet);
            amf_data.push_back((amf_all*)_mix);
            printf("%d \n", rtmp_packet.pos());
        }
    }
    printf(".......finish  ...........\n");
}

//---------------------------------------------------------------
void recieve_data()
    {
    int run=1;
        while(run)
        {
            if(!so.recv_packet((char *)&header, 1, 0)) //1個字節 fmt
            {
                log(__FILE__, __LINE__, "receive data err");
            }
            log(__FILE__, __LINE__, "fmt: %d, csid: %d", header._1.fmt, header._1.cs_id);
            if(header._1.fmt==0)
            {
                so.recv_packet((char *)&header._1.message._0, 11, 0);
//                char buffer[11];
//                so.recv_packet((char *)buffer, 11, 0);
                uint8_t typeID=header._1.message._0.message_type_id;
                switch (typeID)
                {
                    case 0x1:
                    {
                        
                        msg_length = b2_24((const uint8_t*)header._1.message._0.message_length);//header._1.message._0.message_length);
                        rtmp_packet.reset_pointer(); //curr=start
                        so.recv_packet((char *)rtmp_packet.data(), msg_length, 0);
                        chunk_size= rtmp_packet.read_4bytes();
                    }
                        break;
                    case 0x4:
                    {
                        msg_length = b2_24((const uint8_t*) header._1.message._0.message_length);//header._1.message._0.message_length);
//                        msg_length = b2_24(header._1.message._0.message_length);
                        rtmp_packet.reset_pointer();
                        so.recv_packet((char *)rtmp_packet.data(), msg_length, 0);
                        uint16_t type = rtmp_packet.read_2bytes();
                        uint32_t stream = rtmp_packet.read_4bytes();
                    }
                        break;
                    case 0x5:
                    {
                        msg_length = b2_24((const uint8_t*) header._1.message._0.message_length);//header._1.message._0.message_length);
//                        msg_length = b2_24(header._1.message._0.message_length);
                        rtmp_packet.reset_pointer(); //又回到最前面02 connect
                        so.recv_packet((char *)rtmp_packet.data(), msg_length, 0); //接收四個b
                        windows_size=rtmp_packet.read_4bytes(); //在4個b裡填入資料 指標往後移動4格
                        
                    }
                        break;
                    case 0x6:
                    {
                        msg_length = b2_24((const uint8_t*) header._1.message._0.message_length);//header._1.message._0.message_length);
//                        msg_length = b2_24(header._1.message._0.message_length);
                        rtmp_packet.reset_pointer(); //指標往前移動4格
                        so.recv_packet((char *)rtmp_packet.data(), msg_length, 0);
                        windows_size = rtmp_packet.read_4bytes();
                        uint8_t limit_type=0;
                        limit_type = rtmp_packet.read_1bytes();
                    }
                        break;
                    case 0x12:
//                    {
//                        msg_length = b2_24((const uint8_t*) header._1.message._0.message_length);//header._1.message._0.message_length);
////                        msg_length = hex2dec(header._1.message._0.message_length);
//
//                        amf12_14(msg_length);
//                    }
//                        break;
                    case 0x14:
                    {
                        msg_length = b2_24((const uint8_t*) header._1.message._0.message_length);//header._1.message._0
                        amf12_14(msg_length);
                    }
                        break;
                }
            }
            run=0;
        }
    }

//------------------------------------------------------
void releaseStream()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x43);
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(39);
    rtmp_packet.write_1bytes(0x14);
    
    rtmp_packet.write_1bytes(0x02);
    char buff[]={"releaseStream"};
    rtmp_packet.write_2bytes(strlen(buff));
    rtmp_packet.write_bytes(buff,strlen(buff));
    rtmp_packet.write_1bytes(0x0);
    value=2;
    memcpy(&temp, &value, 8);
    rtmp_packet.write_8bytes(temp);
    rtmp_packet.write_1bytes(0x05);
    rtmp_packet.write_1bytes(0x02);
    char buff2[]={"livestream"};
    rtmp_packet.write_2bytes(strlen(buff2));
    rtmp_packet.write_bytes(buff,strlen(buff2));
    
}
//------------------------------------------------------
void send_set_chunk_size()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x2);
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(4);
    rtmp_packet.write_1bytes(0x01);
    rtmp_packet.write_4bytes(0);
    
    rtmp_packet.write_4bytes(60000);
}
//------------------------------------------------------
void send_FCPublish()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x43);
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(35); //body size
    rtmp_packet.write_1bytes(0x14); //type ID
    
    rtmp_packet.write_1bytes(0x02);
    char buff[]={"FCPublish"};
    rtmp_packet.write_2bytes(strlen(buff));
    rtmp_packet.write_bytes(buff, strlen(buff));
    value=3;
    memcpy(&temp, &value, 8);
    rtmp_packet.write_8bytes(temp);
    rtmp_packet.write_1bytes(0x05);
    char buff2[]={"livestream"};
    rtmp_packet.write_2bytes(strlen(buff2));
    rtmp_packet.write_bytes(buff, strlen(buff2));

  
}
//------------------------------------------------------
void send_createStream()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x3);
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(25); //body size
    rtmp_packet.write_1bytes(0x14); //type ID
    rtmp_packet.write_4bytes(0); //Stream ID=0 -> 完成c&s的connect&control
    
    rtmp_packet.write_1bytes(0x02);
    char buff[]={"createStream"};
    rtmp_packet.write_2bytes(strlen(buff)); //00 0c
    rtmp_packet.write_bytes(buff, strlen(buff));
    rtmp_packet.write_1bytes(0x0);
    value = 1;
    memcpy(&temp, &value, 8);
    rtmp_packet.write_8bytes(temp);
    rtmp_packet.write_1bytes(0x05); //null
    
}
//-----------------------------------------------------
void send_checkbw()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x3); //control stream
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(21);
    rtmp_packet.write_1bytes(0x14);
    rtmp_packet.write_4bytes(0);
    
    rtmp_packet.write_1bytes(0x02);
    char buff[]={"_checkbw"};
    rtmp_packet.write_2bytes(strlen(buff)); //00 0c
    rtmp_packet.write_bytes(buff, strlen(buff));
    value=5;
    memcpy(&temp, &value, 8);
    rtmp_packet.write_8bytes(temp);
    rtmp_packet.write_1bytes(0x05);
}
//------------------------------------------------------
void send_play()
{
    rtmp_packet.reset_pointer();
    rtmp_packet.write_1bytes(0x8); //control stream
    rtmp_packet.write_3bytes(0);
    rtmp_packet.write_3bytes(39);
    rtmp_packet.write_1bytes(0x14);
    rtmp_packet.write_4bytes(1);
    
    rtmp_packet.write_1bytes(0x02);
    char buff[]={"play"};
    rtmp_packet.write_2bytes(strlen(buff));
    rtmp_packet.write_bytes(buff, strlen(buff));
    rtmp_packet.write_1bytes(0);
    value = 2;
    memcpy(&temp, &value, 8);
    rtmp_packet.write_8bytes(temp);
    rtmp_packet.write_1bytes(0x05);
    rtmp_packet.write_1bytes(0x02);
    char buff2[]={"livestream"};
    rtmp_packet.write_2bytes(strlen(buff2));
    rtmp_packet.write_bytes(buff2, strlen(buff2));
    rtmp_packet.write_1bytes(0);
    rtmp_packet.write_8bytes(0);
    
}
//------------------------------------------------------
int main(int argc, const char * argv[]){
    handshake();
    send_connection_cmd();
    so.send_packet(rtmp_packet.data(), rtmp_packet.pos());
    
    send_chunk_size();
    so.send_packet(rtmp_packet.data(), rtmp_packet.pos());
    
    recieve_data();//window acknowledgement
    recieve_data();//peer bandwidth
    recieve_data();//set chunk size
    recieve_data();//onBWDone
    
    releaseStream();
    so.send_packet(rtmp_packet.data(), rtmp_packet.pos());
   
    send_FCPublish();
    send_createStream();
    send_checkbw();
    so.send_packet(rtmp_packet.data(), rtmp_packet.pos());
    
    recieve_data();//parse RTMP control pk
    
    send_play();
    so.send_packet(rtmp_packet.data(), rtmp_packet.pos());

    recieve_data();//stream begin1
    recieve_data();// |RtmpSampleAccess
    recieve_data();// onStatus

}

