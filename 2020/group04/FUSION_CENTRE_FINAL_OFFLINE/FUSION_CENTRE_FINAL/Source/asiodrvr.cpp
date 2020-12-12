/*
	Steinberg Audio Stream I/O API
	(c) 1996 - 2019, Steinberg Media Technologies GmbH
	charlie (May 1996)

	asiodrvr.cpp
	c++ superclass to implement asio functionality. from this,
	you can derive whatever required
*/


#include "pch.h"
#include <string.h>
#include "asiosys.h"
#include "asiodrvr.h"

// Extra includes for functionality
#include "AggregateDeviceManager.h"
#include "ActiveInputDevice.h"
#include <vector>
#include <windows.h>
#include <chrono>

extern "C" {
#include "avutil.h"
#include "channel_layout.h"
#include "swresample.h"
}

// MT Synchronization
#include <process.h>
#include <synchapi.h>

// Static variables;

SYNCHRONIZATION_BARRIER barrierBeforeCallback;
SYNCHRONIZATION_BARRIER barrierAfterCallback;
HANDLE AsioOnMutex;
bool m_AsioDriverOn;


AsioDriver::AsioDriver()
{
	m_AsioDriverOn = false;
}

AsioDriver::~AsioDriver()
{
}

ASIOBool AsioDriver::init(std::vector<int> inputIndices)
{
	LPWSTR pwszID;
	int result;
	bool threadResult{ false };
	int numberActiveDevices{ 0 };
	AsioOnMutex = CreateMutex(
		NULL,              // default security attributes
		false,             // initially not owned
		NULL);             // unnamed mutex

	if (AsioOnMutex == NULL) {
		LOGF << "AsioDriver: failed to initialize AsioOnMutex.";
		return ASE_NotPresent;
	}

	WaitForSingleObject(AsioOnMutex, INFINITE);
	m_AsioDriverOn = true;
	ReleaseMutex(AsioOnMutex);

	for (int i : inputIndices) {
		pwszID = m_aggregateDeviceManager.getInputDeviceIdByIndex(i);
		if (pwszID == NULL) {
			LOGF << "AsioDriver: Error initializing - device index out of range!";
				return ASIOFalse;
		}
		result = m_aggregateDeviceManager.addInputDeviceById(pwszID);
		if (result != AGGDEVMAN_SUCCES) {
			LOGF << "AsioDriver: Error initializing - failed to add input device.";
			return ASIOFalse;
		}
	}

	numberActiveDevices = m_aggregateDeviceManager.m_activeInputDevices;

	threadResult = InitializeSynchronizationBarrier(&barrierBeforeCallback, (long)numberActiveDevices, -1);
	if (!threadResult) {
		LOGF << "AsioDriver fatal error: Failed to initialize barrierBeforeCallback.";
		return ASIOFalse;
	}
	threadResult = InitializeSynchronizationBarrier(&barrierAfterCallback, (long)numberActiveDevices, -1);
	if (!threadResult) {
		LOGF << "AsioDriver fatal error: Failed to initialize barrierAfterCallback.";
		return ASIOFalse;
	}

	return ASIOTrue;
}

void AsioDriver::getDriverName(char *name)
{
	strcpy(name, "ASIO4US");
}

long AsioDriver::getDriverVersion()
{
	return 9;
}

void AsioDriver::getErrorMessage(char *string)
{
	//strcpy(string, "ASIO Driver Implementation Error!");
}

DWORD WINAPI mainReaderThread(LPVOID lpParameter)
{
	LOGD << "mainWriter created.";
	SwrContext* swr;
	ASIOThreadParameters* parameters{ (ASIOThreadParameters*)lpParameter };
	ActiveInputDevice* activeInputDevice{ parameters->pActiveInputDevice };
	std::vector<ASIOCustomBufferInfo>* pBufferInfoVector{ parameters->pBufferInfoVector };
	std::vector<DoubleCircularBuffer*>* pBufferVector = activeInputDevice->getBufferVectorPtr();
	int index{ parameters->deviceIndex };
	int result{ 0 }, availableSamples{ 0 };
	bool barrierResult{ false };

	swr = swr_alloc();
	result = activeInputDevice->setupFFMpegSWR(&swr);

	if (result != ACTIVEINDEV_SUCCES) {
		LOGF << "ActiveInputDevice: failed to setup swr. Exiting thread.";
		return -1;
	}
	
	WaitForSingleObject(AsioOnMutex, INFINITE);
	while(m_AsioDriverOn == true){
		ReleaseMutex(AsioOnMutex);
		availableSamples += activeInputDevice->writeToBuffer(swr);
		(*pBufferInfoVector)[index].availableSamples += availableSamples;
		for (DoubleCircularBuffer* pCircularBuffer : (*pBufferVector)) {
			pCircularBuffer->changeWrite();
		}
		barrierResult = EnterSynchronizationBarrier(&barrierBeforeCallback, NULL);
		parameters->bufferSwitchCallback(pBufferInfoVector);
		if ((*pBufferInfoVector)[index].availableSamples == 0) {
			availableSamples = 0;
		}
		barrierResult = EnterSynchronizationBarrier(&barrierAfterCallback, NULL);
		WaitForSingleObject(AsioOnMutex, INFINITE);
	}

	delete parameters;
	swr_free(&swr);
	return 0;
}

DWORD WINAPI sideReaderThread(LPVOID lpParameter)
{
	LOGD << "sideWriter created.";
	SwrContext* swr;
	ASIOThreadParameters* parameters{ (ASIOThreadParameters*)lpParameter };
	ActiveInputDevice* activeInputDevice{ parameters->pActiveInputDevice };
	std::vector<ASIOCustomBufferInfo>* pBufferInfoVector{ parameters->pBufferInfoVector };
	std::vector<DoubleCircularBuffer*>* pBufferVector = activeInputDevice->getBufferVectorPtr();
	int index{ parameters->deviceIndex };
	int result{ 0 }, availableSamples{ 0 };
	bool barrierResult{ false };

	swr = swr_alloc();
	result = activeInputDevice->setupFFMpegSWR(&swr);

	if (result != ACTIVEINDEV_SUCCES) {
		LOGF << "ActiveInputDevice: failed to setup swr. Exiting thread.";
		return -1;
	}

	WaitForSingleObject(AsioOnMutex, INFINITE);
	while (m_AsioDriverOn == true) {
		ReleaseMutex(AsioOnMutex);
		availableSamples += activeInputDevice->writeToBuffer(swr);
		(*pBufferInfoVector)[index].availableSamples += availableSamples;
		for (DoubleCircularBuffer* pCircularBuffer : (*pBufferVector)) {
			pCircularBuffer->changeWrite();
		}

		barrierResult = EnterSynchronizationBarrier(&barrierBeforeCallback, NULL);
		// Bufferswitch callback is done by main writer thread.
		barrierResult = EnterSynchronizationBarrier(&barrierAfterCallback, NULL);
		if ((*pBufferInfoVector)[index].availableSamples == 0) {
			availableSamples = 0;
		}

	WaitForSingleObject(AsioOnMutex, INFINITE);
	}

	delete parameters;
	swr_free(&swr);
	return 0;
}

ASIOError AsioDriver::start(void (*bufferSwitchCallback)(std::vector<ASIOCustomBufferInfo>* bufferInfoVector), std::vector<ASIOCustomBufferInfo>* bufferInfoVector)
{
	bool mainWriterCreated{ false };
	int numberActiveDevices{ m_aggregateDeviceManager.m_activeInputDevices };
	std::vector<HANDLE> writerThreadHandles;
	writerThreadHandles.resize(numberActiveDevices);
	ActiveInputDevice* pActiveInputDevice;
	int deviceIndex{ 0 };
	DWORD lastError;
	
	for (int i = 0; i < numberActiveDevices; i++) {

		ASIOThreadParameters* threadParameters = new ASIOThreadParameters;
		pActiveInputDevice = (ActiveInputDevice*) m_aggregateDeviceManager.m_aggregateInputDevice[i];

		if (mainWriterCreated == false) {
			mainWriterCreated = true;
			threadParameters->pActiveInputDevice = pActiveInputDevice;
			threadParameters->pBufferInfoVector = bufferInfoVector;
			threadParameters->deviceIndex = deviceIndex;
			threadParameters->bufferSwitchCallback = bufferSwitchCallback;
			deviceIndex++,
			writerThreadHandles[i] = (HANDLE) CreateThread(
				NULL,                   // default security attributes
				0,                      // use default stack size  
				&mainReaderThread,       // thread function name
				(LPVOID) threadParameters,          // argument to thread function 
				0,                      // use default creation flags 
				NULL);   // returns the thread identifier
			
		}
		else {
			threadParameters->pActiveInputDevice = pActiveInputDevice;
			threadParameters->pBufferInfoVector = bufferInfoVector;
			threadParameters->deviceIndex = deviceIndex;
			threadParameters->bufferSwitchCallback = bufferSwitchCallback;
			deviceIndex++,
			writerThreadHandles[i] = (HANDLE)CreateThread(
				NULL,                   // default security attributes
				0,                      // use default stack size  
				&sideReaderThread,       // thread function name
				(LPVOID)threadParameters,          // argument to thread function 
				0,                      // use default creation flags 
				NULL);   // returns the thread identifier
		}
	}

	return ASE_NotPresent;
}

ASIOError AsioDriver::stop()	
{
	WaitForSingleObject(AsioOnMutex, INFINITE);
	m_AsioDriverOn = false;
	ReleaseMutex(AsioOnMutex);

	// Wait until all threads have exited, then clean up sync primitives:
	Sleep(1000); // Ugly hack but works.
	LOGD << "AsioDriver: Cleaning up synchronization primitives.";
	CloseHandle(AsioOnMutex);
	DeleteSynchronizationBarrier(&barrierBeforeCallback);
	DeleteSynchronizationBarrier(&barrierAfterCallback);

	return ASE_NotPresent;
}

ASIOError AsioDriver::getAggregateDeviceManager(AggregateDeviceManager** aggregateDeviceManager)
{
	(*aggregateDeviceManager) = &m_aggregateDeviceManager;
	return ASIOError();
}

ASIOError AsioDriver::getChannels(long *numInputChannels, long *numOutputChannels)
{
	if (numInputChannels == nullptr || numOutputChannels == nullptr) {
		return ASE_InvalidParameter;
	}
	int count{ 0 };
	(*numOutputChannels) = 0;
	if (m_aggregateDeviceManager.m_activeInputDevices) {
		for (IDevice* inputDevice : m_aggregateDeviceManager.m_aggregateInputDevice) {
			if (inputDevice->getDeviceType() != NO_DEVICE) {
				count += inputDevice->getChannelCount();
			}
		}
		(*numInputChannels) = count;
		return ASE_OK;
	}
	else {
		(*numInputChannels) = 0;
		return ASE_NotPresent;
	}
}

ASIOError AsioDriver::getLatencies(long *inputLatency, long *outputLatency)
{
	if (inputLatency == nullptr || outputLatency == nullptr) {
		return ASE_InvalidParameter;
	}

	long maxInputLatency{ 0 };
	REFERENCE_TIME winDummyLatency{ 0 }, winMaxLatency{ 0 };
	if (m_aggregateDeviceManager.m_activeInputDevices) {
		for (IDevice* activeDevice : m_aggregateDeviceManager.m_aggregateInputDevice) {
			if (activeDevice->getDeviceType() != NO_DEVICE) {
				winDummyLatency = activeDevice->getMinimalLatency();
				if (winDummyLatency > winMaxLatency) {
					winMaxLatency = winDummyLatency;
					maxInputLatency = (long)winMaxLatency;
					maxInputLatency = maxInputLatency / 10;
				}
			}
		}
		(*outputLatency) = 0;
		(*inputLatency) = maxInputLatency;
		return ASE_OK;
	}
	else {
		return ASE_NotPresent;
	}
	
}

ASIOError AsioDriver::getBufferSize(long *minSize, long *maxSize,
		long *preferredSize, long *granularity)
{
	return ASE_NotPresent;
}

ASIOError AsioDriver::canSampleRate(ASIOSampleRate sampleRate)
{
	DWORD winSampleRate{ 0 };
	winSampleRate = (DWORD) sampleRate;
	std::vector<DWORD> sampleRateRange;
	int result{ 0 };
	bool sampleRateCapable{ false };
	if(m_aggregateDeviceManager.m_activeInputDevices) {
		for (IDevice* activeDevice : m_aggregateDeviceManager.m_aggregateInputDevice) {
			if (activeDevice->getDeviceType() != NO_DEVICE) {
				result = activeDevice->getSampleRateRange(&sampleRateRange);
				if (result != ACTIVEINDEV_SUCCES) {
					LOGF << "AsioDriver fatal error: could not retrieve sample rate range from active device.";
					return ASE_NotPresent;
				}
			}
			for (DWORD dummyRate : sampleRateRange) {
				if (dummyRate == winSampleRate) {
					sampleRateCapable = true;
				}
			}
		}
		if (sampleRateCapable) {
			return ASE_OK;
		}
		else {
			return ASE_NotPresent;
		}
		
	}
	else {
		return ASE_NotPresent;
	}
}

ASIOError AsioDriver::getSampleRate(ASIOSampleRate *sampleRate)
{
	if (sampleRate == nullptr) {
		return ASE_InvalidParameter;
	}
	DWORD maxSampleRate{ 0 };
	if (m_aggregateDeviceManager.m_activeInputDevices) {
		for (IDevice* activeDevice : m_aggregateDeviceManager.m_aggregateInputDevice) {
			if (activeDevice->getDeviceType() != NO_DEVICE) {
				if (maxSampleRate < activeDevice->getCurrentSampleRate()) {
					maxSampleRate = activeDevice->getCurrentSampleRate();
				}
			}
		}
		(*sampleRate) = (double) maxSampleRate;
		return ASE_OK;
	}
	else {
		return ASE_NotPresent;
	}
}

ASIOError AsioDriver::setSampleRate(ASIOSampleRate sampleRate)
{
	DWORD winSampleRate = (DWORD) sampleRate;
	if (m_aggregateDeviceManager.m_activeInputDevices) {
		for (IDevice* activeDevice : m_aggregateDeviceManager.m_aggregateInputDevice) {
			if (activeDevice->getDeviceType() != NO_DEVICE) {
				activeDevice->setSampleRate(winSampleRate);
			}
		}
		return ASE_OK;
	}
	else {
		return ASE_NotPresent;
	}
}

ASIOError AsioDriver::getClockSources(ASIOClockSource *clocks, long *numSources)
{
	*numSources = 0;
	return ASE_NotPresent;
}

ASIOError AsioDriver::setClockSource(long reference)
{
	return ASE_NotPresent;
}

ASIOError AsioDriver::getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp)
{
	return ASE_NotPresent;
}

ASIOError AsioDriver::getChannelInfo(ASIOChannelInfo *info)
{
	return ASE_NotPresent;
}

ASIOError AsioDriver::createBuffers(std::vector<ASIOCustomBufferInfo>* bufferInfoVector,
	long bufferSize)
{
	bufferInfoVector->resize(m_aggregateDeviceManager.m_activeInputDevices);
	int deviceCount{ 0 };
	ActiveInputDevice* pInputDevice;
	for (IDevice* pDevice : m_aggregateDeviceManager.m_aggregateInputDevice) {
		pInputDevice = (ActiveInputDevice*) pDevice;
		if (pInputDevice->m_deviceType != NO_DEVICE) {
			pInputDevice->initStream();
			(*bufferInfoVector)[deviceCount].friendlyName = pInputDevice->getFriendlyName();
			(*bufferInfoVector)[deviceCount].pBufferVector = pInputDevice->getBufferVectorPtr();
			(*bufferInfoVector)[deviceCount].channelNum = pInputDevice->getChannelCount();
			deviceCount++;
		}
	}
	return ASE_NotPresent;
}

ASIOError AsioDriver::disposeBuffers()
{
	int activeInputDevices{ m_aggregateDeviceManager.m_activeInputDevices };
	int result{ 0 };
	LPWSTR pwszID;
	for (int i = (activeInputDevices - 1); i >= 0; i--) {
		pwszID = m_aggregateDeviceManager.m_aggregateInputDevice[i]->getDeviceId();
		result = m_aggregateDeviceManager.removeInputDeviceById(pwszID);
		if (result != AGGDEVMAN_SUCCES) {
			LOGF << "AsioDriver: failed to remove an input device.";
			return ASE_NotPresent;
		}
	}
	return ASE_OK;
}

ASIOError AsioDriver::controlPanel()
{
	return ASE_NotPresent;
}

ASIOError AsioDriver::future(long selector, void *opt)
{
	return ASE_NotPresent;
}

ASIOError AsioDriver::outputReady()
{
	return ASE_NotPresent;
}