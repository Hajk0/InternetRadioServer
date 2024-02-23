//
// Created by hajk0 on 1/27/24.
//

#include "../include/Library.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

#define PORT 12344

using namespace std;

int Library::addSong() {
    // song name send with request to add new song
    const string outputFileName = activeFileName;
    const int chunkSize = 1024 * 1024; // 1 MB chunk size

    // Read file in chunks and write each chunk to a separate file
    char* buffer = new char[chunkSize];

    // Networking
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Błąd podczas powiązywania adresu z gniazdem");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 1) == -1) {
        perror("Błąd podczas nasłuchiwania na gnieździe");
        close(serverSocket);
        return 1;
    }

    cout << "Serwer nasłuchuje na porcie " << PORT << "..." << endl;

    int clientSocket;
    struct sockaddr_in clientAddr{};
    socklen_t clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

    cout << "Połączenie zaakceptowane." << endl;

    ofstream outputFile(outputFileName, ios::binary);
    if (!outputFile.is_open()) {
        cerr << "Unable to create output file: " << outputFileName << endl;
        delete[] buffer;
        return 1;
    }

    int bytesReceived;
    // std::cout << "zapisuję do pliku: " << fileName << std::endl;
    while ((bytesReceived = recv(clientSocket, buffer, chunkSize, 0)) > 0) {
        outputFile.write(buffer, bytesReceived);
    }

    outputFile.close();
    close(clientSocket);
    close(serverSocket);

    delete[] buffer;

    return 0;
}
