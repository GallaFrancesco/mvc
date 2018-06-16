#include <curses.h>

#include "utils_mpd.h"

// used while printing
#define FULL 'X'
#define EMPTY '.'

void init_color_pairs();
WINDOW* curses_init();
void print_col(int col, int length, const int maxR, const int maxC);
void print_row(int row, int length, const int maxR, const int maxC);

#ifdef STATUS_CHECK
void print_mpd_status(STATUS* status, const int maxC, const int row);
#endif