#ifndef IDEVICE_H_301020
#define IDEVICE_H_301020
#include "pch.h"
#include <Windows.h>
#include <initguid.h> // Windows hack: needs to be put before mmdeviceapi.h for PKEY_AudioEngine_DeviceFormat to work.
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include "DoubleCircularBuffer.h"
#include <vector>

typedef int DeviceType;

// Static constant variables:
constexpr int NO_DEVICE = -1;
constexpr int CAPTURE_DEVICE = 0;
constexpr int RENDER_DEVICE = 1;

constexpr int REFTIMES_PER_SEC = 10000000;
constexpr int REFTIMES_PER_MILISEC = 10000;
constexpr int HEADROOM_TIME = 5000;

class LIBAGGREGATOR_API IDevice
{
public:
	
	//virtual int writeToBuffer() = 0
	// Getters:
	virtual WORD getBitDepth() = 0;
	virtual WORD getChannelCount() = 0;
	virtual LPWSTR getDeviceId() = 0;
	virtual DeviceType getDeviceType() = 0;
	virtual IMMDevice* getIMMDevice() = 0;
	virtual REFERENCE_TIME getMinimalLatency() = 0;
	virtual int getSampleRateRange(std::vector<DWORD>* sampleRateRangeVector) = 0;
	virtual DWORD getCurrentSampleRate() = 0;
	virtual DWORD getVirtualSampleRate() = 0;
	virtual std::vector<DoubleCircularBuffer*>* getBufferVectorPtr() = 0;

	// Setters:
	virtual int setActiveDevice(LPWSTR pwszID) = 0;
	virtual int setDeviceType(DeviceType type) = 0;

	// For sample rate change to take place, ASIO must be restarted (init must be called again)!
	virtual int setSampleRate(DWORD rate) = 0;

	// Stream functionality:
	virtual int initStream() = 0;

	

	virtual ~IDevice() {}
};


#endif
