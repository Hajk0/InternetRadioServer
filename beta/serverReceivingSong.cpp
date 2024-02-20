//
// Created by hajk0 on 2/11/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 12345

using namespace std;

int main() {
    // song name send with request to add new song
    const string outputFileName = "outputSong.wav";
    const int chunkSize = 1024 * 1024; // 1 MB chunk size

    // Read file in chunks and write each chunk to a separate file
    char* buffer = new char[chunkSize];

    // Networking
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in serverAddr;
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

    while (true) {
        int clientSocket;
        struct sockaddr_in clientAddr;
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
        while ((bytesReceived = recv(clientSocket, buffer, chunkSize, 0)) > 0) {
            outputFile.write(buffer, bytesReceived);
        }

        outputFile.close();
    }

    delete[] buffer;

    return 0;
}
