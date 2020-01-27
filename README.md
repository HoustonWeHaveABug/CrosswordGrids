# CrosswordGrids

Solver for Reddit Daily Programmer Challenge #382 - https://www.reddit.com/r/dailyprogrammer/comments/etf0al/20200124_challenge_382_hard_crossword_grids/, written in C.

The repository also includes a makefile and inputs from the challenge, in a format compatible with the solver.

The program uses backtracking from both ends of the grid and input simultaneously, taking advantage of constraint 1 - the grid must have a 180 degree rotational symmetry.
