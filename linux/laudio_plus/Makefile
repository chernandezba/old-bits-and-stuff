CC=gcc

PROGRAMS=laudio_plus

all: $(PROGRAMS)

laudio_plus:    laudio_plus.c
	$(CC) -lm -ggdb -o laudio_plus laudio_plus.c

rm_objects:
	rm -f *.o core

clean:
	rm -f *.o core $(PROGRAMS)
