//
// Created by hajk0 on 1/10/24.
//
// run: g++ audioServer.cpp -lSDL2 -lSDL2_mixer -lsndfile
#include <iostream>
#include <vector>
#include <cstring>
#include <sndfile.h>
#include <SDL2/SDL.h>



// Function to cut the WAV file into 2-second portions
std::vector<std::vector<int16_t>> cutWavFile(const char* filePath, double portionDurationSec) {
    SF_INFO sf_info;

    SNDFILE* sndfile = sf_open(filePath, SFM_READ, &sf_info);
    if (!sndfile) {
        std::cerr << "Failed to open WAV file" << sf_strerror(nullptr) << std::endl;
        return {};
    }

    int channels = sf_info.channels;
    int samplerate = sf_info.samplerate;
    int framesPerPortion = samplerate * portionDurationSec;

    std::vector<int16_t> buffer(framesPerPortion * channels, 0);
    std::vector<std::vector<int16_t>> portions;

    while (sf_readf_short(sndfile, buffer.data(), framesPerPortion) > 0) {
        portions.push_back(buffer);
    }

    sf_close(sndfile);
    return portions;
}

// Function to play portions using SDL2
void playPortions(const std::vector<std::vector<int16_t>>& portions) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 1024;
    spec.callback = nullptr; // We will push data manually

    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    if (audioDevice == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_PauseAudioDevice(audioDevice, 0);

    // Play all portions together
    for (const auto& portion : portions) {
        SDL_QueueAudio(audioDevice, portion.data(), portion.size() * sizeof(int16_t));
    }

    // Wait for audio to finish playing
    while (SDL_GetQueuedAudioSize(audioDevice) > 0) {
        SDL_Delay(10);
    }

    SDL_CloseAudioDevice(audioDevice);
    SDL_Quit();
}

int main() {
    const char* filePath = "../res/POLAND-LILYACHTY.wav";
    float portionDurationSec = 0.1;

    // Cut the WAV file into portions
    std::vector<std::vector<int16_t>> portions = cutWavFile(filePath, portionDurationSec);

    // portions[2] = static_cast<const std::vector<short>>(0);
    // Play all portions together using SDL2
    playPortions(portions);

    return 0;
}




/*
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_audio.h>


int main () {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return -1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        SDL_Log("SDL_mixer initialization failed: %s", Mix_GetError());
        SDL_Quit();
        return -1;
    }

    // Load music file as a sound chunk
    Mix_Chunk* sound = Mix_LoadWAV("../res/POLAND-LILYACHTY.wav");
    if (!sound) {
        SDL_Log("Failed to load sound chunk: %s", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Play only part of the sound chunk
    int channel = Mix_PlayChannelTimed(-1, sound, 0, 3000);  // Play for 3 seconds
    if (channel == -1) {
        SDL_Log("Failed to play sound: %s", Mix_GetError());
        Mix_FreeChunk(sound);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Wait for the sound to finish
    while (Mix_Playing(channel) != 0) {
        SDL_Delay(100);
    }

    channel = Mix_PlayChannelTimed(-1, sound, 0, 3000);  // Play for 3 seconds
    if (channel == -1) {
        SDL_Log("Failed to play sound: %s", Mix_GetError());
        Mix_FreeChunk(sound);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Wait for the sound to finish
    while (Mix_Playing(channel) != 0) {
        SDL_Delay(100);
    }

    // Free resources
    Mix_FreeChunk(sound);
    Mix_CloseAudio();
    SDL_Quit();
    */



    /*
    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return -1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        SDL_Log("SDL_mixer initialization failed: %s", Mix_GetError());
        SDL_Quit();
        return -1;
    }

    // Load music file
    Mix_Music* music = Mix_LoadMUS("../res/POLAND-LILYACHTY.mp3");
    if (!music) {
        SDL_Log("Failed to load music: %s", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Play the music
    if (Mix_PlayMusic(music, 0) == -1) {
        SDL_Log("Failed to play music: %s", Mix_GetError());
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Wait for the music to finish
    while (Mix_PlayingMusic()) {
        SDL_Delay(100);
    }

    // Free resources
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
    */

 /*   return 0;
}*/