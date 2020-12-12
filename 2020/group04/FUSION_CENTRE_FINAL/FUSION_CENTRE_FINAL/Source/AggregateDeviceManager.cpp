#include "pch.h"
#include "AggregateDeviceManager.h"
#include "IDevice.h"
#include "plog/Log.h"
#include "IDevice.h"
#include "ActiveInputDevice.h"

#include <Functiondiscoverykeys_devpkey.h>
#include <Mmdeviceapi.h>
#include <Windows.h>
#include <string>
#include <vector>

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

// Static constant variables
constexpr CLSID MMDeviceEnumerator_CLSID = __uuidof(MMDeviceEnumerator);
constexpr IID IMMDeviceEnumerator_IID = __uuidof(IMMDeviceEnumerator);

AggregateDeviceManager::AggregateDeviceManager()
{
    m_pInputDeviceCollection = NULL;
    m_aggregateInputDevice.resize(2); // Resizing is an expensive operation!, setting default size to 2 should be sufficient.
    m_aggregateInputDevice[0] = &dummyInputDevice;
    m_aggregateInputDevice[1] = &dummyInputDevice;
    m_activeInputDevices = 0;
    enumInputDevices();
}

AggregateDeviceManager::~AggregateDeviceManager()
{
    SAFE_RELEASE(m_pInputDeviceCollection);

}

IMMDeviceCollection* AggregateDeviceManager::getDeviceCollection()
{
	return m_pInputDeviceCollection;
}

int AggregateDeviceManager::enumInputDevices()
{
    
    HRESULT hr{ S_OK };
    IMMDeviceEnumerator* pEnumerator{ NULL };

    hr = CoCreateInstance(
        MMDeviceEnumerator_CLSID, NULL,
        CLSCTX_INPROC_SERVER, IMMDeviceEnumerator_IID,
        (void**)&pEnumerator);

    if (FAILED(hr)) {
        PLOGF << "AggregateDeviceManager fatal error: could not create IMMDeviceEnumerator CO.";
        EXIT_ON_ERROR(hr);
    }

    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &m_pInputDeviceCollection);

    if (FAILED(hr)) {
        PLOGF << "AggregateDeviceManager fatal error: could not enumerate active capture devices";
        EXIT_ON_ERROR(hr);
    }
    

    UINT count;
    hr = m_pInputDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        PLOGF << "AggregateDeviceManager fatal error: could not count active devices.";
        EXIT_ON_ERROR(hr);
    }
    if (count == 0){
        PLOGD << "AggregateDeviceManager: No active endpoints found...";
        SAFE_RELEASE(pEnumerator);
        return AGGDEVMAN_NOT_FOUND;
    }

    SAFE_RELEASE(pEnumerator);
    return AGGDEVMAN_SUCCES;

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(m_pInputDeviceCollection);
    return AGGDEVMAN_COM_FAIL;
}

int AggregateDeviceManager::getFriendlyNameInputDevices(std::vector<LPWSTR>* pFriendlyNamesVector)
{
    if (pFriendlyNamesVector == nullptr) {
        LOGF << "AggregateDeviceManager error: pFriendlyNamesVector cannot be a nullptr.";
    }

    HRESULT hr{ S_OK };
    UINT count{ 0 };
    IMMDevice* pEndpoint{ NULL };
    IPropertyStore* pProps{ NULL };

    hr = m_pInputDeviceCollection->GetCount(&count);

    if(FAILED(hr))
    {
        PLOGF << "AggregateDeviceManager fatal error: could not count active input devices.";
        SAFE_RELEASE(pEndpoint);
        SAFE_RELEASE(pProps);
        return AGGDEVMAN_COM_FAIL;
    }
    if(!count){
        SAFE_RELEASE(pEndpoint);
        SAFE_RELEASE(pProps);
        return AGGDEVMAN_COM_FAIL;
    }

    (*pFriendlyNamesVector).resize(count);

    for (ULONG i = 0; i < count; i++) {
        hr = m_pInputDeviceCollection->Item(i, &pEndpoint);
        if (FAILED(hr)) {
            PLOGF << "AggregateDeviceManager fatal error: could not retrieve endpoint input device.";
            EXIT_ON_ERROR(hr);
        }

        hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
        if (FAILED(hr)) {
            PLOGF << "AggregateDeviceManager fatal error: could not retrieve properties endpoint input device.";
            EXIT_ON_ERROR(hr);
        }
        
        PROPVARIANT varName;
        PropVariantInit(&varName);

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        if (FAILED(hr)) {
            PLOGF << "AggregateDeviceManager fatal error: could not retrieve friendly name.";
            EXIT_ON_ERROR(hr);
        }

        (*pFriendlyNamesVector)[i] = varName.pwszVal;
    }

    SAFE_RELEASE(pEndpoint);
    SAFE_RELEASE(pProps);

    return AGGDEVMAN_SUCCES;

Exit:
    SAFE_RELEASE(pEndpoint);
    SAFE_RELEASE(pProps);
    return AGGDEVMAN_COM_FAIL;
}

LPWSTR AggregateDeviceManager::getInputDeviceIdByIndex(int index)
{
    if (index < 0) {
        LOGF << "AggregateDeviceManager: index should be >= 0.";
        return NULL;
    }

    HRESULT hr{ S_OK };
    IMMDevice* pEndpoint{ NULL };
    LPWSTR pwszID{ NULL };

    hr = m_pInputDeviceCollection->Item(index, &pEndpoint);
    if (FAILED(hr)) {
        PLOGF << "AggregateDeviceManager fatal error: could not retrieve IMMDevice object from IMMDeviceCollection.";
        EXIT_ON_ERROR(hr);
    }

    hr = pEndpoint->GetId(&pwszID);
    if (FAILED(hr)) {
        PLOGF << "AggregateDeviceManager fatal error: could not retrieve ID from IMMDevice object.";
        EXIT_ON_ERROR(hr);
    }

    SAFE_RELEASE(pEndpoint);
    return pwszID;

Exit:
    SAFE_RELEASE(pEndpoint);
    return NULL;
}

int AggregateDeviceManager::addInputDeviceById(LPWSTR pwszID)
{
    if (pwszID == NULL) {
        LOGD << "AggregateDeviceManager: cannot remove NULL ID.";
        return AGGDEVMAN_FAIL;
    }

    int size{ 0 };
    size = m_aggregateInputDevice.size();
    LPWSTR dummyID{ NULL };
    bool devFound{ false }, added{ false };

    for (IDevice* dummyDevice : m_aggregateInputDevice) {
        dummyID = dummyDevice -> getDeviceId();
        if (dummyID == NULL) {
        }
        else if (wcscmp(pwszID, dummyID) == 0) {
            devFound = true;
        }
    }

    if (devFound) {
        LOGD << "AggregatorDeviceManager: trying to add an already active device to aggreate device.";
        return AGGDEVMAN_DEV_ALREADY_ACTIVE;
    }

    else {
        int index = 0;
        IDevice* dummyDevice;
        // Create new active device and allocate memory
        ActiveInputDevice* pActiveDevice{ new ActiveInputDevice(pwszID, CAPTURE_DEVICE) };

        while (!added && index != size) {
            dummyDevice = m_aggregateInputDevice.at(index);
            if (dummyDevice->getDeviceId() == NULL) {
                m_aggregateInputDevice[index] = pActiveDevice;
                added = true;
            }
            index++;
        } 

        if (!added && index == size) {
            m_aggregateInputDevice.resize(size + 1);
            m_aggregateInputDevice[index] = pActiveDevice;
            added = true;
        }
        LOGD << "AggregatorDeviceManager: successfully added " << pwszID << " to aggregate device!";
        m_activeInputDevices++;
        return AGGDEVMAN_SUCCES;
    }
}

int AggregateDeviceManager::removeInputDeviceById(LPWSTR pwszID)
{
    if (pwszID == NULL) {
        LOGD << "AggregateDeviceManager: cannot remove NULL ID.";
        return AGGDEVMAN_FAIL;
    }

    bool devFound{ false };
    int size = m_aggregateInputDevice.size();
    int index{ 0 };
    LPWSTR dummyID;
    while (!devFound && index < size) {
        dummyID = m_aggregateInputDevice.at(index)->getDeviceId();
        if (dummyID != NULL) {
            if (wcscmp(dummyID, pwszID) == 0) {
                devFound = true;
                break;
            }
        }
        index++;
    }
    if (devFound) {
        delete m_aggregateInputDevice.at(index);
        m_aggregateInputDevice[index] = &dummyInputDevice;
        LOGD << "AggregateDeviceManager: successfully removed " << pwszID << " from aggregate device.";
        m_activeInputDevices--;
        return AGGDEVMAN_SUCCES;
    }
    else {
        LOGD << "AggregateDeviceManager: " << pwszID << " not found in aggregate device.";
        return AGGDEVMAN_NOT_FOUND;
    }
}





