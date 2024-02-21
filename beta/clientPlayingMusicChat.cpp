//
// Created by hajk0 on 2/21/24.
//
#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <SDL2/SDL.h>
#include <cstdint>

#define PORT 12343

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

void writeWavHeader(std::vector<char>& buffer, uint32_t dataSize, uint32_t sampleRate, uint16_t numChannels, uint16_t bitsPerSample) {
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

    // Dodaj nagłówek do bufora
    buffer.insert(buffer.begin(), reinterpret_cast<char*>(&header), reinterpret_cast<char*>(&header) + sizeof(header));
}

int main() {
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        std::cerr << "Błąd podczas inicjalizacji SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

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
    std::vector<char> audioBuffer;
    int partIndex = 0;
    while (true) {
        // Odbieranie rozmiaru chunka
        int currentChunkSize;
        int bytesReceived = recv(clientSocket, &currentChunkSize, sizeof(currentChunkSize), 0);
        if (bytesReceived <= 0) {
            break; // Zakończ jeśli nie można odebrać danych
        }

        // Odbieranie danych i dodawanie ich do bufora
        int totalBytesReceived = 0;
        while (totalBytesReceived < currentChunkSize) {
            char buffer[1024];
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                std::cerr << "Błąd podczas odbierania danych" << std::endl;
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
            audioBuffer.insert(audioBuffer.end(), buffer, buffer + bytesRead);
            totalBytesReceived += bytesRead;
        }

        // Dodanie nagłówka do bufora
        const uint32_t sampleRate = 44100; // Częstotliwość próbkowania
        const uint16_t numChannels = 2; // Liczba kanałów
        const uint16_t bitsPerSample = 16; // Liczba bitów na próbkę
        writeWavHeader(audioBuffer, currentChunkSize, sampleRate, numChannels, bitsPerSample);

        std::cout << "Odebrano część: " << partIndex++ << std::endl;
    }

    // Odtwarzanie dźwięku z bufora przy użyciu SDL
    SDL_AudioSpec wavSpec;
    wavSpec.freq = 44100;
    wavSpec.format = AUDIO_S16SYS;
    wavSpec.channels = 2;
    wavSpec.samples = 4096;
    wavSpec.callback = nullptr;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (device == 0) {
        std::cerr << "Błąd podczas otwierania urządzenia audio: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_QueueAudio(device, audioBuffer.data(), audioBuffer.size());
    SDL_PauseAudioDevice(device, 0);

    // Oczekiwanie na zakończenie odtwarzania
    while (SDL_GetQueuedAudioSize(device) > 0) {
        SDL_Delay(1000);
    }

    // Zamknięcie gniazda
    close(clientSocket);
    close(serverSocket);

    std::cout << "Zakończono połączenie" << std::endl;

    // Zwolnienie urządzenia audio
    SDL_CloseAudioDevice(device);
    SDL_Quit();

    return 0;
}