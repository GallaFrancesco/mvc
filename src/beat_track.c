#include "beat_track.h"
#include "settings.h"

#include <stdio.h>

void cb_init(cebuffer *cb, const int capacity)
{
    cb->buffer = (energy_t*)malloc(capacity*sizeof(energy_t));
    memset(cb->buffer, 0, capacity*sizeof(energy_t));
    cb->capacity = capacity;
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
}

void cb_free(cebuffer *cb)
{
    free(cb->buffer);
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
}

// insert, deleting oldest element if needed
void cb_push_back(cebuffer *cb, const energy_t item)
{
    // insert on head+1
    cb->buffer[cb->head % 43] = item;
    cb->head++;
    if(cb->count >= cb->capacity) {
        cb->tail++;
    } else {
        cb->count++;
    }
}

unsigned int cb_avg(cebuffer *cb)
{
    unsigned int avg = 0;
    unsigned int i = 0;

    for(i=cb->tail; i<cb->head; ++i) {
        avg += cb->buffer[i % cb->capacity];
    }
    return (int)(avg / cb->count);
}

int cb_variance(cebuffer *cb)
{
    double variance = 0;
    unsigned int i = 0;
    unsigned int avg = cb_avg(cb);

    for(i=cb->tail; i<cb->head; ++i) {
        variance += pow(cb->buffer[i % cb->capacity] - avg, 2);
        fprintf(stderr, "head: %d, tail: %d, i: %d, buffer: %d, avg: %d\t", cb->head, cb->tail, i%cb->capacity, cb->buffer[i % cb->capacity], avg);
        fprintf(stderr, "variance: %f\n", variance);
    }
    fprintf(stderr, "variance TOTAL: %f\n", variance / cb->count);
    return (int)(variance / cb->count);
}

bool cb_beat(cebuffer *cb, const energy_t item, energy_t* energyThreshold)
{
    bool beat = false;

    // compute the avg of the circular buffer up to now
    *energyThreshold = (int)(-15*cb_variance(cb) + 1.55);

    if(item > *energyThreshold + *energyThreshold/2) {
        beat = true;
    }

    fprintf(stderr, "threshold: %d, item: %d\n", *energyThreshold, item);

    return beat;
}
