#include "CircularBuffer.h"

void cbInit(CircularBuffer *cb, int size) {
    cb->size  = size+1; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->elems = (float *)calloc((cb->size+1), sizeof(float));
    cb->tstamp = (unsigned long *)calloc((cb->size+1), sizeof(unsigned long));
}

/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
   
void cbWrite(CircularBuffer *cb, float elem) {
    cb->elems[cb->end] = elem;
    cb->tstamp[cb->end] = (unsigned long)millis();
    cb->end = (cb->end + 1) % cb->size;
    if (cb->end == cb->start)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
}


float cbIndex(CircularBuffer *cb, int index) {
    return cb->elems[((cb->start+index) % cb->size )];
}  

unsigned long cbTstamp(CircularBuffer *cb, int index) {
    return cb->tstamp[((cb->start+index) % cb->size )];
}  
