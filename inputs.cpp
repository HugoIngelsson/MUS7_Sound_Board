#include <string.h>

#include "terminal.h"
#include "audio.h"

int AMPLITUDE_ID = 6;
double AMPLITUDE_MAP[] = {0.05, 0.1, 0.25, 0.45, 0.65, 0.85, 1.0, 1.25, 1.5, 1.75, 2.0, 3.0, 4.0};

char SOUND_BOARD[5][20] = {
    {'t','e','s','t','_','i','n','p','u','t','.','w','a','v','\0'},
    {'\0'},
    {'\0'},
    {'p','i','p','e','.','w','a','v','\0'},
    {'\0'}
};

void inputI()
{
    if (!PAUSED) {
        stopStream();
    }

    cycleInputDevice();
    printInputDevice();

    if (!PAUSED) {
        startStream();
    }
}

void inputO()
{
    if (!PAUSED) {
        stopStream();
    }

    cycleOutputDevice();
    printOutputDevice();

    if (!PAUSED) {
        startStream();
    }
}

void inputP()
{
    if (!PAUSED) {
        stopStream();
    }
    else {
        startStream();
    }

    PAUSED = !PAUSED;
    printPause();
}

void inputA()
{
    if (AMPLITUDE_ID < 12) {
        AMPLITUDE_ID++;
        AMPLITUDE_SHIFT = AMPLITUDE_MAP[AMPLITUDE_ID];
    }

    printAmplitude();
}

void inputZ()
{
    if (AMPLITUDE_ID > 0) {
        AMPLITUDE_ID--;
        AMPLITUDE_SHIFT = AMPLITUDE_MAP[AMPLITUDE_ID];
    }

    printAmplitude();
}

void inputS()
{
    PITCH_SHIFT++;
    printPitch();
}

void inputX()
{
    PITCH_SHIFT--;
    printPitch();
}

void inputD()
{
    FREQUENCY_SHIFT += 10;
    printFrequency();
}

void inputC()
{
    FREQUENCY_SHIFT -= 10;
    printFrequency();
}

void inputV()
{
    ROBOT_VOICE = !ROBOT_VOICE;
    printRobot();
}

void inputB()
{
    WHISPER_VOICE = !WHISPER_VOICE;
    printWhisper();
}

void input1()
{
    if (strlen(SOUND_BOARD[0]) > 0)
        setSoundEffect(SOUND_BOARD[0]);
}

void input2()
{
    if (strlen(SOUND_BOARD[1]) > 0)
        setSoundEffect(SOUND_BOARD[1]);
}

void input3()
{
    if (strlen(SOUND_BOARD[2]) > 0)
        setSoundEffect(SOUND_BOARD[2]);
}

void input4()
{
    if (strlen(SOUND_BOARD[3]) > 0)
        setSoundEffect(SOUND_BOARD[3]);
}

void input5()
{
    if (strlen(SOUND_BOARD[4]) > 0)
        setSoundEffect(SOUND_BOARD[4]);
}