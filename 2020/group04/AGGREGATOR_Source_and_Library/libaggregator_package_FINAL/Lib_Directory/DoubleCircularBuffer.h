#ifndef DOUBLE_CIRC_BUFFER
#define DOUBLE_CIRC_BUFFER
/*
  ==============================================================================

    DoubleCircularBuffer.h
    Created: 29 Oct 2020 3:50:15pm
    Author:  simondebeus

  ==============================================================================
*/

#include "pch.h"
#include "Element.h"

class LIBAGGREGATOR_API DoubleCircularBuffer
{
    private:
    Element * readPointerA;
    Element * writePointerA;
    Element * readPointerB;
    Element * writePointerB;

    bool writeBufferA;
    int sizeBuffer;

    public:
    DoubleCircularBuffer(int sizeBuffer, bool writeBufferA);
    ~DoubleCircularBuffer();

    float read();
    void write(float sample);
    void changeWrite();
    int getSizeBuffer();
    void changeSizeBuffer(int sizeBuffer, bool writeBufferA);
    void emptyBuffer();

};


#endif // DOUBLE_CIRC_BUFFER
