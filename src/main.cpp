#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <thread>
#include <chrono>
#include "../include/Server.h"
#include <netinet/in.h>



int sendBroadcast(UdpServer udpServer) {
    if (udpServer.setUp() != 0) {
        return 1;
    }

    /*if (udpServer.epollSetUp() != 0) {
        return 1;
    }*/

    std::cout << "Udp server started on port 12344" << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        const char* message = "Broadcast message from server\n";
        sockaddr_in clientAddress{};
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_port = htons(udpServer.getPort());
        clientAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

        sendto(udpServer.getUdpSocket(), message, strlen(message), 0, (sockaddr*) &clientAddress, sizeof(clientAddress));
    }
}

int main() {
    // UdpServer udpServer = UdpServer();
    // std::thread broadcastThread(sendBroadcast, udpServer);
    Server server = Server();
    server.runServer();
    return 0;
}

/*void sendBroadcast(int udpSocket) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        const char* message = "Broadcast message from server";
        sockaddr_in clientAddress{};
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_port = htons(PORT_UDP);
        clientAddress.sin_addr.s_addr = INADDR_BROADCAST;

        sendto(udpSocket, message, strlen(message), 0, reinterpret_cast<struct sockaddr*>(&clientAddress), sizeof(clientAddress));
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT_TCP);

    if (bind(serverSocket, (struct sockaddr*)(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding to port\n";
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening for connections\n";
        close(serverSocket);
        return 1;
    }

    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Error creating epoll file descriptor\n";
        close(serverSocket);
        return 1;
    }

    epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = serverSocket;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        std::cerr << "Error adding server socket to epoll\n";
        close(epollFd);
        close(serverSocket);
        return 1;
    }

    // UDP socket setup
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Error creating UDP socket\n";
        close(epollFd);
        close(serverSocket);
        return 1;
    }

    sockaddr_in udpServerAddress{};
    udpServerAddress.sin_family = AF_INET;
    udpServerAddress.sin_addr.s_addr = INADDR_ANY;
    udpServerAddress.sin_port = htons(PORT_UDP);

    if (bind(udpSocket, reinterpret_cast<struct sockaddr*>(&udpServerAddress), sizeof(udpServerAddress)) == -1) {
        std::cerr << "Error binding to UDP port\n";
        close(epollFd);
        close(serverSocket);
        close(udpSocket);
        return 1;
    }

    // Add UDP socket to epoll
    epoll_event udpEvent;
    udpEvent.events = EPOLLIN;
    udpEvent.data.fd = udpSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, udpSocket, &udpEvent) == -1) {
        std::cerr << "Error adding UDP socket to epoll\n";
        close(epollFd);
        close(serverSocket);
        close(udpSocket);
        return 1;
    }

    std::cout << "Server started on TCP port " << PORT_TCP << " and UDP port " << PORT_UDP << std::endl;

    // Start the broadcast thread
    std::thread broadcastThread(sendBroadcast, udpSocket);

    while (true) {
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == serverSocket) {
                // New connection
                sockaddr_in clientAddress{};
                socklen_t clientAddressLen = sizeof(clientAddress);
                int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressLen);
                if (clientSocket == -1) {
                    std::cerr << "Error accepting connection\n";
                } else {
                    std::cout << "New connection from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;

                    event.events = EPOLLIN | EPOLLET;  // Edge-triggered
                    event.data.fd = clientSocket;
                    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);
                }
            } else if (events[i].data.fd == udpSocket) {
                // UDP data
                char buffer[MAX_BUFFER_SIZE];
                sockaddr_in udpClientAddress{};
                socklen_t udpClientAddressLen = sizeof(udpClientAddress);
                ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr*>(&udpClientAddress), &udpClientAddressLen);
                if (bytesRead <= 0) {
                    std::cerr << "Error receiving UDP data\n";
                } else {
                    std::cout << "Received UDP data from " << inet_ntoa(udpClientAddress.sin_addr) << ":" << ntohs(udpClientAddress.sin_port) << ": " << std::string(buffer, bytesRead) << std::endl;
                }
            } else {
                // Existing connection
                char buffer[MAX_BUFFER_SIZE];
                ssize_t bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    std::cout << "Connection closed\n";
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                    close(events[i].data.fd);
                } else {
                    std::cout << "Received data: " << std::string(buffer, bytesRead) << std::endl;
                }
            }
        }
    }

    // Cleanup
    broadcastThread.join();
    close(epollFd);
    close(serverSocket);
    close(udpSocket);

    return 0;
}*/
