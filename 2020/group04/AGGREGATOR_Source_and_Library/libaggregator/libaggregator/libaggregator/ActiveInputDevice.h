#ifndef ACTIVEINPUDDEVICE_H_301020
#define ACTIVEINPUDDEVICE_H_301020

#include "pch.h"
#include "IDevice.h"
#include "DoubleCircularBuffer.h"

#include <Windows.h>

#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <mmreg.h>

extern "C" {
#include "avutil.h"
#include "channel_layout.h"
#include "swresample.h"
}

constexpr int ACTIVEINDEV_SUCCES = 0;
constexpr int ACTIVEINDEV_FAIL = -1;

class LIBAGGREGATOR_API ActiveInputDevice :
    public IDevice
{
public:
    // Attributes:
    IMMDevice* m_pIMMDevice;
    DeviceType m_deviceType;
    WAVEFORMATEX* m_pDeviceFormat;
    DWORD m_virtualSampleRate; // If this is zero, it means desired sample rate is supported by hardware. If -1 means uninitialized.
    std::vector<DoubleCircularBuffer*> m_bufferVector;
    IAudioClient* m_pAudioClient;
    IAudioCaptureClient* m_pCaptureClient;
    bool m_bufferSizeOptimized;

    // Constructor & Destructor:
    ActiveInputDevice();
    ActiveInputDevice(LPWSTR pwszID, DeviceType deviceType);
    ~ActiveInputDevice() override;

    // Getters:

    // Returns the valid bit depth if ActiveDevice is working properly, if no device was activated returns 0.
    WORD getBitDepth() override;

    // Returns the channel count if ActiveDevice is working properly, if no device was activated returns 0.
    WORD getChannelCount() override;

    // Returns device id if ActiveDevice is working properly, if no device was active returns NULL.
    LPWSTR getDeviceId() override;

    // Returns device type if ActiveDevice is working properly, if no device was active returns NO_DEVICE.
    DeviceType getDeviceType() override;

    // Returns friendly name of device. ActiveInputDevice needs to be properly instantiated otherwise returns NULL.
    LPWSTR getFriendlyName();

    // Returns IMMDevice Component Object if ActiveDevice is working properly, if no device was activied returns NULL.
    IMMDevice* getIMMDevice() override;

    // Returns the minimum device period supported by audio hardware in unit 100 ns. (So if it is 5, the minimal latency is 500 ns).
    // If an error occured returns 0.
    REFERENCE_TIME getMinimalLatency() override;

    // Will try to check which of the following sample frequencies the device supports: [8000, 16000, 32000, 44100, 48000, 96000, 192000).
    // Returns a vector containing the supported sample rates, if an error occured returns a vector with 1 element = 0.
    int getSampleRateRange(std::vector<DWORD>* sampleRateRangeVector) override;

    // Returns the sample rate if ActiveDevice is working properly, if no device was activated returns 0.
    DWORD getCurrentSampleRate() override;
    // Returns virtual sample rate
    DWORD getVirtualSampleRate() override;
    // Returns a pointer to a vector containing pointers to double ciruclar buffer.
    std::vector<DoubleCircularBuffer*>* getBufferVectorPtr() override;

    // Setters:
    // All setters return ACTIVEINDEV_SUCCES on succes and ACTIVEINDEV_FAIL on failure.
  
    int setActiveDevice(LPWSTR pwszID) override;
    int setDeviceType(DeviceType type) override;


    // Will first call getSampleRateRange to check whether sample rate is supported.
    // If not, will set virtual sample rate to desired sample rate and will perform upsampling or downsampling when streaming audio.
    int setSampleRate(DWORD desiredRate) override;

   // Stream functionality:
    int initStream() override;

    int getPacketStreamToPtr(BYTE** ppData);
    int setupFFMpegSWR(SwrContext** swr);
    int writeToBuffer(SwrContext* swr);
    HRESULT getStreamFormat(IAudioClient* pAudioClient, WAVEFORMATEX** ppFormat);

};

#endif

