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
#include <locale.h>
#include <mpd/client.h> // status
#include "utils_mpd.h"
static _Atomic bool getstatus = true;
#endif

#define BEATC 27 // experiment with it

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

int
test_beat(unsigned int* fftEHist, int sEnergyLen, unsigned int* sEnergy)
{
    int i,j;
    for(i=0; i<sEnergyLen; i++) {
        // shift right
        for(j=sEnergyLen-1; j>=0; j--) {
            fftEHist[j] = fftEHist[j-1];  
        }
        // insert element
        fftEHist[0] =  sEnergy[i];
    
        for(j=0; j<sEnergyLen; j++) {
            /*fprintf(stderr, "- %d - %d\n", sEnergy[i], fftEHist[j]);*/
            if (sEnergy[i] > fftEHist[j]+BEATC && sEnergy[i] < 200) { // 200 avg would be false read
                /*fprintf(stderr, "beat: %d - %d\n", sEnergy[i], fftEHist[j]);*/
                return 1;
                /*} else {*/
                /*fprintf(stderr, "no beat!");*/
            }
        }
    }
    return 0;
}

void
process_fifo (uint16_t* buf, unsigned int* fftBuf, unsigned int* fftAvg, \
        unsigned int* sEnergy, int nsamples, int sEnergyLen)
{
    fast_fft(nsamples, (uint16_t*)buf, fftBuf);

    // computes an average of the signals in fftBuf
    // based on the number of columns of the screen
    average_signal(fftBuf, nsamples, maxC, fftAvg);

    // compute the energy of each subband
    /*energy_sub_signal(fftBuf, nsamples, nsamples/sEnergyLen, sEnergy);*/
}

void
print_visual(unsigned int* fftAvg, PATTERN pattern)
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
        print_col(i-X_CORRECTION, fftAvg[i], maxR, maxC, pattern);
    }
}

void
main_event(int fifo)
{
    uint16_t buf[N_SAMPLES];
#ifdef STATUS_CHECK
    struct mpd_connection *session;
    STATUS* status = NULL;
#endif
    bool over = false;
    fd_set set;
    int ret;
	int c;
	int cnt = 0; // used to set resolution (wether to skip / not to skip a read)
    uint32_t sampleRate = 0;
    int nsamples = N_SAMPLES; // adapt processing to sample rate
    int sEnergyLen = N_SAMPLES/32;
	PATTERN pattern = CURVE;
	int statusHeight = 0;
	int statusCol = 0;
	bool toggleStatus = true;
    short cnt_over = 0; // in case no data is available for too much

    // add it to select() set
    FD_ZERO(&set);
    FD_SET(fifo, &set);

    // allocate buffers used
    unsigned int* fftBuf= (unsigned int*)malloc(N_SAMPLES*sizeof(unsigned int));
    unsigned int* fftAvg = (unsigned int*)malloc(N_SAMPLES*sizeof(unsigned int));
    unsigned int* fftEHist = (unsigned int*)malloc(sEnergyLen*sizeof(unsigned int));
    unsigned int* sEnergy = (unsigned int*)malloc(sEnergyLen*sizeof(unsigned int));
    /*// set the result buffer to 0s*/
    memset(fftBuf, 0, N_SAMPLES*sizeof(unsigned int));
    memset(fftAvg, 0, N_SAMPLES*sizeof(unsigned int));
    memset(fftEHist, 0, sEnergyLen*sizeof(unsigned int));
    memset(sEnergy, 0, sEnergyLen*sizeof(unsigned int));

    // open connection to mpd and set alarm to refresh status
#ifdef STATUS_CHECK
    session = open_connection();
    signal(SIGALRM, alarm_status);
	alarm(STATUS_REFRESH);
#endif

    while(!over) {
        if ((c = wgetch(stdscr)) == 'q') {
            over = true;
        } else if (c == ' ') {
			pattern = (pattern + 1) % 4;
		} else if (c == KEY_UP) {
			statusHeight -= 1;
		} else if (c == KEY_DOWN) {
			statusHeight += 1;
		} else if (c == KEY_LEFT) {
			statusCol -= 1;
		} else if (c == KEY_RIGHT) {
			statusCol += 1;
		} else if (c == 'r') {
			statusCol = 0;
			statusHeight = 0;
		} else if (c == 'h') {
			print_help(maxR,maxC);
		} else if (c == 't') {
			toggleStatus = (toggleStatus == true) ? false : true;
		}

        // select on fifo socket
        // if > 0 means data to be read
        if ((ret = select(fifo+1, &set, NULL, NULL, NULL)) > 0) {
            // read data when avaiable
            // adapt to sample rate for buffer processing
            // the first time default to 44100 setting
            ret = read(fifo, (uint16_t*)buf, 2*nsamples);
            // process read buffer
			if(cnt == NICENESS) {
            	process_fifo(buf, fftBuf, fftAvg, sEnergy, nsamples, sEnergyLen);
				cnt = 0;
			} else {
				cnt++;
			}
        } else {
            cnt_over++;
            mvprintw(cnt_over-1, 0, "[ERR %d] No data available for reading from %s, is MPD playing?",cnt_over, MPD_FIFO);
            if(cnt_over == 4) mvprintw(cnt_over+1, 0, "Exiting.");
            if(cnt_over == 5) break;
        }
        // refresh status at SIGALRM
#ifdef STATUS_CHECK
        if(getstatus) {
            // get mpd status
            free_status_st(status);
            status = get_current_status(session);
            if (status) {
                if (((sampleRate = status->sampleRate) >= 96000) && ADAPTIVE_SAMPLING) {
                    nsamples = N_SAMPLES/2;
                } else if (sampleRate < 96000) {
                    nsamples = N_SAMPLES;
                }
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
           		print_visual(fftAvg, pattern);
			}
        }
#ifdef STATUS_CHECK
        // print mpd status even if no new data is avaiable
		if (toggleStatus) {
			if (status && status->song && status->song->duration_sec) {
				print_rate_info(sampleRate, nsamples, maxC, status->song->duration_sec, \
						0, 5*test_beat(fftEHist, sEnergyLen, sEnergy));
			}
			print_mpd_status(status, maxC, statusHeight+maxR/2+maxR/6, statusCol);
		}
#endif
        // refresh screen
        refresh();
	    getmaxyx(stdscr, maxR, maxC);
    }

#ifdef STATUS_CHECK
    close_connection(session);
    free_status_st(status);
#endif
    free(fftAvg);
    free(fftBuf);
    free(fftEHist);
    free(sEnergy);
    close(fifo);
}

int
main(int argc, char *argv[])
{
	WINDOW *mainwin;
    int fifo;

    // open mpd fifo
    if ((fifo = open(MPD_FIFO, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open %s\n", MPD_FIFO);
        exit(EXIT_FAILURE);
    }

	if((mainwin = curses_init()) == NULL){
		exit(EXIT_FAILURE);
	}
#ifdef STATUS_CHECK
    setlocale(LC_ALL, "");
    import_var_from_settings();
	keypad(stdscr, TRUE); // arrow keys
#endif

	// get screen properties
	getmaxyx(stdscr, maxR, maxC);
	curs_set(0);
 	cbreak();
	nodelay(stdscr, TRUE);


	// call the fifo processor
    main_event(fifo);

	// free resources
	endwin();
	delwin(mainwin);
	return 0;
}
