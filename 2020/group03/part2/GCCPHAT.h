/*
  ==============================================================================

    GCCPHAT.h
    Created: 10 Dec 2020 12:12:00pm
    Author:  Jomi Verwimp

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define MAX_FRAME_LENGTH 8192

class GCCPHAT {
public:
    GCCPHAT();
    ~GCCPHAT();

    void push(float* data1, float* data2, int numSamples);
    int getDOA();

private:
	float gInFIFO[MAX_FRAME_LENGTH];

	juce::dsp::FFT fft_j;

	static const int fftOrder = 12;
	static const int fftSize = 1 << fftOrder;

	std::array<std::complex<float>, fftSize> fftin;
	std::array<std::complex<float>, fftSize> fftout;
	std::array<std::complex<float>, fftSize> data_spec;

	float buffer_in[5000][2];
	int buffer_in_size = 5000;
	int buffer_in_index[2] = { 0, 0 };
	int buffer_in_samples_available[2] = { 0, 0 };
	float buffer_out[5000][2];
	int buffer_out_size = 5000;
	int buffer_out_index[2] = { 0, 0 };
	int buffer_out_samples_available[2] = { 3000, 3000 };

	float buffer[50000];

	float delay[8000]; // 44kHz, 50ms so 2000 samples ... should be way less because 8kHz was suggested by coach
	int delay_index = 0;
	int delay_size = 8000;
	int delay_samples = 0;
	int offset = 0;
	int offset_history[20];
	int offset_history_index = 0;

	bool first_call = false;

	float input_buffer[4][fftSize];
	int input_index = 0;
};