#include "AggregatingAudioAppComponent.h"


AggregatingAudioAppComponent::AggregatingAudioAppComponent()
    : secondaryDeviceManager(defaultDeviceManager2)

{ }

AggregatingAudioAppComponent::~AggregatingAudioAppComponent()

{
    jassert(audioSourcePlayer2.getCurrentSource() == nullptr);
}

void AggregatingAudioAppComponent::setAudioChannelsPrimaryDevice(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings)
{
    setAudioChannels(numInputChannels, numOutputChannels);
}

void AggregatingAudioAppComponent::setAudioChannelsSecondaryDevice(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings)
{
    String audioError;
    audioError = secondaryDeviceManager.initialise(numInputChannels, numOutputChannels, storedSettings, true);

    jassert(audioError.isEmpty());
    secondaryDeviceManager.addAudioCallback(&audioSourcePlayer2);
    audioSourcePlayer2.setSource(this);
    audioSourcePlayer2.setDeviceId(1);
}

void AggregatingAudioAppComponent::shutdownAudioForAllDevices()
{
    shutdownAudio();
    audioSourcePlayer2.setSource(nullptr);
    secondaryDeviceManager.removeAudioCallback(&audioSourcePlayer2);

    //assuming by default we use 2 devices
    secondaryDeviceManager.closeAudioDevice();

}
