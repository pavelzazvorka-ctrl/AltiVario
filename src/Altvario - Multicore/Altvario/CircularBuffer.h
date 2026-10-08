#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <Arduino.h>

/* Circular buffer object */
struct CircularBuffer{
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    float       *elems;  /* vector of elements                  */
    unsigned long *tstamp;  /* vector of timestamps          */
};

void cbInit(CircularBuffer *cb, int size);
/*
void cbFree(CircularBuffer *cb);
int cbIsFull(CircularBuffer *cb);
int cbIsEmpty(CircularBuffer *cb);
float cbRead(CircularBuffer *cb);
*/
void cbWrite(CircularBuffer *cb, float elem);
float cbIndex(CircularBuffer *cb, int index);
unsigned long cbTstamp(CircularBuffer *cb, int index);

#endif
