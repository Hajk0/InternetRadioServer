//
// Created by hajk0 on 1/29/24.
//

#ifndef INTERNETRADIOSERVER_STREAM_H
#define INTERNETRADIOSERVER_STREAM_H

#include <vector>
#include <string>
#include <netinet/in.h>

class Stream {
    std::vector<std::string> ipList;
    std::vector<int> clientSockets;
    bool closeServer = false;
    int STREAM_PORT = 12344;
    int sock;
    struct sockaddr_in servAddr, clientAddr;

    int acceptClientSock();

public:
    void addIp(std::string ip);
    void removeIp(std::string ip);
    int streamSetUp();
    int stream();
    void streamEnd();
};


#endif //INTERNETRADIOSERVER_STREAM_H
