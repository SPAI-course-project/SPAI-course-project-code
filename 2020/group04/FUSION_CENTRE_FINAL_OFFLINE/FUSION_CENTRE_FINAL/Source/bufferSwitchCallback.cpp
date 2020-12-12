/*
  ==============================================================================

    bufferSwitchCallback.cpp
    Created: 5 Dec 2020 6:06:49pm
    Author:  dries

  ==============================================================================
*/

#include "bufferSwitchCallback.h"
#include "aggregator_config.h"
#include "TimeAligner.h"
#include <string>
#include <chrono>
#include "libdsp.h"
#include "ClientUdp.h"
#include "compression.h"

// Audio files for exporting to .wav
AudioFile<float> audioFileBeforeDSP;
AudioFile<float> audioFileAfterDSP;
// Audio file auxilary sample counters
std::vector<int> audioFileCounters;
std::vector<int> audioFileTestCounter;
// DSP buffers
std::vector<float*> CompressorOutputBuffer;
std::vector<float*> DSPInputBuffer;
std::vector<float*> DSPOutputBuffer;
std::vector<DSP*> DSPObjects;
std::vector<int> availableSamplesPerDevice;
float* tempStereoBuffer[2];
// DSP objects
TimeAligner* pTimeAligner;
// UDP Connection
ClientUdp clientUdp;
PCSTR ipAddress{ "192.168.137.142" };
int PORT{ 18000 };
// CONSTANTS
constexpr int SAMPLESTOWAV{ 20*32000 };

// Dries, link these static variables to GUI
std::vector<float> volumeControlVector;
int windowSize{ 1024 };
int hopsize{ 256 };
WindowType windowType { HAMMING };
int padSize{ 0 };
int sampleRate{ 32000 };
std::vector<float> dummyVector;
Compression compressor;

ProcessActivity audioEffect;
bool driverExists = false;
std::vector<float> pitchshiftValues;
std::vector<float> compressionValues;



void bufferSwitchCallback(std::vector<ASIOCustomBufferInfo>* pBufferInfoVector) {
    
    readFromAggregator(pBufferInfoVector);
    // test scale one input twice as big
    volumeControlVector[1] = 1.0f;
    volumeControlVector[0] = 1.0f;
    for (int i = 0; i < DSPInputBuffer.size(); i++) {
        pTimeAligner->scaling(DSPInputBuffer[i], volumeControlVector[i], availableSamplesPerDevice[i]);
    }
    // Alignment should come here



    if (audioEffect == VOICE_COMPRESSION) {
        for (int i = 0; i < DSPInputBuffer.size(); i++) {
            compressor.doTheCompressionBaby(DSPInputBuffer[i], DSPOutputBuffer[i], availableSamplesPerDevice[i], 48000, compressionValues[0], 2.5, 10, compressionValues[1], compressionValues[2], compressionValues[3]);
        }
    }
   
    // Process
   else if (audioEffect == PITCH_SHIFT) {
            for (int i = 0; i < DSPObjects.size(); i++) {
                DSPObjects[i]->process(PITCH_SHIFT, DSPInputBuffer[i], DSPOutputBuffer[i], availableSamplesPerDevice[i], 48000, pitchshiftValues[i], dummyVector, 100, 25);
            }
   }
    else if(audioEffect==VOICE_COMPRESSION_AND_PITCH_SHIFT){
            for (int i = 0; i < DSPObjects.size(); i++) {
                DSPObjects[i]->process(PITCH_SHIFT, DSPInputBuffer[i], CompressorOutputBuffer[i], availableSamplesPerDevice[i], 48000, pitchshiftValues[i], dummyVector, 100, 25);
            }
            if (audioEffect == VOICE_COMPRESSION_AND_PITCH_SHIFT) {
                for (int i = 0; i < DSPOutputBuffer.size(); i++) {
                    compressor.doTheCompressionBaby(CompressorOutputBuffer[i], DSPOutputBuffer[i], availableSamplesPerDevice[i], 48000, compressionValues[0], 2.5, 10, compressionValues[1], compressionValues[2], compressionValues[3]);
                }
            }
     
    }
    

    float* pInputSample{ nullptr };
    for (int i = 0; i < DSPOutputBuffer.size(); i++) {
        pInputSample = DSPOutputBuffer[i];
        for (int j = 0; j < availableSamplesPerDevice[i]; j++) {
            if (audioFileTestCounter[i] < SAMPLESTOWAV) {
                audioFileAfterDSP.samples[i][audioFileTestCounter[i]] = *pInputSample;
                pInputSample++;
                audioFileTestCounter[i]++;
            }
        }
    }

  
}

void readFromAggregator(std::vector<ASIOCustomBufferInfo>* pBufferInfoVector) {

    // Clean implementation:
    float receivedSample{ 0 };
    int availableSamples{ 0 }, stereoSamples{ 0 };
    int channelCounter{ 0 };
    bool stereoChannels{ false };
    int stereoIndices[2]{ 0,0 };
    float* pDSPInputSample;
    float* pTempStereoInputSample;
    float* pTempStereoL;
    float* pTempStereoR;

    int bufferVectorSize{ 0 };
    int deviceVectorSize{ 0 };
    DoubleCircularBuffer* pDCBuffer{ nullptr };


    deviceVectorSize = pBufferInfoVector->size();
    // Loop for each input device.
    for (int deviceCounter = 0; deviceCounter < deviceVectorSize; deviceCounter++) {
        // Loop over all channels of each device.  
        bufferVectorSize = (pBufferInfoVector->at(deviceCounter).pBufferVector)->size();
        for (int bufferCounter = 0; bufferCounter < bufferVectorSize; bufferCounter++) {
            pDCBuffer = (pBufferInfoVector->at(deviceCounter).pBufferVector)->at(bufferCounter);
            availableSamples = pBufferInfoVector->at(deviceCounter).availableSamples;
            availableSamplesPerDevice[deviceCounter] = availableSamples;
            // Limit available samples to DSP maximum input buffer size.
            if (availableSamples > 2048) {
                availableSamples = 2048;
            }
            // Let pDSPInputSample point to first element of corresponding input buffer in DSPInputBuffer.
            pDSPInputSample = DSPInputBuffer[deviceCounter];
            // Let pTempStereoInputSample point to the first element of corresponding temporary stereo buffer.
            pTempStereoInputSample = tempStereoBuffer[channelCounter % 2];
            for (int i = 0; i < availableSamples; i++) {
                receivedSample = pDCBuffer->read();
                // Write to Audio File here
                if (audioFileCounters[channelCounter] < SAMPLESTOWAV) {
                    audioFileBeforeDSP.samples[channelCounter][audioFileCounters[channelCounter]] = receivedSample;
                    audioFileCounters[channelCounter] ++;
                }

                // Write to DSP buffer here
                // Check if stereo and if yes, store in temp buffer first.
                if (stereoChannels == false && pBufferInfoVector->at(deviceCounter).channelNum == 2) {
                    stereoChannels = true;
                    stereoSamples = availableSamples;
                }
                if (pBufferInfoVector->at(deviceCounter).channelNum == 2) {
                    *pTempStereoInputSample = receivedSample;
                    pTempStereoInputSample++;
                }
                else if (pBufferInfoVector->at(deviceCounter).channelNum == 1) {
                    *pDSPInputSample = receivedSample;
                    pDSPInputSample++; // Pointer arithmatic :)
                }
                else {
                    LOGF << "BufferSwitchCallback fatal error: device with 0 or more than 2 channels not supported.";
                }

            }
            channelCounter++;
        }
        pBufferInfoVector->at(deviceCounter).availableSamples = 0;
        // Merge stereo to mono.
        if (stereoChannels == true) {
            pTempStereoL = tempStereoBuffer[0];
            pTempStereoR = tempStereoBuffer[1];
            for (int i = 0; i < stereoSamples; i++) {
                *pDSPInputSample = ((*pTempStereoL) + (*pTempStereoR)) / 2;
                pDSPInputSample++;
                pTempStereoL++;
                pTempStereoR++;
            }
        }
    }
}

void initBufferSwitchCallback(int inputChannels, int inputDevices, int sampleRate) {
    bool result;
    DSPInputBuffer.resize(inputDevices);
    DSPOutputBuffer.resize(inputDevices);
    DSPObjects.resize(inputDevices);
    audioFileCounters.resize(inputChannels);
    volumeControlVector.resize(inputDevices);
    availableSamplesPerDevice.resize(inputDevices);
    audioFileTestCounter.resize(inputDevices);
    CompressorOutputBuffer.resize(inputDevices);
    pitchshiftValues.resize(inputDevices);
    compressionValues.resize(4);
    for (int i = 0; i < inputChannels; i++) {
        audioFileCounters[i] = 0;
        
    }

    for (int i = 0; i < inputDevices; i++) {
        DSPInputBuffer[i] = new float[2048];
        DSPOutputBuffer[i] = new float[2048];
        CompressorOutputBuffer[i] = new float[2048];
        DSPObjects[i] = new DSP(windowSize, hopsize, padSize, windowType, sampleRate);
        pitchshiftValues[i] = 1.5;
        volumeControlVector[i] = 1.0f;
        audioFileTestCounter[i] = 0;
    }

    tempStereoBuffer[0] = new float[2048];
    tempStereoBuffer[1] = new float[2048];

    audioFileBeforeDSP.setNumChannels(inputChannels);
    audioFileBeforeDSP.setNumSamplesPerChannel(SAMPLESTOWAV);
    audioFileBeforeDSP.setBitDepth(32);
    audioFileBeforeDSP.setSampleRate(sampleRate);

    audioFileAfterDSP.setNumChannels(2);
    audioFileAfterDSP.setNumSamplesPerChannel(SAMPLESTOWAV);
    audioFileAfterDSP.setBitDepth(32);
    audioFileAfterDSP.setSampleRate(sampleRate);
 
    pTimeAligner = new TimeAligner(10);

    result = clientUdp.setupClient(ipAddress, PORT);
    if (!result) {
        LOGF << "BufferSwitchCallback Init fail: failed to set up client UDP.";
    }
}

void saveAudioFile() {
    std::string filePath = "..\\"; 
    audioFileBeforeDSP.save("audiotest_BeforeDSP_1012.wav", AudioFileFormat::Wave);
    audioFileAfterDSP.save("audiotest_AfterDSP_1012.wav", AudioFileFormat::Wave);
}

void cleanBufferSwitchCallBack() {
    for (int i = 0; i < 2; i++) {
        if (tempStereoBuffer[i] != nullptr) {
            delete tempStereoBuffer[i];
        }
    }

    for (int i = 0; i < DSPInputBuffer.size(); i++) {
        if (DSPInputBuffer[i] != nullptr) {
            delete DSPInputBuffer[i];
        }
    }

    for (int i = 0; i < DSPOutputBuffer.size(); i++) {
        if (DSPOutputBuffer[i] != nullptr) {
            delete DSPOutputBuffer[i];
        }
    }

    for (int i = 0; i < DSPObjects.size(); i++) {
        if (DSPObjects[i] != nullptr) {
            delete DSPObjects[i];
        }
    }

    if (pTimeAligner != nullptr) {
        delete pTimeAligner;
    }
    
}

void setAudioEffect(int index) {
    audioEffect = (ProcessActivity)index;
    LOGD << "audioeffect: " << audioEffect;
}

void setPitch(int device, float value) {
    pitchshiftValues.resize(2);
    pitchshiftValues[device] = (value);
    LOGD << "new value for device " << device << " is " << pitchshiftValues[device];
}

bool firstStart() {
    if (driverExists) {
        return false;
    }
    else {
        return true;
    }
}

void setFirstTime() {
    driverExists = true;
}

void setMix(int device, int value) {
    volumeControlVector.resize(2);
    volumeControlVector[device] = ((float)value) / 100;
}

void setCompressionValues(int param, float value) {
    compressionValues.resize(4);
    compressionValues[param] = value;
}