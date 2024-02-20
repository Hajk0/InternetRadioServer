//
// Created by hajk0 on 2/20/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdint>

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
    // Tworzenie gniazda
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Błąd podczas tworzenia gniazda");
        return 1;
    }

    // Konfiguracja adresu serwera
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Powiązanie adresu z gniazdem
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Błąd podczas powiązywania adresu z gniazdem");
        close(serverSocket);
        return 1;
    }

    // Nasłuchiwanie na gnieździe
    if (listen(serverSocket, 1) == -1) {
        perror("Błąd podczas nasłuchiwania na gnieździe");
        close(serverSocket);
        return 1;
    }

    std::cout << "Serwer nasłuchuje na porcie " << PORT << "..." << std::endl;

    // Akceptowanie połączenia
    int clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == -1) {
        perror("Błąd podczas akceptowania połączenia");
        close(serverSocket);
        return 1;
    }

    std::cout << "Połączenie zaakceptowane." << std::endl;

    // Odbieranie plików
    int partIndex = 0;
    while (true) {
        // Odbieranie rozmiaru chunka
        int currentChunkSize;
        int bytesReceived = recv(clientSocket, &currentChunkSize, sizeof(currentChunkSize), 0);
        if (bytesReceived <= 0) {
            break; // Zakończ jeśli nie można odebrać danych
        }

        // Tworzenie pliku wyjściowego
        std::string outputFileName = "part_" + std::to_string(partIndex++) + ".wav";
        std::ofstream outputFile(outputFileName, std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "Nie można utworzyć pliku wyjściowego: " << outputFileName << std::endl;
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // Odbieranie danych i zapisywanie ich do pliku i dodanie nagłówka
        const uint32_t sampleRate = 44100; // Częstotliwość próbkowania
        const uint16_t numChannels = 2; // Liczba kanałów
        const uint16_t bitsPerSample = 16; // Liczba bitów na próbkę
        writeWavHeader(outputFile, currentChunkSize, sampleRate, numChannels, bitsPerSample);
        int totalBytesReceived = 0;
        while (totalBytesReceived < currentChunkSize) {
            char buffer[1024];
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                std::cerr << "Błąd podczas odbierania danych" << std::endl;
                outputFile.close();
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
            outputFile.write(buffer, bytesRead);
            totalBytesReceived += bytesRead;
        }

        std::cout << "Odebrano plik: " << outputFileName << std::endl;
        outputFile.close();
    }

    // Zamknięcie gniazda
    close(clientSocket);
    close(serverSocket);

    std::cout << "Zakończono połączenie" << std::endl;

    return 0;
}
