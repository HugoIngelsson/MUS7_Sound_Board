#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cmath>

#include <portaudio.h>
#include <fftw3.h>

#include "wav_player.h"
#include "audio.h"
#include "terminal.h"

#define SAMPLE_RATE 44100.0
#define FRAMES_PER_BUFFER 512
#define HOP_SIZE 128
#define NUM_INPUT_CHANNELS 1

#define WINDOW_AMPLITUDE_ADJ (0.375 * (FRAMES_PER_BUFFER / HOP_SIZE + 1))

#define WAVEFILE_AMPLITUDE_SHIFT 0.5
#define ROBOT 0
#define WHISPER 0

#define PI 3.1415926535

int NUM_DEVICES = -1;
int OUTPUT_CHANNELS = -1;
int INPUT_DEVICE_ID = -1;
int OUTPUT_DEVICE_ID = -1;

typedef struct {
    int writePtr;
    int readPtr;
    int size;
    double* values;
} circularArray;

typedef struct {
    double* in;
    fftw_complex* out;
    fftw_complex* test;
    fftw_plan forw;
    fftw_plan back;
    circularArray* inputBuffer;
    circularArray* outputBuffer;
    wav_data waveform;
} streamCallbackData;

static streamCallbackData* spectroData;
static double* windowFunction;
static double* lastInputPhases;
static double* lastOutputPhases;

static const char* CSI = "\33[";

static void checkErr(PaError err) {
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }
}

void setSoundEffect(const char* s) {
    get_wav(&(spectroData->waveform), s);
}

static inline float max(float a, float b) {
    return a > b ? a : b;
}

static inline float min(float a, float b) {
    return a < b ? a : b;
}

static bool clamp(double min, double max, double n) {
    return n >= min && n <= max;
}

static double calcAmplitude(fftw_complex input) {
    return std::sqrt(std::pow(input[0], 2) + std::pow(input[1], 2));
}

static double calcPhase(fftw_complex input) {
    return std::atan2(input[1], input[0]);
}

static double wrapPhase(double phase) {
    return fmod(phase + PI * 1000000.0, 2.0*PI) - PI;
}

static void buildWindowFunction() {
    windowFunction = (double*)malloc(sizeof(double) * FRAMES_PER_BUFFER);

    for (int i=0; i<FRAMES_PER_BUFFER; i++) {
        windowFunction[i] = std::pow(std::sin(i * PI / (FRAMES_PER_BUFFER-1)), 2);
    }
}

static circularArray* createCircularArray(int size) {
    circularArray* ret = (circularArray*)malloc(sizeof(circularArray));
    ret->values = (double*)malloc(sizeof(double) * size);

    for (int i=0; i<size; i++) {
        ret->values[i] = 0.0;
    }

    ret->size = size;
    ret->writePtr = FRAMES_PER_BUFFER;
    ret->readPtr = 0;

    return ret;
}

static void transformAndModify(streamCallbackData* data) {
    fftw_execute(data->forw);

    for (int i=0; i<FRAMES_PER_BUFFER/2+1; i++) {
        double amplitude = calcAmplitude(data->out[i]);
        double phase = calcPhase(data->out[i]);

        double phaseDiff = phase - lastInputPhases[i];

        double binCenterFreq = (2.0 * PI) * i / FRAMES_PER_BUFFER;
        phaseDiff = wrapPhase(phaseDiff - binCenterFreq * HOP_SIZE);

        double binDeviation = phaseDiff * FRAMES_PER_BUFFER / HOP_SIZE / (2.0 * PI);

        data->out[i][0] = i + binDeviation;
        data->out[i][1] = amplitude;

        lastInputPhases[i] = phase;
    }

    for (int i=0; i<FRAMES_PER_BUFFER/2+1; i++) {
        data->test[i][0] = 0.0;
        data->test[i][1] = 0.0;
    }

    double pitchShift = std::pow(2, PITCH_SHIFT / 12.0);
    double linearShift = FREQUENCY_SHIFT * FRAMES_PER_BUFFER / SAMPLE_RATE;
    for (int i=0; i<FRAMES_PER_BUFFER/2+1; i++) {
        int newBin = (int)(i * pitchShift + linearShift + 0.5);

        if (clamp(0, FRAMES_PER_BUFFER/2, newBin)) {
            data->test[newBin][0]  = data->out[i][0] * pitchShift + linearShift;
            data->test[newBin][1] += data->out[i][1];
        }
    }

    for (int i=0; i<FRAMES_PER_BUFFER/2+1; i++) {
        double amplitude = data->test[i][1];
        double binDeviation = data->test[i][0] - i;

        double phaseDiff = binDeviation * (2.0 * PI) / FRAMES_PER_BUFFER * HOP_SIZE;

        double binCenterFreq = (2.0 * PI) / FRAMES_PER_BUFFER * i;
        phaseDiff += binCenterFreq * HOP_SIZE;

        double outPhase = wrapPhase(lastOutputPhases[i] + phaseDiff);

        data->test[i][0] = amplitude * std::cos(outPhase);
        data->test[i][1] = amplitude * std::sin(outPhase);

        lastOutputPhases[i] = outPhase;
    }

    if (ROBOT_VOICE) {
        for (unsigned long i=0; i<FRAMES_PER_BUFFER/2+1; i++) {
            double amplitude = calcAmplitude(data->test[i]);
            data->test[i][0] = amplitude;
            data->test[i][1] = 0.0;
        }
    }

    if (WHISPER_VOICE) {
        for (unsigned long i=0; i<FRAMES_PER_BUFFER/2+1; i++) {
            double amplitude = calcAmplitude(data->test[i]);
            double phase = rand() * PI * 2.0 / RAND_MAX;

            data->test[i][0] = amplitude * std::cos(phase);
            data->test[i][1] = amplitude * std::sin(phase);
        }
    }  

    fftw_execute(data->back);
}

static int streamCallback(
    const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
    void* userData
) {
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;
    streamCallbackData* data = (streamCallbackData*)userData;

    for (int i=0; i<FRAMES_PER_BUFFER; i++) {
        data->inputBuffer->values[(data->inputBuffer->writePtr + i) 
            % data->inputBuffer->size] = in[i];
    }

    data->inputBuffer->writePtr += FRAMES_PER_BUFFER;
    data->inputBuffer->writePtr %= data->inputBuffer->size;

    int outID = 0;
    while ((data->inputBuffer->readPtr + FRAMES_PER_BUFFER - HOP_SIZE) 
                % data->inputBuffer->size != data->inputBuffer->writePtr) {
        for (int i=0; i<FRAMES_PER_BUFFER; i++) {
            data->in[i] = data->inputBuffer->values[(data->inputBuffer->readPtr + i) 
                % data->inputBuffer->size] * windowFunction[i];
        }

        transformAndModify(data);

        for (int i=0; i<FRAMES_PER_BUFFER; i++) {
            data->outputBuffer->values[(data->outputBuffer->writePtr+i)
                % data->outputBuffer->size] += data->in[i] / FRAMES_PER_BUFFER * windowFunction[i];
        }

        data->inputBuffer->readPtr += HOP_SIZE;
        data->inputBuffer->readPtr %= data->inputBuffer->size;

        int limiter = 0;
        while (limiter < HOP_SIZE) {
            out[2*outID] = data->outputBuffer->values[(data->outputBuffer->writePtr+limiter)
                % data->outputBuffer->size];
            out[2*outID+1] = data->outputBuffer->values[(data->outputBuffer->writePtr+limiter)
                % data->outputBuffer->size];

            data->outputBuffer->values[(data->outputBuffer->writePtr+limiter)
                % data->outputBuffer->size] = 0.0;

            outID++;
            limiter++;
        }

        data->outputBuffer->writePtr += HOP_SIZE;
        data->outputBuffer->writePtr %= data->inputBuffer->size;
    }

    int dispSize = SCREEN_WIDTH-2;
    for (int i=0; i<7; i++) {
        setXY(2, 9+i);

        for (int j=0; j<SCREEN_WIDTH-2; j++) {
            double proportion = std::pow(j / (double)dispSize, 2);
            double amplitude = 7.0 * calcAmplitude(data->test[(int)(proportion * (framesPerBuffer/2+1))]) - 6.0 + i;

            if (amplitude >= 1.000) printf("\33[40m█\33[0m");
            else if (amplitude >= 0.875) printf("▇");
            else if (amplitude >= 0.750) printf("▆");
            else if (amplitude >= 0.625) printf("▅");
            else if (amplitude >= 0.500) printf("▄");
            else if (amplitude >= 0.375) printf("▃");
            else if (amplitude >= 0.250) printf("▂");
            else if (amplitude >= 0.125 || i == 6) printf("▁");
            else printf(" ");
        }
    }

    for (unsigned long i=0; i<FRAMES_PER_BUFFER*2; i++) {
        out[i] = out[i] * AMPLITUDE_SHIFT / WINDOW_AMPLITUDE_ADJ;
    }

    if (data->waveform.data != NULL) {
        for (int i=0; i<FRAMES_PER_BUFFER*2 && i<data->waveform.totalLengthLeft; i++) {
            out[i] += data->waveform.data[data->waveform.readOffset+i] / WAV_DIVIDE_INT * WAVEFILE_AMPLITUDE_SHIFT;
        }

        data->waveform.readOffset += FRAMES_PER_BUFFER*2;
        data->waveform.totalLengthLeft -= FRAMES_PER_BUFFER*2;

        if (data->waveform.totalLengthLeft <= 0) {
            free(data->waveform.data);
            data->waveform.data = NULL;
        }
    }

    fflush(stdout);

    return 0;
}

PaStreamParameters inputParameters;
PaStreamParameters outputParameters;
PaStream* stream;

int startStream()
{
    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = 1;
    inputParameters.device = INPUT_DEVICE_ID;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(INPUT_DEVICE_ID)->defaultLowInputLatency;

    memset(&outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = 2;
    outputParameters.device = OUTPUT_DEVICE_ID;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(OUTPUT_DEVICE_ID)->defaultLowInputLatency;

    int err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paNoFlag,
        streamCallback,
        spectroData
    );
    checkErr(err);

    err = Pa_StartStream(stream);
    checkErr(err);

    return EXIT_SUCCESS;
}

int stopStream()
{
    int err = Pa_StopStream(stream);
    checkErr(err);

    err = Pa_CloseStream(stream);
    checkErr(err);

    return EXIT_SUCCESS;
}

int cycleInputDevice()
{
    NUM_DEVICES = Pa_GetDeviceCount();

    for (int i=1; i<=NUM_DEVICES; i++) {
        if (Pa_GetDeviceInfo((INPUT_DEVICE_ID+i)%NUM_DEVICES)->maxInputChannels >= 1) {
            INPUT_DEVICE_ID = (INPUT_DEVICE_ID+i)%NUM_DEVICES;
            
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}

int cycleOutputDevice()
{
    NUM_DEVICES = Pa_GetDeviceCount();

    for (int i=1; i<=NUM_DEVICES; i++) {
        if (Pa_GetDeviceInfo((OUTPUT_DEVICE_ID+i)%NUM_DEVICES)->maxOutputChannels >= 1) {
            OUTPUT_DEVICE_ID = (OUTPUT_DEVICE_ID+i)%NUM_DEVICES;

            if (Pa_GetDeviceInfo(OUTPUT_DEVICE_ID)->maxOutputChannels >= 2) {
                OUTPUT_CHANNELS = 2;
            }
            else {
                OUTPUT_CHANNELS = 1;
            }

            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}

int initAudio()
{
    PaError err;
    err = Pa_Initialize();
    checkErr(err);

    spectroData = (streamCallbackData*)malloc(sizeof(streamCallbackData));
    spectroData->in  = (double*)malloc(sizeof(double) * FRAMES_PER_BUFFER);
    spectroData->out = (fftw_complex*)malloc(sizeof(fftw_complex) * (FRAMES_PER_BUFFER/2+1));
    spectroData->test = (fftw_complex*)malloc(sizeof(fftw_complex) * (FRAMES_PER_BUFFER/2+1));
    spectroData->inputBuffer = createCircularArray(FRAMES_PER_BUFFER * 2);
    spectroData->outputBuffer = createCircularArray(FRAMES_PER_BUFFER * 2);
    buildWindowFunction();
    spectroData->forw = fftw_plan_dft_r2c_1d(
        FRAMES_PER_BUFFER, spectroData->in, spectroData->out,
        FFTW_ESTIMATE
    );
    spectroData->back = fftw_plan_dft_c2r_1d(
        FRAMES_PER_BUFFER, spectroData->test, spectroData->in,
        FFTW_ESTIMATE
    );
    lastInputPhases = (double*)calloc(FRAMES_PER_BUFFER/2+1, sizeof(double));
    lastOutputPhases = (double*)calloc(FRAMES_PER_BUFFER/2+1, sizeof(double));
    spectroData->waveform.data = NULL;

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf("Error getting device count.\n");
        exit(EXIT_FAILURE);
    } else if (numDevices == 0) {
        printf("There are no available audio devices on this machine.\n");
        exit(EXIT_SUCCESS);
    }

    if (cycleInputDevice() || cycleOutputDevice()) {
        printf("Missing suitable audio devices.\n");
        exit(EXIT_FAILURE);
    }

    startStream();
    return 0;
}