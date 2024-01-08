#include "../include/Server.h"
#include "../include/TcpServer.h"

int Server::runServer() {
    TcpServer tcpServer = TcpServer();
    if (tcpServer.socketSetUp() != 0) {
        return 1;
    }

    if (tcpServer.epollSetUp() != 0) {
        return 1;
    }

    while (true) {
        int numEvents = tcpServer.epollWait();
        for (int i = 0; i < numEvents; i++) {
            if (tcpServer.getEvents()[i].data.fd == tcpServer.getServerSocket()) {
                tcpServer.newConnection();
            } else {
                tcpServer.existingConnection(i);
            }
        }
    }
}
