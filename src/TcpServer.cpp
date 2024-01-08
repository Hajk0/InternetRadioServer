//
// Created by hajk0 on 12/23/23.
//

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "../include/TcpServer.h"

int TcpServer::socketSetUp() {
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    this->serverAddress.sin_family = AF_INET;
    this->serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->serverAddress.sin_port = htons(PORT);

    if (bind(this->serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding to port\n";
        close(serverSocket);
        return 1;
    }

    if (listen(this->serverSocket, 5) == -1) {
        std::cerr << "Error listening for connections\n";
        close(serverSocket);
        return 1;
    }

    this->epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Error creating epoll file descriptor\n";
        close(serverSocket);
        return 1;
    }

    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    return 0;
}

int TcpServer::epollSetUp() {
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        std::cerr << "Error adding server socket to epoll\n";
        close(epollFd);
        close(serverSocket);
        return 1;
    }

    std::cout << "Server started on port " << PORT << std::endl;
    return 0;
}

void TcpServer::newConnection() {
    sockaddr_in clientAddress{};
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (sockaddr*) &clientAddress, &clientAddressLen);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection\n";
    } else {
        std::cout << "New connection from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;

        event.events = EPOLLIN | EPOLLET;
        event.data.fd = clientSocket;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);
    }
}

void TcpServer::existingConnection(int i) {
    char buffer[1024];
    ssize_t bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cout << "Connection closed\n";
        epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        close(events[i].data.fd);
    } else {
        std::cout << "Received data: " << std::string(buffer, bytesRead) << std::endl;
    }
}

int TcpServer::epollWait() {
    return epoll_wait(epollFd, events, MAX_EVENTS, -1);
}

epoll_event *TcpServer::getEvents() {
    return events;
}

int TcpServer::getServerSocket() const {
    return serverSocket;
}
