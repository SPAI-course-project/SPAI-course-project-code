#pragma once

#include <JuceHeader.h>
#include "smbPitchShift.h"
#include "AudioFX.h"
#include <stdio.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================


    
    // Audio Setup Component
    juce::AudioDeviceSelectorComponent audioSetupComp;

    // Controls
    juce::Label readyLabel;
    juce::ToggleButton readyToggle;
    juce::Slider attSlider1;
    juce::Slider attSlider2;
    juce::Slider mixSlider;
    juce::Slider pitchSlider;
    juce::ToggleButton pitchToggle;
    juce::Slider fxSlider;
    juce::ToggleButton fxToggle;
    juce::Slider outputSlider;

    // Control labels
    juce::Label attLabel1;
    juce::Label attLabel2;
    juce::Label mixLabel;
    juce::Label pitchLabel;
    juce::Label fxLabel;
    juce::Label fxLevelLabel;
    juce::Label outputLabel;

    // Pitch shifter cores
    PitchShiftCore pitchCoreLeft;
    PitchShiftCore pitchCoreRight;

    // Audio FX
    FXCore fxCore;

    // WAV Export
    bool recording1;
    juce::TimeSliceThread writeThread1;
    juce::ScopedPointer<juce::FileOutputStream> stream1;
    juce::ScopedPointer<juce::AudioFormatWriter> writer1;
    juce::ScopedPointer<juce::AudioFormatWriter::ThreadedWriter> threaded1;

    bool recording2;
    juce::TimeSliceThread writeThread2;
    juce::ScopedPointer<juce::FileOutputStream> stream2;
    juce::ScopedPointer<juce::AudioFormatWriter> writer2;
    juce::ScopedPointer<juce::AudioFormatWriter::ThreadedWriter> threaded2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
