#ifndef AGGREGATEDEVICEMANAGER_H_291020
#define AGGREGATEDEVICEMANAGER_H_291020

#include "pch.h"
#include "plog/Log.h"
#include "IDevice.h"
#include "ActiveInputDevice.h"

#include <Mmdeviceapi.h>
#include <Windows.h>
#include <string>
#include <vector>



// ERROR CODES:
constexpr int AGGDEVMAN_SUCCES{ 0 };
constexpr int AGGDEVMAN_FAIL{ -1 };
constexpr int AGGDEVMAN_COM_FAIL{ -2 };
constexpr int AGGDEVMAN_DEV_ALREADY_ACTIVE{ -3 };
constexpr int AGGDEVMAN_NOT_FOUND{ -4 };


class AggregateDeviceManager
{
	

public:
	// Attributes:
	IMMDeviceCollection* m_pInputDeviceCollection;
	std::vector<IDevice*> m_aggregateInputDevice;
	ActiveInputDevice dummyInputDevice;
	int m_activeInputDevices;

	IMMDeviceCollection* m_pOutputDeviceCollection;
	std::vector<IDevice*> m_aggregateOutputDevice;
	int m_activeOutputDevices;

	// Constructor & Destructor
	AggregateDeviceManager();
	~AggregateDeviceManager();

	// Getters & Setters:
	IMMDeviceCollection* getDeviceCollection();

	// Other functions:

	/*
	 * enumInputDevices() is automatically called upon the creation of a AggregateDeviceManager object.
	 * Make sure that the Component Object pDeviceCollection is properly initialized and contains a collection of active MMDevices.
	 * RETURNS: AGGDEVMAN_SUCCESS on succes, AGGDEVMAN_NOTFOUND if no devices were found and AGGDEVMAN_FAIL on failure.
	 */
	int enumInputDevices();

	/*
	 * getFriendlyNameInputDevices() will store the friendly names of active devices in a vector. (You need to provide a pointer to this vector.)
	 * It will do so in the order they occur in de pDeviceCollection object.
	 * The indices hence match.
	 * You should pass this function a vector, these things deallocate themselves when they go out of scope :)
	 * 
	 * RETURNS: std::string* (= array of std::string)
	 * If there are no active devices, it will be NULL.
	 */

	int getFriendlyNameInputDevices(std::vector<LPWSTR>* pFriendlyNamesVector);

	/*
	* getDeviceByIndex() returns the device ID which is stored as an LPWSTR (Some kind of string)
	* This function uses the pDeviceCollection compontent object, hence the order of the devices is as the one found in this component.
	* (The same order in which you retrieve the friendly names)
	* 
	* RETURNS the MMDevice object ID on success, NULL if an error happened.
	*/
	LPWSTR getInputDeviceIdByIndex(int index);

	/*
	* addInputDeviceById(LPWSTR endpointId) creates a new ActiveInputDevice object and adds this to the aggregateDevice (= array of IDevice*)
	* I decided to implement an add by id function instead of an add by index functions
	* to circumvent the problem in case indices change (e.g. a device connects/disonnects).
	* You do need to get the Id from getInputDeviceIdByIndex, which uses an index but after that devices are decoupled from their index.
	* 
	* IMPORTANT: addInputDeviceById allocats memory to store the new ActiveDevice object!
	* UPDATE: memory can now easily be freed by calling ASIO disposebuffers().
	* 
	* RETURNS: AGGDEVMAN_SUCCES on success and AGGDEVMAN_DEV_ALREADY_ACTIVE if device was already active.
	*/
	int addInputDeviceById(LPWSTR pwszID);

	/*
	* Loops through m_aggregateInputDevice to find ActiveInputDevice with corresponding device ID.
	* RETURNS: AGGDEVMAN_SUCESS on success and AGGDEVMAN_NOT_FOUND if corresponding ActiveInputDevice was not found.
	*/
	int removeInputDeviceById(LPWSTR pwszID);


};



#endif