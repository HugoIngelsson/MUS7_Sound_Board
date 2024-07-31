#ifndef WAV_PLAYER_H
#define WAV_PLAYER_H

#define WAV_DIVIDE_INT 32768.0

typedef struct WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t     RIFF[4];
    uint32_t    ChunkSize;
    uint8_t     WAVE[4];
    /* "fmt" sub-chunk */
    uint8_t     fmt[4];
    uint32_t    Subchunk1Size;
    uint16_t    AudioFormat;
    uint16_t    NumChannels;
    uint32_t    SamplesPerSec;
    uint32_t    bytesPerSec;
    uint16_t    blockAlign;
    uint16_t    bitsPerSample;
    /* "data" sub-chunk */
    uint8_t     Subchunk2ID[4];
    uint32_t    Subchunk2Size;
} wav_hdr;

typedef struct
{
    int16_t* data;
    int readOffset;
    int bufferLength;
    int totalLengthLeft;
    const char *file_path;
} wav_data;

int get_wav(wav_data *data, const char *file_path);

#endif