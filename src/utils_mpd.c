#ifdef STATUS_CHECK
#include "utils_mpd.h" 
#include <mpd/client.h> // libmpdclient
#include <stdio.h> // fprintf
#include <stdbool.h> // true false
#include <string.h> // strcmp
#include "settings.h"

/* This macro is a first step towards a sort of try except macro system
 * the objective of this macro is to have y = f(x) non NULL
 * but if it fails it trashes x and generates a new X using newX
 * and starts again
 */
/* it is not used anymore since now open_connection is called every time it is needed */
#define TRY(x, delX, newX, y, f) do {\
		if ((y = f (x)) == NULL) {\
		delX (x); doNotClose = false;\
		}\
	} while (y == NULL && (x = newX () ) != NULL)

char *_host;
char *_DBlocation;
unsigned int _port;

uint32_t
get_sample_rate(const struct mpd_status* status)
{
    uint32_t rate;
    const struct mpd_audio_format* format = mpd_status_get_audio_format(status);

    if (format && format->sample_rate) {
        rate = format->sample_rate;
    } else {
        rate = 0;
    }
    return rate;
}

void
import_var_from_settings ()
{
	// import variables from settings.h to be passed to other functions
	_host = strdup (defHost);
	_port = defPort;
	_DBlocation = strdup (defDBlocation);
}

void
free_var_from_settings ()
{
	free (_host);
	free (_DBlocation);
}

bool change_host (char **args, int n)
{
	if (n == 0) {
		STANDARD_USAGE_ERROR ("port");
		return false;
	}
	_host = strdup (args[0]);
	return true;
}
bool change_port (char **args, int n)
{
	if (n == 0) {
		STANDARD_USAGE_ERROR ("port");
		return false;
	}
	_port = atoi (args[0]);
	return true;
}

/* opens a new connection to the mpd server
 * arguments: host, port (timeout defined)
 * returns a pointer to the connection structure if successful
 * returns a null pointer if error occours
 */
struct mpd_connection*
open_connection()
{
	struct mpd_connection *newConn = NULL;
	enum mpd_error connErr;

    newConn = mpd_connection_new(_host, _port, TIMEOUT);
	connErr = mpd_connection_get_error(newConn);

	/*if error occours*/
	if(connErr != MPD_ERROR_SUCCESS){

		fprintf(stderr, "MPD connection error: ");

		/*error codes are defined in libmpdclient, file error.h*/
		switch(connErr){
			case MPD_ERROR_OOM:
				fprintf(stderr, "Out of memory\n");
				break;

            case MPD_ERROR_ARGUMENT:
                fprintf(stderr, "Unrecognized argument\n");
                break;

            case MPD_ERROR_STATE:
                fprintf(stderr, "State\n");
                break;

            case MPD_ERROR_TIMEOUT:
                fprintf(stderr, "Timeout was reached\n");
                break;

            case MPD_ERROR_SYSTEM:
                fprintf(stderr, "System error\n");
                break;

            case MPD_ERROR_RESOLVER:
                fprintf(stderr, "Unknown host\n");
                break;

            case MPD_ERROR_MALFORMED:
                fprintf(stderr, "Malformed response received from MPD\n");
                break;

            case MPD_ERROR_CLOSED:
                fprintf(stderr, "Connection closed\n");
                break;

			case MPD_ERROR_SERVER:
				fprintf(stderr, "Server error\n");
				break;
			default:
				break;
		}

		exit (69); // can't open the connection, mpd is unavailable
	}

	return newConn;
}

/* closes connection and frees all memory
 * arguments: connection (struct mpd_connection *)
 */
void
close_connection(struct mpd_connection *mpdConnection)
{
	if (mpdConnection != NULL) {
		mpd_connection_free(mpdConnection);
	}
}

/* converts a (struct mpd_song*) defined in libmpdclient
 * into a SONG* structure defined in util2.h
 * returns NULL if error
 * returns a SONG* element if successful
 */
SONG*
parse_mpd_song(struct mpd_song* mpdSong)
{
	SONG *song = NULL;
	int duration;
	const char *tmp;

	song = (SONG*)malloc(sizeof(SONG));
	if(song == NULL){
		fprintf(stderr, "Memory allocation error.\n");
		return NULL;
	}

	// get every field of mpdsong and allocate it on kpd song struct
	tmp = mpd_song_get_tag(mpdSong, MPD_TAG_TITLE, 0);
	if (tmp != NULL) { 
		song->title = strdup (tmp);
	} else {
		song->title = NULL;
	}
	tmp = mpd_song_get_tag(mpdSong, MPD_TAG_ARTIST, 0);
	if (tmp != NULL) {
		song->artist = strdup (tmp);
	} else {
		song->artist = NULL;
	}
	tmp = mpd_song_get_tag(mpdSong, MPD_TAG_ALBUM, 0);
	if (tmp != NULL) {
		song->album = strdup (tmp);
	} else {
		song->album = NULL;
	}
	tmp = mpd_song_get_uri (mpdSong);
	if (tmp != NULL) {
		song->uri = strdup (tmp);
	} else {
		song->uri = NULL;
	}
	duration = mpd_song_get_duration(mpdSong);
	song->duration_min = duration/60;
	song->duration_sec = duration%60;
	song->position = mpd_song_get_pos(mpdSong);

	mpd_song_free (mpdSong);
	return song;
}

/* function to free song_struct */
void free_song_st (SONG *s)
{
	if (s!=NULL) {
		if (s->title != NULL) {
			free (s->title);
		}
		if (s->artist != NULL) {
			free (s->artist);
		}
		if (s->album != NULL) {
			free (s->album);
		}
		if (s->uri != NULL) {
			free (s->uri);
		}
		free (s);
	}
}

/* function to free status_struct */
void free_status_st (STATUS *s)
{
	if (s != NULL) {
		free (s->state);
		free_song_st (s->song);
		free (s);
	}
}

/* queries the server for the current song, inserts it into a structure
 * arguments: Connection
 * returns a NULL structure if no current song / error,
 * returns a pointer to a SONG structure if successful
 */

SONG*
get_current_song()
{
	struct mpd_connection *mpdConnection = NULL;
	struct mpd_song* mpdSong = NULL;

	mpdConnection = open_connection ();
	mpdSong = mpd_run_current_song(mpdConnection);
	close_connection (mpdConnection);
	/*TRY (mpdConnection, close_connection, open_connection, mpdSong, mpd_run_current_song);*/
		if(mpdSong == NULL){
			return NULL;
		}
	return parse_mpd_song(mpdSong);	
}

/* queries the server for the current state (play, pause, stop, unknown)
 * arguments: status (struct mpd_status*)
 * returns NULL if error,
 * returns a pointer to a string with the status if successful
 */
static char* 
get_current_state(struct mpd_status* mpdStatus)
{
	int mpdState;
	char *state;
	mpdState = mpd_status_get_state(mpdStatus);
	if(!mpdState){
		return NULL;
	}

	switch(mpdState){
		case MPD_STATE_STOP:
			state = strdup ("stop"); 
			break;
		case MPD_STATE_PLAY:
			state = strdup ("play"); 
			break;
		case MPD_STATE_PAUSE:
			state = strdup ("pause");
			break;
		default:
			state = NULL;
			break;
	}
	return state;	
}

/* queries the server for the current status structure, 
 * parses it and inserts it into another structure
 * arguments: Connection
 * returns NULL if error,
 * returns a pointer to a STATUS structure if successful
 */
STATUS* 
get_current_status()
{
	struct mpd_connection *mpdConnection = NULL;
	struct mpd_status* mpdStatus = NULL;
	STATUS *status = NULL;
	int eltime;

	mpdConnection = open_connection ();
	mpdStatus = mpd_run_status(mpdConnection);
	close_connection (mpdConnection);
	/*TRY (mpdConnection, close_conneccion, open_connection, mpdStatus, mpd_run_status);*/

	if(mpdStatus == NULL){
		fprintf(stderr, "Unable to retrieve status. Connection error.\n");
		return NULL;
	}
	status = (STATUS*)malloc(sizeof(STATUS));
	if(status == NULL){
		fprintf(stderr, "Memory allocation error.\n");
		return NULL;
	}
	status->random = mpd_status_get_random(mpdStatus);
	status->repeat = mpd_status_get_repeat(mpdStatus);
	status->single = mpd_status_get_single(mpdStatus);
	status->consume = mpd_status_get_consume(mpdStatus);
	status->update = mpd_status_get_update_id (mpdStatus); 
	status->crossfade = mpd_status_get_crossfade(mpdStatus);
	status->song = get_current_song();
	status->state = get_current_state(mpdStatus);			
	eltime = (float)mpd_status_get_elapsed_time(mpdStatus);
	status->elapsedTime_min = eltime/60;
	status->elapsedTime_sec = eltime%60;	
	status->queueLenght = mpd_status_get_queue_length(mpdStatus);
    status->sampleRate = get_sample_rate((const struct mpd_status*)mpdStatus);

	mpd_status_free (mpdStatus);
	return status;
}
#endif
