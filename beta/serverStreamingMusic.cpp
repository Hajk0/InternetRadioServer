//
// Created by hajk0 on 2/20/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <SDL2/SDL.h>

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
        if (send(serverSocket, &currentChunkSize, sizeof(currentChunkSize), 0) == -1) {
            perror("Błąd podczas wysyłania danych do klienta");
            close(serverSocket);
            delete[] buffer;
            return 1;
        }

        if (send(serverSocket, buffer, currentChunkSize, 0) == -1) {
            perror("Błąd podczas wysyłania danych do klienta");
            close(serverSocket);
            delete[] buffer;
            return 1;
        }

        // Wysyłanie muzyki synchronicznie z czasem odtwarzania muzyki
        SDL_QueueAudio(device, buffer, currentChunkSize);

        // Poczekaj, aby upewnić się, że bufor został opróżniony przed kolejnym odtworzeniem
        while (SDL_GetQueuedAudioSize(device) > 0) {
            SDL_Delay(10);
        }

        cout << "Część: " << partIndex << " odtworzona." << endl;
    }

    close(serverSocket);
    inputFile.close();
    delete[] buffer;

    SDL_CloseAudioDevice(device);
    SDL_Quit();

    return 0;
}

/* #include <iostream>
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