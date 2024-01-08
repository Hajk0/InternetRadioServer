//
// Created by hajk0 on 12/23/23.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include "../include/UdpServer.h"

int UdpServer::setUp() {
    epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Error creating epoll file descriptor\n";
        return 1;
    }
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Error creating UDP socket\n";
        return 1;
    }

    udpServerAddress.sin_family = AF_INET;
    udpServerAddress.sin_addr.s_addr = INADDR_ANY;
    udpServerAddress.sin_port = htons(PORT);

    if (bind(udpSocket, (sockaddr*) &udpServerAddress, sizeof(udpServerAddress)) == -1) {
        std::cerr << "Error binding to UDP port\n";
        close(udpSocket);
        return 1;
    }
    return 0;
}

int UdpServer::epollSetUp() {
    udpEvent.events = EPOLLIN;
    udpEvent.data.fd = udpSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, udpSocket, &udpEvent) == -1) {
        std::cerr << "Error adding UDP socket to epoll\n";
    }
}