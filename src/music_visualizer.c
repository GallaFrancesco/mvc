#include <stdio.h>
#include <stdlib.h>		// exit
#include <unistd.h>		// read
#include <sys/stat.h>	// open	
#include <fcntl.h>		// open
#include <stdint.h>		// uint16_t
#include <string.h>     // strcmp
#include "fft.h"		// fast fourier transform
#include "utils_curses.h"
#include "alsa_fifo.h"

#define MPD_FIFO "/tmp/mpd.fifo"
/*FREQ		44100*/
#define N_SAMPLES 1024
/*FPS		FREQ/N_SAMPLES*/

static int maxR;
static int maxC;

void process_mpd (int fifo) {
	uint16_t buf[N_SAMPLES];
	unsigned int *fftBuf, *fftAvg;
	int correction; //curses

	while(read(fifo, (uint16_t*)buf, 2*N_SAMPLES) != 0){
		// close on keypress 'q'
		if(wgetch(stdscr)=='q'){
			break;
		}

		// performs a Fourier Transform of the buffer data
		fftBuf = fast_fft(N_SAMPLES, (uint16_t*)buf);

		// computes an average of the signals in fftBuf
		// based on the number of columns of the screen
		fftAvg = average_signal(fftBuf, N_SAMPLES, maxC);	
		free(fftBuf);

		// clear the screen
		erase();

		// correction can be used to exclude certain frequencies
		// not advised nor customary
		correction = 0;
		int i;
		for(i=correction; i<maxC; i=i+2){
			// check boundaries of the signals respect the boundaries of the screen
			// if they don't, setting them to 1 is a safety measure
			// 0 and maxR can give segmentation errors on curses printing
			if(fftAvg[i] > maxR || fftAvg[i] < 0){
				fftAvg[i] = 1;
			}
			// print the column fftAvg[i]
			print_col(i-correction, fftAvg[i], maxR);
		}
		// refresh the screen, free the allocated buffer
		refresh();
		free(fftAvg);
	}
}

void
process_alsa (int fifo)
{}

int
main(int argc, char *argv[])
{
	int fifo, i, j;
	WINDOW *mainwin;

	while((fifo = open(MPD_FIFO, O_RDONLY)) == -1);

	if (argc > 2 || argc < 1){
		fprintf(stderr, "Usage: mvc [--alsa/--mpd]\nDefault is MPD.");
		exit(EXIT_FAILURE);
	}

	if((mainwin = curses_init()) == NULL){
		exit(EXIT_FAILURE);
	}
	// get screen properties
	getmaxyx(stdscr, maxR, maxC);
	curs_set(0);
 	cbreak();
	nodelay(stdscr, TRUE);

	// call the fifo processor
	if (strcmp (argv[2], "--alsa") == 0) {
		process_alsa(fifo);
	} else {
		process_mpd(fifo);
	}

	// free resources
	close(fifo);
	endwin();
	delwin(mainwin);

	return 0;
}
