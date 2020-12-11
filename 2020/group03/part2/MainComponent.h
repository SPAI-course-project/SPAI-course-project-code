#pragma once

#include <JuceHeader.h>
#include "smbPitchShift.h"
#include "AudioFX.h"
#include <stdio.h>
#include <complex>
#include <limits>

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

    int gccphat(float data[], float data2[], int index);

    int get_most_occuring(int offsets[1000], int backlogSize);

    int wrap_index(int ind);

    int wrap_index(int ind, int wrapValue);

private:
    //==============================================================================


    
    // Audio Setup Component
    juce::AudioDeviceSelectorComponent audioSetupComp;

    // Controls
    juce::Label readyLabel;
    juce::ToggleButton readyToggle;
    juce::Slider attSlider1;
    juce::Slider attSlider2;
    juce::ToggleButton delayToggle;
    juce::Slider mixSlider;
    juce::Slider pitchSlider1;
    juce::ToggleButton pitchToggle1;
    juce::Slider pitchSlider2;
    juce::ToggleButton pitchToggle2;
    juce::Slider fxSlider;
    juce::ToggleButton fxToggle;
    juce::Slider outputSlider;
    juce::Slider delayBacklogSize;
    juce::Slider delayTresholdLevel;

    // Control labels
    juce::Label attLabel1;
    juce::Label attLabel2;
    juce::Label mixLabel;
    juce::Label pitchLabel1;
    juce::Label pitchLabel2;
    juce::Label fxLabel;
    juce::Label fxLevelLabel;
    juce::Label outputLabel;
    juce::Label delayLabel;
    juce::String delayString;

    // Pitch shifter cores
    PitchShiftCore pitchCoreLeft1;
    PitchShiftCore pitchCoreRight1;
    PitchShiftCore pitchCoreLeft2;
    PitchShiftCore pitchCoreRight2;

    // Audio FX
    FXCore fxCore;

    // WAV Export
    bool isRecording = false;
    juce::File wavFile;
    juce::WavAudioFormat wavFormat;
    juce::AudioFormatWriter* writer;
    juce::FileOutputStream* outStream;

    // GCC PHAT STUFF

    juce::AudioBuffer<float> delayBuffer;
    int delayIndex = 0;

    int offset1 = 0;
    int offset2 = 0;

    juce::dsp::FFT fft_j;
    static constexpr auto fftOrder = 12;
    static constexpr auto fftSize = 1 << fftOrder;

    std::array<std::complex<float>, fftSize> fftin;
    std::array<std::complex<float>, fftSize> fftout;
    std::array<std::complex<float>, fftSize> data_spec;

    float input_buffer[4][fftSize];
    int input_index = 0;

    float delay[8000]; // 44kHz, 50ms so 2000 samples ... should be way less because 8kHz was suggested by coach
    int delay_index = 0;
    int delay_size = 8000;
    int delay_samples = 0;
    int offset = 0;
    int offset_history[1000];
    int offset_history_index = 0;

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