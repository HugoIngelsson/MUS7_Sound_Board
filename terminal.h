#ifndef TERMINAL_H
#define TERMINAL_H

#define SCREEN_WIDTH 75
#define SCREEN_HEIGHT 20

extern double AMPLITUDE_SHIFT;
extern int PITCH_SHIFT;
extern int FREQUENCY_SHIFT;

extern bool ROBOT_VOICE;
extern bool WHISPER_VOICE;

extern bool PAUSED;

extern char SOUND_BOARD[5][20];

extern int INPUT_DEVICE_ID;
extern int OUTPUT_DEVICE_ID;

void die(const char *s);
void disableRawMode();
void enableRawMode();

void setTerminalSize(int h, int w);
void setXY(int x, int y);
void printScreen();

void printAmplitude();
void printPitch();
void printFrequency();

void printRobot();
void printWhisper();

void printInputDevice();
void printOutputDevice();
void printPause();

#endif