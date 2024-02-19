//
// Created by hajk0 on 1/29/24.
//

#include <csignal>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include "../include/Stream.h"

void Stream::addIp(std::string ip) {
    ipList.push_back(ip);
    acceptClientSock();
}

void Stream::removeIp(std::string ip) {
    for (int i = 0; i < ipList.size(); i++) {
        if (ip == ipList[i]) {
            ipList.erase(ipList.begin() + i);
            close(clientSockets[i]);
            clientSockets.erase(clientSockets.begin() + i);
        }
    }

}

int Stream::stream() {

    while (!closeServer) {
        sleep(5);
        for (auto client : clientSockets) {
            char buf[16];
            int n = sprintf(buf, "Siemano %d", client);

            write(client, buf, n);
        }
    }
    close(sock);

    return 0;
}

int Stream::acceptClientSock() {
    socklen_t tmp = sizeof(struct sockaddr);
    int clientSocket = accept(sock, (struct sockaddr*)&clientAddr, &tmp);
    if (clientSocket < 0) {
        std::cout << "Can't create a connection's socket." << std::endl;
        return 1; // can cause errors
    }
    clientSockets.push_back(clientSocket);
    return 0;
}

int Stream::streamSetUp() {
    int sBind, sListen;
    int n = 1;

    memset(&servAddr, 0, sizeof(sockaddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(STREAM_PORT);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cout << "Can't create a socket." << std::endl;
        return 1;
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof(n));

    sBind = bind(sock, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));
    if (sBind < 0) {
        std::cout << "Can't bind a name to a socket." << std::endl;
        return 1;
    }

    sListen = listen(sock, 10);
    if (sListen < 0) {
        std::cout << "Can't set queue size." << std::endl;
        return 1;
    }
    return 0;
}

void Stream::streamEnd() {
    closeServer = true;
}

