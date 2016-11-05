#include <stdio.h>
#include <stdlib.h>		// exit
#include <unistd.h>		// read
#include <sys/stat.h>	// open	
#include <fcntl.h>		// open
#include <stdint.h>		// uint16_t
#include "fft.h"		// fast fourier transform
#include "utils_curses.h"

/*FREQ		44100*/
#define N_SAMPLES 	1024
/*FPS		FREQ/N_SAMPLES*/

int
main(int argc, char *argv[])
{
	int fifo, i;
	WINDOW *mainwin;
	int maxR, maxC, avgLen; //curses
	uint16_t buf[N_SAMPLES];
	unsigned int *fftBuf, *fftAvg;

	fifo = open("/tmp/mpd.fifo", O_RDONLY);
	
	if((mainwin = curses_init()) == NULL){
		exit(EXIT_FAILURE);
	}
	getmaxyx(stdscr, maxR, maxC);
	color_set(2, NULL);
	mvprintw(maxR/2, maxC/2, "%d %d\n", maxR, maxC);
	refresh();
	
	while(read(fifo, (uint16_t*)buf, 2*N_SAMPLES) != 0){
		
		fftBuf = fast_fft(N_SAMPLES, (uint16_t*)buf);
		fftAvg = average_signal(fftBuf, N_SAMPLES, maxC, &avgLen);	
		free(fftBuf);

		for(i=0; i<avgLen; i++){
			print_col(i, fftAvg[i], maxR);
		}
		refresh();
		clear();
	}
	close(fifo);
	delwin(mainwin);
	endwin();

	return 0;

}
