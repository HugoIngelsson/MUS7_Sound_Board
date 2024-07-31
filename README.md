This is a technical description of my final project for MUS 7: Music, Media, and Technology. To find a showcase of it, watch the YouTube video I made here: https://youtu.be/WCnZX4ta3PM. Note that some parts of the explanation may not be fully accurate to make it clearer; if you want a true telling of all details, the best idea is probably to look at the code. The project can be broken down into a few central components.

Perhaps the most important one is the audio processing. The audio I/O is built upon PortAudio, a library specifically made to allow for real-time audio processing. Using PortAudio, I scan the available audio devices–both for input and output. Initially, the app just selects the first available device for each category it can find, but it also allows users to cycle through devices to select what microphone and speaker should be used.

PortAudio uses an interrupt system, where it calls a certain function every few milliseconds or so. That function gets input data through a buffer and is expected to output audio data to an output buffer that then gets sent to the correct device. An important feature of this function is that it allows you to pass in user data, which is how I could build the pitch vocoder.

A pitch vocoder is a device that uses several overlapping windows to take input data, transform them in some way, then scale them onto the output. To get a better understanding of how they work, I’d recommend watching the tutorials by the Bela Platform, cited in my sources used. For my vocoder, I’ve been using four overlapping windows per function call. The purpose of using such a device is to smooth out discontinuities at the edges between consecutive function calls, which otherwise creates massive problems when modifying the frequency domain.

Speaking of the frequency domain, I use the FFTW package (cited in sources used) to convert the input time-based data into the frequency domain using a Fast-Fourier Transform. It was slightly confusing to get started with, so I used an excellent tutorial by Chris Rouck to get familiar with it.

Once I’m in the frequency domain, I shift the frequencies in that block of data depending on the parameters for frequency and pitch shift the user has given. This is a careful process, mainly because the FFT gives us discrete buckets with ranges of frequencies, not exact numbers (i.e. it’s a Discrete Fourier Transform). This means we need to use the phase data from the previous function call in addition to this call’s phase data to figure out the true frequencies of the input. All of this is done to avoid impurities in the final sound. If the robot or whisper modes are selected, I also apply some final transformations afterwards; these are also described in the Bela Platform tutorials, which is where I got them from.

Finally, I transform the data back into the time domain. Then, all that is left is to apply a synthesis window, scale by the amplitude adjustment, and add it to the output buffer. We also print out the spectogram gotten from the transformation for some visual spice.

After the data has been transformed, we layer the currently playing wav file on top of the output audio track. This is a part of the project that I’d like to have done better. In its current state, it simply loads all of the selected file at once; the user also can’t change the available files without manually editing the code themselves. This might be an improvement I make in the future.

All I’ve described up until now is the raw functionality of the program, yet the final product also has a pretty display with immediate user inputs. This is achieved in two parts.

First, I put the terminal into raw mode. This means the user doesn’t have to press “enter” to have their inputs registered, which makes for smoother interaction. I learned how to do this with a guide cited in the sources used.

Second, I use ANSI codes to hide the terminal cursor and move it around wherever I want it to go. This means I don’t need to reprint the entire screen every time I need to adjust a value on it, so the visual response can be immediate.

I think that is pretty much all of the technical details out of the way without going into too much detail. If you’d like to run the app by yourself, be aware that I haven’t tested it on any computer other than my own. You’ll also likely have to figure out how to compile it on your own; if you’re on a Mac with all of the dependencies installed, it should be as easy as typing “make main” in the terminal, but I doubt things will go that smoothly.

**Sources Used:**
Chris Rouck’s PortAudio + FFTW guides, which helped me get started with these libraries:
https://www.youtube.com/watch?v=jpsJCji71Ec
https://www.youtube.com/watch?v=yt7i4zPbVDs

Bela Platform tutorials 18-20, which helped me build a phase vocoder:
https://www.youtube.com/watch?v=xGmRaTaBNZA
https://www.youtube.com/watch?v=2Esfl8uw-2U
https://www.youtube.com/watch?v=2p_-jbl6Dyc

viewsourcecode.org, which provided me with the info I needed to set the terminal to raw mode, along with some other useful settings
https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

PortAudio documentation:
https://www.portaudio.com/

FFTW documentation:
https://www.fftw.org/
