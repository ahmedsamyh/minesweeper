#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

typedef enum {
    EMPTY,
    MINE,
    COUNT
} Cell_Type;

typedef struct {
    Cell_Type type;
    bool open, flag;
} Cell;

#define COLS 10
#define ROWS 10
typedef struct {
    Cell cells[ROWS*COLS];
    size_t cur_row, cur_col;
    size_t mines_count;
    size_t open_cells_count;
} Grid;

static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }
static inline int randi(size_t max_inclusize) {
    return rand() % max_inclusize;
}

Cell cell_at(Grid* grid, size_t row, size_t col) {
    size_t idx = row * COLS + col;
    assert(idx < ROWS*COLS);
    return grid->cells[idx];
}

void set_cell(Grid* grid, size_t row, size_t col, const Cell_Type c) {
    size_t idx = row * COLS + col;
    grid->cells[idx].type = c;
}

void randomize_grid(Grid* grid) {
    for (int i = 0; i < grid->mines_count; ++i) {
        size_t rand_row = randi(ROWS-1);
        size_t rand_col = randi(COLS-1);

        while (cell_at(grid, rand_row, rand_col).type == MINE) {
            rand_row = randi(ROWS-1);
            rand_col = randi(COLS-1);
        }
        set_cell(grid, rand_row, rand_col, MINE);
    }
}


void init_grid(Grid* grid, size_t mines_count) {
    grid->open_cells_count = 0;
    grid->mines_count = mines_count;
    randomize_grid(grid);
}

size_t count_nbors(Grid* grid, size_t row, size_t col) {
    size_t nbors = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx != 0 || dy != 0) {
                size_t r = row + dy;
                size_t c = col + dx;
                if (r < 0 || r > ROWS-1 || c < 0 || c > COLS-1) {
                    continue;
                }
                if (cell_at(grid, r, c).type == MINE) {
                    nbors++;
                }
            }
        }
    }
    return nbors;
}

void draw_grid(Grid* grid) {
    printf("%zu x %zu | mines: %d | open: %d/%d\n", COLS, ROWS, grid->mines_count, grid->open_cells_count, ROWS*COLS);

    for (int c = 0; c < COLS*3 + 2; ++c) {
        printf("-");
    }
    printf("\n");
    for (int r = 0; r < ROWS; ++r) {
        printf("|");
        for (int c = 0; c < COLS; ++c) {
            size_t idx = r * COLS + c;
            Cell cell = cell_at(grid, r, c);
            printf("%c", grid->cur_row == r && grid->cur_col == c ? '[' : ' ');
            if (!cell.open) {
                if (cell.flag) {
                    printf("F");
                } else {
                    printf("#");
                }
            } else {
                switch (cell.type) {
                    case EMPTY: {
                        size_t nbors = count_nbors(grid, r, c);
                        printf("%c", nbors == 0 ? ' ' : '0' + nbors);
                    } break;
                    case MINE: {
                        printf("*");
                    } break;
                    case COUNT:
                    default:
                        assert(false && "Unreachable!");
                }
            }
            printf("%c", grid->cur_row == r && grid->cur_col == c ? ']' : ' ');
        }
        printf("|\n");
    }
    for (int c = 0; c < COLS*3 + 2; ++c) {
        printf("-");
    }
    printf("\n");
}

int main(void) {
    srand(time(0));
    Grid grid = {0};
    size_t max_mines = 25;
    init_grid(&grid, max_mines);

    struct termios tattr = {0};

    // Make sure stdin is a terminal
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "ERROR: stdin is not a terminal!\n");
        exit(1);
    }

    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON|ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);

    char cmd;

    bool quit = false;
    while (!quit) {
        draw_grid(&grid);
        read(STDIN_FILENO, &cmd, 1);

        switch (cmd) {
            case 'd': if (grid.cur_col < COLS-1) grid.cur_col++; break;
            case 'a': if (grid.cur_col > 0) grid.cur_col--; break;
            case 's': if (grid.cur_row < ROWS-1) grid.cur_row++; break;
            case 'w': if (grid.cur_row > 0) grid.cur_row--; break;
            case ' ': {
                size_t idx = grid.cur_row * COLS + grid.cur_col;
                assert(idx < COLS*ROWS);
                grid.cells[idx].open = true;
                grid.open_cells_count++;
                if (grid.cells[idx].type == MINE) {
                    for (int r = 0; r < ROWS; ++r) {
                        for (int c = 0; c < COLS; ++c) {
                            size_t idx = r * COLS + c;
                            assert(idx < COLS*ROWS);
                            grid.cells[idx].open = true;
                            grid.open_cells_count++;
                        }
                    }
                }
            } break;
            case 'f': {
                size_t idx = grid.cur_row * COLS + grid.cur_col;
                assert(idx < COLS*ROWS);
                grid.cells[idx].flag = !grid.cells[idx].flag;
            } break;
            case 'q': quit = true;
            case 'r': init_grid(&grid, max_mines); break;
        }

        printf("\033[%dA", ROWS+3);
        printf("\033[%dD", COLS*3+2);
    }

    return 0;
}
