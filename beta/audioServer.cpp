//
// Created by hajk0 on 1/10/24.
//
// run: g++ audioServer.cpp -lSDL2 -lSDL2_mixer
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

    return 0;
}