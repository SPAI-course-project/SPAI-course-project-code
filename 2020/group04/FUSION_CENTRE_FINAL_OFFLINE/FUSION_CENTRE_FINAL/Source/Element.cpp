/*
  ==============================================================================

    SourceCode.cpp
    Created: 29 Oct 2020 3:50:29pm
    Author:  simondebeus

  ==============================================================================
*/
#include "pch.h"
#include "Element.h"

Element::Element(){
    this->data = 0;
}

Element::Element(float data){
    this->data = data;
}

float Element::getData() const{
    return data;
}


void Element::setData(float data){
    this->data = data;
}
