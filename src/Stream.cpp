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
#include "../include/Stream.h"

int Stream::streamSong(string songName) {

////////////////////
    const string inputFileName = songName;
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

    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        cerr << "Nie można zainicjować SDL: " << SDL_GetError() << endl;
        delete[] buffer;
        return 1;
    }

    SDL_AudioSpec wavSpec;
    wavSpec.freq = 44100;
    wavSpec.format = AUDIO_S16SYS;
    wavSpec.channels = 2;
    wavSpec.samples = 4096;
    wavSpec.callback = nullptr;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (device == 0) {
        cerr << "Błąd podczas otwierania urządzenia audio: " << SDL_GetError() << endl;
        delete[] buffer;
        SDL_Quit();
        return 1;
    }



    SDL_PauseAudioDevice(device, 0); // Start audio playback

    for (int partIndex = 0; partIndex < numParts; ++partIndex) {
        // Określ rozmiar chunka dla aktualnej części
        int currentChunkSize = min(chunkSize, static_cast<int>(fileSize - inputFile.tellg()));

        // Read chunk from input file
        inputFile.read(buffer, currentChunkSize);
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


        // Wysyłanie muzyki synchronicznie z czasem odtwarzania muzyki
        SDL_QueueAudio(device, buffer, currentChunkSize);

        // Poczekaj, aby upewnić się, że bufor został opróżniony przed kolejnym odtworzeniem
        while (SDL_GetQueuedAudioSize(device) > 0) {
            SDL_Delay(10);
        }

        cout << "Część: " << partIndex << " odtworzona." << endl;
    }

    inputFile.close();
    delete[] buffer;

    SDL_CloseAudioDevice(device);
    SDL_Quit();

    return 0;

}

int Stream::start(const char *ip) {
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

    ClientInfo clientInfo = {serverSocket, clientAddr};
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
        while (songsQueue.empty())
            sleep(1);
        this->streamSong(songsQueue.front());
        songsQueue.pop();
    }
    return 0;
}

int Stream::addToQueue(string songName) {
    songsQueue.push(songName);
    return 0;
}
