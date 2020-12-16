TEAM4_COLLECTION ZIP READ ME

-- AGGREGATOR_Source_and_library.zip --

This folder contains the  aggregator source and library code.
This is not a working program, but rather a package that can be used in another program.

-- AlignAndMix.zip --

This code contains the signal alignment functionality.

-- FUSION_CENTRE_FINAL --

This version of our program sends data via UDP to the receiver.
The IP address 127.0.0.1 is hardcoded.
So if you run this program and the receiver program (in receiverJuce.zip), you should hear some distored sound coming out of your speakers.

To run and compile this code:
1. Open the juce project in Projucer
2. Press the Visual Studio button
3. Go to project properties -> linker -> input
   And add: swresample.lib
	    avutil.lib
	    dspDLL.lib

A recording of before and after DSP can be found in either the visual studio folder or the Win32 build folder.
This depends if you run the program from within Visual Studio or if you run the .exe in the build folder.

-- FUSION_CENTRE_FINAL_OFFLINE --

This is a offline version of our program.
Note that lots of GUI components are broken and wrong parameter values are used for the processing.

To run and compile this code:
1. Open the juce project in Projucer
2. Press the Visual Studio button
3. Go to project properties -> linker -> input
   And add: swresample.lib
	    avutil.lib
	    dspDLL.lib

A recording of before and after DSP can be found in either the visual studio folder or the Win32 build folder.
This depends if you run the program from within Visual Studio or if you run the .exe in the build folder.

-- libDSP_package_final.zip --

This is our DSP library that works on Windows.
Can be reused in other projects.

-- receiverJuce.zip --

Simple Juce application that acts as receiver.
It will receive samples from a UDP connection and play them back.

-- vocoder-DSP-mac-edtion --

Original source code of DSP functionality except for signal alignment.
It includes the pitch shifting and the autotune.

-- COMPRESSOR (NOT IN A ZIP) --

The compression functionality can be found in compressor.cpp and compressor.h in either FUSION_CENTRE programs.
