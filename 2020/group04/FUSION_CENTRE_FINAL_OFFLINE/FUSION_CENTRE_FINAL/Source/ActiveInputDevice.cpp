#include "pch.h"
#include "ActiveInputDevice.h"
#include "DoubleCircularBuffer.h"
#include "plog/Log.h"

#include <Windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <functiondiscoverykeys_devpkey.h>
#include <Ksmedia.h>
#include <wmsdk.h> // Definition WMMEDIASUBTYPE_PCM
#include <vector>
#include "AudioFile.h"

extern "C" {
#include "avutil.h"
#include "channel_layout.h"
#include "swresample.h"
}


#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

// Static constant variables:
constexpr CLSID MMDeviceEnumerator_CLSID = __uuidof(MMDeviceEnumerator);
constexpr IID IMMDeviceEnumerator_IID = __uuidof(IMMDeviceEnumerator);
constexpr IID IAudioClient_IID = __uuidof(IAudioClient);
constexpr IID IAudioCaptureClient_IID = __uuidof(IAudioCaptureClient);


const WAVEFORMATEX commonSampleRatesFormats[] = {
													{WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 11025, 1102, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 16000, 16000, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 32000, 32000, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 44100, 44100, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 44000, 44000, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 48000, 44000, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 96000, 96000, 1, 16, 0},
													{WAVE_FORMAT_PCM, 1, 128000, 128000, 1, 16, 0}
};

// CONSTRUCTORS AND DESTRUCTOR:

ActiveInputDevice::ActiveInputDevice()
{
	m_deviceType = NO_DEVICE;
	m_pIMMDevice = nullptr;
	m_pDeviceFormat = nullptr;
	m_virtualSampleRate = NO_DEVICE;
	m_pAudioClient = nullptr;
	m_pCaptureClient = nullptr;
	m_bufferSizeOptimized = false;
}


ActiveInputDevice::ActiveInputDevice(LPWSTR pwszID, DeviceType deviceType)
{
	m_pIMMDevice = nullptr;
	m_pDeviceFormat = nullptr;
	m_deviceType = deviceType;
	m_pAudioClient = nullptr;
	m_pCaptureClient = nullptr;
	m_bufferSizeOptimized = false;

	int result{ 0 };
	result = setActiveDevice(pwszID);
	if (result != ACTIVEINDEV_SUCCES) {
		LOGF << "ActiveInputDevice fatal error: constructor error, could not register " << pwszID << " as an active device.";
	}

	m_virtualSampleRate = 0;
}

ActiveInputDevice::~ActiveInputDevice()
{
	if (m_pDeviceFormat != NULL) {
		CoTaskMemFree(m_pDeviceFormat);
	}

	for (DoubleCircularBuffer* buffer : m_bufferVector) {
		if (buffer != nullptr) {
			delete buffer;
		}
	}
	SAFE_RELEASE(m_pCaptureClient);
	SAFE_RELEASE(m_pAudioClient);
	SAFE_RELEASE(m_pIMMDevice);
}

// GETTERS:

WORD ActiveInputDevice::getBitDepth()
{
	if (m_pIMMDevice == NULL) {
		LOGD << "ActiveInputDevice error: trying to retrieve bit depth of inactive input device.";
		return 0;
	}
	WAVEFORMATEXTENSIBLE* pDeviceFormatEx = (WAVEFORMATEXTENSIBLE*)m_pDeviceFormat;
	WORD bitDepth = pDeviceFormatEx->Samples.wValidBitsPerSample;
	return bitDepth;
}

WORD ActiveInputDevice::getChannelCount()
{
	if (m_pIMMDevice == NULL) {
		return 0;
	}
	WORD channelCount = m_pDeviceFormat->nChannels;
	return channelCount;
}

LPWSTR ActiveInputDevice::getDeviceId()
{
	if (m_pIMMDevice == NULL) {
		return NULL;
	}
	else {
		HRESULT hr{ S_OK };
		LPWSTR pwszID{ NULL };
		hr = m_pIMMDevice->GetId(&pwszID);
		if (FAILED(hr)) {
			LOGF << "ActiveInputDevice fatal error: failed to retrieve MMDevice ID.";
			return NULL;
		}
		return pwszID;
	}
	return LPWSTR();
}


DeviceType ActiveInputDevice::getDeviceType()
{
	return m_deviceType;
}

LPWSTR ActiveInputDevice::getFriendlyName()
{
	if (m_pIMMDevice == NULL) {
		return NULL;
	}
	HRESULT hr{ S_OK };
	IPropertyStore* pProps{ NULL };
	PROPVARIANT varName;
	PropVariantInit(&varName);

	hr = m_pIMMDevice->OpenPropertyStore(STGM_READ, &pProps);
	if (FAILED(hr)) {
		PLOGF << "ActiveInputDevice fatal error: could not retrieve properties endpoint input device.";
		EXIT_ON_ERROR(hr);
	}
	hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
	if (FAILED(hr)) {
		PLOGF << "ActiveInputDevice fatal error: could not retrieve friendly name.";
		EXIT_ON_ERROR(hr);
	}

	return varName.pwszVal;

Exit:
	SAFE_RELEASE(pProps);
	return NULL;
}

IMMDevice* ActiveInputDevice::getIMMDevice()
{
	if (m_pIMMDevice == NULL) {
		return nullptr;
	}
	else {
		return m_pIMMDevice;
	}
}

REFERENCE_TIME ActiveInputDevice::getMinimalLatency()
{
	REFERENCE_TIME minPeriod{ 0 };
	HRESULT hr{ S_OK };
	if (m_pIMMDevice == NULL) {
		LOGD << "ActiveInputDevice error: trying to retrieve minimal latency from inactive input device.";
		return minPeriod;
	}
	else {
		hr = m_pAudioClient->GetDevicePeriod(NULL, &minPeriod);
		return minPeriod;
	}
}

int ActiveInputDevice::getSampleRateRange(std::vector<DWORD>* sampleRateRange)
{
	HRESULT hr{ S_OK };
	WAVEFORMATEX* receivedFormat{ NULL };
	bool oneSampleRateFound{ false };
	sampleRateRange->resize(std::size(commonSampleRatesFormats));

	if (m_pIMMDevice == NULL) {
		LOGD << "ActiveInputDevice error: trying to retrieve minimal latency from inactive input device.";
		sampleRateRange->resize(1);
		(*sampleRateRange)[0] = 0;
		return ACTIVEINDEV_FAIL;
	}
	else {

		int added{ 0 };
		for (int i = 0; i < std::size(commonSampleRatesFormats); i++) {

			// What works in shared mode, probably works in exclusive mode as well... I hope
			hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &commonSampleRatesFormats[i], &receivedFormat);
			if (FAILED(hr) || receivedFormat == NULL) {
				LOGF << "ActiveInputDevice fatal error: failed to retrieve supported format.";
				return ACTIVEINDEV_FAIL;
			}

			if (receivedFormat != NULL) {
				if (commonSampleRatesFormats[i].nSamplesPerSec == receivedFormat->nSamplesPerSec) {
					(*sampleRateRange)[added] = commonSampleRatesFormats[i].nSamplesPerSec;
					oneSampleRateFound = true;
					added++;
				}
				CoTaskMemFree(receivedFormat);
			}

		}
		if (oneSampleRateFound) {
			sampleRateRange->resize(added);
			return ACTIVEINDEV_SUCCES;
		}

		else {
			sampleRateRange->resize(1);
			(*sampleRateRange)[0] = 0;
			return ACTIVEINDEV_SUCCES;
		}

	}
}

DWORD ActiveInputDevice::getCurrentSampleRate()
{
	if (m_pIMMDevice == NULL) {
		LOGD << "ActiveInputDevice error: trying to retrieve sample rate from inactive device.";
	}
	DWORD sampleRate = m_pDeviceFormat->nSamplesPerSec;
	return sampleRate;
}

DWORD ActiveInputDevice::getVirtualSampleRate()
{
	return m_virtualSampleRate;
}

std::vector<DoubleCircularBuffer*>* ActiveInputDevice::getBufferVectorPtr()
{
	return &m_bufferVector;
}

// SETTERS:

int ActiveInputDevice::setActiveDevice(LPWSTR pwszID)
{
	if (pwszID == nullptr) {
		LOGF << "ActiveInputDevice fatal error: cannot setActiveDevice() of pwsz == nullptr.";
		return ACTIVEINDEV_FAIL;
	}

	IMMDeviceEnumerator* pEnumerator{ NULL };
	HRESULT hr{ S_OK };

	hr = CoCreateInstance(
		MMDeviceEnumerator_CLSID, NULL,
		CLSCTX_ALL, IMMDeviceEnumerator_IID,
		(void**)&pEnumerator);

	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to create MMDeviceEnumerator component object.";
		m_pIMMDevice = NULL;
		EXIT_ON_ERROR(hr);
	}

	hr = pEnumerator->GetDevice(pwszID, &m_pIMMDevice);
	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to register " << pwszID << " as an active device.";
		m_pIMMDevice = NULL;
		EXIT_ON_ERROR(hr);
	}
	LOGD << "ActiveInputDevice: registered " << pwszID << " as an active device.";

	hr = m_pIMMDevice->Activate(
		IAudioClient_IID, CLSCTX_INPROC_SERVER,
		NULL, (void**)&m_pAudioClient
	);

	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to create AudioClient Component Object.";
		m_pAudioClient = NULL;
		EXIT_ON_ERROR(hr);
	}

	hr = getStreamFormat(m_pAudioClient, &m_pDeviceFormat);

	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to retrieve device format.";
		EXIT_ON_ERROR(hr);
	}

	SAFE_RELEASE(pEnumerator);
	return ACTIVEINDEV_SUCCES;

Exit:
	if (m_pDeviceFormat != nullptr) {
		CoTaskMemFree(m_pDeviceFormat);
	}
	SAFE_RELEASE(m_pAudioClient);
	SAFE_RELEASE(pEnumerator);
	return ACTIVEINDEV_FAIL;
}

int ActiveInputDevice::setDeviceType(DeviceType type)
{
	if (type == NO_DEVICE || type == CAPTURE_DEVICE || type == RENDER_DEVICE) {
		m_deviceType = type;
		return ACTIVEINDEV_SUCCES;
	}
	else {
		LOGD << "ActiveInputDevice error: trying to set device type to an invalid device type.";
		return ACTIVEINDEV_FAIL;
	}
}

int ActiveInputDevice::setSampleRate(DWORD desiredRate)
{
	std::vector<DWORD> sampleRateRange;
	int result{ 0 };
	if (m_pIMMDevice == NULL || m_pDeviceFormat == NULL) {
		LOGD << "ActiveInputDevice: trying to change sample rate of inactive device.";
	}
	result = this->getSampleRateRange(&sampleRateRange);
	if (result != ACTIVEINDEV_SUCCES) {
		LOGD << "ActiveInputDevice: error setting sample rate, could not retrieve supported sample rates.";
		return ACTIVEINDEV_FAIL;
	}

	bool sampleRateSupported{ false };

	for (DWORD rate : sampleRateRange) {
		if (rate == desiredRate) {
			sampleRateSupported = true;
		}
	}
	if (sampleRateSupported == true) {
		m_pDeviceFormat->nSamplesPerSec = desiredRate;
	}
	else {
		m_virtualSampleRate = desiredRate;
	}
	return ACTIVEINDEV_SUCCES;
}

int ActiveInputDevice::initStream()
{
	REFERENCE_TIME devicePeriod{ 0 };
	HRESULT hr{ S_OK };
	UINT32 bufferFrameCount{ 0 };
	double actualDurationBuffer{ 0 };
	WORD channelCount{ 0 };

	if (FAILED(hr)) {
		LOGD << "ActiveInputDevice: could not create audio client component object.";
		EXIT_ON_ERROR(hr);
	}

	devicePeriod = this->getMinimalLatency() + HEADROOM_TIME;
	if (!devicePeriod) {
		LOGD << "ActiveInputDevice error: trying to stream from inactive device.";
		return ACTIVEINDEV_FAIL;
	}

	hr = getStreamFormat(m_pAudioClient, &m_pDeviceFormat);

	hr = m_pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_EXCLUSIVE,
		0,
		devicePeriod,
		devicePeriod,
		m_pDeviceFormat,
		NULL
	);
	if (FAILED(hr)) {
		LOGD << "ActiveInputDevice error: failed to initialize audio client.";
		EXIT_ON_ERROR(hr);
	}

	hr = m_pAudioClient->GetBufferSize(&bufferFrameCount);
	LOGD << "This is the buffer size: " << bufferFrameCount;

	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: could not retrieve endpoint buffer size.";
		EXIT_ON_ERROR(hr);
	}

	channelCount = this->getChannelCount();
	m_bufferVector.resize(channelCount);
	bufferFrameCount += 100;
	for (int i = 0; i < channelCount; i++) {
		DoubleCircularBuffer* buffer = new DoubleCircularBuffer(2048, true);
		m_bufferVector[i] = buffer;
	}

	hr = m_pAudioClient->GetService(
		IAudioCaptureClient_IID,
		(void**)&m_pCaptureClient
	);

	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to create AudioCaptureClient Component Object.";
		EXIT_ON_ERROR(hr);
	}

	hr = m_pAudioClient->Start();
	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed start audio client. (Failed to active mic.)";
		EXIT_ON_ERROR(hr);
	}

	return ACTIVEINDEV_SUCCES;

Exit:
	SAFE_RELEASE(m_pCaptureClient);
	return ACTIVEINDEV_FAIL;

}


int ActiveInputDevice::getPacketStreamToPtr(BYTE** ppData)
{
	HRESULT hr{ S_OK };
	UINT32 packetLength{ 0 }, numFramesAvailable{ 0 };
	DWORD flags{ 0 };
	hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
	//LOGD << "ActiveInputDevice: packet length = " << packetLength;

	if (packetLength != 0) {
		// Get available data in shared buffer (endpoint buffer = stream)
		hr = m_pCaptureClient->GetBuffer(
			ppData,
			&numFramesAvailable,
			&flags, NULL, NULL
		);
	}

	EXIT_ON_ERROR(hr);

	if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
		*ppData = nullptr;
	}

	hr = m_pCaptureClient->ReleaseBuffer(numFramesAvailable);
	EXIT_ON_ERROR(hr);
	return numFramesAvailable;

Exit:
	m_pAudioClient->Stop();
	SAFE_RELEASE(m_pCaptureClient);
	LOGD << "ActiveInputDevice error: an error occured while getting next packet. Stopping the audio client.";
	LOGD << "ActiveInputDevice error: an error occured while getting next packet. Released m_pCaptureClient";
	return ACTIVEINDEV_FAIL;
}

int ActiveInputDevice::setupFFMpegSWR(SwrContext** swr)
{
	BYTE* pData;
	int result{ 0 };
	int channelCount{ this->getChannelCount() };
	int physSampleRate{ (int)this->getCurrentSampleRate() };
	int virtualSampleRate{ (int)this->getVirtualSampleRate() };
	enum AVSampleFormat src_sample_fmt;

	if (virtualSampleRate == 0) {
		virtualSampleRate = physSampleRate;
	}

	// Set up conversion tool:
	// Mono or stereo source?
	int64_t src_ch_layout, dst_ch_layout;
	if (channelCount == 1) {
		src_ch_layout = AV_CH_LAYOUT_MONO;
	}
	else if (channelCount == 2) {
		src_ch_layout = AV_CH_LAYOUT_STEREO;
	}
	else {
		LOGF << "ActiveInputDevice fatal error: zero or more than two channels is not supported.";
		return ACTIVEINDEV_FAIL;
	}
	
	dst_ch_layout = src_ch_layout;

	// Uncompressed PCM?
	if (m_pDeviceFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE* waveFormat = (WAVEFORMATEXTENSIBLE*)m_pDeviceFormat;
		if (waveFormat->SubFormat == WMMEDIASUBTYPE_PCM) {
			src_sample_fmt = AV_SAMPLE_FMT_S16;
		}
		else {
			LOGF << "ActiveInputDevice fatal error: only uncompressed PCM is possible. Device format should be of type WAVEFOMRATEXSTENSIBLE";
			return ACTIVEINDEV_FAIL;
		}
	}
	else {
		LOGF << "ActiveInputDevice fatal error: only uncompressed PCM is possible. Device format should be of type WAVEFOMRATEXSTENSIBLE";
		return ACTIVEINDEV_FAIL;
	}

	(*swr) = swr_alloc_set_opts(NULL,  // we're allocating a new context
		dst_ch_layout,			// out_ch_layout
		AV_SAMPLE_FMT_FLT,		// out_sample_fmt
		virtualSampleRate,		// out_sample_rate
		src_ch_layout,			// in_ch_layout
		src_sample_fmt,			// in_sample_fmt
		physSampleRate,			// in_sample_rate
		0,						// log_offset
		NULL);					// log_ctx

	swr_init((*swr)); // SWR Set up done

	return 0;
}

int decodeCount{ 0 };

int ActiveInputDevice::writeToBuffer(SwrContext* swr)
{
	BYTE* pData[1];
	const uint8_t** inputPtr{(const uint8_t**) &pData[0] };
	BYTE* pDecoded{ nullptr };
	int result{ 0 };
	uint8_t** dst_buffer;
	int dst_linesize{ 0 };
	int src_nb_samples{ 0 }, dst_nb_samples{ 0 };
	int dst_bufsize{ 0 };
	int virtualSampleRate = m_virtualSampleRate;
	
	int physSampleRate = this->getCurrentSampleRate();
	if (virtualSampleRate == 0) {
		virtualSampleRate = physSampleRate;
	}
	int channelCount = (int) this->getChannelCount();

	src_nb_samples = this->getPacketStreamToPtr(&pData[0]);

	dst_nb_samples = av_rescale_rnd(src_nb_samples, virtualSampleRate, physSampleRate, AV_ROUND_UP);
	result = av_samples_alloc_array_and_samples(&dst_buffer, &dst_linesize, channelCount,
		dst_nb_samples, AV_SAMPLE_FMT_FLT, 0);


	if (src_nb_samples) {
		// Convert (and resample)
		result = swr_convert(swr, dst_buffer, dst_nb_samples, inputPtr, src_nb_samples);
		if (result < 0) {
			LOGF << "ActiveInputDevice fatal error : something went wrong converting.";
			return 0;
		}

		float* outputSample{ nullptr };
		outputSample = (float*)dst_buffer[0];
		for(int i = 0; i < dst_nb_samples; i++) {
			if (channelCount > 1) {
				for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
					this->m_bufferVector[channelIndex]->write(*outputSample);
					outputSample++;
				}
			}
			else {
				this->m_bufferVector[0]->write(*outputSample);
				outputSample++;
			}
		}
		if (dst_buffer)
			av_freep(&dst_buffer[0]);
		av_freep(&dst_buffer);

		return dst_nb_samples;
	}

	else {
		if (dst_buffer)
			av_freep(&dst_buffer[0]);
		av_freep(&dst_buffer);
		return 0;
	}
	
}


HRESULT ActiveInputDevice::getStreamFormat(IAudioClient* pAudioClient, WAVEFORMATEX** ppFormat)
{
	IPropertyStore* pStore{ nullptr };
	HRESULT hr{ S_OK };
	PROPVARIANT prop;
	PWAVEFORMATEX validFormat{ nullptr };

	hr = m_pIMMDevice->OpenPropertyStore(STGM_READ, &pStore);

	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to retrieve proprty store from device.";
		EXIT_ON_ERROR(hr);
	}

	hr = pStore->GetValue(PKEY_AudioEngine_DeviceFormat, &prop);
	if (FAILED(hr)) {
		LOGF << "ActiveInputDevice fatal error: failed to retrieve device format.";
		EXIT_ON_ERROR(hr);
	}

	hr = pAudioClient->IsFormatSupported(
		AUDCLNT_SHAREMODE_EXCLUSIVE,
		(PWAVEFORMATEX)prop.blob.pBlobData,
		NULL
	);

	if (FAILED(hr)) {
		LOGD << "ActiveInputDevice: device format from property store not supported.";
		EXIT_ON_ERROR(hr);
	}
	else {
		validFormat = (WAVEFORMATEX*)prop.blob.pBlobData;
		WAVEFORMATEXTENSIBLE* validFormatEx = (WAVEFORMATEXTENSIBLE*)validFormat;
		// Stream format should be WMMEDIASUBTYPE_PCM (Subguid)
		*ppFormat = (PWAVEFORMATEX)prop.blob.pBlobData;
	}

	SAFE_RELEASE(pStore);
	return S_OK;

Exit:
	SAFE_RELEASE(pStore);
	return E_FAIL;
}











