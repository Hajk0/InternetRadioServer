//
// Created by hajk0 on 2/21/24.
//
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <SDL2/SDL.h>
#include <cstdint>

#define PORT 12343
#define SECONDS_BUFFERED 5 // Liczba sekund buforowanych przed rozpoczęciem odtwarzania

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

std::queue<std::vector<char>> audioQueue; // Kolejka fragmentów muzyki
std::mutex audioMutex;
std::condition_variable audioCV;
bool quit = false;
int queueCounter = 0; // Licznik ilości chunków dodanych do kolejki
// SDL_AudioSpec wavSpec;
// SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);

void playAudio() {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec wavSpec;
    wavSpec.freq = 44100;
    wavSpec.format = AUDIO_S16SYS;
    wavSpec.channels = 2;
    wavSpec.samples = 4096;
    wavSpec.callback = nullptr;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (device == 0) {
        std::cerr << "Błąd podczas otwierania urządzenia audio: " << SDL_GetError() << std::endl;
        return;
    }

    while (!quit) {
        std::unique_lock<std::mutex> lock(audioMutex);
        audioCV.wait(lock, [] { return !audioQueue.empty() || quit; });

        if (!quit) {
            std::vector<char> buffer = audioQueue.front();
            audioQueue.pop();
            size_t bufferSize = buffer.size();
            SDL_QueueAudio(device, buffer.data(), bufferSize);
            lock.unlock();
            SDL_PauseAudioDevice(device, 0);
            SDL_Delay(100); // Daj czas na odtworzenie dźwięku
            queueCounter--;
            std::cout << "Queue counter: " << queueCounter << std::endl;
            std::cout << "Queue size: " << SDL_GetQueuedAudioSize(device) << std::endl;
        }
    }

    SDL_CloseAudioDevice(device);
    SDL_Quit();
}

int main() {
    std::thread audioThread(playAudio);

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
        // Przesyłanie gotowości
        while (SDL_GetQueuedAudioSize(device) > 5000000) { //
            sleep(1);
        }
        bool ready = true;
        if (send(clientSocket, &ready, sizeof(ready), 0) == -1) {
            perror("Błąd podczas wysyłania danych do serwera");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // Odbieranie rozmiaru chunka
        int currentChunkSize;
        int bytesReceived = recv(clientSocket, &currentChunkSize, sizeof(currentChunkSize), 0);
        if (bytesReceived <= 0) {
            break; // Zakończ jeśli nie można odebrać danych
        }

        // Odbieranie danych i dodawanie ich do kolejki
        std::vector<char> buffer(currentChunkSize);
        int totalBytesReceived = 0;
        while (totalBytesReceived < currentChunkSize) {
            int bytesRead = recv(clientSocket, buffer.data() + totalBytesReceived, currentChunkSize - totalBytesReceived, 0);
            if (bytesRead <= 0) {
                std::cerr << "Błąd podczas odbierania danych" << std::endl;
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
            totalBytesReceived += bytesRead;
        }
        std::lock_guard<std::mutex> lock(audioMutex);
        audioQueue.push(buffer);

        // Sprawdź, czy bufor zawiera wystarczająco dużo danych na kilka sekund muzyki // Źle
        size_t bytesPerSecond = 44100 * 2 * 16 / 8; // sampleRate * numChannels * bitsPerSample / 8
        size_t secondsBuffered = audioQueue.size() * currentChunkSize / bytesPerSecond;
        //if (secondsBuffered >= SECONDS_BUFFERED) {
        audioCV.notify_one(); // Odtwórz muzykę
        //}
        queueCounter++;

        std::cout << "Odebrano część: " << partIndex++ << std::endl;
    }

    // Zamknięcie gniazda
    close(clientSocket);
    close(serverSocket);

    std::cout << "Zakończono połączenie" << std::endl;

    quit = true;
    audioCV.notify_one();
    audioThread.join();

    return 0;
}


/*#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
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

std::vector<char> audioBuffer;
std::mutex audioMutex;
std::condition_variable audioCV;
bool quit = false;
bool readyToSend = false; // flaga oznaczająca gotowość do wysłania

void playAudio() {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec wavSpec;
    wavSpec.freq = 44100;
    wavSpec.format = AUDIO_S16SYS;
    wavSpec.channels = 2;
    wavSpec.samples = 4096;
    wavSpec.callback = nullptr;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (device == 0) {
        std::cerr << "Błąd podczas otwierania urządzenia audio: " << SDL_GetError() << std::endl;
        return;
    }

    while (!quit) {
        std::unique_lock<std::mutex> lock(audioMutex);
        audioCV.wait(lock, [] { return readyToSend || quit; }); // czekaj na flagę gotowości lub zakończenie

        if (!quit) {
            size_t bufferSize = audioBuffer.size();
            SDL_QueueAudio(device, audioBuffer.data(), bufferSize);
            audioBuffer.clear();
            lock.unlock();
            SDL_PauseAudioDevice(device, 0);
            SDL_Delay(100); // Daj czas na odtworzenie dźwięku
            readyToSend = false; // Zresetuj flagę gotowości
        }
    }

    SDL_CloseAudioDevice(device);
    SDL_Quit();
}

int main() {
    std::thread audioThread(playAudio);

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
            std::lock_guard<std::mutex> lock(audioMutex);
            audioBuffer.insert(audioBuffer.end(), buffer, buffer + bytesRead);
            totalBytesReceived += bytesRead;
        }

        // Dodanie nagłówka do bufora
        const uint32_t sampleRate = 44100; // Częstotliwość próbkowania
        const uint16_t numChannels = 2; // Liczba kanałów
        const uint16_t bitsPerSample = 16; // Liczba bitów na próbkę
        writeWavHeader(audioBuffer, currentChunkSize, sampleRate, numChannels, bitsPerSample);

        std::cout << "Odebrano część: " << partIndex++ << std::endl;
        readyToSend = true; // Ustaw flagę gotowości do wysłania
        audioCV.notify_one();
    }

    // Zamknięcie gniazda
    close(clientSocket);
    close(serverSocket);

    std::cout << "Zakończono połączenie" << std::endl;

    quit = true;
    audioCV.notify_one();
    audioThread.join();

    return 0;
}

*/