#include "MainComponent.h"
#include <fstream>

//==============================================================================
MainComponent::MainComponent()
    : audioSetupComp(deviceManager,
        0,     // minimum input channels
        256,   // maximum input channels
        0,     // minimum output channels
        256,   // maximum output channels
        false, // ability to select midi inputs
        false, // ability to select midi output device
        false, // treat channels as stereo pairs
        false) // hide advanced options
    ,
    audioSetupComp2(secondaryDeviceManager,
        0,     // minimum input channels
        256,   // maximum input channels
        0,     // minimum output channels
        256,   // maximum output channels
        false, // ability to select midi inputs
        false, // ability to select midi output device
        false, // treat channels as stereo pairs
        false) // hide advanced options
{
    setSize(600, 400);

    applayout = new LookAndFeel_V3();
    

    /*******************************************************************************
                                 GUI INITIALIZATION CODE 
    ********************************************************************************/

    //************* Initialize opening screen of application *********************//

    addAndMakeVisible(deviceOptions);
    deviceOptions.setButtonText("Device Options");
    deviceOptions.onClick = [this] {deviceOptionsClicked();};
    deviceOptions.setClickingTogglesState(true);
    deviceOptions.setVisible(false);

    addAndMakeVisible(DSPModule);
    DSPModule.setButtonText("DSP Module");
    DSPModule.onClick = [this] {DSPModuleClicked();};
    DSPModule.setClickingTogglesState(true);
    DSPModule.setVisible(false);

    addAndMakeVisible(networkSettings);
    networkSettings.setButtonText("Network settings");
    networkSettings.onClick = [this] {networkOptionsClicked();};
    networkSettings.setVisible(false);

    //************ Initialize device aggregation interface **********************//

    addAndMakeVisible(audioSetupComp);
    addAndMakeVisible(audioSetupComp2);
    audioSetupComp.setVisible(false);
    audioSetupComp2.setVisible(false);


    //************ DSP screen initialization ************************************//
       
    //------------- Dropdown menu -----------------------------------------------//

    addAndMakeVisible(textLabel);
    textLabel.setFont(textFont);
    textLabel.setVisible(false);

    addAndMakeVisible(dropdown);
    dropdown.addItem("Pitch Shift", 1);
    dropdown.addItem("Flanger", 2);
    dropdown.onChange = [this] { dropdownChanged(); };
    dropdown.setSelectedId(0);
    dropdown.setVisible(false);

    //------------- Manual mixing slider ---------------------------------------//

    addAndMakeVisible(deviceLevel);
    deviceLevel.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    deviceLevel.setLookAndFeel(applayout);
    deviceLevel.setRange(0,1, 0.05);
    deviceLevel.onValueChange = [this] { deviceLevelChanged(); };
    deviceLevel.setVisible(false);
    deviceLevel.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    
    
    //------------- Time delay estimation button -------------------------------//

    addAndMakeVisible(TDE_Button);
    TDE_Button.setButtonText("Time delay estimation");
    TDE_Button.onClick = [this] { TimeDelayEstimation(); };
    TDE_Button.setClickingTogglesState(true);
    TDE_Button.setVisible(false);

    addAndMakeVisible(timeDifference);
    timeDifference.setFont(textFont);
    timeDifference.setVisible(false);
        

    //------------- Pitch Shifting user interface -----------------------------//
    
    addAndMakeVisible(upButton0);
    upButton0.setButtonText("UP");
    upButton0.onClick = [this] { upButton0Clicked(); };
    upButton0.setVisible(false);
    upButton0.setClickingTogglesState(true);
    upButton0.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    upButton0.setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightseagreen);

    addAndMakeVisible(downButton0);
    downButton0.setButtonText("DOWN");
    downButton0.onClick = [this] { downButton0Clicked(); };
    downButton0.setVisible(false);
    downButton0.setClickingTogglesState(true);
    downButton0.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    downButton0.setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightseagreen);

    addAndMakeVisible(upButton1);
    upButton1.setButtonText("UP");
    upButton1.onClick = [this] { upButton1Clicked(); };
    upButton1.setVisible(false);
    upButton1.setClickingTogglesState(true);
    upButton1.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    upButton1.setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightseagreen);

    addAndMakeVisible(downButton1);
    downButton1.setButtonText("DOWN");
    downButton1.onClick = [this] { downButton1Clicked(); };
    downButton1.setVisible(false);
    downButton1.setClickingTogglesState(true);
    downButton1.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    downButton1.setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightseagreen);

    addAndMakeVisible(pitchLevel0);
    pitchLevel0.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    pitchLevel0.setLookAndFeel(applayout);
    pitchLevel0.setRange(0, 60, 1);
    pitchLevel0.onValueChange = [this] { pitchLevelChanged(); };
    pitchLevel0.setVisible(false);
    pitchLevel0.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    
    addAndMakeVisible(pitchLevel1);
    pitchLevel1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    pitchLevel1.setLookAndFeel(applayout);
    pitchLevel1.setRange(0, 60, 1);
    pitchLevel1.onValueChange = [this] { pitchLevelChanged(); };
    pitchLevel1.setVisible(false);
    pitchLevel1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    
    addAndMakeVisible(device0Parameters);
    device0Parameters.setVisible(false);
    addAndMakeVisible(device1Parameters);
    device1Parameters.setVisible(false);
        

    //------------- Flanger user interface--------------------------------------//
    
    addAndMakeVisible(depth0);
    depth0.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    depth0.setLookAndFeel(applayout);
    depth0.setRange(0, 1, 0.01);
    depth0.onValueChange = [this] { depthChanged(); };
    depth0.setVisible(false);
    depth0.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    depthLabel0.setText("depth", juce::dontSendNotification);
    depthLabel0.attachToComponent(&depth0, false);
    depthLabel0.setVisible(false);

    addAndMakeVisible(depth1);
    depth1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    depth1.setLookAndFeel(applayout);
    depth1.setRange(0, 1, 0.01);
    depth1.onValueChange = [this] { depthChanged(); };
    depth1.setVisible(false);
    depth1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    depthLabel1.setText("depth", juce::dontSendNotification);
    depthLabel1.attachToComponent(&depth1, false);
    depthLabel1.setVisible(false);

    addAndMakeVisible(feedback0);
    feedback0.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    feedback0.setLookAndFeel(applayout);
    feedback0.setRange(0, 1, 0.01);
    feedback0.onValueChange = [this] { feedbackChanged(); };
    feedback0.setVisible(false);
    feedback0.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    feedbackLabel0.setText("feedback", juce::dontSendNotification);
    feedbackLabel0.attachToComponent(&feedback0, false);
    feedbackLabel0.setVisible(false);

    addAndMakeVisible(feedback1);
    feedback1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    feedback1.setLookAndFeel(applayout);
    feedback1.setRange(0, 1, 0.01);
    feedback1.onValueChange = [this] { feedbackChanged(); };
    feedback1.setVisible(false);
    feedback1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    feedbackLabel1.setText("feedback", juce::dontSendNotification);
    feedbackLabel1.attachToComponent(&feedback1, false);
    feedbackLabel1.setVisible(false);

    addAndMakeVisible(LFO_rate0);
    LFO_rate0.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    LFO_rate0.setLookAndFeel(applayout);
    LFO_rate0.setRange(0, 5, 0.5);
    LFO_rate0.onValueChange = [this] { LFO_rateChanged(); };
    LFO_rate0.setVisible(false);
    LFO_rate0.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    LFO_rate0.setTextValueSuffix("Hz");
    LFO_rateLabel0.setText("LFO", juce::dontSendNotification);
    LFO_rateLabel0.attachToComponent(&LFO_rate0, false);
    feedbackLabel0.setVisible(false);

    addAndMakeVisible(LFO_rate1);
    LFO_rate1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    LFO_rate1.setLookAndFeel(applayout);
    LFO_rate1.setRange(0, 5, 0.5);
    LFO_rate1.onValueChange = [this] { LFO_rateChanged(); };
    LFO_rate1.setVisible(false);
    LFO_rate1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 100, 25);
    LFO_rate1.setTextValueSuffix("Hz");
    LFO_rateLabel1.setText("LFO", juce::dontSendNotification);
    LFO_rateLabel1.attachToComponent(&LFO_rate1, false);
    feedbackLabel1.setVisible(false);


    //********************** UDP Socket communication setup ******************************//

    auto info = startSocketUDPSend();
    UDP_socket_descriptor = std::get<0>(info);
    UDP_socket_address = std::get<1>(info);
    
    //************************************************************************************//


    if (RuntimePermissions::isRequired(RuntimePermissions::recordAudio)
        && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request(RuntimePermissions::recordAudio,
            [&](bool granted) { if (granted)  setAudioChannels(2, 2); });
    }
    else
    {
        // Channel configuration for internal mic and BT, just sets max nr. of used input and output channels

        setAudioChannelsPrimaryDevice(2, 2);
        setAudioChannelsSecondaryDevice(2, 2);

        
    }

    showBeginScreen();

}

MainComponent::~MainComponent()
{
    shutdownAudioForAllDevices();
    delete(applayout);
}

//**************************************************************************************************//
/********************** GUI initialization methods per screen/tab ***********************************/
//**************************************************************************************************//

void MainComponent::showBeginScreen()
{
    deviceOptions.setVisible(true);
    DSPModule.setVisible(true);
    networkSettings.setVisible(true);

}

void MainComponent::FlangerWindow() {

    audioSetupComp.setVisible(false);
    audioSetupComp2.setVisible(false);

    upButton0.setVisible(false);
    downButton0.setVisible(false);
    upButton1.setVisible(false);
    downButton1.setVisible(false);
    
    pitchLevel0.setVisible(false);
    pitchLevel1.setVisible(false);

    depth0.setVisible(true);
    feedback0.setVisible(true);
    LFO_rate0.setVisible(true);

    depth1.setVisible(true);
    feedback1.setVisible(true);
    LFO_rate1.setVisible(true);
}

void MainComponent::pitchShift() {

    depth0.setVisible(false);
    feedback0.setVisible(false);
    LFO_rate0.setVisible(false);

    depth1.setVisible(false);
    feedback1.setVisible(false);
    LFO_rate1.setVisible(false);

    audioSetupComp.setVisible(false);
    audioSetupComp2.setVisible(false);

    upButton0.setVisible(true);
    downButton0.setVisible(true);
    upButton1.setVisible(true);
    downButton1.setVisible(true);

    pitchLevel0.setVisible(true);
    pitchLevel1.setVisible(true);
}


//**************************************************************************************************//
/********************** GUI callback methods per component (slider, button, ..)**********************/
//**************************************************************************************************//


/****************************************  Tab switching buttons  ******************************************/

void MainComponent::deviceOptionsClicked()
{
    upButton0.setVisible(false);
    downButton0.setVisible(false);
    upButton1.setVisible(false);
    downButton1.setVisible(false);

    pitchLevel0.setVisible(false);
    pitchLevel1.setVisible(false);

    depth0.setVisible(false);
    feedback0.setVisible(false);
    LFO_rate0.setVisible(false);

    depth1.setVisible(false);
    feedback1.setVisible(false);
    LFO_rate1.setVisible(false);

    dropdown.setVisible(false);
    textLabel.setVisible(false);
    deviceLevel.setVisible(false);

    TDE_Button.setVisible(false);
    timeDifference.setVisible(false);

    if (deviceOptions.getToggleState())
    {
        device0Parameters.setVisible(true);
        device1Parameters.setVisible(true);

        audioSetupComp.setVisible(true);
        audioSetupComp2.setVisible(true);
    }

    else
    {
        device0Parameters.setVisible(false);
        device1Parameters.setVisible(false);

        audioSetupComp.setVisible(false);
        audioSetupComp2.setVisible(false);
    }
}

void MainComponent::DSPModuleClicked()
{
    dropdown.setSelectedId(1);

    upButton0.setVisible(false);
    downButton0.setVisible(false);
    upButton1.setVisible(false);
    downButton1.setVisible(false);

    pitchLevel0.setVisible(false);
    pitchLevel1.setVisible(false);
    depth0.setVisible(false);
    feedback0.setVisible(false);
    LFO_rate0.setVisible(false);

    depth1.setVisible(false);
    feedback1.setVisible(false);
    LFO_rate1.setVisible(false);

    audioSetupComp.setVisible(false);
    audioSetupComp2.setVisible(false);

    device0Parameters.setVisible(true);
    device1Parameters.setVisible(true);

    timeDifference.setVisible(false);

    if (DSPModule.getToggleState())
    {
        device0Parameters.setVisible(true);
        device1Parameters.setVisible(true);
        TDE_Button.setVisible(true);

        dropdown.setVisible(true);
        textLabel.setVisible(true);
        deviceLevel.setVisible(true);

    }

    else
    {
        device0Parameters.setVisible(false);
        device1Parameters.setVisible(false);
        TDE_Button.setVisible(false);

        dropdown.setVisible(false);
        textLabel.setVisible(false);
        deviceLevel.setVisible(false);
    }
}

void MainComponent::networkOptionsClicked()
{
    // not implemented
}


/****************************************  Effect selection  ******************************************/

void MainComponent::dropdownChanged()
{
    switch (dropdown.getSelectedId())
    {
    case 1:  pitchShift();  break;
    case 2:  FlangerWindow();  break;
    }
}

/****************************************  Pitch shifting  ******************************************/

void MainComponent::upButton0Clicked()
{
    pitchshifter0.setUp();
}

void MainComponent::downButton0Clicked()
{
    pitchshifter0.setDown();
}

void MainComponent::upButton1Clicked()
{
    pitchshifter1.setUp();
}

void MainComponent::downButton1Clicked()
{
    pitchshifter1.setDown();
}

void MainComponent::pitchLevelChanged()
{
    pitchshifter0.setLevel(pitchLevel0.getValue());
    pitchshifter1.setLevel(pitchLevel1.getValue());
}


/****************************************  Flanger  **************************************************/

void MainComponent::depthChanged()
{
    flanger0.setDepth(depth0.getValue());
    flanger1.setDepth(depth1.getValue());
}

void MainComponent::feedbackChanged()
{
    flanger0.setFeedback(feedback0.getValue());
    flanger1.setFeedback(feedback1.getValue());
}

void MainComponent::LFO_rateChanged()
{
    flanger0.setLFO(LFO_rate0.getValue());
    flanger1.setLFO(LFO_rate1.getValue());
}


/****************************************  Manual mixing  ********************************************/

void MainComponent::deviceLevelChanged()
{
    levelPerDevice = deviceLevel.getValue();
}



//**************************************************************************************************//
/********************** Time Delay estimation code **************************************************/
//**************************************************************************************************//


void MainComponent::fillTDEbuffer(int device_id, int bufferLength, int channel, const float* bufferData)
{
    if (device_id == 0)
    {
        if (TDECircularBuffer.getNumSamples() > bufferLength + TDEWritePosition0)
        {
            TDECircularBuffer.copyFrom(device_id, TDEWritePosition0, bufferData, bufferLength);
        }
        else
        {
            const int TDEBufferRemaining = TDECircularBuffer.getNumSamples() - TDEWritePosition0;
            TDECircularBuffer.copyFrom(device_id, TDEWritePosition0, bufferData, TDEBufferRemaining);
            TDECircularBuffer.copyFrom(device_id, 0, bufferData + TDEBufferRemaining, bufferLength - TDEBufferRemaining);

        }
    }

    if (device_id == 1)
    {
        if (TDECircularBuffer.getNumSamples() > bufferLength + TDEWritePosition1)
        {
            TDECircularBuffer.copyFrom(device_id, TDEWritePosition1, bufferData, bufferLength);
        }
        else
        {
            const int TDEBufferRemaining = TDECircularBuffer.getNumSamples() - TDEWritePosition1;
            TDECircularBuffer.copyFrom(device_id, TDEWritePosition1, bufferData, TDEBufferRemaining);
            TDECircularBuffer.copyFrom(device_id, 0, bufferData + TDEBufferRemaining, bufferLength - TDEBufferRemaining);

        }
    }
}

void MainComponent::adjustTDEbuffer(int device_id, int numSamplesInBuffer)
{
    if (device_id == 0)
    {
        TDEWritePosition0 += numSamplesInBuffer;
        TDEWritePosition0 %= TDECircularBuffer.getNumSamples();
    }

    if (device_id == 1)
    {
        TDEWritePosition1 += numSamplesInBuffer;
        TDEWritePosition1 %= TDECircularBuffer.getNumSamples();
    }
}

void MainComponent::TimeDelayEstimation()
{
    performingTDE = true;

    const float* deviceBuffer0 = TDECircularBuffer.getReadPointer(0);
    const float* deviceBuffer1 = TDECircularBuffer.getReadPointer(1);

    double linear_x_in[48000], linear_y_in[48000];
   
    for (int k = 0; k < 48000; k++) {  // unwrap circular into linear buffers

            linear_x_in[k] = deviceBuffer0[(k + TDEWritePosition0) % 48000];
            linear_y_in[k] = deviceBuffer1[(k+ TDEWritePosition1) % 48000];
        
        
    }

    int delay = alignsigs(linear_x_in, linear_y_in);
    
    timeDifference.setText(std::to_string(delay) + " samples", juce::dontSendNotification);
    timeDifference.setVisible(true);
   
    performingTDE = false;

    
}


//**************************************************************************************************//
/********************** Main visualization methods **************************************************/
//**************************************************************************************************//

// This method is called in the beginning of the program to define the size and relative positions of the UI components 

void MainComponent::resized()
{
    int windowWidth = getWidth();
    int windowHeight = getHeight();

    textLabel.setBounds(10, 10, windowWidth - 20, 20);
    dropdown.setBounds(10, 40, windowWidth /5, 20);

    upButton0.setBounds(0.1 * windowWidth + 10, 0.2 * windowHeight + 40, 100, 20);
    downButton0.setBounds (0.1 * windowWidth + 10, 0.2 * windowHeight + 80, 100, 20);

    upButton1.setBounds(0.55 * windowWidth + 10, 0.2 * windowHeight + 40, 100, 20);
    downButton1.setBounds(0.55 * windowWidth + 10, 0.2 * windowHeight + 80, 100, 20);

    device0Parameters.setBounds(0.1*windowWidth, 0.2 * windowHeight, 0.35*windowWidth, 0.5 * windowHeight);
    device1Parameters.setBounds(0.55*windowWidth, 0.2 * windowHeight, 0.35*windowWidth, 0.5 * windowHeight);

    TDE_Button.setBounds(0.75 * windowWidth, 0.07 * windowHeight, 0.15 * windowWidth, 0.07 * windowHeight);
    timeDifference.setBounds(0.79 * windowWidth, 0.1 * windowHeight, 0.1 * windowWidth, 0.05 * windowHeight);

    pitchLevel0.setBounds(0.15*windowWidth, 0.35*windowHeight, 0.25 *windowWidth, 0.25 * windowHeight);
    pitchLevel1.setBounds(0.6*windowWidth, 0.35*windowHeight, 0.25 * windowWidth, 0.25 * windowHeight);

    depth0.setBounds(0.18 * windowWidth, 0.25 * windowHeight, 0.2 * windowWidth, 0.2 * windowHeight);
    feedback0.setBounds(0.12 * windowWidth, 0.5 * windowHeight, 0.2 * windowWidth, 0.2 * windowHeight);
    LFO_rate0.setBounds(0.27 * windowWidth, 0.5 * windowHeight, 0.2 * windowWidth, 0.2 * windowHeight);

    depth1.setBounds(0.63 * windowWidth, 0.25 * windowHeight, 0.2 * windowWidth, 0.2 * windowHeight);
    feedback1.setBounds(0.57 * windowWidth, 0.5 * windowHeight, 0.2 * windowWidth, 0.2 * windowHeight);
    LFO_rate1.setBounds(0.72 * windowWidth, 0.5 * windowHeight, 0.2 * windowWidth, 0.2 * windowHeight);

    deviceOptions.setBounds(0.1*windowWidth, 0.8*windowHeight, 0.2*windowWidth, 40);
    DSPModule.setBounds(0.4* windowWidth, 0.8 * windowHeight, 0.2*windowWidth, 40);
    networkSettings.setBounds(0.7 * windowWidth, 0.8 * windowHeight, 0.2 * windowWidth, 40);

    audioSetupComp.setBounds(0.1*windowWidth, 0.25*windowHeight, 0.35*windowWidth, 0.6*windowHeight);
    audioSetupComp2.setBounds(0.55 * windowWidth, 0.25* windowHeight, 0.35 * windowWidth, 0.6 * windowHeight);

    deviceLevel.setBounds(0.33 * windowWidth, 0.05 * windowHeight, 0.35 * windowWidth, 0.1 * windowHeight);
}


// Loads the application icon to display in the background screen

void MainComponent::paint(Graphics& g) 
{
    Image background = ImageCache::getFromMemory(BinaryData::StartScreen_image_png, BinaryData::StartScreen_image_pngSize);
    g.drawImageAt(background, 0.43*getWidth(), 0.3*getHeight());
}


//**************************************************************************************************//
/********************** Main DSP callback methods ***************************************************/
//**************************************************************************************************//


// This initializes the sample rates and samples per audio block expected.
// For this use case, it is important that the aggregated devices are configured to use the same sample rate in the device manager tab
// For example, the internal mic is configured to 48 kHz as device 0, then BT should also be configured to 48 kHz and use sam buffer size

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)   
{
     this->sampleRate = sampleRate;
    
     TDECircularBuffer.setSize(2, sampleRate);   // each device reserves one channel in the TDE buffer
    
     pitchshifter0.initialize(samplesPerBlockExpected, sampleRate);
     flanger0.initialize(samplesPerBlockExpected, sampleRate);

     pitchshifter1.initialize(samplesPerBlockExpected, sampleRate);
     flanger1.initialize(samplesPerBlockExpected, sampleRate);
}


// The main callback method of our application. This callback is added by each device manager to its corresponding device.
// To identify the device, the AudioSourceChannelInfo struct (part of JUCE modules) was extended by a member variable "device_id" 
// Since we use only two devices, a simple if statement is sufficiently neat to distinguish the calling device.
// However, in case of multiple devices, just use a data structure (e.g. std::map) that stores pairs of device_id and function pointers to the apt. device callback

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    
    int bufferLength = bufferToFill.buffer->getNumSamples();

   
    //******************************************* Primary device - internal microphone**************************************************************************//
    
    if (bufferToFill.device_id == 0)    
    {
      
        for (int channel = 0; channel < 2; ++channel)
        {
            
            //---- Copy input buffer data to TDE circular buffer --------------------------------------------//

            const float* bufferData = bufferToFill.buffer->getReadPointer(channel);
            if (!performingTDE && channel == 0) fillTDEbuffer(0, bufferLength, channel, bufferData);


            //------Process input buffer data --------------------------------------------------------------//

            if (dropdown.getSelectedId() == 1) pitchshifter0.process(bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples, 300, channel, 1-levelPerDevice);
            else if (dropdown.getSelectedId() == 2) flanger0.process(bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples, 300, channel, 1-levelPerDevice);

            
            //------Send data through UDP ------------------------------------------------------------------//

            sendDataUDPJuce(UDP_socket_descriptor, UDP_socket_address, bufferData, 0, channel);
        }


        if (dropdown.getSelectedId() == 1) pitchshifter0.adjustDelayBufferWritePosition(bufferLength);
        else if (dropdown.getSelectedId() == 2)
        {
            flanger0.adjustDelayBufferWritePosition(bufferLength);
            flanger0.adjustFeedBackBufferWritePosition(bufferLength);
        }

        if (!performingTDE) adjustTDEbuffer(0, bufferLength);
               
    }

    //******************************************* Secondary device - BT headset ********************************************************************************//

    if (bufferToFill.device_id == 1)        
    {
               
        for (int channel = 0; channel < 1; ++channel)
        {

            //------ Copy input buffer data to TDE circular buffer -----------------------------------------//

            const float* bufferData = bufferToFill.buffer->getReadPointer(channel);
            if (!performingTDE) fillTDEbuffer(1, bufferLength, channel, bufferData);


            //------Process input buffer data --------------------------------------------------------------//

            if (dropdown.getSelectedId() == 1) pitchshifter1.process(bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples, 300, channel, levelPerDevice);
            else if (dropdown.getSelectedId() == 2) flanger1.process(bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples, 300, channel, levelPerDevice);

            
            //------Send data through UDP ------------------------------------------------------------------//

            sendDataUDPJuce(UDP_socket_descriptor, UDP_socket_address, bufferData, 1, channel);

        }

        bufferToFill.buffer->copyFrom(1, 0, bufferToFill.buffer->getReadPointer(0), bufferLength); // copy data from channel 0 to channel 1 of the buffer

        if (dropdown.getSelectedId() == 1) pitchshifter1.adjustDelayBufferWritePosition(bufferLength);
        else if (dropdown.getSelectedId() == 2)
        {
            flanger1.adjustDelayBufferWritePosition(bufferLength);
            flanger1.adjustFeedBackBufferWritePosition(bufferLength);
        }

        if (!performingTDE) adjustTDEbuffer(1, bufferLength);
    }
    
   
}

void MainComponent::releaseResources()
{
    //
}

//==============================================================================