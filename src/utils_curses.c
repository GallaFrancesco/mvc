#include <stdbool.h>

#include "utils_curses.h"
#include "utils_mpd.h"
#include "string.h"
#include "settings.h"
#include "mt/mt19937ar.h"


// initialize color pairs
// used in print_col
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

// returns a WINDOW object
// called by main() on startup
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
print_pattern(int col, int row, int l, const int maxR, const int maxC, PATTERN pattern, int seed)
{
	bool randc;
	switch(pattern) {
		case CURVE:
			if (row > maxR-l && row < maxR-l/3) { // center of the screen
				mvaddch(row, col, FULL);
			} else if (row == maxR-l) { // center of the screen
				mvaddch(row, col, HALFFULL);
			} else if (row == maxR-l/3) { // center of the screen
				mvaddch(row, col, HALFEMPTY);
			} else if(EMPTY != ' ') {
				mvaddch(row, col, EMPTY);
			}
			break;

		case MOUTH:
            if (!(row > l && row < maxR-l/3)) { // center of the screen
                mvaddch(row, col, FULL);
			} else if(EMPTY != ' ') {
                mvaddch(row, col, EMPTY);
            }
			break;

		case MOUTH_REV:
            if (row > l && row < maxR-l/3) { // center of the screen
                mvaddch(row, col, FULL);
			} else if(EMPTY != ' ') {
                mvaddch(row, col, EMPTY);
            }
			break;
		case LINE:
            if (row > (maxR-l) && row < maxR) { // center of the screen
                mvaddch(row, col, FULL);
			} else if(EMPTY != ' ') {
                mvaddch(row, col, EMPTY);
            }
			break;
		case RANDOM:
			row = genrand_int32() % maxR;
			if(seed > 28) {
				mvaddch(row,col,FULL);
			} else if (seed < 28 && seed > 24) {
				mvaddch(row,col,EMPTY);
			} else if (seed < 14 && seed > 10) {
				mvaddch(row,col,HALFEMPTY);
			} else if (seed < 10) {
				mvaddch(row,col,HALFFULL);
			} else {
				mvaddch(row,col,' ');
			}
			break;
		case ANOTHER:
			if(row >= l+24) {
				mvaddch(row-maxR/2,col,FULL);
				mvaddch(maxR/2+maxR-row,col,FULL);
				mvaddch(maxR/2, col, FULL);
			} else {
				mvaddch(row,col,EMPTY);
			}
			break;
		default:
			break;
	}
}

// print a column to screen
// while doing so, perform a rotation of the color
// modify here to change color output
void
print_col(int col, int l, const int maxR, const int maxC, PATTERN pattern, int seed)
{
	int row;
	int color = 5;

	for (row=0; row<maxR; row++){
        if (col < maxC) {
            color = (color+1) % 6;
            color_set(color, NULL);
			print_pattern(col, row, l, maxR, maxC, pattern, seed);
        }
	}
}

void
print_help(const int maxR, const int maxC)
{
	erase();
    color_set(1, NULL);
	mvprintw(0, 0, "MVC is a curses-based music visualizer");
    color_set(2, NULL);
	mvprintw(2, 0, "Configuration: see src/settings.h");
	mvprintw(4, 0, "Keybindings:");
    color_set(3, NULL);
	mvprintw(5, 0, "* Quit: q");
	mvprintw(6, 0, "* Change drawing mode (style): Space bar");
	mvprintw(7, 0, "* Move status panel (if built with libmpdclient): up / down / left / right keys");
	mvprintw(8, 0, "* Reset status panel position: r");
	mvprintw(9, 0, "* Toggle status display: t");
	mvprintw(10, 0, "* Print this help: h");
    color_set(1, NULL);
	mvprintw(11, 0, "--> Press any key to continue.");
	timeout(-1);
	getch();
	timeout(0);
}

#ifdef STATUS_CHECK
void
print_rate_info(const int rate, const int nsamples, const int maxC, int seed)
{
    int center = (int) maxC/2-18; // adjust to screen center
    int i;
    srand(seed);
    color_set(rand() % 7, NULL);

    mvprintw(1, center, " || rate: %d - samples: %d || ", rate, nsamples);
    if (nsamples != 512 && rate > 0) {
        mvprintw(0, center, " --------------------------------- ", rate, nsamples);
        mvprintw(2, center, " -------------[mvc]--------------- ", rate, nsamples);
    } else if (rate == 0) {
        mvprintw(0, center, " ----------------------------- ", rate, nsamples);
        mvprintw(2, center, " -----------[mvc]------------- ", rate, nsamples);
    } else {
        mvprintw(0, center, " -------------------------------- ", rate, nsamples);
        mvprintw(2, center, " ------------[mvc]--------------- ", rate, nsamples);
    }
}


/* prints a STATUS structure to stdout */
void
print_mpd_status(STATUS* status, const int maxC, const int row, const int moveCol)
{
    SONG* song = NULL;
    bool nullFlag = false;
    int i,j;

    if(status == NULL){
        return;
    }

    song = status->song;
    int maxlen;
    int center;
    int first_len;
    int second_len;
    int third_len;

    if (song == NULL) {
        return;
    }

    if (song->artist == NULL || song->title == NULL) {
        maxlen = strlen(song->uri) + 3;
        center = maxC/2 - maxlen/2;
    } else {
        first_len = strlen(song->artist) + strlen(song->title) + 5; // artist + title
        second_len = strlen(status->state)+18; // state + elapsed
        third_len = strlen(song->album) + 3; // album

        maxlen = first_len;
        if (second_len > maxlen){
            maxlen=second_len;
        }
        if (third_len > maxlen) {
            maxlen=third_len;
        }
        center = maxC/2 - (maxlen)/2; 
    }
	center += moveCol;
    srand(song->duration_sec);
    color_set(rand() % 7, NULL);
    for (i=0; i<maxlen+4; i++) {
        for(j=0; j<3; j++) {
            mvaddch(row+1+j, center-1+i, ' ');
        }
    }
    if(song != NULL){
        if(song->artist != NULL){
            mvprintw(row+1, center+1, "%s - ", song->artist);
            nullFlag = true;
        }
        if(song->title != NULL){
            mvprintw(row+1, center+1+strlen(song->artist) + 3, "%s", song->title);
            nullFlag = true;
        }
        if(song->album != NULL){
            mvprintw(row+2, center+1, "%s", song->album);
        }
        if (nullFlag == false) {
            // title, artist, album fields are missing, will print filesystem name
            mvprintw (row+1, center+1, "%s", song->uri);
            mvprintw (row+2, center+1, "[No metadata]");
        }
        if(status->state != NULL){
            mvprintw(row+3, center+1, "(%s)", status->state);
        }
        mvprintw(row+3, center+maxlen-11, "%d:%.2d/%d:%.2d ", status->elapsedTime_min, status->elapsedTime_sec, song->duration_min, song->duration_sec);
    }
    for (i=0; i<maxlen+4; i++) {
        mvaddch(row, center-2+i, '-');
        mvaddch(row+4, center-2+i, '-');
    }
    mvaddch(row, center-2+i, ' ');
    mvaddch(row+4, center-2+i, ' ');
    mvaddch(row, center-3, ' ');
    mvaddch(row+4, center-3, ' ');
    for (i=0; i<3; i++){
        mvaddch(row+i+1, center-3, ' ');
        mvaddch(row+i+1, center-1, '|');
        mvaddch(row+i+1, center-2, '|');
        mvaddch(row+i+1, center+maxlen, '|');
        mvaddch(row+i+1, center+1+maxlen, '|');
    }
    return;
}
#endif
