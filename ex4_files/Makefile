
CC	     = gcc

LDLIBS = -lm   -lglut -lGL -lGLU -lX11 -lXmu -lXi -L/usr/X11R6/lib
DEBUG        = -g -D DEBUG #  -ggdb3
OPT          = -O3
FLAGS        = # -std=gnu99 -pedantic -Wall -Wextra -march=native

CFLAGS	     = $(FLAGS) $(INCLUDE) $(OPT) $(DEBUG)

SOURCES = $(shell echo *.c)
HEADERS = $(shell echo ../include/*.h *.h)

OBJECTS = $(SOURCES:.c=.o)

showBMP: $(OBJECTS)
	$(CC) -o  $@ $(OBJECTS) $(LDLIBS)

.c.o: 
	$(CC) $(CFLAGS) -c $<

clean:
	@rm -f $(OBJECTS) showBMP


