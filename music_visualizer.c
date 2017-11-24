#include <stdio.h>
#include <stdlib.h>		// exit
#include <unistd.h>		// read
#include <sys/stat.h>	// open	
#include <fcntl.h>		// open
#include <stdint.h>		// uint16_t
#include "fft.h"		// fast fourier transform
#include "utils_curses.h"
#include "time.h"

#define MPD_FIFO "/tmp/mpd.fifo"
/*FREQ		44100*/
#define N_SAMPLES 1024
/*FPS		FREQ/N_SAMPLES*/

void process_mpd (int fifo) {
	uint16_t buf[N_SAMPLES];
	unsigned int *fftBuf, *fftAvg;
	int maxR, maxC, correction; //curses


	getmaxyx(stdscr, maxR, maxC);

	while(read(fifo, (uint16_t*)buf, 2*N_SAMPLES) != 0){
		/*usleep(45000);*/
		if(wgetch(stdscr)=='q'){
			break;
		}

		fftBuf = fast_fft(N_SAMPLES, (uint16_t*)buf);
		fftAvg = average_signal(fftBuf, N_SAMPLES, maxC);	
		free(fftBuf);

		erase();
		correction = 0;
		for(i=correction; i<maxC; i=i+2){
			/*color_set(atoi(argv[1]), NULL);*/
			if(fftAvg[i] > maxR || fftAvg[i] < 0){
				fftAvg[i] = 1;
			}
			print_col(i-correction, fftAvg[i], maxR, atoi(argv[1]));
			/*print_col(i-correction+1, fftAvg[i], maxR);*/
		}
		refresh();
		free(fftAvg);
	}
}

int
main(int argc, char *argv[])
{
	int fifo, i, j;
	WINDOW *mainwin;

	if (strcmp (argv[2], "--alsa") == 0) {
	
	}
	while((fifo = open(MPD_FIFO, O_RDONLY)) == -1);

	if (argc != 2){
		fprintf(stderr, "Usage: mvc [color number]\n");
		exit(EXIT_FAILURE);
	}


	if((mainwin = curses_init()) == NULL){
		exit(EXIT_FAILURE);
	}
	getmaxyx(stdscr, maxR, maxC);
	curs_set(0);
 	cbreak();
	nodelay(stdscr, TRUE);

	while(read(fifo, (uint16_t*)buf, 2*N_SAMPLES) != 0){
		/*usleep(45000);*/
		if(wgetch(stdscr)=='q'){
			break;
		}

		fftBuf = fast_fft(N_SAMPLES, (uint16_t*)buf);
		fftAvg = average_signal(fftBuf, N_SAMPLES, maxC);	
		free(fftBuf);

		erase();
		correction = 0;
		for(i=correction; i<maxC; i=i+2){
			/*color_set(atoi(argv[1]), NULL);*/
			if(fftAvg[i] > maxR || fftAvg[i] < 0){
				fftAvg[i] = 1;
			}
			print_col(i-correction, fftAvg[i], maxR, atoi(argv[1]));
			/*print_col(i-correction+1, fftAvg[i], maxR);*/
		}
		refresh();
		free(fftAvg);
	}
	close(fifo);
	endwin();
	delwin(mainwin);

	return 0;
}
