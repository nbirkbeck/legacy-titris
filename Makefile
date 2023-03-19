
OBJS=ai.o sound.o compat.o tetris.o 
FLAGS=-Wall # -O doesn't work


all: tetris ${OBJS}
clean:
	rm -fv *.o tetris

%.o: %.cc
	g++ ${FLAGS} -c -o $@ $<

tetris: ${OBJS}
	g++ ${FLAGS} -o $@ ${OBJS} -lSDL2 -lSDL2_ttf

titris.html: Makefile tetris.cc compat.cc sound.cc
	emcc -O2 -o titris.html -sASYNCIFY -sSDL2_IMAGE_FORMATS='["bmp"]' -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -sSDL2_MIXER_FORMATS='["wav"]' -sUSE_SDL=2 --embed-file textures --embed-file sounds --embed-file lvl --embed-file Vera.ttf ai.cc sound.cc compat.cc tetris.cc 
