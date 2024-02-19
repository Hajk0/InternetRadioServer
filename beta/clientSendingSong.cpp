//
// Created by hajk0 on 2/11/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 12344

using namespace std;

int main() {
    const string inputFileName = "POLAND-LILYACHTY.wav";
    const int chunkSize = 1024 * 1024; // 1 MB chunk size

    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Unable to open file: " << inputFileName << endl;
        return 1;
    }

    // Get the size of the input file
    inputFile.seekg(0, ios::end);
    streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, ios::beg);

    // Calculate the number of parts
    int numParts = ((int)fileSize + chunkSize - 1) / chunkSize;

    // Networking
    int clientSocket;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    int serverSocket = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (serverSocket == -1) {
        perror("Błąd podczas łączenia z serwerem");
        close(clientSocket);
        return 1;
    }


    cout << "Połączono z serwerem." << endl;

    // Read file in chunks and write each chunk to a separate file
    char* buffer = new char[chunkSize];
    for (int partIndex = 0; partIndex < numParts; ++partIndex) {
        // Determine the chunk size for the current part
        int currentChunkSize = min(chunkSize, static_cast<int>(fileSize - inputFile.tellg()));

        // Read chunk from input file
        inputFile.read(buffer, currentChunkSize);

        if (send(clientSocket, buffer, currentChunkSize, 0) == -1) {
            perror("Błąd podczas wysyłania danych do serwera");
            close(clientSocket);
            close(clientSocket);
            delete[] buffer;
            return 1;
        }

    }
    close(clientSocket);
    close(serverSocket);
    inputFile.close();
    delete[] buffer;

    cout << "File split into " << numParts << " parts." << endl;
    cout << "Connection closed" << endl;

    return 0;
}
