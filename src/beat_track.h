#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

typedef unsigned int energy_t;

// circular energy buffer
typedef struct cebuffer
{
    energy_t* buffer;     // data buffer
    int capacity;  // maximum number of items in the buffer
    int count;     // number of items in the buffer
    int head;       // pointer to head
    int tail;       // pointer to tail
} cebuffer;

void cb_init(cebuffer *cb, int capacity);
void cb_free(cebuffer *cb);
void cb_push_back(cebuffer *cb, const energy_t item);
bool cb_beat(cebuffer *cb, const energy_t item, energy_t* energyThreshold);
