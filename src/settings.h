/*
 **** MPD options
 */
static char defHost[] = "192.168.1.131";

static unsigned int defPort = 6600;

// database location, needed for libmpdclient
static char defDBlocation[] = "~/.mpd/mpd.db";

// timeout for MPD connection to fail
#define TIMEOUT 1000

/*
 **** Visualizer options
 */
// number of samples
// (a power of 2)
#define N_SAMPLES 1024

// if adaptive sampling should be used
// (adjust N_SAMPLES to sample rate of current song)
// uses N_SAMPLES as maximum for 44100 Hz
// set to 0 to disable
#define ADAPTIVE_SAMPLING 1

// path to the MPD named pipe
// (defined: 'mpd.conf')
#define MPD_FIFO "/tmp/mpd.fifo"

// seconds between each mpd status refresh
#define STATUS_REFRESH 5

// nuber of buffer reads whose computation is skipped (cyclically).
// More skips mean less precision, nicer on older CPUs
// 0 -> do not skip any
// 1 -> process one out of two
// 2 -> process one out of three
// etc
#define NICENESS 0

// subtracted to each component (visualized column)
// adjust according to screen height
#define Y_CORRECTION 28

// adjust lateral shift on screen
// might prevent full borders from being printed, adjust
#define X_CORRECTION 1

// used while printing
#define FULL 'X'
#define EMPTY '.'
