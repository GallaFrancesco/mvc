#include <curses.h>

#define SHARP '#'

void init_color_pairs();
WINDOW* curses_init();
void print_col(int col, int length, int maxR);
