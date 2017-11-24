#include <curses.h>

// used while printing
#define SHARP 'x'
#define HEAVY '.'

void init_color_pairs();
WINDOW* curses_init();
void print_col(int col, int length, int maxR);
