
OBJS=ai.o sound.o compat.o work.o 
FLAGS=-g


all: tetris ${OBJS}
clean:
	rm -fv *.o tetris

%.o: %.cc
	g++ ${FLAGS} -c -o $@ $<

tetris: ${OBJS}
	g++ ${FLAGS} -o $@ ${OBJS} -lSDL2 -lSDL2_ttf

tetris.html: Makefile work.cc compat.cc sound.cc
	emcc -O2 -o tetris.html -sASYNCIFY -sSDL2_IMAGE_FORMATS='["bmp"]' -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -sSDL2_MIXER_FORMATS='["wav"]' -sUSE_SDL=2 --embed-file textures --embed-file sounds --embed-file lvl --embed-file Vera.ttf ai.cc sound.cc compat.cc work.cc 
