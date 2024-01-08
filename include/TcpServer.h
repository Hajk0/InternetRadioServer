//
// Created by hajk0 on 12/23/23.
//

#ifndef INTERNETRADIOSERVER_TCPSERVER_H
#define INTERNETRADIOSERVER_TCPSERVER_H


#include <netinet/in.h>
#include <sys/epoll.h>

class TcpServer {
    static const int MAX_EVENTS = 10;
    static const int PORT = 12345;

    int serverSocket;
    sockaddr_in serverAddress{};
    int epollFd;
    epoll_event event, events[MAX_EVENTS];

public:
    int socketSetUp();
    int epollSetUp();
    int epollWait();
    void newConnection();
    void existingConnection(int i);

    epoll_event* getEvents();
    int getServerSocket() const;

};


#endif //INTERNETRADIOSERVER_TCPSERVER_H
