//
// Created by hajk0 on 2/8/24.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <cstring>

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
    const string inputFileName = "POLAND-LILYACHTY.wav";
    const string outputPrefix = "part_"; // Prefix dla nazw plików wyjściowych
    const int chunkSize = 1024 * 1024; // Rozmiar chunka 1 MB
    const uint32_t sampleRate = 44100; // Częstotliwość próbkowania
    const uint16_t numChannels = 2; // Liczba kanałów
    const uint16_t bitsPerSample = 16; // Liczba bitów na próbkę

    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Nie można otworzyć pliku: " << inputFileName << endl;
        return 1;
    }

    // Pobierz rozmiar pliku wejściowego
    inputFile.seekg(0, ios::end);
    uint32_t fileSize = inputFile.tellg();
    inputFile.seekg(0, ios::beg);

    // Oblicz liczbę części
    int numParts = (fileSize + chunkSize - 1) / chunkSize;

    // Odczytuj plik w chunkach i zapisuj każdy chunk do osobnego pliku wyjściowego
    char* buffer = new char[chunkSize];
    for (int partIndex = 0; partIndex < numParts; ++partIndex) {
        // Określ rozmiar chunka dla aktualnej części
        int currentChunkSize = min(chunkSize, static_cast<int>(fileSize - inputFile.tellg()));

        // Odczytaj chunk z pliku wejściowego
        inputFile.read(buffer, currentChunkSize);

        // Utwórz nazwę pliku wyjściowego
        ostringstream outputFileName;
        outputFileName << outputPrefix << partIndex << ".wav";

        // Otwórz plik wyjściowy
        ofstream outputFile(outputFileName.str(), ios::binary);
        if (!outputFile.is_open()) {
            cerr << "Nie można utworzyć pliku wyjściowego: " << outputFileName.str() << endl;
            delete[] buffer;
            return 1;
        }

        // Zapisz nagłówek WAV do pliku wyjściowego
        writeWavHeader(outputFile, currentChunkSize, sampleRate, numChannels, bitsPerSample);

        // Zapisz dane audio do pliku wyjściowego
        outputFile.write(buffer, currentChunkSize);
        outputFile.close();
    }

    inputFile.close();
    delete[] buffer;

    cout << "Plik podzielony na " << numParts << " części." << endl;

    return 0;
}
