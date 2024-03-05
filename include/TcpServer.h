//
// Created by hajk0 on 12/23/23.
//

#ifndef INTERNETRADIOSERVER_TCPSERVER_H
#define INTERNETRADIOSERVER_TCPSERVER_H


#include <netinet/in.h>
#include <sys/epoll.h>
#include "Library.h"
#include "Stream.h"

class TcpServer {
    static const int MAX_EVENTS = 10;
    static const int PORT = 12345;

    int serverSocket{};
    sockaddr_in serverAddress{};
    int epollFd{};
    epoll_event event{}, events[MAX_EVENTS]{};
    Library library;
    int clientCounter;
    Stream stream;
    std::thread streamToClients;
    bool closeServer = false;

public:
    TcpServer();
    int socketSetUp();
    int epollSetUp();
    int epollWait();
    void newConnection();
    void existingConnection(int i);
    int receiveFile(std::string filename);

    epoll_event* getEvents();
    int getServerSocket() const;

    int sendAvaiableSongs(int sock, vector<string> songs);

};


#endif //INTERNETRADIOSERVER_TCPSERVER_H
