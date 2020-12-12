#pragma once

#include <JuceHeader.h>

//Aggregator includes
#include "aggregator_config.h"
#include "AudioFile.h"
#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"
#include  "bufferSwitchCallback.h"


#include <Mmdeviceapi.h>
#include <Windows.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <audioclient.h>
#include <vector>

extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libswresample/swresample.h"
}



//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component,
    public juce::Slider::Listener


{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

    void init_juceGUIcomponents();
    void init_asioDriver();
    void start_asioDriver();
    //  bool checkDeviceType();

    void buttonHandler();
    void comboboxHandler(juce::ComboBox* box);
    void play();
    void stop();



private:

    enum class PlayState
    {
        Play,
        Stop
    };


    PlayState playState{ PlayState::Stop };
    juce::TextButton startBroadcasting;

    AsioDriver* asioDriver;
    ASIOError asioResult;
    std::vector<LPWSTR> friendlyNames;
    AggregateDeviceManager* aggregateDevice;


    std::vector<int> desiredIndices{ -1,-1 }; // use index 0 for device 1 and index 1 for device 2!
    std::vector<ASIOCustomBufferInfo> bufferInfoVector;

    juce::Label devicelist1_header;
    juce::Label devicelist2_header;
    juce::ComboBox devicelist1_combobox;
    juce::ComboBox devicelist2_combobox;
    juce::Label aggregator_label;
    juce::Label soundeffect_label;

    juce::TextEditor enter_ip;
    juce::Label enter_ip_label;

    // for dsp part part
    juce::Label DSP_label;
    juce::Slider slider_pitch_device1;
    juce::Slider slider_pitch_device2;
    juce::Slider slider_mix_device1;
    juce::Slider slider_mix_device2;
    juce::Label slider_pitch_device1_label;
    juce::Label slider_pitch_device2_label;
    juce::Label slider_mix_device1_label;
    juce::Label slider_mix_device2_label;

    juce::Label soundeffect_combobox_label;
    juce::ComboBox soundeffect_combobox;

    juce::Slider compression_slider_treshold;
    juce::Slider compression_slider_ratio;
    juce::Slider compression_slider_attacktime;
    juce::Slider compression_slider_releasetime;
    juce::Label compression_slider_treshold_label;
    juce::Label compression_slider_ratio_label;
    juce::Label compression_slider_attacktime_label;
    juce::Label compression_slider_releasetime_label;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent);
};
