//
// Created by hajk0 on 1/10/24.
//
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <SDL2/SDL.h>
#include <unistd.h>

const int PORT = 12346;
const int BUFFER_SIZE = 1024;

SDL_AudioSpec audioSpec;
SDL_AudioDeviceID audioDevice;

void errorCallback(const char* message) {
    std::cerr << "Error: " << message << std::endl;
    SDL_Quit();
    exit(EXIT_FAILURE);
}

void audioCallback(void* userdata, Uint8* stream, int len) {
    // UDP client doesn't directly write to the SDL audio buffer in this example
    // Instead, the received audio data is passed to the SDL audio callback
    // You might want to implement a buffer for better performance in a production scenario
    memset(stream, 0, len);  // Clear the audio buffer
}

int receiveAndPlayAudio() {
    int udpSocket;
    struct sockaddr_in serverAddress;
    socklen_t serverAddressSize = sizeof(serverAddress);
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind socket
    if (bind(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Bind failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        errorCallback("SDL initialization failed");
    }

    // Set audio specifications
    audioSpec.freq = 44100;
    audioSpec.format = AUDIO_F32SYS;  // 32-bit floating-point, system byte order
    audioSpec.channels = 2;  // Stereo
    audioSpec.samples = 1024;
    audioSpec.callback = audioCallback;

    // Open audio device
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audioDevice == 0) {
        errorCallback("SDL audio device opening failed");
    }

    SDL_PauseAudioDevice(audioDevice, 0);  // Unpause audio device

    std::cout << "Client listening on port " << PORT << std::endl;

    // Receive and play audio
    while (true) {
        ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, &serverAddressSize);

        if (bytesRead == -1) {
            perror("Receive failed");
            break;
        }

        // Pass the received audio data to the SDL audio callback
        SDL_QueueAudio(audioDevice, buffer, bytesRead);
    }

    // Close audio device
    SDL_CloseAudioDevice(audioDevice);

    // Quit SDL
    SDL_Quit();

    // Close the UDP socket
    close(udpSocket);

    return 0;
}

int main() {
    receiveAndPlayAudio();
    return 0;
}