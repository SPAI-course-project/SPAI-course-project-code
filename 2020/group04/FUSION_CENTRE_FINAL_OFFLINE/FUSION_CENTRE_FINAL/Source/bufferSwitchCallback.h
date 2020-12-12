/*
  ==============================================================================

    bufferSwitchCallback.h
    Created: 5 Dec 2020 6:06:49pm
    Author:  dries

  ==============================================================================
*/

#ifndef callback_h
#define callback_h



#include "AudioFile.h"
#include "aggregator_config.h"
//#include "TimeAligner.h"


void bufferSwitchCallback(std::vector<ASIOCustomBufferInfo>* bufferInfoVector);
void readFromAggregator(std::vector<ASIOCustomBufferInfo>* pBufferInfoVector);
void initBufferSwitchCallback(int inputChannels, int inputDevices, int sampleRate);
void cleanBufferSwitchCallBack();
void setAudioEffect(int index);
bool firstStart();
void setFirstTime();
void setPitch(int device, float value);
void setMix(int device, int value);
void setCompressionValues(int param, float value);

void saveAudioFile();


enum devices {
    BT_headset,
    InternalMic
};


#endif