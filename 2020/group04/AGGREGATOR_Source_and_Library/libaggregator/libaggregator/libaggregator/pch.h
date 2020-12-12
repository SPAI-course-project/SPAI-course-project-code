// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <Windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <audioclient.h>
#include <chrono>
#include <Ksmedia.h>
#include <mmreg.h>
#include <process.h>
#include <string>
#include <synchapi.h>
#include <vector>
#include <wmsdk.h>

#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"
#include "AudioFile.h"

#ifdef LIBAGGREGATOR_EXPORTS
#define LIBAGGREGATOR_API __declspec(dllexport)
#else
#define LIBAGGREGATOR_API __declspec(dllimport)
#endif


#endif //PCH_H
