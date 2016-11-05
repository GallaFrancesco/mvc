#include "utils_curses.h"


void
init_color_pairs()
{
	if(has_colors()){
		start_color();

		init_pair(1, COLOR_RED,     COLOR_BLACK);
		init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);		
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
print_col(int col, int length, int maxR)
{
	int row, l;
	 
	l = maxR - length;
	for(row=maxR; row>l; row--){
		mvaddch(row, col, SHARP);
	}
}

