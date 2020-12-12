/*
	Steinberg Audio Stream I/O API
	(c) 1996 - 2019, Steinberg Media Technologies GmbH
	charlie (May 1996)

	asiodrvr.h
	c++ superclass to implement asio functionality. from this,
	you can derive whatever required
*/

#ifndef _asiodrvr_
#define _asiodrvr_
#include "pch.h"

// cpu and os system we are running on
#include "asiosys.h"
// basic "C" interface
#include "asio.h"
// Extra includes for functionality
#include "AggregateDeviceManager.h"
#include "ActiveInputDevice.h"
#include "DoubleCircularBuffer.h"
// MT libraries:
#include <synchapi.h>

// Custom ASIO structure to give access to the buffers to the user
typedef struct LIBAGGREGATOR_API ASIOCustomBufferInfo
{
	LPWSTR friendlyName;
	long channelNum;
	std::vector<DoubleCircularBuffer*>* pBufferVector;			// Typically bufferVector[0] is left and bufferVector[1] is right.
	int availableSamples;

} ASIOCustomBufferInfo;

// Custom ASIO structure to make multithreading work.
typedef struct ASIOThreadParameters {
	ActiveInputDevice* pActiveInputDevice;
	std::vector<ASIOCustomBufferInfo>* pBufferInfoVector;
	int deviceIndex;
	void (*bufferSwitchCallback)(std::vector<ASIOCustomBufferInfo>* bufferInfoVector);
} ASIO ;


class LIBAGGREGATOR_API AsioDriver;
//extern AsioDriver *getDriver();		// for generic constructor 

class LIBAGGREGATOR_API AsioDriver
{
private:
	AggregateDeviceManager m_aggregateDeviceManager;

public:

	// Constructor & destructor:
	AsioDriver();
	virtual ~AsioDriver();	

	// Methods:

	// Pass it a vector containing all the indices of the input devices you would like to add to aggregateInputDevice
	virtual ASIOBool init(std::vector<int> inputIndices);
	// Obvious
	virtual void getDriverName(char *name);	// max 32 bytes incl. terminating zero
	// Obious
	virtual long getDriverVersion();

	// Not implemented
	virtual void getErrorMessage(char *string);	// max 124 bytes incl.

	/*
	 * Parameters:	You need to pass the address to the callback function void bufferSwitchCallback(std::vector<ASIOCustomBufferInto>* bufferInfoVector)
	 *				You also need to initialize an empty vector std::vector<ASIOCustomBufferInfo> bufferInfoVector and pass it to start().
	*/				
	virtual ASIOError start(void (*bufferSwitchCallback)(std::vector<ASIOCustomBufferInfo>* bufferInfoVector), std::vector<ASIOCustomBufferInfo>* bufferInfoVector);
	// Stops the driver. You need to do this if you want to change settings e.g. sample rate, add another device...
	virtual ASIOError stop();

	// IMPLEMENTED FUNCTIONS:

	// Set a pointer to this class' m_aggregateDeviceManager. You will need this object to add/remove devices.
	virtual ASIOError getAggregateDeviceManager(AggregateDeviceManager** aggregateDeviceManager);
	// Will add up all channels of devices and store count in numInputChannels. Note no output device is active so outputChannels is set to 0.
	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels);
	// Will find the slowest device and will store this latency in ms in inputLatency. Note no output device is active so outputLatency is set to 0.
	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency);
	// Not implemented
	virtual ASIOError getBufferSize(long *minSize, long *maxSize,
		long *preferredSize, long *granularity);
	// Will return ASE_OK if there is a device present that can physically handle the desired sample rate.
	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate);
	// Will return the highest sample rate of all active devies (devices with a lower sample rate, should have a virtual sample rate that is equal to highest sample rate)
	// You should call setSampleRate first though. Always call setSampleRate before starting the ASIO Driver.
	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate);
	// Will change the sample rate to chosen sample rate of all active devices.
	// NOTE: for changes to take effect, the streams need to be stopped, reinitialized and started again.
	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate);
	// Not implemented.
	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources);
	// Not implemented.
	virtual ASIOError setClockSource(long reference);
	// Not implemented.
	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp);
	// Not implemented.
	virtual ASIOError getChannelInfo(ASIOChannelInfo *info);
	// Uses my custom buffer info struct to store pointer to the buffer and provide extra information.
	// Leave bufferSize 0 for default value (usually best value)
	virtual ASIOError createBuffers(std::vector<ASIOCustomBufferInfo>* bufferInfoVector,
		long bufferSize);
	// Call this function to free allocated ActiveInputDevices. This will also get rid of the buffers.
	// If you wish to run ASIO Driver again, you will need to add ActiveInputDevices again to the AggregateDeviceManager.
	virtual ASIOError disposeBuffers();
	// Not implemented
	virtual ASIOError controlPanel();
	// Not implemented
	virtual ASIOError future(long selector, void *opt);
	// Not implemented
	virtual ASIOError outputReady();
};

#endif

