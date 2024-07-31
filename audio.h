#ifndef AUDIO_H
#define AUDIO_H

extern bool PAUSED;

void setSoundEffect(const char* s);

int startStream();
int stopStream();
int cycleInputDevice();
int cycleOutputDevice();
int initAudio();

#endif