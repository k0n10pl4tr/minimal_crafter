
cc=c99
cflags=-Wall -Werror 
libs=-lX11 -lGLX -lm

sources= main.o \
		 glutil.o \
		 glad.o \
		 util.o \
		 rendering.o \
		 world.o \
		 blocks.o \
		 aabb.o

all: crafting_game

.c.o:
	$(cc) -c $(cflags) $<

crafting_game: $(sources)
	$(cc) -o $@ $(sources) $(libs)

clean:
	rm $(sources)
	rm crafting_game
