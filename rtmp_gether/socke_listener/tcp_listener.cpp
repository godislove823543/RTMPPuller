//
//  RtmpListener.cpp
//  RtmpS01
//
//  Created by 努力的工人 on 2021/1/5.
//

#include "tcp_listener.hpp"
//#include "rtmp_connectorMgr.hpp"
#include "tcp_socket.hpp"
#define MAX_CONNECTIONS 1000
//#include "parameterDefine.h"
//#include "interface_connector.h"
//extern rtmp_connectorMgr connectorMgr;

//tcp_listener::tcp_listener(rtmp_connectorMgr *mgr)
//{
////    connectorMgr=mgr;
////    m_port=1935;
////    m_bWork=true;
//}
tcp_listener::tcp_listener()//interface_connectorMgr *mgr)
{
//    connectorMgr=mgr;
    m_port=1935;
    m_bWork=true;
}
tcp_listener::~tcp_listener()
{
    
}
int tcp_listener::start(int port)
{
    signal(SIGPIPE, SIG_IGN); //信號處理
    if((m_fd=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        printf("Socket Create ERROR \n");
        return 1;
//        return srs_error_new(VALUE_SOCKET_CREATE,"Socket Create ERROR");
    }
    int n = 1;
    m_port=port;
#if defined(SO_NOSIGPIPE) && !defined(MSG_NOSIGNAL) //前面是linux 後面是windows
    //在寫入socket時不要觸發sigpipe信號 而是返回錯誤碼
    // We do not want SIGPIPE if writing to socket.
    const int value = 1;
    //setsockopt(fd, SOCKET(const), 沒有信號通知, 1, value長度)
    setsockopt(m_fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(int));
#endif
    //可供多個程式重複使用ip
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0)
    {
        printf("ERROR: can't set SO_REUSEADDR: setsockopt \n");
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, (char *)&n, sizeof(n))<0)
    {
        printf("ERROR: can't set SO_REUSEPORT: setsockopt \n");
    }
//    int nZero=0;
//    if(setsockopt(m_fd, SOL_SOCKET,SO_SNDBUF, (char *)&nZero,sizeof(nZero))!=0)
//    {
//        printf("ERROR: can't set SO_SNDBUF \n");
//    }
//    if(setsockopt(m_fd, SOL_SOCKET,SO_RCVBUF,(char *)&nZero,sizeof(int))!=0)
//    {
//        printf("ERROR: can't set SO_RCVBUF \n");
//    }
//    if(setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv))
//    {
//        printf("recv no timeout \n");
//    }
//    if(setsockopt(m_fd, IPPROTO_TCP, TIOCPKT_NOSTOP, (char *)&n, sizeof(int))<0)
//    {
//        printf("ERROR: can't set TIOCPKT_NOSTOP: setsockopt \n");
//    }
    //keepalive保持連線
    if(setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&n, sizeof(int))<0)
    {
        printf("ERROR: can't set Keep alive \n");
    }
    //不要延遲
    if( setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &n, sizeof(int))<0)   // 1 - on, 0 - off
    {
        printf("ERROR: can't set NO_DELAY \n");
    }
    //listen_add 結構
    memset(&listen_add,0,sizeof(listen_add)); //清零
    listen_add.sin_family=AF_INET;
    listen_add.sin_addr.s_addr=htonl(INADDR_ANY); //0.0.0.0默認本機位置
    listen_add.sin_port=htons(m_port);
//    std::cout <<"..............socket ..........1\n";
    //socket要用上面設定的資料當網址
    if(::bind(m_fd,(struct sockaddr *)&listen_add, sizeof(listen_add))==-1)
    {
        printf("Socket Bind error \n");
        return 1;
//        return srs_error_new(VALUE_SOCKET_BIND, "Socket Bind error");
    }
//    std::cout <<"..............socket ..........2\n";
    if(listen(m_fd, MAX_CONNECTIONS)==-1) //啟動監聽
    {
        printf("Socket listen error   \n");
        return 1;
//        return srs_error_new(VALUE_SOCKET_LISTEN, "Socket listen error");
    }
//    std::cout <<"..............socket ..........3\n";
     pthread_create(&trd, NULL, cycle, this);
//    std::cout <<"..............socket ..........3\n";

    return 0;//srs_success;
}
void tcp_listener::do_cycle()
{
    int client_fd;
    tcp_socket *client;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    std::cout <<"..............start  thread..........\n";
    while(m_bWork) //接受別人連線
    {
        if((client_fd=::accept(m_fd,(struct sockaddr *)&cliaddr, &clilen))==-1)
        {
            continue;
        }
        client=new tcp_socket(client_fd, &cliaddr); //產生tcp_socket物件做宋和收 封包資料
//        connectorMgr->generate_connector(client);

    }

}
void tcp_listener::close()
{
    ::close(m_fd);
}
void *tcp_listener::cycle(void *sev)
{

    tcp_listener *listener=(tcp_listener *)sev;
    listener->do_cycle();
    listener->close();
    return NULL;
}

