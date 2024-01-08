#ifndef INTERNETRADIOSERVER_SERVER_H
#define INTERNETRADIOSERVER_SERVER_H


#include "TcpServer.h"
#include "UdpServer.h"

class Server {

    TcpServer tcpServer;
    UdpServer udpServer;

public:
    int runServer();
};


#endif //INTERNETRADIOSERVER_SERVER_H
