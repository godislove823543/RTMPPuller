//
//  udt_socket.hpp
//  RtmpS01
//
//  Created by user on 2021/3/23.
//

#ifndef udt_socket_hpp
#define udt_socket_hpp

#include <stdio.h>
#include <string>
#include <errno.h>
using namespace std;
enum SOCKET_TYPE
{
    TCP_TYPE=0,
    UDP_TYPE,
};
class generalSocket
{
private:
    int fd;
    string ip;
    int port;
    bool isConnect;
public:
//    generalSocket();
    generalSocket(int fd);
    ~generalSocket();
public:
    bool send_packet(char *buffer, size_t size);
    bool recv_packet(char *data, size_t size, int offset);
    size_t recv_packet(char *buffer, size_t max_size);
    void close();
    int connectTo(string ip, int port);
    int get_fd();
    void set_fd(int fd_);
};
extern int createSocket(int type);
#endif /* udt_socket_hpp */
