#include "utils_curses.h"
#include "utils_mpd.h"
#include "string.h"

#include <stdbool.h>
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

// print a column to screen
// while doing so, perform a rotation of the color
// modify here to change color output
void
print_col(int col, int l, const int maxR, const int maxC)
{
	int row, changeCol;
	int color = 5;

	for(row=maxR; row>=0; row--){
        if (col < maxC) {
            color++;
            changeCol = color % 6;
            if ((row > l && row < maxR-l/3)) { // center of the screen
                color_set(changeCol, NULL);
                mvaddch(row, col, SHARP);
            } else {
                color_set(changeCol, NULL);
                mvaddch(row, col, HEAVY);
            }
        }
	}
}

void
print_row(int row, int l, const int maxR, const int maxC)
{
	int col, changeCol;
	int color = 5;

	for(col=maxC; col>=0; col--){
        if (row < maxR) {
            color++;
            changeCol = color % 6;
            if ((col > l && col < maxR-l/3)) { // center of the screen
                color_set(changeCol, NULL);
                mvaddch(row, col, SHARP);
            } else {
                color_set(changeCol, NULL);
                mvaddch(col, col, HEAVY);
            }
        }
	}
}

/* prints a STATUS structure to stdout */
void 
print_mpd_status(STATUS* status, const int maxC, const int row)
{
    SONG* song = NULL;
    bool CRflag = false, nullFlag = false;
    int i,j;

    if(status == NULL){
        return;
    }

    song = status->song;
    int maxlen;
    int center;
    if (song->artist == NULL) {
        maxlen = strlen(song->uri) + 3;
        center = maxC/2 - maxlen/2;
    } else {
        maxlen = strlen(song->artist) + strlen(song->title) + 5;
        if (strlen(status->state)+18 > maxlen){
            maxlen=strlen(status->state)+18;
        }
        if (strlen(song->album + 3) > maxlen) {
            maxlen=strlen(song->album) + 3;
        }
        center = maxC/2 - (maxlen)/2; 
    }
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
            mvprintw (row+2, center+6, "---- No metadata avaiable ----");
        }
        if(status->state != NULL){
            mvprintw(row+3, center+1, "(%s)", status->state);
        }
        mvprintw(row+3, center+maxlen-10, "%d:%.2d/%d:%.2d ", status->elapsedTime_min, status->elapsedTime_sec, song->duration_min, song->duration_sec);
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

