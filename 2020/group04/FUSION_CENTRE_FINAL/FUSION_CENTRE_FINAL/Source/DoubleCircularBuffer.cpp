/*
  ==============================================================================

    DoubleCircularBuffer.cpp
    Created: 29 Oct 2020 3:50:15pm
    Author:  simondebeus

  ==============================================================================
*/
#include "pch.h"
#include "DoubleCircularBuffer.h"

/** @brief all elements will be set to zero
  */
void DoubleCircularBuffer::emptyBuffer()
{
    for(int i = 0; i < this->sizeBuffer; i++){
        writePointerA->setData(0);
        writePointerA = writePointerA->next;
        writePointerB->setData(0);
        writePointerB = writePointerB->next;
    }
}

/** @brief change the size of the buffer
  */
void DoubleCircularBuffer::changeSizeBuffer(int sizeBuffer, bool writeBufferA)
{
    Element* prev,*current,*first;
    if(sizeBuffer > this->sizeBuffer){
        //adding samples to buffer A
        current = readPointerA;
        first = readPointerA->next;
        for(int i = 0; i < sizeBuffer-this->sizeBuffer; i++){
            prev = current;
            current = new Element();
            prev->next = current;
        }
        current->next = first;
        // adding samples to buffer B
        current = readPointerB;
        first = readPointerB->next;
        for(int i = 0; i < sizeBuffer-this->sizeBuffer; i++){
            prev = current;
            current = new Element();
            prev->next = current;
        }
        current->next = first;
    }else{
        //deleting element in buffer A
        current = readPointerA->next;
        first = readPointerA;
        for(int i = 0; i < this->sizeBuffer - sizeBuffer; i++){
            prev = current;
            current = current->next;
            delete prev;
        }
        first->next = current;
        //deleting element in buffer B
        current = readPointerB->next;
        first = readPointerB;
        for(int i = 0; i < this->sizeBuffer - sizeBuffer; i++){
            prev = current;
            current = current->next;
            delete prev;
        }
        first->next = current;
    }
    this->writeBufferA = writeBufferA;
    this->sizeBuffer = sizeBuffer;
}

/** @brief
  */
int DoubleCircularBuffer::getSizeBuffer()
{
    return sizeBuffer;
}

/** @brief change the buffer that is written to
  */
void DoubleCircularBuffer::changeWrite()
{
    writeBufferA = !writeBufferA;
}

/** @brief write one sample to the writebuffer
  */
void DoubleCircularBuffer::write(float sample)
{
    if(writeBufferA){
        writePointerA->setData(sample);
        writePointerA = writePointerA->next;
    }else{
        writePointerB->setData(sample);
        writePointerB = writePointerB->next;
    }
}

/** @brief read one sample from the current readbuffer
  */
float DoubleCircularBuffer::read()
{
    float response;
    if(writeBufferA){
        response = readPointerB->getData();
        readPointerB = readPointerB->next;
    }else{
        response = readPointerA->getData();
        readPointerA = readPointerA->next;
    }
    return response;
}

/** @brief Delete buffer object
  *
  */
 DoubleCircularBuffer::~DoubleCircularBuffer()
{
    Element* prev,*current;
    //deleting buffer A
    current = writePointerA;
    for(int i = 0; i < sizeBuffer; i++){
        prev = current;
        current = prev->next;
        delete prev;
    }
    //deleting buffer B
    current = writePointerB;
    for(int i = 0; i < sizeBuffer; i++){
        prev = current;
        current = prev->next;
        delete prev;
    }
}

/** @brief make a new buffer object and set the A & B pointers
  *
  */
 DoubleCircularBuffer::DoubleCircularBuffer(int sizeBuffer, bool writeBufferA)
{
    this->writeBufferA = writeBufferA;
    this->sizeBuffer = sizeBuffer;
    Element * prev;
    //preparing buffer A
    Element * current = new Element();
    Element * first = current;
    for (int i = 0; i < sizeBuffer; i++)
    {
        prev = current;
        current = new Element();
        prev->next = current;
    }
    current->next = first;
    readPointerA = current;
    writePointerA = current;
    //preparing buffer B
    current = new Element();
    first = current;
    for (int i = 0; i < sizeBuffer; i++)
    {
        prev = current;
        current = new Element();
        prev->next = current;
    }
    current->next = first;
    readPointerB = current;
    writePointerB = current;

}

