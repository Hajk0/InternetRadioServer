//
// Created by hajk0 on 1/29/24.
//

#include <csignal>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <SDL2/SDL.h>
#include <thread>
#include <mutex>
#include "../include/Stream.h"

mutex queueMutex;

int Stream::streamSong(string songName) {
    this->skipFlag = false;

    const string inputFileName = "../res/" + songName;
    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Nie można otworzyć pliku: " << inputFileName << endl;
        return 1;
    }

    // Pobierz rozmiar pliku wejściowego
    int headerSize = 44;
    inputFile.seekg(0, ios::end);
    uint32_t fileSize = (int)inputFile.tellg() - headerSize;
    inputFile.seekg(44, ios::beg);

    // Oblicz liczbę części
    long numParts = ((long)fileSize + chunkSize - 1) / chunkSize;
    cout << "numParts: " << numParts << " fileSize: " << fileSize << endl;

    // Read file in chunks and write each chunk to a separate file
    char* buffer = new char[chunkSize];

    for (int partIndex = 0; partIndex < numParts; ++partIndex) {
        // Określ rozmiar chunka dla aktualnej części
        int currentChunkSize = min(chunkSize, static_cast<int>(fileSize - inputFile.tellg()));

        // Read chunk from input file
        inputFile.read(buffer, currentChunkSize);
        std::cout << "clients: " << clients[0].socket << std::endl;
        for (auto &client : clients) {
            if (send(client.socket, &currentChunkSize, sizeof(currentChunkSize), 0) == -1) {
                perror("Błąd podczas wysyłania danych do klienta");
                close(client.socket);
                delete[] buffer;
                return 1;
            }

            if (send(client.socket, buffer, currentChunkSize, 0) == -1) {
                perror("Błąd podczas wysyłania danych do klienta");
                close(client.socket);
                delete[] buffer;
                return 1;
            }
        }

        double chunkDuration = ((double)currentChunkSize / (44100.0 * 4.0)) * 990; // 1000
        // Poczekaj odpowiedni czas przed wysłaniem kolejnej części
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(chunkDuration)));
        cout << "Część: " << partIndex << " odtworzona." << endl;

        if (this->skipFlag == true)
            break;
    }

    inputFile.close();
    delete[] buffer;

    return 0;

}

int Stream::start(const char *ip, int mainSocket) {
    // Networking
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(ip);
    clientAddr.sin_port = htons(PORT);

    int connectResult = connect(serverSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if (connectResult == -1) {
        perror("Błąd podczas łączenia z serwerem");
        close(serverSocket);
        return 1;
    }

    cout << "Połączono z serwerem." << endl;

    // Bind main socket to streaming socket
    ClientInfo clientInfo = {serverSocket, clientAddr, mainSocket};
    clients.push_back(clientInfo);

    return 0;
}

int Stream::end() {
    for (auto &client : clients) {
        close(client.socket);
    }
    return 0;
}

int Stream::playQueue() {
    while(true) {
        while (true) {
            queueMutex.lock();
            bool empty = songsQueue.empty();
            queueMutex.unlock();
            if (!empty)
                break;
            std::cout << "Queue is empty" << std::endl;
            sleep(5);
        }
        queueMutex.lock();
        //this->streamSong(songsQueue.front());
        std::string songName = songsQueue.front();
        songsQueue.pop();
        queueMutex.unlock();
        this->streamSong(songName);
    }
    return 0;
}

int Stream::addToQueue(string songName) {
    queueMutex.lock();
    songsQueue.push(songName);
    queueMutex.unlock();
    return 0;
}

void Stream::skipSong() {
    this->skipFlag = true;
}

void Stream::deleteClient(int clientSocket) { // chwilowo do testowania
    for (auto it = this->clients.begin(); it != this->clients.end();) {
        if (it->mainSocket == clientSocket) {
            close(it->socket);
            it = this->clients.erase(it);
        } else {
            ++it;
        }
    }
}

void Stream::showQueue() {
    std::queue<string> copyQueue = this->songsQueue;

    while (!copyQueue.empty()) {
        string element = copyQueue.front();
        cout << "Next song: " << element << endl;

        // Wykonaj operacje na elemencie
        copyQueue.pop();
    }
}
