//
// Created by hajk0 on 2/20/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 12343

using namespace std;

// Struktura nagłówka WAV
#pragma pack(push, 1)
struct WavHeader {
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char subchunk2ID[4];
    uint32_t subchunk2Size;
};
#pragma pack(pop)

void writeWavHeader(ofstream& file, uint32_t dataSize, uint32_t sampleRate, uint16_t numChannels, uint16_t bitsPerSample) {
    // Utwórz nagłówek WAV
    WavHeader header;
    strncpy(header.chunkID, "RIFF", 4);
    strncpy(header.format, "WAVE", 4);
    strncpy(header.subchunk1ID, "fmt ", 4);
    header.subchunk1Size = 16;
    header.audioFormat = 1; // PCM
    header.numChannels = numChannels;
    header.sampleRate = sampleRate;
    header.bitsPerSample = bitsPerSample;
    header.byteRate = sampleRate * numChannels * bitsPerSample / 8;
    header.blockAlign = numChannels * bitsPerSample / 8;
    strncpy(header.subchunk2ID, "data", 4);
    header.subchunk2Size = dataSize;

    // Zapisz nagłówek do pliku
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
}

int main() {
    // Networking
    int clientSocket;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }
    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(PORT);

    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
        perror("Błąd podczas powiązywania adresu z gniazdem");
        close(clientSocket);
        return 1;
    }

    if (listen(clientSocket, 1) == -1) {
        perror("Błąd podczas nasłuchiwania na gnieździe");
        close(clientSocket);
        return 1;
    }

    cout << "Klient nasłuchuje na porcie " << PORT << "..." << endl;

    int serverSocket;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrSize = sizeof(serverAddr);
    serverSocket = accept(clientSocket, (struct sockaddr*)&serverAddr, &serverAddrSize);

    cout << "Połączenie zaakceptowane." << endl;


    const string inputFileName = "POLAND-LILYACHTY.wav";
    const string outputPrefix = "part_"; // Prefix dla nazw plików wyjściowych
    const int chunkSize = 1024 * 1024; // Rozmiar chunka 1 MB
    const uint32_t sampleRate = 44100; // Częstotliwość próbkowania
    const uint16_t numChannels = 2; // Liczba kanałów
    const uint16_t bitsPerSample = 16; // Liczba bitów na próbkę

    // Odczytuj plik w chunkach i zapisuj każdy chunk do osobnego pliku wyjściowego
    char* buffer = new char[chunkSize];
    string fileNamePrefix = "siemano";
    int i = 0;
    while (i < 15) {
        ostringstream outputFileName;
        outputFileName << fileNamePrefix << i << ".wav";
        cout << outputFileName.str() << endl;
        ofstream outputFile(outputFileName.str(), ios::binary);
        if (!outputFile.is_open()) {
            cerr << "Unable to create output file: " << outputFileName.str() << endl;
            delete[] buffer;
            return 1;
        }

        int bytesReceived, totalBytesReceived = 0;

        int fileSize;
        bytesReceived = recv(serverSocket, &fileSize, sizeof(fileSize), 0);
        cout << "fileSize: " << fileSize << endl;

        writeWavHeader(outputFile, chunkSize, sampleRate, numChannels, bitsPerSample);
        // (bytesReceived = recv(serverSocket, buffer, chunkSize, 0)) > 0 &&
        while (totalBytesReceived < fileSize) {
            outputFile.write(buffer, bytesReceived);
            totalBytesReceived += bytesReceived;
        }

        outputFile.close();
        i++;
    }

    delete[] buffer;

    return 0;
}
