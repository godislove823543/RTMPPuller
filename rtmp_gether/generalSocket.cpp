//
//  udt_socket.cpp
//  RtmpS01
//
//  Created by user on 2021/3/23.
//

#include "generalSocket.hpp"
//#include <netdb.h>
#include <netinet/in.h>
//#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/ipc.h>
//#include <sys/sem.h>
//#include <sys/shm.h>
//#include <netinet/tcp.h>
#include <fcntl.h>
#include <vector>
#include <list>
#include <cstring>
#include <dirent.h>
//#include <errno.h>
#include <arpa/inet.h>
//#include <errno.h>
#ifndef TAG
#define TAG "generalSocket"
#endif
int createSocket(int type)
{
    int fd=-1;
    if(type==UDP_TYPE)
    {
        fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    }else{
        fd = ::socket(AF_INET , SOCK_STREAM , 0);
    }
    return fd;
}
generalSocket::generalSocket(int _fd)
{
    fd=_fd;
    isConnect = false;
}
generalSocket::~generalSocket()
{
    if(fd!=-1)
        close();
    fd=-1;
}

bool generalSocket::send_packet(char *buffer, size_t size)
{
    bool bSuccess=false;
    size_t access_size=0;
    size_t target_size=0;
    char *data_point;
    if(buffer!=NULL)
    {
        target_size=size;
        data_point=buffer; //指針指到buff最前面
        bSuccess=true;
        while( target_size>0)
        {
            access_size=::send(fd, data_point , target_size, MSG_WAITALL);
            if(access_size>0 && access_size<=target_size)
            {
                target_size-=access_size;
                data_point+=access_size;
                    
            }else
            {
                printf("Send...sendPacket .底層錯誤!! %d ==>%d \n", access_size, errno);
                if(errno==EINTR) //interrupt
                {
                    continue;
                }
                //EAGAIN暫時無法用
                else if(errno==EAGAIN || errno==EWOULDBLOCK)
                {
                    usleep(5);
                    continue;
                }
                bSuccess=false;
                break;
            }
        }
    }
    return bSuccess;
}
size_t generalSocket::recv_packet(char *buffer, size_t max_size)
{
    size_t rcvd = 0;
    rcvd = ::recv(fd, (char*)buffer, max_size, 0);
    return rcvd;
}
bool generalSocket::recv_packet(char *buffer, size_t size, int offset)
{
    bool bSuccess=true;
    size_t total_len=size;//g_TotalNum * sizeof(int32_t);
    size_t target_size=size;
    size_t got_size=0;
//    int errno;
    int rcvd = 0;
    while (target_size > 0)
    {
        //ensure that new coming data wont be overwritten and will follow the original data
        //got_size=actual all data recieved
       rcvd = ::recv(fd, (char*)buffer+offset + got_size, target_size, MSG_WAITALL);
       if (rcvd <= 0)
       {
//           srt_getlasterror(&errno);
           if(errno==EINTR)
           {
               continue;
           }
           else if(errno==EAGAIN || errno==EWOULDBLOCK)
           {
               usleep(5);
               continue;
           }
           bSuccess=false;
           break;
       }else
       {
           target_size -= rcvd;
           got_size += rcvd;
       }
        
    }
    return bSuccess;
}
void generalSocket::close()
{
    ::close(fd);
}
int generalSocket::connectTo(string _ip, int _port)
{
   
    ip = _ip; //string
    port = _port; //int
//    fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if(fd <0) //fd=int
    {
        printf("error, no soket object \n");
        return -1;
    }
    struct sockaddr_in addr = {}; //sockaddr_in是一個結構
    socklen_t addrLength = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    int n=1;
    //inet_addr(ip.c_str()):十進制表示的 IPv4 地址轉換為 32 位的二進制形式
    //int setsockopt(int, int, int, const void *, socklen_t); 設置socket選項
    //SOL_SOCKET 通用的socket選項；SO_REUSEADDR 可以在同一個端口上重用處於TIME_WAIR狀態的socket
    //SOL_REUSEPORT 允許多個socket在同一個端口上監聽
    //n是指標 用來選擇SO_REUSEADDR和SO_REUSEPORT
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0)
    {
        printf("ERROR: can't set SO_REUSEADDR: setsockopt \n");
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&n, sizeof(n)) < 0)
    {
        printf("ERROR: can't set SO_REUSEPORT: setsockopt \n");
    }
    int err=::connect(fd, (sockaddr*)&addr, addrLength);
    if(err<0)
    {
        fprintf(stderr, "error, connect: %d\n", err);
        return -1;
    }
    isConnect = true;
    return fd;

}
int generalSocket::get_fd()
{
    return fd;
}
void generalSocket::set_fd(int fd_)
{
    fd=fd_;
}
