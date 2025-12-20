//
//  RtmpListener.hpp
//  RtmpS01
//
//  Created by 努力的工人 on 2021/1/5.
//

#ifndef RtmpListener_hpp
#define RtmpListener_hpp
/*#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>*/


#include <iostream>
#include <string>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
//#include "parameterDefine.h"
//#include "srs_app_st.hpp"
#include "pthread.h"
//#include "rtmp_connector.hpp"
//#include "srs_kernel_buffer.hpp"
//class interface_connectorMgr;
////class SrsCoroutine;
//class rtmp_connectorMgr;
//通用規範
class interface_rtmp_listener//:ISrsCoroutineHandler
{
public:
    virtual int start(int port=1935)=0; //derived class可以被redefined
    virtual void do_cycle()=0;
    virtual void close()=0;
public:
    int m_fd;
    int m_port;
    std::string m_ip;
    struct sockaddr_in listen_add;
    pthread_t trd;
    bool m_bWork;
//    interface_connectorMgr *connectorMgr;
};

class tcp_listener:public interface_rtmp_listener//:ISrsCoroutineHandler
{
public:
    tcp_listener();//interface_connectorMgr *mgr);
    ~tcp_listener();
public:
    int start(int port=1935);
    void do_cycle();
    void close();
private:
    static void *cycle(void *);
};
#endif /* RtmpListener_hpp */
