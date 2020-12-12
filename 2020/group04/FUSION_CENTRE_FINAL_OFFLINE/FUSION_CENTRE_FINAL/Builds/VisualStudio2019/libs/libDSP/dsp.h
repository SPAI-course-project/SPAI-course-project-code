#pragma once

#include "pch.h"
#include "DFT.h"
using namespace gam;


///Holds and defines what process will be used:
///PITCH_SHIFT/ AUTO_TUNE
enum ProcessActivity {
    PITCH_SHIFT,
    AUTO_TUNE,
    VOICE_COMPRESSION,
    VOICE_COMPRESSION_AND_PITCH_SHIFT
};

class DSPDLL_API DSP {
public:
    ///\param[in] WindowSize                        Stft: Size of window (use power of 2 or equall to size of buffer)
    ///\param[in] HopSize                               Stft: Size of the hop that is applied between two consecutive windows ( use WindowSize/(power of 2) ), 4 is great!
    ///\param[in] padSize                               Stft: How many zeros that need to be added, generally 0
    ///\param[in] windowType                        Stft: Pass the window type  ( Hamming favorite): BARTLETT, BLACKMAN, BLACKMAN_HARRIS, HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
    ///\param[in] sampleRate                        set Sample rate
    DSP(unsigned WindowSize, unsigned HopSize, unsigned padSize, WindowType windowType, unsigned sampleRate);

    ///\param[in] processActivity               Select which process needs to be executed, check ProcessActivity for all options.
    ///\param[in] inputArrayPointer          Pass a const float pointer to the input-array buffer.
    ///\param[in] outputArrayPointer        Will write to a float pointer that points to the output-array buffer.
    ///\param[in] numberOfSamples               Number of samples in inputarray and outputarray, both need to match this amount.
    ///\param[in] pshift                                   Float value that will be used to shift the frequency spectrum. Generally between [ 0.5, 2 ]
    ///\param[in] notes                                     Array which holds to what notes the input signals needs to be shifted.
      void process(ProcessActivity processActivity, const float* inputArrayPointer, float* outputArrayPointer, const unsigned numberOfSamples, unsigned sampleRate, float pshift, std::vector<float> notes, float tresholdFrequency, float compressionFactor);


      unsigned getcurrentSampleRate() { return currentSampleRate; }

      void setcurrentSampleRate(unsigned SampleRate) { this->currentSampleRate = SampleRate; }


private:
    STFT stft;
    unsigned currentSampleRate;
    enum {
        PREV_MAG = 0,
        TEMP_MAG,
        TEMP_FRQ
    };
    ///\param[in] pshift        Float value that will be used to shift the frequency spectrum. Generally between [ 0.5, 2 ] (two octaves)
    void pitchShift(float pshift);

    ///\param[in] notes         Array which holds to what notes the input signals needs to be shifted.
    void autoTune(std::vector<float> notes);

    void voiceCompression(float tresholdFrequency, float factor);
};

