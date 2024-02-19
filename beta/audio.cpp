//
// Created by hajk0 on 1/11/24.
//
#include <iostream>
#include <vector>
#include <cstring>
#include <sndfile.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <arpa/inet.h>

const int PORT = 12345;
const int BUFFER_SIZE = 1024;

// Function to cut the WAV file into portions
std::vector<std::vector<int16_t>> cutWavFile(const char* filePath, double portionDurationSec) {
    SF_INFO sf_info;

    SNDFILE* sndfile = sf_open(filePath, SFM_READ, &sf_info);
    if (!sndfile) {
        std::cerr << "Failed to open WAV file" << sf_strerror(nullptr) << std::endl;
        return {};
    }

    int channels = sf_info.channels;
    int samplerate = sf_info.samplerate;
    int framesPerPortion = static_cast<int>(samplerate * portionDurationSec);

    std::vector<int16_t> buffer(framesPerPortion * channels, 0);
    std::vector<std::vector<int16_t>> portions;

    while (sf_readf_short(sndfile, buffer.data(), framesPerPortion) > 0) {
        portions.push_back(buffer);
    }

    sf_close(sndfile);
    return portions;
}

// Function to send portions over UDP
void sendPortionsUDP(const std::vector<std::vector<int16_t>>& portions) {
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        perror("Socket creation failed");
        return;
    }

    sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(PORT);
    clientAddress.sin_addr.s_addr = INADDR_ANY;

    for (const auto& portion : portions) {
        // Send portion data to the client
        sendto(udpSocket, portion.data(), portion.size() * sizeof(int16_t), 0,
               reinterpret_cast<sockaddr*>(&clientAddress), sizeof(clientAddress));

        usleep(portion.size() * sizeof(int16_t) / 2);  // Small delay to simulate playback speed
    }

    close(udpSocket);
}

int main() {
    const char* filePath = "../res/POLAND-LILYACHTY.wav";
    float portionDurationSec = 0.1;

    // Cut the WAV file into portions
    std::vector<std::vector<int16_t>> portions = cutWavFile(filePath, portionDurationSec);

    // Send portions over UDP
    sendPortionsUDP(portions);

    return 0;
}
