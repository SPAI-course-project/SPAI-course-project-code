#pragma once
#include <JuceHeader.h>
#include <vector>
#include <memory>
using namespace std;
enum State : int16_t { FillingBuffer, Working };

class TimeAligner
{
	
public:
	TimeAligner(int bufferOrder);//2^order = bufferSize
	~TimeAligner();
	// alignWithReference will return a religned version of the lagging channel

	int gccPHAT(vector<float> ref, vector<float> sec, float* maxC);
    int getLength();
    int getOrder();
    int getChannel();
    int scaling(float* input, float scaler, int sampleCount);
    vector<float> mix(float* reference, float* secondChannel);
	vector<float> alignWithReference(float* reference, float* secondChannel);

private:
    int order;
	int oneFrameLength;
	int numOfFrames;
	const int storeSize = 4096;
    int FFTorder;
    int outputChannel = 0;

	float maxCorre = 0;
	int maxFrameIndex = 0;
	int maxLag = 0;

	State workingState;
	vector<vector<float>> referenceBuffer; // This buffer acts as a circular buffer that stores previous input signals
	vector<vector<float>> secondChannelBuffer;

//	vector<float>* outputS;
    vector<float>* outputM;
    vector<float>* outputA;
	

	juce::dsp::FFT* oneBlockFFT;
    juce::dsp::FFT* bigFrameFFT;
    juce::dsp::WindowingFunction<float>* oneBlockWin;
    juce::dsp::WindowingFunction<float>* bigFrameWin;
};


