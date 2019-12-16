/* =========== Generic Options ============ */

// path to the MPD named pipe
// (defined: 'mpd.conf')
#define MPD_FIFO "/tmp/mpd.fifo"

// number of samples
// (POWER OF 2)
#define N_SAMPLES 1024

// if adaptive sampling should be used
// (adjust N_SAMPLES to sample rate of current song)
// uses N_SAMPLES as maximum for 44100 Hz
// set to 0 to disable
#define ADAPTIVE_SAMPLING 1


/* =========== MPD Options ============ */

#ifdef STATUS_CHECK

// same of mpd.conf
static char defHost[] = "localhost";

static unsigned int defPort = 6600;

// database location, needed for libmpdclient
static char defDBlocation[] = "~/.mpd/mpd.db";

// timeout for MPD connection to fail
#define TIMEOUT 1000

#endif

/* =========== Visualizer Options ============ */

// define characters used while printing on screen
// FULL == 1 (char is set)
// EMPTY == 0 (char is not set)
//
// PRESET 1: vertical sharp
#define FULL '|'
#define HALFFULL '|'
#define HALFEMPTY '-'
#define EMPTY ' '

//// PRESET 2: shadow
//#define FULL ':'
//#define HALFFULL '|'
//#define HALFEMPTY ' '
//#define EMPTY '.'

// PRESET 3: colorful
//#define FULL 'X'
//#define HALFFULL 'x'
//#define HALFEMPTY '^'
//#define EMPTY '.'

// seconds between each mpd status refresh
#define STATUS_REFRESH 1

// nuber of buffer reads whose computation is skipped (cyclically).
// More skips mean less precision, nicer on older CPUs
// 0 -> do not skip any
// 1 -> process one out of two (almost imperceptible, should be sufficient for slower machines)
// 2 -> process one out of three
// etc (accuracy decreases visibly after niceness = 4)
#define NICENESS 0

// define output window
// roughly corresponds to the number of bars of the display replicated
#define FOCUS 2

// subtracted to each component (visualized column)
// adjust according to screen height
#define Y_CORRECTION 40

// adjust lateral shift on screen
// some optimal values are provided
// adjust based on terminal dimensions
#define X_CORRECTION 1   // 1280x1024 screens (5:4)
/* #define X_CORRECTION 8 // 1920x1080 screens (16:9) */

/* =========== ALSA Support (WIP) ============ */

#define USE_ALSA
#define ALSA_CHANNELS 1
#define ALSA_SAMPLE_RATE 44100
