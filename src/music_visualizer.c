#include <stdio.h>
#include <stdlib.h>		// exit
#include <unistd.h>		// read
#include <sys/stat.h>	// open	
#include <sys/time.h>   // timeout
#include <sys/types.h>
#include <fcntl.h>		// open
#include <stdint.h>		// uint16_t
#include <string.h>     // strcmp
#include <signal.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "fft.h"		// fast fourier transform
#include "utils_curses.h"
#include "settings.h"

#ifdef STATUS_CHECK
#include <mpd/client.h> // status
#include "utils_mpd.h"
static _Atomic bool getstatus = true;
#endif

static int maxR = 0;
static int maxC = 0;

#ifdef STATUS_CHECK
void
get_mpd_status(struct mpd_connection* session, STATUS* status)
{
    if(!session) {
        session = open_connection();
    }
    status = get_current_status(session);
}

void
alarm_status()
{
    getstatus = true;
}
#endif

void
process_fifo (uint16_t* buf, unsigned int* fftBuf, unsigned int* fftAvg, int nsamples) {
    // performs a Fourier Transform of the buffer data
    fast_fft(nsamples, (uint16_t*)buf, fftBuf);

    // computes an average of the signals in fftBuf
    // based on the number of columns of the screen
    average_signal(fftBuf, nsamples, maxC, fftAvg);
}

void
print_visual(unsigned int* fftBuf, unsigned int* fftAvg)
{
    int i;

    // main loop to print column by column
    for(i=X_CORRECTION; i<maxC+X_CORRECTION; i++){
        fftAvg[i] -= Y_CORRECTION; // adjust to display height

        // check boundaries (respect the boundaries of the screen, otherwise segvs)
        // if they don't, setting them to 1 is a safety measure
        if(fftAvg[i] > maxR || fftAvg[i] < 0){
            fftAvg[i] = 1;
        }
        // print the column fftAvg[i]
        print_col(i-X_CORRECTION, fftAvg[i], maxR, maxC);
    }
}

void
main_event()
{
    uint16_t buf[N_SAMPLES];
    int fifo;
#ifdef STATUS_CHECK
    struct mpd_connection *session;
    STATUS* status = NULL;
#endif
    bool over = false;
    fd_set set;
    int ret;
	int cnt = 0; // used to set resolution (wether to skip / not to skip a read)
    uint32_t sampleRate = 0;
    int nsamples = N_SAMPLES; // adapt processing to sample rate

    // open mpd fifo
    while((fifo = open(MPD_FIFO, O_RDONLY)) == -1);
    // add it to select() set
    FD_ZERO(&set);
    FD_SET(fifo, &set);

    // allocate buffers used
    unsigned int* fftBuf= (unsigned int*)malloc(N_SAMPLES*sizeof(unsigned int));
    unsigned int* fftAvg = (unsigned int*)malloc(N_SAMPLES*sizeof(unsigned int));
    /*// set the result buffer to 0s*/
    memset(fftBuf, 0, N_SAMPLES*sizeof(unsigned int));
    memset(fftAvg, 0, N_SAMPLES*sizeof(unsigned int));

    // open connection to mpd and set alarm to refresh status
#ifdef STATUS_CHECK
    session = open_connection();
    signal(SIGALRM, alarm_status);
	alarm(STATUS_REFRESH);
#endif

    while(!over) {
        if (wgetch(stdscr) == 'q') {
            over = true;
        }
        // select on fifo socket
        // if > 0 means data to be read
        if ((ret = select(fifo+1, &set, NULL, NULL, NULL)) > 0) {
            // read data when avaiable
            if (sampleRate == 96000) {
                ret = read(fifo, (uint16_t*)buf, N_SAMPLES);
            } else if (sampleRate < 96000) {
                ret = read(fifo, (uint16_t*)buf, 2*N_SAMPLES);
            }
            // process read buffer
			if(cnt == NICENESS) {
            	process_fifo(buf, fftBuf, fftAvg, nsamples);
				cnt = 0;
			} else {
				cnt++;
			}
        }
        // refresh status at SIGALRM
#ifdef STATUS_CHECK
        if(getstatus) {
            // get mpd status
            free_status_st(status);
            status = get_current_status(session);
            if ((sampleRate = status->sampleRate) == 96000) {
                nsamples = N_SAMPLES/2;
            } else if (sampleRate < 96000) {
                nsamples = N_SAMPLES;
            }
            getstatus = false;
            // set alarm for status refresh
			alarm(STATUS_REFRESH);
        }
#endif
        if (ret > 0) {
            // clear screen for printing (only if new data on fifo)
			if(cnt == NICENESS) {
            	erase();
           		print_visual(fftBuf, fftAvg);
			}
        }
#ifdef STATUS_CHECK
        // print mpd status even if no new data is avaiable
        print_mpd_status(status, maxC, maxR/2+maxR/6);
#endif
        // refresh screen
        refresh();
    }
#ifdef STATUS_CHECK
    close_connection(session);
    free_status_st(status);
#endif
    free(fftAvg);
    free(fftBuf);
    close(fifo);
}

int
main(int argc, char *argv[])
{
	WINDOW *mainwin;
#ifdef STATUS_CHECK
    import_var_from_settings();
#endif

	if((mainwin = curses_init()) == NULL){
		exit(EXIT_FAILURE);
	}

	// get screen properties
	getmaxyx(stdscr, maxR, maxC);
	curs_set(0);
 	cbreak();
	nodelay(stdscr, TRUE);

	// call the fifo processor
    main_event();

	// free resources
	endwin();
	delwin(mainwin);
	return 0;
}
