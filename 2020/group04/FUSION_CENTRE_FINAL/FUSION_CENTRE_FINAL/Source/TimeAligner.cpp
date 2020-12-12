#include "TimeAligner.h"
#include <cmath>
#include <algorithm>

constexpr int TIMEALIGN_ERROR{ -1 };
constexpr int TIMEALIGN_SUCCES{ 0 };

TimeAligner::TimeAligner(int bufferOrder) {
    this->order = bufferOrder;
	this->oneFrameLength = pow(2, order);
//  int order = int(log2(bufferSize));
//  this->oneFrameLength = bufferSize;
	this->numOfFrames = storeSize/oneFrameLength;
	this->workingState = FillingBuffer;
	int FFTorder = log2(storeSize); // half of the store size plus zero padding
    this->oneBlockFFT = new juce::dsp::FFT(order + 1); // frames + zeropadding: double the length
    this->bigFrameFFT = new juce::dsp::FFT(FFTorder); // 8192=> 4096*2
    this->oneBlockWin = new juce::dsp::WindowingFunction<float>(oneFrameLength + 1, juce::dsp::WindowingFunction<float>::hann, false, 0.0f);  //one frame
    this->bigFrameWin = new juce::dsp::WindowingFunction<float>(storeSize/2 +1, juce::dsp::WindowingFunction<float>::hann, false, 0.0f);  //four frames
    this->outputM = new vector<float>(oneFrameLength, 0);
//  this->outputS = new vector<float>(oneFrameLength, 0);
    this->outputA = new vector<float>(oneFrameLength, 0);
}

TimeAligner::~TimeAligner()
{}


int TimeAligner::scaling(float* input, float scaler, int sampleCount) {
	if (input == nullptr) {
		return TIMEALIGN_ERROR;
	}
	float* pInputSample{ input };
	for (int i = 0; i < sampleCount; i++) {
		*pInputSample *= scaler;
		pInputSample++;
	}
	return TIMEALIGN_SUCCES;
}


vector<float> TimeAligner::mix(float* reference, float* secondChannel) {
    vector<float> referenceInput(reference, reference + oneFrameLength);
    vector<float> secondInput(secondChannel, secondChannel + oneFrameLength);
    std::transform(referenceInput.begin(), referenceInput.end(), secondInput.begin(),
        (*outputM).begin(), std::plus<float>());
    return *outputM;
}


vector<float> TimeAligner::alignWithReference(float* reference, float* secondChannel) {
	//step 1: fill in the buffer
	vector<float> referenceInput(reference, reference + oneFrameLength);
	vector<float> secondInput(secondChannel, secondChannel + oneFrameLength);
    
//  int outputChannel = 0; // 2 => output = second; 1 => output = ref.

    
	switch (workingState)
	{
	case FillingBuffer:
	{
        /*step 1: fill in the buffer*/
        referenceBuffer.push_back(referenceInput);
        secondChannelBuffer.push_back(secondInput);
        while(referenceBuffer.size() < oneFrameLength){
//          cout << referenceBuffer.size() << " / " << oneFrameLength << " filled\n";
            referenceBuffer.push_back(referenceInput);
            secondChannelBuffer.push_back(secondInput);
			workingState = FillingBuffer;
        }
        workingState = Working;
	}
	case Working:
	{
		//do time alignment first
		//step 1: loop each frame and calculate gccphat, save the maxNum, maxIndex, maxLag
		vector<float> bigRef = referenceBuffer[0];
		vector<float> bigSec = secondChannelBuffer[0];
		vector<float> calculateRef = referenceBuffer[0];

		int tempLag;
		float tempMaxC;
//        cout << "1. " << tempMaxC << "\n";
		for (int i = 0; i < numOfFrames; i++)
		{
			// append vectors
			if (i > 0)
			{
				bigRef.insert(bigRef.end(), referenceBuffer[i].begin(), referenceBuffer[i].end());
				bigSec.insert(bigSec.end(), secondChannelBuffer[i].begin(), secondChannelBuffer[i].end());
			}
			
			vector<float> calculateSec = secondChannelBuffer[i];
			//step 1: GCC-PHAT
			tempLag = gccPHAT(calculateRef, calculateSec, &tempMaxC);
//            cout << "2. " << tempMaxC << "\n";
			if (tempMaxC > maxCorre)
			{
				maxFrameIndex = i;
				maxLag = tempLag;
				maxCorre = tempMaxC;
			}
		}

		
		//step 2: align
		if (maxLag > 0) //reference lead => start from the first frame of the reference => mostly the case => send aligned second signal to the output
		{
			//reference: 23456789
			//second:    12345678 => 2345678
            outputChannel = 2;
            cout << "i1 leads, i2 lags.\n";
			int lag = maxFrameIndex * oneFrameLength + maxLag;
            vector<float> second;
			if (lag + oneFrameLength < bigSec.size())
			{
				second = vector<float>(bigSec.begin() + lag, bigSec.begin() + lag + oneFrameLength);
				// (done in mix function separately) mix them together
//				std::transform(referenceBuffer[0].begin(), referenceBuffer[0].end(), second.begin(), (*output).begin(), std::plus<float>());

			}
			else
			{
				second = secondChannelBuffer[numOfFrames - 1];
				// (done in mix function separately) mix them together
//				std::transform(referenceBuffer[0].begin(), referenceBuffer[0].end(), second.begin(), (*output).begin(), std::plus<float>());

			}
//          return second;
            *outputA = second;
		}
        
        
		else if (maxLag < 0)//reference lag => start from the first frame of the second channel => send aligned reference signal to the output
		{
			// reference: 000123456789 => 123456789
			// second:    123456789
            outputChannel = 1;
            cout << "i1 lags, i2 leads.\n";
            int lag = maxFrameIndex * oneFrameLength + (-maxLag);
            vector<float> refe;
			if (lag + oneFrameLength < bigRef.size())
			{
				refe = vector<float>(bigRef.begin() + lag, bigRef.begin() + lag + oneFrameLength);
				// (done in mix function separately) mix them together
//				std::transform(secondChannelBuffer[0].begin(), secondChannelBuffer[0].end(), refe.begin(), (*output).begin(), std::plus<float>());

			}
			else
			{
				refe = referenceBuffer[numOfFrames - 1];
				// (done in mix function separately) mix them together
//				std::transform(secondChannelBuffer[0].begin(), secondChannelBuffer[0].end(), refe.begin(), (*output).begin(), std::plus<float>());

			}
//          return refe;
            *outputA = refe;
		}
        
        
		else// no alignment needed => no lag between the two signals => send ref to output
		{
            outputChannel = 0;
            cout << "No alignment needed; i1 sent to output";
			// (done in mix function separately) mix them together
//			std::transform(secondChannelBuffer[0].begin(), secondChannelBuffer[0].end(), referenceBuffer[0].begin(), (*output).begin(), std::plus<float>());
            auto ref = vector<float>(1);
            ref.push_back(*reference);
//          return ref;
            *outputA = ref;
		}

		// step 3: delete the oldest data (but don't insert new data, as commented)
        referenceBuffer.clear();
//		referenceBuffer.erase(referenceBuffer.begin());
//		referenceBuffer.push_back(referenceInput);
        secondChannelBuffer.clear();
//		secondChannelBuffer.erase(secondChannelBuffer.begin());
//		secondChannelBuffer.push_back(secondInput);
        workingState = FillingBuffer;
        maxCorre = 0;

		break;
	}
	
	}
    
    return *outputA;
}


int TimeAligner::gccPHAT(vector<float> ref, vector<float> sec, float* maxC) {
    
	float* reference = ref.data();
	float* secondChannel = sec.data();

	int calculateLength = oneFrameLength;

	int newlength = 2 * calculateLength - 1;

	oneBlockWin->multiplyWithWindowingTable(reference, calculateLength);
	oneBlockWin->multiplyWithWindowingTable(secondChannel, calculateLength);

	vector<complex<float>> vReference(reference, reference + calculateLength);
	vector<complex<float>> vSecond(secondChannel, secondChannel + calculateLength);

	vector<complex<float>> zeros(calculateLength, 0);
	vector<complex<float>> XResult(calculateLength * 2, 0);
	vector<complex<float>> tresult(calculateLength * 2, 0);
	vector<complex<float>> XReference(calculateLength * 2, 0);
	vector<complex<float>> XSecond(calculateLength * 2, 0);

	/*step 1: zero padding*/
	vReference.insert(vReference.end(), zeros.begin(), zeros.end());
	vSecond.insert(vSecond.end(), zeros.begin(), zeros.end());
	
	/*step 2: fft*/
	oneBlockFFT->perform(vReference.data(), XReference.data(), false);
	oneBlockFFT->perform(vSecond.data(), XSecond.data(), false);
    
	/*step 3: vSecond time complex conjugate of vReference*/
	for (int i = 0; i <= newlength; i++)
	{
		complex<float> temp = XSecond[i] * std::conj(XReference[i]);
		float normalize = abs(temp);

		if (normalize == 0)
		{
			XResult[i] = 0;
		}
		else
		{
			XResult[i] = temp / normalize;
		}
	}

	/*step 4: ifft to get cross correlation*/
	oneBlockFFT->perform(XResult.data(), tresult.data(), true);
	/*step 5: reconstruct the vector*/
	vector<complex<float>> first(tresult.begin() + calculateLength + 1, tresult.end());
	vector<complex<float>> second(tresult.begin(), tresult.begin() + calculateLength);
	first.insert(first.end(), second.begin(), second.end());
	/*step 6: find the max crox*/
	float max = 0;
	int lag = 0;
	int index = 0;
	for (auto value : first) {
		if (value.real() > max)
		{
			max = value.real();
			lag = index;
		}
		index++;
	}
	*maxC = max;
	lag = lag - calculateLength + 1;
//  cout << "lag = " << lag << ".\n";
	return lag;
}


int TimeAligner::getOrder()
{
    return order;
}


int TimeAligner::getLength()
{
    return oneFrameLength;
}

int TimeAligner::getChannel()
{
    return outputChannel;
}
