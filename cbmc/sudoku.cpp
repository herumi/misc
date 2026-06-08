// cbmc --trace --unwind 10 sudoku.cpp
#include <assert.h>

// sudoku problem (0=empty, 1-9=fixed value)
const int tbl[9][9] = {
	{ 8, 0, 0,  0, 0, 5,  1, 0, 0 },
	{ 0, 0, 1,  0, 0, 0,  8, 0, 0 },
	{ 0, 4, 0,  2, 0, 0,  0, 9, 0 },

	{ 0, 0, 0,  0, 3, 0,  0, 0, 2 },
	{ 1, 2, 3,  4, 0, 6,  7, 8, 9 },
	{ 6, 0, 0,  0, 1, 0,  0, 0, 0 },

	{ 0, 8, 0,  0, 0, 9,  0, 5, 0 },
	{ 0, 0, 2,  0, 0, 0,  4, 0, 0 },
	{ 0, 0, 7,  6, 0, 0,  0, 0, 1 },
};

bool isValid1Row(const int sol[9][9], int i) {
    int v = 0;
    for (int j = 0; j < 9; j++) {
        v |= 1 << sol[i][j];
    }
    return v == 0b1111111110;
}

bool isValid1Col(const int sol[9][9], int j) {
    int v = 0;
    for (int i = 0; i < 9; i++) {
        v |= 1 << sol[i][j];
    }
    return v == 0b1111111110;
}

bool isValid1Box(const int sol[9][9], int bi, int bj) {
    int v = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            v |= 1 << sol[bi + i][bj + j];
        }
    }
    return v == 0b1111111110;
}

int main() {
    int sol[9][9];

    // restriction of all cells to 1-9 and fixing the given cells
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            __CPROVER_assume(sol[i][j] >= 1 && sol[i][j] <= 9);
            __CPROVER_assume(tbl[i][j] == 0 || sol[i][j] == tbl[i][j]);
        }
    }

    // row
    for (int i = 0; i < 9; i++) {
        __CPROVER_assume(isValid1Row(sol, i));
    }

    // column
    for (int j = 0; j < 9; j++) {
        __CPROVER_assume(isValid1Col(sol, j));
    }

    // 3x3 box
    for (int bi = 0; bi < 9; bi += 3) {
        for (int bj = 0; bj < 9; bj += 3) {
            __CPROVER_assume(isValid1Box(sol, bi, bj));
        }
    }

    assert(0); // output a counterexample that satisfies the constraints
}

