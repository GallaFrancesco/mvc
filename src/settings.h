/*
 **** MPD options
 */
static char defHost[] = "localhost";
static unsigned int defPort = 6600;
// database location, needed for libmpdclient
static char defDBlocation[] = "/home/francesco/.mpd/mpd.db";
// timeout for MPD connection to fail
#define TIMEOUT 1000
/*
 **** Visualizer options
 */

// number of samples
// (a power of 2)
// NOTE: modifying the original value of 1024
// might result in messy output
#define N_SAMPLES 1024
// path to the MPD named pipe
// (defined: 'mpd.conf')
#define MPD_FIFO "/tmp/mpd.fifo"
// seconds between each mpd status refresh
#define STATUS_REFRESH 5
// nuber of buffers whose computation is skipped (cyclically).
// More skips mean less precision, nicer on older CPUs
// 0 -> do not skip
// 1 -> process one out of two
// 2 -> process one out of three
// etc
#define NICENESS 0
// subtracted to each component (visualized column)
// adjust according to screen height
#define REDUCTION 20
