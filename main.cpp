#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"
#include "audio.h"
#include "inputs.h"

double AMPLITUDE_SHIFT = 1.0;
int PITCH_SHIFT = 0;
int FREQUENCY_SHIFT = 0;

bool ROBOT_VOICE = false;
bool WHISPER_VOICE = false;

bool PAUSED = false;

int main()
{
    enableRawMode();
    setTerminalSize(SCREEN_HEIGHT, SCREEN_WIDTH);

    initAudio();
    printScreen();

    while (true) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");

        switch (c) {
            case 'i':
            case 'I': inputI(); break;
            case 'o':
            case 'O': inputO(); break;
            case 'p':
            case 'P': inputP(); break;
            case 'a':
            case 'A': inputA(); break;
            case 'z':
            case 'Z': inputZ(); break;
            case 's':
            case 'S': inputS(); break;
            case 'x':
            case 'X': inputX(); break;
            case 'd':
            case 'D': inputD(); break;
            case 'c':
            case 'C': inputC(); break;
            case 'v':
            case 'V': inputV(); break;
            case 'b':
            case 'B': inputB(); break;
            case '1': input1(); break;
            case '2': input2(); break;
            case '3': input3(); break;
            case '4': input4(); break;
            case '5': input5(); break;
            case 3: printf("\33[25h");
        }

        setXY(2, 15);
        fflush(stdout);

        // ctrl-C
        if (c == 3) break;
    }

    return 0;
}