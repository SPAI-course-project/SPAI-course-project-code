#pragma once
#include <JuceHeader.h>

class AggregatingAudioAppComponent : public AudioAppComponent
{
public:
    AggregatingAudioAppComponent();
    ~AggregatingAudioAppComponent() override;

    // specific implementations

    void setAudioChannelsPrimaryDevice(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings = nullptr);
    void setAudioChannelsSecondaryDevice(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings = nullptr);
    virtual void prepareToPlay(int samplesPerBlockExpected,
        double sampleRate) override = 0;

    virtual void releaseResources() override = 0;

    virtual void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override = 0;


    void shutdownAudioForAllDevices();


    AudioDeviceManager& secondaryDeviceManager; // public attribute, share with audioAppcomponent

private:

    AudioSourcePlayer audioSourcePlayer2;
    AudioDeviceManager defaultDeviceManager2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AggregatingAudioAppComponent)
};



