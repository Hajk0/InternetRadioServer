#ifndef INTERNETRADIOSERVER_SERVER_H
#define INTERNETRADIOSERVER_SERVER_H


#include "TcpServer.h"

class Server {

    TcpServer tcpServer;
    bool closeServer = false;

public:
    int runServer();
};


#endif //INTERNETRADIOSERVER_SERVER_H
