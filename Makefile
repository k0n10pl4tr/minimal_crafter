
cc=gcc
cflags=-Wall -Werror -g
libs=-lX11 -lEGL -lm

sources= main.o \
		 glutil.o \
		 glad.o \
		 util.o \
		 rendering.o \

all: crafting_game

.c.o:
	$(cc) -c $(cflags) $<

crafting_game: $(sources)
	$(cc) -o $@ $(sources) $(libs)

clean:
	rm $(sources)
	rm crafting_game
