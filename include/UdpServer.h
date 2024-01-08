//
// Created by hajk0 on 12/23/23.
//

#ifndef INTERNETRADIOSERVER_UDPSERVER_H
#define INTERNETRADIOSERVER_UDPSERVER_H


#include <netinet/in.h>
#include <sys/epoll.h>

class UdpServer {
    static const int PORT = 12344;

    int udpSocket;
    sockaddr_in udpServerAddress{};
    epoll_event udpEvent;
    int epollFd;

public:
    int setUp();
    int epollSetUp();

    int getUdpSocket();
    int getPort();
};


#endif //INTERNETRADIOSERVER_UDPSERVER_H
