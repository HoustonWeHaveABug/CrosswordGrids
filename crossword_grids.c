#include <stdio.h>
#include <stdlib.h>

#define SQUARE_VAL_WHITE '.'
#define SQUARE_VAL_BLACK '#'

typedef struct {
	unsigned long row;
	unsigned long col;
	int val;
	unsigned long len_hor;
	unsigned long len_vert;
	int marked;
}
square_t;

unsigned long *set_wns(const char *, unsigned long *);
void set_square(square_t *, unsigned long, unsigned long);
void crossword(square_t *, unsigned long *, unsigned long *, square_t *, unsigned long *, unsigned long *, unsigned long);
int check_white_forward(square_t *, unsigned long *, unsigned long *);
int check_black_forward(square_t *);
int check_backward(square_t *, unsigned long **, unsigned long **);
int whites_connected(unsigned long);
void add_neighbours_to_queue(square_t *);
void add_to_queue(square_t *);

unsigned long grid_size, word_len_min, squares_n, wn_max, solutions_n, queue_size;
square_t *squares, **queue;

int main(void) {
	unsigned long len_wns_hor, *wns_hor, len_wns_vert, *wns_vert, i, j;
	if (scanf("%lu", &grid_size) != 1 || grid_size%2UL == 0UL) {
		fprintf(stderr, "Invalid grid size\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &word_len_min) != 1 || word_len_min < 1UL || word_len_min > grid_size) {
		fprintf(stderr, "Invalid word minimum length\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	squares_n = grid_size*grid_size;
	squares = malloc(sizeof(square_t)*(squares_n+1UL));
	if (!squares) {
		fprintf(stderr, "Could not allocate memory for squares\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (i = 0UL; i < grid_size; i++) {
		for (j = 0UL; j < grid_size; j++) {
			set_square(squares+i*grid_size+j, i, j);
		}
	}
	set_square(squares+squares_n, grid_size, 0UL);
	queue = malloc(sizeof(square_t *)*squares_n);
	if (!queue) {
		fprintf(stderr, "Could not allocate memory for queue\n");
		fflush(stderr);
		free(squares);
		return EXIT_FAILURE;
	}
	wns_hor = set_wns("right", &len_wns_hor);
	if (!wns_hor) {
		free(queue);
		free(squares);
		return EXIT_FAILURE;
	}
	wns_vert = set_wns("down", &len_wns_vert);
	if (!wns_vert) {
		free(wns_hor);
		free(queue);
		free(squares);
		return EXIT_FAILURE;
	}
	i = 1UL;
	j = 1UL;
	wn_max = 1UL;
	while (i <= len_wns_hor || j <= len_wns_vert) {
		if (wns_hor[i] == wn_max) {
			i++;
			if (wns_vert[j] == wn_max) {
				j++;
			}
			wn_max++;
		}
		else {
			if (wns_vert[j] == wn_max) {
				j++;
				wn_max++;
			}
			else {
				break;
			}
		}
	}
	if (i <= len_wns_hor || j <= len_wns_vert) {
		fprintf(stderr, "Inconsistent word numbers\n");
		fflush(stderr);
		free(wns_vert);
		free(wns_hor);
		free(queue);
		free(squares);
		return EXIT_FAILURE;
	}
	wns_hor[len_wns_hor+1UL] = wn_max;
	wns_vert[len_wns_vert+1UL] = wn_max;
	solutions_n = 0UL;
	crossword(squares, wns_hor+1UL, wns_vert+1UL, squares+squares_n-1UL, wns_hor+len_wns_hor, wns_vert+len_wns_vert, 0UL);
	printf("Solutions %lu\n", solutions_n);
	fflush(stdout);
	free(wns_vert);
	free(wns_hor);
	free(queue);
	free(squares);
	return EXIT_SUCCESS;
}

void set_square(square_t *square, unsigned long row, unsigned long col) {
	square->row = row;
	square->col = col;
	square->val = SQUARE_VAL_WHITE;
	square->len_hor = 0UL;
	square->len_vert = 0UL;
	square->marked = 0;
}

unsigned long *set_wns(const char *name, unsigned long *len_wns) {
	unsigned long *wns, i;
	if (scanf("%lu", len_wns) != 1) {
		fprintf(stderr, "Invalid length of %s word numbers\n", name);
		fflush(stderr);
		return NULL;
	}
	wns = malloc(sizeof(unsigned long)*(*len_wns+2UL));
	if (!wns) {
		fprintf(stderr, "Could not allocate memory for %s word numbers\n", name);
		fflush(stderr);
		return NULL;
	}
	wns[0] = 0UL;
	if (*len_wns > 0UL) {
		if (scanf("%lu", wns+1UL) != 1 || wns[1] != 1UL) {
			fprintf(stderr, "Invalid %s word number\n", name);
			fflush(stderr);
			free(wns);
			return NULL;
		}
		for (i = 2UL; i <= *len_wns && scanf("%lu", wns+i) == 1 && wns[i] > wns[i-1UL]; i++);
		if (i <= *len_wns) {
			fprintf(stderr, "Invalid %s word number\n", name);
			fflush(stderr);
			free(wns);
			return NULL;
		}
	}
	return wns;
}

void crossword(square_t *square_for, unsigned long *wn_for_hor, unsigned long *wn_for_vert, square_t *square_back, unsigned long *wn_back_hor, unsigned long *wn_back_vert, unsigned long whites_n) {
	if (square_for->row == grid_size) {
		if (*wn_for_hor == wn_max && *wn_for_vert == wn_max && whites_connected(whites_n)) {
			unsigned long i;
			solutions_n++;
			puts("Solution found");
			for (i = 0UL; i < grid_size; i++) {
				unsigned long j;
				for (j = 0UL; j < grid_size; j++) {
					putchar(squares[i*grid_size+j].val);
				}
				puts("");
			}
			fflush(stdout);
		}
		return;
	}
	if (square_for < square_back) {
		if (check_white_forward(square_for, wn_for_hor, wn_for_vert)) {
			unsigned long *wn_back_hor_old, *wn_back_vert_old;
			if (square_for->len_hor == 1UL) {
				wn_for_hor++;
			}
			if (square_for->len_vert == 1UL) {
				wn_for_vert++;
			}
			wn_back_hor_old = wn_back_hor;
			wn_back_vert_old = wn_back_vert;
			if (check_backward(square_back, &wn_back_hor, &wn_back_vert)) {
				crossword(square_for+1UL, wn_for_hor, wn_for_vert, square_back-1UL, wn_back_hor, wn_back_vert, whites_n+2UL);
			}
			wn_back_vert = wn_back_vert_old;
			wn_back_hor = wn_back_hor_old;
			if (square_for->len_vert == 1UL) {
				wn_for_vert--;
			}
			if (square_for->len_hor == 1UL) {
				wn_for_hor--;
			}
		}
		if (check_black_forward(square_for)) {
			unsigned long *wn_back_hor_old, *wn_back_vert_old;
			square_for->val = SQUARE_VAL_BLACK;
			square_back->val = SQUARE_VAL_BLACK;
			wn_back_hor_old = wn_back_hor;
			wn_back_vert_old = wn_back_vert;
			if (check_backward(square_back, &wn_back_hor, &wn_back_vert)) {
				crossword(square_for+1UL, wn_for_hor, wn_for_vert, square_back-1UL, wn_back_hor, wn_back_vert, whites_n);
			}
			wn_back_vert = wn_back_vert_old;
			wn_back_hor = wn_back_hor_old;
			square_back->val = SQUARE_VAL_WHITE;
			square_for->val = SQUARE_VAL_WHITE;
		}
	}
	else if (square_for == square_back) {
		if (check_white_forward(square_for, wn_for_hor, wn_for_vert)) {
			unsigned long *wn_back_hor_old, *wn_back_vert_old;
			if (square_for->len_hor == 1UL) {
				wn_for_hor++;
			}
			if (square_for->len_vert == 1UL) {
				wn_for_vert++;
			}
			wn_back_hor_old = wn_back_hor;
			wn_back_vert_old = wn_back_vert;
			if (check_backward(square_back, &wn_back_hor, &wn_back_vert)) {
				crossword(square_for+1UL, wn_for_hor, wn_for_vert, square_back-1UL, wn_back_hor, wn_back_vert, whites_n+1UL);
			}
			wn_back_vert = wn_back_vert_old;
			wn_back_hor = wn_back_hor_old;
			if (square_for->len_vert == 1UL) {
				wn_for_vert--;
			}
			if (square_for->len_hor == 1UL) {
				wn_for_hor--;
			}
		}
		if (check_black_forward(square_for)) {
			unsigned long *wn_back_hor_old, *wn_back_vert_old;
			square_for->val = SQUARE_VAL_BLACK;
			wn_back_hor_old = wn_back_hor;
			wn_back_vert_old = wn_back_vert;
			if (check_backward(square_back, &wn_back_hor, &wn_back_vert)) {
				crossword(square_for+1UL, wn_for_hor, wn_for_vert, square_back-1UL, wn_back_hor, wn_back_vert, whites_n);
			}
			wn_back_vert = wn_back_vert_old;
			wn_back_hor = wn_back_hor_old;
			square_for->val = SQUARE_VAL_WHITE;
		}
	}
	else {
		if (square_for->val == SQUARE_VAL_WHITE) {
			if (check_white_forward(square_for, wn_for_hor, wn_for_vert)) {
				unsigned long *wn_back_hor_old, *wn_back_vert_old;
				if (square_for->len_hor == 1UL) {
					wn_for_hor++;
				}
				if (square_for->len_vert == 1UL) {
					wn_for_vert++;
				}
				wn_back_hor_old = wn_back_hor;
				wn_back_vert_old = wn_back_vert;
				if (check_backward(square_back, &wn_back_hor, &wn_back_vert)) {
					crossword(square_for+1UL, wn_for_hor, wn_for_vert, square_back-1UL, wn_back_hor, wn_back_vert, whites_n);
				}
				wn_back_vert = wn_back_vert_old;
				wn_back_hor = wn_back_hor_old;
				if (square_for->len_vert == 1UL) {
					wn_for_vert--;
				}
				if (square_for->len_hor == 1UL) {
					wn_for_hor--;
				}
			}
		}
		else if (square_for->val == SQUARE_VAL_BLACK) {
			if (check_black_forward(square_for)) {
				unsigned long *wn_back_hor_old = wn_back_hor, *wn_back_vert_old = wn_back_vert;
				if (check_backward(square_back, &wn_back_hor, &wn_back_vert)) {
					crossword(square_for+1UL, wn_for_hor, wn_for_vert, square_back-1UL, wn_back_hor, wn_back_vert, whites_n);
				}
				wn_back_vert = wn_back_vert_old;
				wn_back_hor = wn_back_hor_old;
			}
		}
	}
}

int check_white_forward(square_t *square, unsigned long *wn_hor, unsigned long *wn_vert) {
	square_t *square_l, *square_u;
	if (square->col > 0UL) {
		square_l = square-1UL;
	}
	else {
		square_l = NULL;
	}
	if (square->row > 0UL) {
		square_u = square-grid_size;
	}
	else {
		square_u = NULL;
	}
	if (!square_l || square_l->val == SQUARE_VAL_BLACK) {
		if (square->col+word_len_min > grid_size) {
			return 0;
		}
		if (!square_u || square_u->val == SQUARE_VAL_BLACK) {
			if (square->row+word_len_min > grid_size || *wn_hor != *wn_vert || *wn_hor == wn_max) {
				return 0;
			}
			square->len_vert = 1UL;
		}
		else {
			if (*wn_hor >= *wn_vert) {
				return 0;
			}
			square->len_vert = square_u->len_vert+1UL;
		}
		square->len_hor = 1UL;
	}
	else {
		if (!square_u || square_u->val == SQUARE_VAL_BLACK) {
			if (square->row+word_len_min > grid_size || *wn_hor <= *wn_vert) {
				return 0;
			}
			square->len_vert = 1UL;
		}
		else {
			square->len_vert = square_u->len_vert+1UL;
		}
		square->len_hor = square_l->len_hor+1UL;
	}
	return 1;
}

int check_black_forward(square_t *square) {
	square_t *square_l, *square_u;
	if (square->col > 0UL) {
		square_l = square-1UL;
	}
	else {
		square_l = NULL;
	}
	if (square->row > 0UL) {
		square_u = square-grid_size;
	}
	else {
		square_u = NULL;
	}
	if ((square_l && square_l->val == SQUARE_VAL_WHITE && square_l->len_hor < word_len_min) || (square_u && square_u->val == SQUARE_VAL_WHITE && square_u->len_vert < word_len_min)) {
		return 0;
	}
	square->len_hor = 0UL;
	square->len_vert = 0UL;
	return 1;
}

int check_backward(square_t *square_back, unsigned long **wn_hor, unsigned long **wn_vert) {
	square_t *square, *square_l, *square_u;
	if (square_back->row == grid_size-1UL) {
		return 1;
	}
	square = square_back+grid_size;
	if (square->val == SQUARE_VAL_BLACK) {
		return 1;
	}
	if (square->col > 0UL) {
		square_l = square-1UL;
	}
	else {
		square_l = NULL;
	}
	if (square->row > 0UL) {
		square_u = square-grid_size;
	}
	else {
		square_u = NULL;
	}
	if (!square_l || square_l->val == SQUARE_VAL_BLACK) {
		if (!square_u || square_u->val == SQUARE_VAL_BLACK) {
			if (**wn_hor != **wn_vert || **wn_hor == 0UL) {
				return 0;
			}
			*wn_hor -= 1UL;
			*wn_vert -= 1UL;
		}
		else {
			if (**wn_hor <= **wn_vert) {
				return 0;
			}
			*wn_hor -= 1UL;
		}
	}
	else {
		if (!square_u || square_u->val == SQUARE_VAL_BLACK) {
			if (**wn_hor >= **wn_vert) {
				return 0;
			}
			*wn_vert -= 1UL;
		}
	}
	return 1;
}

int whites_connected(unsigned long whites_n) {
	unsigned long i;
	square_t *square;
	if (whites_n == 0UL) {
		return 1;
	}
	for (square = squares; square < squares+squares_n && square->val != SQUARE_VAL_WHITE; square++);
	queue_size = 0UL;
	add_to_queue(square);
	for (i = 0UL; i < queue_size; i++) {
		add_neighbours_to_queue(queue[i]);
	}
	for (i = 0UL; i < queue_size; i++) {
		queue[i]->marked = 0;
	}
	return queue_size == whites_n;
}

void add_neighbours_to_queue(square_t *square) {
	if (square->col > 0UL) {
		add_to_queue(square-1UL);
	}
	if (square->row > 0UL) {
		add_to_queue(square-grid_size);
	}
	if (square->col < grid_size-1UL) {
		add_to_queue(square+1UL);
	}
	if (square->row < grid_size-1UL) {
		add_to_queue(square+grid_size);
	}
}

void add_to_queue(square_t *square) {
	if (square->val == SQUARE_VAL_WHITE && !square->marked) {
		square->marked = 1;
		queue[queue_size++] = square;
	}
}
