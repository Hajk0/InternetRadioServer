//
// Created by hajk0 on 2/21/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

#define PORT 12343

using namespace std;

int main() {
    // Networking
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddr.sin_port = htons(PORT);

    int connectResult = connect(serverSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if (connectResult == -1) {
        perror("Błąd podczas łączenia z serwerem");
        close(serverSocket);
        return 1;
    }

    cout << "Połączono z serwerem." << endl;

    const int chunkSize = 1024 * 1024; // 1 MB chunk size

    const string inputFileName = "POLAND-LILYACHTY.wav";
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
    int numParts = (fileSize + chunkSize - 1) / chunkSize;

    // Read file in chunks and write each chunk to a separate file
    char* buffer = new char[chunkSize];

    for (int partIndex = 0; partIndex < numParts; ++partIndex) {
        // Odbieranie gotowości
        bool ready = false;
        int bytesReceived = recv(serverSocket, &ready, sizeof(ready), 0);
        if (bytesReceived <= 0) {
            break;
        }
        cout << "Ready: " << ready << endl;

        // Określ rozmiar chunka dla aktualnej części
        int currentChunkSize = min(chunkSize, static_cast<int>(fileSize - inputFile.tellg()));

        // Czekaj na gotowość serwera do odebrania danych
        cout << "Czekam na gotowość serwera..." << endl;
        sleep(1); // Symulacja oczekiwania

        // Read chunk from input file
        inputFile.read(buffer, currentChunkSize);
        if (send(serverSocket, &currentChunkSize, sizeof(currentChunkSize), 0) == -1) {
            perror("Błąd podczas wysyłania danych do klienta");
            close(serverSocket);
            delete[] buffer;
            return 1;
        }
        cout << "chunkSize: " << currentChunkSize << endl;

        if (send(serverSocket, buffer, currentChunkSize, 0) == -1) {
            perror("Błąd podczas wysyłania danych do klienta");
            close(serverSocket);
            delete[] buffer;
            return 1;
        }
        sleep(1);
        cout << partIndex << endl;
    }

    sleep(60);
    close(serverSocket);
    inputFile.close();
    delete[] buffer;

    return 0;
}

/*
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

#define PORT 12343

using namespace std;

int main() {
    // Networking
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddr.sin_port = htons(PORT);

    int connectResult = connect(serverSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if (connectResult == -1) {
        perror("Błąd podczas łączenia z serwerem");
        close(serverSocket);
        return 1;
    }

    cout << "Połączono z serwerem." << endl;

    const int chunkSize = 1024 * 1024; // 1 MB chunk size

    const string inputFileName = "POLAND-LILYACHTY.wav";
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
    int numParts = (fileSize + chunkSize - 1) / chunkSize;

    // Read file in chunks and write each chunk to a separate file
    char* buffer = new char[chunkSize];

    for (int partIndex = 0; partIndex < numParts; ++partIndex) {
        // Określ rozmiar chunka dla aktualnej części
        int currentChunkSize = min(chunkSize, static_cast<int>(fileSize - inputFile.tellg()));

        // Czekaj na gotowość serwera do odebrania danych
        cout << "Czekam na gotowość serwera..." << endl;
        sleep(1); // Symulacja oczekiwania

        // Read chunk from input file
        inputFile.read(buffer, currentChunkSize);
        if (send(serverSocket, &currentChunkSize, sizeof(currentChunkSize), 0) == -1) {
            perror("Błąd podczas wysyłania danych do klienta");
            close(serverSocket);
            delete[] buffer;
            return 1;
        }
        cout << "chunkSize: " << currentChunkSize << endl;

        if (send(serverSocket, buffer, currentChunkSize, 0) == -1) {
            perror("Błąd podczas wysyłania danych do klienta");
            close(serverSocket);
            delete[] buffer;
            return 1;
        }
        sleep(1);
        cout << partIndex << endl;
    }

    close(serverSocket);
    inputFile.close();
    delete[] buffer;

    return 0;
}
*/