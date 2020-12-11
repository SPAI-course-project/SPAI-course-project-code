/*
  ==============================================================================

    AudioFX.cpp
    Created: 9 Dec 2020 11:59:09am
    Author:  Jomi Verwimp

  ==============================================================================
*/

#include "AudioFX.h"

FXCore::FXCore() : fft_j(fftOrder) {
	for (int i = 1; i < (M - 1); i++) {
		OLAP[i][0] = 0;
		OLAP[i][1] = 0;
	}



	// imported as integers for some reason ...
	for (auto i = 0; i < size_coef; i++)
	{
		coef[i] = (float)(((double)coef_i[i]) / 32768.0);
	}

	for (auto i = 0; i <= N - (M - 1); i++)
	{
		fftin[i] = coef[i], 0;
	}
	for (auto i = N - (M - 1) + 1; i < N; i++)
	{
		fftin[i] = 0, 0;
	}


	fft_j.perform(fftin.data(), fftout.data(), false);
	for (auto i = 0; i < N; i++)
	{
		H[i] = fftout[i];
	}
}

FXCore::~FXCore() {}

void FXCore::performFX(const float* x, int size_x, float* y, int channel)
{
	// generalize for multiple audio channels!

	for (auto i = 0; i < size_x; i++)
	{
		buffer_in[(buffer_in_index[channel] + i + buffer_in_samples_available[channel]) % buffer_in_size][channel] = x[i];
		//DBG("in: " + String(x[i]) + "\n");
	}
	buffer_in_samples_available[channel] += size_x;

	bool copied_output = false;

	// block also at the end of method for ... ?
	if (buffer_out_samples_available[channel] > size_x)
	{
		//DBG("copied to output");
		copied_output = true;
		for (auto i = 0; i < size_x; i++)
		{
			y[i] = buffer_out[(buffer_out_index[channel] + i) % buffer_out_size][channel];
			//DBG(String(y[i]) + "\n");
		}
		buffer_out_index[channel] = (buffer_out_index[channel] + size_x) % buffer_out_size;
		buffer_out_samples_available[channel] -= size_x;
	}

	if (buffer_in_samples_available[channel] > L)
	{

		//DBG("L = " + String(L) + " N = " + String(N) + "\n");

		// 1 channel
		///OLS////

		// combineer deze 3
		for (int j = 0; j < N; j++)
		{
			if (j < size_OLAP)
			{
				xr_zeropadded[j] = OLAP[j][channel];
			}
			else if (j < (size_OLAP + L))
			{
				xr_zeropadded[j] = buffer_in[(buffer_in_index[channel] + j - size_OLAP) % buffer_in_size][channel];
			}
			else
			{
				xr_zeropadded[j] = 0;
				// redundant for fixed block size as input instead of variable input size?
				DBG("reached this section of code ... \n");
			}
		}

		for (int j = 0; j < M; j++)
		{
			OLAP[j][channel] = xr_zeropadded[L + j];
		}

		for (int j = 0; j < N; j++)
		{
			fftin[j] = xr_zeropadded[j], 0;
		}

		fft_j.perform(fftin.data(), fftout.data(), false);

		for (auto j = 0; j < N; j++)
		{
			fftin[j] = fftout[j] * H[j];
		}

		fft_j.perform(fftin.data(), fftout.data(), true);

		for (int j = 0; j < L; j++)
		{
			buffer_out[(buffer_out_index[channel] + j + buffer_out_samples_available[channel]) % buffer_out_size][channel] = real(fftout[j + M]);
			//buffer_out[(buffer_out_index + j + buffer_out_samples_available) % buffer_out_size] = buffer_in[(buffer_in_index+j) % buffer_in_size];
		}

		buffer_in_samples_available[channel] -= L;
		buffer_in_index[channel] = (buffer_in_index[channel] + L) % buffer_in_size;
		buffer_out_samples_available[channel] += L;
	}

	if (!copied_output)
	{
		DBG("output was not copied\n");
		if (buffer_out_samples_available[channel] >= size_x)
		{
			copied_output = true;
			for (auto i = 0; i < size_x; i++)
			{
				y[i] = buffer_out[(buffer_out_index[channel] + i) % buffer_out_size][channel];
			}
			buffer_out_index[channel] = (buffer_out_index[channel] + size_x) % buffer_out_size;
			buffer_out_samples_available[channel] -= size_x;
		}
		else
		{
			//???? output zeros? this should not be possible
			// this could occur if the block size L is too large relative to the amount of requested samples
			//DBG("not enough samples available to provide to output");
			for (auto i = 0; i < size_x; i++)
			{
				y[i] = 0;
			}
		}
	}
}