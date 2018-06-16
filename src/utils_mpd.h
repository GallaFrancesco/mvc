#ifdef STATUS_CHECK

#ifndef KPD_UTIL_H
#define KPD_UTIL_H
#include <stdbool.h>
#include <mpd/client.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAXFEAT 4
#define STANDARD_USAGE_ERROR(commandname) fprintf (stderr,"kpd: incorrect usage of %s\nTry 'kpd --help' for more information.\n", commandname); 

typedef struct so {
	char *title;
	char *artist;
	char *album;
	char *uri; // name on the filesystem
	int duration_min;
	int duration_sec;
	unsigned int position;
} SONG;

typedef struct st {
	char *state;
	bool random;
	bool consume;
	bool repeat;
	bool single;
	bool crossfade;
	bool update;
	int elapsedTime_min;
	int elapsedTime_sec;
	int queueLenght;
	SONG* song;
} STATUS;


typedef struct q {
	SONG* song;
	struct q *next;	
} QUEUE;

extern char *_host;
extern char *_DBlocation;
extern unsigned int _port;

bool change_port ();
bool change_host ();
void free_var_from_settings ();
void import_var_from_settings ();

struct mpd_connection *open_connection();
void close_connection(struct mpd_connection *mpdConnection);
SONG* get_current_song();
void free_song_st (SONG *s);
void free_status_st (STATUS *s);
STATUS* get_current_status();
void print_current_status(STATUS* status);
QUEUE* get_current_playlist();
int count_playlist_elements (QUEUE *q);
bool enqueue(QUEUE* q, SONG* s);
SONG* dequeue(QUEUE* q);
void destroy_queue (QUEUE *q);
void print_current_playlist(QUEUE *q);
bool list (char **argv, int n);

bool play (char **args, int n);
//bool pause (char **args, int n);
bool next ( char **args, int n);
bool previous (char **args, int n);
bool stop();
//bool clear();
bool delete_song(  int pos);
int convert_to_int(char *arg);
bool delete(  char **args, int n);
bool delete_range(  char **args, int n);
int compare_pos(const void *pos1, const void *pos2);
bool random_kpd(  char **args, int n);
bool consume(  char **args, int n);
bool single(  char **args, int n);
bool repeat(  char **args, int n);
bool seek(  char **args, int n);
bool forward(char **args, int n);
bool backward(char **args, int n);
bool swap(  char **args, int n);
//bool move(  char **args, int n);
bool update(  char **args, int n);
bool shuffle(  char **args, int n);
bool shuffle_range(  char **args, int n);
bool output_enable(  char **args, int n);

bool search_util (  char **args, int n);
void destroy_search_results ();
void print_search_results (char **results, int size);
bool filter_helper (char **args, int n);
bool vfilter_helper (char **args, int n);
bool add(  char **args, int n);
bool print_full_names (char **args, int n);

#endif
#endif
