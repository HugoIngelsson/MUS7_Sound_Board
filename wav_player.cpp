#include <iostream>
#include <fstream>

#include "wav_player.h"

int get_wav(wav_data *data, const char *file_path) {
    wav_hdr wavHeader;
    int header_size = sizeof(wavHeader);
    FILE *wavFile = fopen(file_path, "r");

    fread(&wavHeader, header_size, 1, wavFile);
    if (wavHeader.AudioFormat != 1 || wavHeader.NumChannels != 2 || wavHeader.bitsPerSample != 16) {
        fclose(wavFile);
        return 1;
    }

    data->data = (int16_t*)malloc(wavHeader.Subchunk2Size);
    fread(data->data, wavHeader.Subchunk2Size, 1, wavFile);
    data->readOffset = 0;
    data->bufferLength = wavHeader.Subchunk2Size / 2;
    data->totalLengthLeft = wavHeader.Subchunk2Size / 2;
    data->file_path = file_path;
    fclose(wavFile);

    return 0;
}