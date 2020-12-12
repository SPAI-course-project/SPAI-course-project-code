/*
  ==============================================================================

    compression.h
    Created: 11 Dec 2020 8:41:53am
    Author:  victor Letens

  ==============================================================================
*/

#pragma once
#include <stdio.h>      /* printf */
#include <math.h>       /* log10 */
#include <stdlib.h>
#include <vector>
class Compression{
    public:
    Compression();
    void doTheCompressionBaby(const float * inputBuffer,float * outBuffer,int numberOfSamples,int samplingRate,int treshold,float makeUpGain,int kneeWidth,int compressionRatio,int attackTime,int releaseTime);

};
