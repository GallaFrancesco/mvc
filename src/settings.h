static char defHost[] = "localhost"; // the ip of mpd
static unsigned int defPort = 6600;// the port as defined in mpd configs
static char defDBlocation[] = "/home/francesco/.mpd/mpd.db"; // the location of the database to be used with the custom search
#define TIMEOUT 1000 // timeout for connection, define at your own risk. Good values range between 1000 and 5000
#define STATUS_REFRESH 5 // #seconds between each mpd status refresh
