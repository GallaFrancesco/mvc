#pragma once
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
    uint32_t sampleRate;
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
uint32_t get_sample_rate(const struct mpd_status* status);
#endif
#endif
