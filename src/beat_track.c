#include "beat_track.h"
#include "settings.h"

/**
 * initialize circular buffer to [capacity] items
 */
void cb_init(cebuffer *cb, const int capacity)
{
    cb->buffer = (energy_t*)malloc(capacity*sizeof(energy_t));
    memset(cb->buffer, 0, capacity*sizeof(energy_t));
    cb->capacity = capacity;
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
}

/**
 * free and reset buffer
 */
void cb_free(cebuffer *cb)
{
    free(cb->buffer);
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
}

/**
 * insert, replacing oldest element if needed
 */
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
    if(cb->tail > cb->capacity) {
        cb->head -= cb->capacity;
        cb->tail -= cb->capacity;
    }
}

/**
 * compute the average energy in the buffer
 */
double cb_avg(cebuffer *cb)
{
    double avg = 0;
    unsigned int i = 0;
    unsigned int cnt = 0;

    for(i=cb->tail; i<cb->head; ++i) {
        double val = cb->buffer[i % cb->capacity];
        if(val > 0) {
            cnt++;
            avg += cb->buffer[i % cb->capacity];
        }
    }
    if(cnt == 0) return 0;

    return avg / cnt;
}

/**
 * compute energy variance in the buffer
 */
double cb_variance(cebuffer *cb)
{
    double variance = 0;
    unsigned int i = 0;
    unsigned int avg = cb_avg(cb);
    unsigned int cnt = 0;

    for(i=cb->tail; i<cb->head; ++i) {
        double val = cb->buffer[i % cb->capacity];
        if(val > 0) {
            cnt++;
            double diff = fabs(val - avg);
            variance += pow(diff, 2);
        }
    }
    return variance / cb->count;
}

/**
 * Beat detection entry point (simple/faster algorithm: no variance)
 * - threshold: compute avg of values, multiply it by a scaling factor
 * - item > thresold -> beat
 * - item <= threshold -> no beat
 * store thresold value to re-use it (TODO improve algorithm)
 */
bool cb_beat(cebuffer *cb, const energy_t current, energy_t* energyThreshold)
{
    bool beat = false;

    // compute the avg of the circular buffer up to now
    double mult = (double)17/16;
    double et = mult*cb_avg(cb);

    if(current > et) {
        beat = true;
    }

    *energyThreshold = (energy_t)et;

    return beat;
}
