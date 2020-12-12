/*
  ==============================================================================

    SourceCode.h
    Created: 29 Oct 2020 3:50:29pm
    Author:  simondebeus

  ==============================================================================
*/


#ifndef ELEMENT
#define ELEMENT
#include "pch.h"
class Element
{
    private:
    float data;

    public:
    Element * next;
    Element();
    Element(float data);
    float getData() const;
    void setData(float data);
};


#endif // ELEMENT
