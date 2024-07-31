CLIB = -I./lib/portaudio/include ./lib/portaudio/lib/.libs/libportaudio.a \
			-I./lib/fftw-3.3.10/api -lfftw3
FRAMEWORK = -framework CoreAudio -framework AudioToolbox \
			-framework AudioUnit -framework CoreServices \
			-framework CoreFoundation

main: main.cpp wav_player.cpp terminal.cpp audio.cpp inputs.cpp
	g++ -o $@ $^ $(CLIB) $(FRAMEWORK)

install-deps: install-portaudio install-fftw
.PHONY: install-deps

uninstall-deps: uninstall-portaudio uninstall-fftw
.PHONY: uninstall-deps

install-portaudio:
	mkdir -p lib

	curl https://files.portaudio.com/archives/pa_stable_v190700_20210406.tgz | tar -zx -C lib
	cd lib/portaudio && ./configure && $(MAKE) -j
.PHONY: install-portaudio

uninstall-portaudio:
	cd lib/portaudio && $(MAKE) uninstall
	rm -rf lib/portaudio
.PHONY: uninstall-portaudio

install-fftw:
	mkdir -p lib

	curl http://www.fftw.org/fftw-3.3.10.tar.gz | tar -zx -C lib
	cd lib/fftw-3.3.10 && ./configure && $(MAKE) -j && sudo $(MAKE) install
.PHONY: install-portaudio

uninstall-fftw:
	cd lib/fftw-3.3.10 && $(MAKE) uninstall
	rm -rf lib/fftw-3.3.10
.PHONY: uninstall-portaudio

clean:
	rm -f $(EXEC)
.PHONY: clean