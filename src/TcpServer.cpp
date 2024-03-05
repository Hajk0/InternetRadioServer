//
// Created by hajk0 on 12/23/23.
//

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <fstream>
#include <thread>
#include "../include/TcpServer.h"
#include "../include/Library.h"

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

        stream.start(inet_ntoa(clientAddress.sin_addr), clientSocket);
        if (clientCounter == 0) {
            clientCounter++;
            this->streamToClients = std::thread(&Stream::playQueue, std::ref(this->stream)); // stream queue
            //std::thread streamToClients(&Stream::playQueue, this->stream);
            streamToClients.detach(); // zmien na join w kończeniu połączenia
        }
    }
}

void TcpServer::existingConnection(int i) {
    char buffer[32];
    ssize_t bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cout << "Connection closed\n";
        stream.deleteClient(events[i].data.fd);
        epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        close(events[i].data.fd);
    } else {
        std::cout << "Received data: " << std::string(buffer, bytesRead) << std::endl;
        if (std::string(buffer, 8) == "ADD SONG") {
            std::cout << "ADD SONG requested" << std::endl;
            std::string fileName(buffer + 9, buffer + bytesRead - 1);
            fileName += ".wav";
            std::cout << fileName << std::endl;

            this->library.activeFileName = "../res/" + fileName;
            std::thread addSongThread(&Library::addSong, this->library);
            //std::thread addSongThread([this]() {
            //    this->library.addSong();
            //});
            addSongThread.detach();
        } else if (std::string(buffer, 9) == "SKIP SONG") {
            std::cout << "SKIP SONG requested" << std::endl;
            stream.skipSong();
        } else if (std::string(buffer, 9) == "ADD QUEUE") {
            std::cout << "ADD QUEUE requested" << std::endl;
            std::string fileName(buffer + 10, buffer + bytesRead - 1);
            fileName += ".wav";
            std::cout << fileName << std::endl;

            stream.addToQueue(fileName);
        } else if (std::string(buffer, 10) == "SHOW SONGS") {
            vector<string> songs = library.showSongs();
            this->sendAvaiableSongs(events[i].data.fd, songs);
        } else if (std::string(buffer, 10) == "SHOW QUEUE") {
            // TODO(send songs in queue to client)
            stream.showQueue();
        }
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

TcpServer::TcpServer() {
    library = Library();
    clientCounter = 0;
}

int TcpServer::sendAvaiableSongs(int sock, vector<string> songs) {
    string message;
    for (const auto& str : songs) {
        message += str;
        message += ";";
    }

    // Wyślij długość wiadomości jako nagłówek
    int msg_length = message.length(); // htons(message.length());
    cout << message << ": " << msg_length << endl;
    if (send(sock, &msg_length, sizeof(msg_length), 0) == -1) {
        cerr << "Error sending message length" << endl;
        return -1;
    }

    // Wyślij zawartość wektora
    if (send(sock, message.c_str(), message.length(), 0) == -1) {
        cerr << "Error sending message" << endl;
        return -1;
    }

    return 0;
}
