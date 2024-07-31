#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <portaudio.h>

#include "terminal.h"
extern PaStream* stream;

void die(const char *s) 
{
    perror(s);
    exit(1);
}

struct termios orig_termios;
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

void setTerminalSize(int h, int w)
{
    printf("\33[8;%d;%dt", h, w);
}

void setXY(int x, int y)
{
    printf("\33[%d;%dH", y, x);
}

void printAmplitude()
{
    setXY(3, 2);
    if (AMPLITUDE_SHIFT == 1.0) printf("Amplitude (A/Z): no change");
    else if (AMPLITUDE_SHIFT < 1.0) printf("Amplitude (A/Z): %d%%", (int)((AMPLITUDE_SHIFT - 1.0) * 100));
    else printf("Amplitude (A/Z): +%d%%", (int)((AMPLITUDE_SHIFT - 1.0) * 100));
    
    // ensure no leftovers remain
    printf("      ");
}

void printPitch()
{
    setXY(3, 3);
    if (PITCH_SHIFT == 0) printf("Pitch (S/X): no change");
    else if (PITCH_SHIFT < 0) printf("Pitch (S/X): %d semitone", PITCH_SHIFT);
    else printf("Pitch (S/X): +%d semitone", PITCH_SHIFT);

    // fix grammar
    if (PITCH_SHIFT > 1 || PITCH_SHIFT < -1) printf("s");
    
    // ensure no leftovers remain
    printf("    ");
}

void printFrequency()
{
    setXY(3, 4);
    if (FREQUENCY_SHIFT == 0) printf("Frequency (D/C): no change");
    else if (FREQUENCY_SHIFT < 0) printf("Frequency (D/C): %d Hz", FREQUENCY_SHIFT);
    else printf("Frequency (D/C): +%d Hz", FREQUENCY_SHIFT);

    // ensure no leftovers remain
    printf("    ");
}

void printRobot()
{
    setXY(3, 6);
    if (ROBOT_VOICE) printf("Robot Voice (V): ON ");
    else printf("Robot Voice (V): OFF");
}

void printWhisper()
{
    setXY(3, 7);
    if (WHISPER_VOICE) printf("Whisper Mode (B): ON ");
    else printf("Whisper Mode (B): OFF");
}

void printSoundBoard()
{
    setXY(50, 2);
    printf("SOUND BOARD:");

    for (int i=0; i<5; i++) {
        setXY(51, 3+i);
        printf("%d. %s", i+1, SOUND_BOARD[i]);
    }
}

void printInputDevice()
{
    setXY(3, 17);
    printf("Input Device  (Cycle: I):                                 ");

    setXY(3, 17);
    printf("Input Device  (Cycle: I): %s", Pa_GetDeviceInfo(INPUT_DEVICE_ID)->name);
}

void printOutputDevice()
{
    setXY(3, 18);
    printf("Output Device  (Cycle: O):                                 ");

    setXY(3, 18);
    printf("Output Device (Cycle: O): %s", Pa_GetDeviceInfo(OUTPUT_DEVICE_ID)->name);
}

void printPause()
{
    setXY(3, 19);
    if (PAUSED) printf("(P)ause: ON ");
    else printf("(P)ause: OFF");
}

void printScreen()
{
    printf("\33[2J\33[?25l");

    printAmplitude();
    printPitch();
    printFrequency();

    printRobot();
    printWhisper();

    printSoundBoard();

    printInputDevice();
    printOutputDevice();
    printPause();
}