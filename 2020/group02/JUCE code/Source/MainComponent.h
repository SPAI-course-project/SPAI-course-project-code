#pragma once

/****************************************************************************************************
The main class of our application. Inherits from our self-defined class AggregatingAudioComponent.
It implements the GUI functionality and contains objects of our DSP classes, which it calls in the 
main callback method getNextAudioBlock.
*****************************************************************************************************/

#include <JuceHeader.h>
#include "Flanger.h"
#include "PitchShifter.h"
#include "AggregatingAudioAppComponent.h"
#include "Time_Delay_Estimation/alignsigs.h"
#include "UDP_test/UDP.h"


class MainComponent : public AggregatingAudioAppComponent
{
public:
    
    MainComponent();
    ~MainComponent();

    // ******* GUI-related methods *************************//

    void resized() override;
    void paint(Graphics& g) override;
    void showBeginScreen();
    void dropdownChanged();
    void upButton0Clicked();
    void upButton1Clicked();
    void downButton0Clicked();
    void downButton1Clicked();
    void FlangerWindow();
    void pitchShift();
    void pitchLevelChanged();
    void deviceLevelChanged();
    void depthChanged();
    void feedbackChanged();
    void LFO_rateChanged();

    void deviceOptionsClicked();
    void DSPModuleClicked();
    void networkOptionsClicked();

    //******** DSP-related methods **************************//

    void fillTDEbuffer(int device_id, int bufferLength, int channel, const float* bufferData);
    void adjustTDEbuffer(int device_id, int numSamplesInBuffer);
    void TimeDelayEstimation();
    

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;        // main DSP initialization method
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;        // main DSP callback method
    void releaseResources() override;


private:
        
    /*********************************************************/
    //******** GUI-related attributes ***********************//
    /*********************************************************/

    // ------- Start screen of application -----------------------------------------------------------//
    
    juce::LookAndFeel_V3* applayout;
    juce::TextButton deviceOptions, DSPModule, networkSettings;
    

    // ------- Device settings & aggregation menu ----------------------------------------------------//

    juce::AudioDeviceSelectorComponent audioSetupComp, audioSetupComp2;

    //-------- Device settings menus for DSP module --------------------------------------------------//

    juce::GroupComponent device0Parameters{ "group", "Device1" }, device1Parameters{ "group", "Device2" };
    juce::ComboBox dropdown;

    // Effect selection -------------------------------------------

    juce::Label textLabel{ {}, "Select audio effect" }, deviceLevelLabel{ {}, "Configure device levels" };
    juce::Font textFont{ 12.0f };

    // Time delay estimation --------------------------------------

    juce::TextButton TDE_Button;
    juce::Label timeDifference;

    // Pitch shifting ---------------------------------------------

    juce::TextButton upButton0, upButton1;
    juce::TextButton downButton0, downButton1;
    juce::Slider pitchLevel0, pitchLevel1;

    // Flanger ----------------------------------------------------

    juce::Slider depth0, depth1;
    juce::Label  depthLabel0, depthLabel1;
    juce::Slider feedback0, feedback1;
    juce::Label  feedbackLabel0, feedbackLabel1;
    juce::Slider LFO_rate0, LFO_rate1;
    juce::Label  LFO_rateLabel0, LFO_rateLabel1;

    // Manual Mixing ----------------------------------------------
    
    juce::Slider deviceLevel;



    /*********************************************************/
    //******** DSP module-related attributes ***************//
    /*********************************************************/


    //-------- Audio effect objects ------------------------//

    Flanger  flanger0, flanger1;
    PitchShifter pitchshifter0, pitchshifter1;

    //-------- Circular buffer for TDE ---------------------//  
    
    juce::AudioBuffer<float> TDECircularBuffer;  
    int TDEWritePosition0{ 0 }, TDEWritePosition1{0};
    bool performingTDE{ false };

    //--------- Manual mixing control parameter -----------//

    float levelPerDevice{0.5};

    //--------- Global sample rate ------------------------//

    int sampleRate{ 48000 };

    
    
    /*******************************************************/
    //************** UDP-related attributes **************//
    /******************************************************/

    int UDP_socket_descriptor;
    sockaddr_in UDP_socket_address;

         
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
