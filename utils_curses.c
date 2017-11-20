#include "utils_curses.h"


void
init_color_pairs()
{
	if(has_colors()){
		start_color();
		use_default_colors();
		init_pair(1, COLOR_RED,     -1);
		init_pair(2, COLOR_GREEN,   -1);
        init_pair(3, COLOR_YELLOW,  -1);
        init_pair(4, COLOR_BLUE,    -1);
        init_pair(5, COLOR_CYAN,    -1);
        init_pair(6, COLOR_MAGENTA, -1);
        init_pair(8, COLOR_WHITE,   -1);		
	}
}

WINDOW*
curses_init()
{
	WINDOW *w;
	if((w = initscr()) == NULL){ //initialize curses library on current screen
		fprintf(stderr, "Error opening screen\n");
		return NULL;
	}
	init_color_pairs();
	return w;
}

void
print_col(int col, int l, int maxR, int color)
{
	int row;
	int changeCol;
	for(row=maxR; row>=0; row--){
		color++;
		changeCol = color % 6;
		if (!(row > l && row < maxR-l/3)) { // center of the screen
			color_set(changeCol, NULL);
			mvaddch(row, col, SHARP);
		}
	}
}

