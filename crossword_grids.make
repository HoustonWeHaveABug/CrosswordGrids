CROSSWORD_GRIDS_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

crossword_grids: crossword_grids.o
	gcc -o crossword_grids crossword_grids.o

crossword_grids.o: crossword_grids.c crossword_grids.make
	gcc -c ${CROSSWORD_GRIDS_C_FLAGS} -o crossword_grids.o crossword_grids.c

clean:
	rm -f crossword_grids crossword_grids.o
