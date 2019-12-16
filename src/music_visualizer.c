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
#include <pthread.h>

#include "fft.h"		// fast fourier transform
#include "utils_curses.h"
#include "beat_track.h"
#include "settings.h"
#include "mt/mt19937ar.h"

#ifdef STATUS_CHECK
#include <locale.h>
#include <mpd/client.h> // status
#include "utils_mpd.h"
static _Atomic bool getstatus = true;
void
alarm_status()
{
    getstatus = true;
}
#endif

#define BEATC 25 // experiment with it

static int maxR = 0;
static int maxC = 0;

void
process_fifo (uint16_t* buf, unsigned int* fftBuf, unsigned int* fftAvg, \
              cebuffer* energyBuffer, int nsamples, unsigned int* energyThreshold, bool* beat)
{
    // fft of samples array
    fast_fft(nsamples, (uint16_t*)buf, fftBuf);

    // computes an average of the signals in fftBuf
    // based on the number of columns of the screen
    average_signal(fftBuf, nsamples, maxC, fftAvg);

    unsigned int avg = 0;
    if(energyBuffer->count > 0) {
        // compute the energy of the samples array
        avgEnergy(fftBuf, nsamples, &avg);

        *beat = cb_beat(energyBuffer, avg, energyThreshold);
    }

    // insert the current energy sample into the circular buffer
    cb_push_back(energyBuffer, avg);
}

void
print_visual(unsigned int* fftAvg, PATTERN pattern, bool beat)
{
    int i;

    // main loop to print column by column
    for(i=0; i<maxC; i++){
        fftAvg[i] -= (Y_CORRECTION - i/(maxC/8)); // adjust to display height

        // check boundaries (respect the boundaries of the screen, otherwise segvs)
        // if they don't, setting them to 1 is a safety measure
        if(fftAvg[i] > maxR || fftAvg[i] < 0){
            fftAvg[i] = 1;
        }

        // print the column fftAvg[i]
        print_col(i, fftAvg[i], maxR, maxC, pattern, (int)fftAvg[i]);
    }
}

int
main_event(int fifo, WINDOW* mainwin, WINDOW* sub)
{
#ifdef THREADED
	pthread_t tid;
#endif
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
    int sEnergyLen = N_SAMPLES;
	PATTERN pattern = LINE;
	int statusHeight = 0;
	int statusCol = 0;
	bool toggleStatus = true;
	bool subWindow = true;
    short cnt_over = 0; // in case no data is available for too much
    unsigned int energyThreshold = 0;
    bool beat = false;

    // add it to select() set
    FD_ZERO(&set);
    FD_SET(fifo, &set);

    // allocate buffers used
    unsigned int* fftBuf= (unsigned int*)malloc(N_SAMPLES*sizeof(unsigned int));
    unsigned int* fftAvg = (unsigned int*)malloc(maxC*sizeof(unsigned int));
    cebuffer energyBuffer;

    cb_init(&energyBuffer, 43);

	// set the result buffer to 0s
    memset(fftBuf, 0, N_SAMPLES*sizeof(unsigned int));
    memset(fftAvg, 0, maxC*sizeof(unsigned int));

    // open connection to mpd and set alarm to refresh status
#ifdef STATUS_CHECK
    session = open_connection();
    signal(SIGALRM, alarm_status);
	alarm(STATUS_REFRESH);
#endif

    while(!over) {
        switch((c = wgetch(stdscr))) {
        case 'q':
            over = true;
            break;
        case ' ':
            pattern = (pattern + 1) % 6;
            break;
        case KEY_UP:
            statusHeight -= 1;
            break;
        case KEY_DOWN:
            statusHeight += 1;
            break;
        case KEY_LEFT:
            statusCol -= 1;
            break;
        case KEY_RIGHT:
            statusCol += 1;
            break;
        case 'r':
            statusCol = 0;
            statusHeight = 0;
            break;
        case 'h':
            print_help(maxR,maxC);
            break;
        case 't':
            toggleStatus = (toggleStatus == true) ? false : true;
            break;
        case 's':
            subWindow = (subWindow == true) ? false : true;
            break;
        default:
            break;
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
            	process_fifo(buf, fftBuf, fftAvg, &energyBuffer, nsamples, &energyThreshold, &beat);
				cnt = 0;
			} else {
				cnt++;
			}
			cnt_over = 0;
        } else {
            cnt_over++;
			if (cnt_over == 60) break; // wait ~ 1 min
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
				print_visual(fftAvg, pattern, beat);
			}
        }
#ifdef STATUS_CHECK
        // print mpd status even if no new data is avaiable
		if (toggleStatus) {
			if (status && status->song && status->song->duration_sec) {
				print_rate_info(sampleRate, nsamples, maxC, status->song->duration_sec, beat);
				cnt_over = 0;
			}
			print_mpd_status(status, maxC, maxR/2-statusHeight-2, statusCol);
		}
#endif
        // refresh screen
		box(mainwin, 0, 0);
        wrefresh(mainwin);

        if(subWindow) {
            print_subw(sub, beat, maxR/10, maxC);
            box(sub, 0, 0);

            wrefresh(sub);

            // refresh sub window
            wresize(sub, maxR/10, maxC);
            mvwin(sub, maxR*9/10+1, 0);
        }

        getmaxyx(stdscr, maxR, maxC);
    }

#ifdef STATUS_CHECK
    close_connection(session);
    free_status_st(status);
#endif
    free(fftAvg);
    free(fftBuf);
    //    cb_free(&energyBuffer);
    close(fifo);

    if(cnt_over) return 1;
	return 0;
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

	unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
	init_genrand(time(NULL));

	// get screen properties
	getmaxyx(stdscr, maxR, maxC);
	curs_set(0);
 	cbreak();
	nodelay(stdscr, TRUE);

    // space invaders window
    WINDOW* sub = subwin(mainwin, maxR/10, maxC, maxR*9/10+1, 0);

	// call the fifo processor
    int res = main_event(fifo, mainwin, sub);

	// free resources
	endwin();
	delwin(sub);
	delwin(mainwin);

	if(res) fprintf(stderr, "No data in %s\nQuit.\n", MPD_FIFO);
	else fprintf(stdout, "Received exit command.\nQuit.");

	return 0;
}
