
cc=tcc
cflags=-Wall -Werror
libs=-lX11 -lEGL

all: crafting_game

.c.o:
	$(cc) -c $(cflags) $<

crafting_game: main.o
	$(cc) -o $@ main.o $(libs)

clean:
	rm main.o
	rm crafting_game
