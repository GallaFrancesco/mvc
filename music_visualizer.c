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
	curs_set(0);
	
	while(read(fifo, (uint16_t*)buf, 2*N_SAMPLES) != 0){
		
		fftBuf = fast_fft(N_SAMPLES, (uint16_t*)buf);
		fftAvg = average_signal(fftBuf, N_SAMPLES, maxC, &avgLen);	
		free(fftBuf);

		erase();
		for(i=0; i<avgLen; i++){
			if(i<avgLen/3){
				color_set(1, NULL);
			} else if(i>avgLen*2/3){
				color_set(2, NULL);	
			} else {
				color_set(3, NULL);
			}
			print_col(i, fftAvg[i], maxR);
		}
		refresh();
	}
	close(fifo);
	delwin(mainwin);
	endwin();

	return 0;

}
