#pragma once
#include <curses.h>

#include "utils_mpd.h"


typedef enum {CURVE=0, MOUTH=1, MOUTH_REV=2, LINE=3, RANDOM=4, ANOTHER=5} PATTERN;

void init_color_pairs();
WINDOW* curses_init();
void print_col(int col, int length, const int maxR, const int maxC, PATTERN pattern, int seed);
void print_subw(WINDOW* sub, const bool beat, const int maxR, const int maxC);
void print_rate_info(const int rate, const int nsamples, const int maxC, int seed, const double basefreq, const int oratio);
void print_help(const int maxR, const int maxC);

#ifdef STATUS_CHECK
void print_mpd_status(STATUS* status, const int maxC, const int row, const int moveCol);
#endif
