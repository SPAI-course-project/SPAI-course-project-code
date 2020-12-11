/*
  ==============================================================================

    smbPitchShift.h
    Created: 2 Dec 2020 3:05:15am
    Author:  Jomi Verwimp

  ==============================================================================
*/

#pragma once

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>

#define M_PI 3.14159265358979323846
#define MAX_FRAME_LENGTH 8192

class PitchShiftCore {
public:
	//PitchShiftCore();
	//~PitchShiftCore();

    void smbFft(float* fftBuffer, long fftFrameSize, long sign);
    
	double smbAtan2(double x, double y);
    
	void smbPitchShift(float pitchShift, long numSampsToProcess,
        long fftFrameSize, long osamp, float sampleRate,
        float* indata, float* outdata);

	void reset();

private:
	float gInFIFO[MAX_FRAME_LENGTH];
	float gOutFIFO[MAX_FRAME_LENGTH];
	float gFFTworksp[2 * MAX_FRAME_LENGTH];
	float gLastPhase[MAX_FRAME_LENGTH / 2 + 1];
	float gSumPhase[MAX_FRAME_LENGTH / 2 + 1];
	float gOutputAccum[2 * MAX_FRAME_LENGTH];
	float gAnaFreq[MAX_FRAME_LENGTH];
	float gAnaMagn[MAX_FRAME_LENGTH];
	float gSynFreq[MAX_FRAME_LENGTH];
	float gSynMagn[MAX_FRAME_LENGTH];
	long gRover = false, gInit = false;
};
