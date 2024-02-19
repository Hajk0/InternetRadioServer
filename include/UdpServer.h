//
// Created by hajk0 on 12/23/23.
//

#ifndef INTERNETRADIOSERVER_UDPSERVER_H
#define INTERNETRADIOSERVER_UDPSERVER_H


#include <netinet/in.h>
#include <sys/epoll.h>
#include <vector>

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

    std::vector<std::vector<int16_t>> cutWavFile(const char* filePath, double portionDurationSec);
    };


#endif //INTERNETRADIOSERVER_UDPSERVER_H
