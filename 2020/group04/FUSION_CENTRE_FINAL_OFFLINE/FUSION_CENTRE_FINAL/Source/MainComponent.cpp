#include "MainComponent.h"
#include "aggregator_config.h"

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }


//==============================================================================
MainComponent::MainComponent()
{
    remove("Logfile.txt"); // Delete file if already exists
    plog::init(plog::debug, "Logfile.txt");
    init_juceGUIcomponents();
    init_asioDriver();
    // Cannot change default value sliders -> init buffer switch not yet called
    startBroadcasting.onClick = [this]() { buttonHandler(); };

    devicelist1_combobox.onChange = [this]() {  comboboxHandler(&devicelist1_combobox); };
    devicelist2_combobox.onChange = [this]() {  comboboxHandler(&devicelist2_combobox); };
    soundeffect_combobox.onChange = [this]() {  comboboxHandler(&soundeffect_combobox); };
    setSize(600, 400);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::Colour(61, 64, 81));
}

// only happens when a value actually changed!
void MainComponent::comboboxHandler(juce::ComboBox* box)
{
    if (box == &devicelist1_combobox) {
        for (int i = 1; i <= devicelist2_combobox.getNumItems(); i++) {
            devicelist2_combobox.setItemEnabled(i, true); //when device is changed, enable all of the id's
        }
        int id = box->getSelectedId();
       // devicelist2_combobox.setItemEnabled(id, false);
        //combobox starts at index 1, not 0. the visible order of combobox is also the desiredIndices order!
        desiredIndices[0] = id - 1;
        LOGD << "For device 1, we are using " << box->getText() << "it has a combobox id of" << id;
    }
    else if (box == &devicelist2_combobox) {

        for (int i = 1; i <= devicelist1_combobox.getNumItems(); i++) {
            devicelist1_combobox.setItemEnabled(i, true);
        }
        int id = box->getSelectedId();
        //devicelist1_combobox.setItemEnabled(id, false);
        desiredIndices[1] = id - 1;
        LOGD << "For device 2, we are using " << box->getText() << "it has a combobox id of" << id;
    }
    else if (box == &soundeffect_combobox) {
        int id = box->getSelectedId();
        if (id == 1) { //pitchshift
            
            slider_pitch_device1.setVisible(true);
            slider_pitch_device2.setVisible(true);
            compression_slider_attacktime.setVisible(false);
            compression_slider_releasetime.setVisible(false);
            compression_slider_treshold.setVisible(false);
            compression_slider_ratio.setVisible(false);

        }
        else if (id == 4) { // pitch shift en compression
            slider_pitch_device1.setVisible(true);
            slider_pitch_device2.setVisible(true);
            compression_slider_attacktime.setVisible(true);
            compression_slider_releasetime.setVisible(true);
            compression_slider_treshold.setVisible(true);
            compression_slider_ratio.setVisible(true);

        }
        else if (id == 3) { // compression
            slider_pitch_device1.setVisible(false);
            slider_pitch_device2.setVisible(false);
            compression_slider_attacktime.setVisible(true);
            compression_slider_releasetime.setVisible(true);
            compression_slider_treshold.setVisible(true);
            compression_slider_ratio.setVisible(true);

        }
        else {
            slider_pitch_device1.setVisible(false);
            slider_pitch_device2.setVisible(false);
            compression_slider_attacktime.setVisible(false);
            compression_slider_releasetime.setVisible(false);
            compression_slider_treshold.setVisible(false);
            compression_slider_ratio.setVisible(false);

        }
        setAudioEffect(id-1); // change id's 
    }
}


void MainComponent::buttonHandler() {
    if (playState == PlayState::Play)
    {

        saveAudioFile();
        (*asioDriver).stop();
        (*asioDriver).disposeBuffers();
        cleanBufferSwitchCallBack();
        LOGD << "stopping the driver...";
        stop();
    }
    else
    {
        if ((devicelist1_combobox.getSelectedId() != 0) && (devicelist2_combobox.getSelectedId() != 0)) {
            start_asioDriver();

            LOGD << "indices of the devices used:" << desiredIndices.at(0) << " en " << desiredIndices.at(1);
            play();
        }
        else {
            LOGD << "not enough devices selected";
            devicelist1_combobox.setTextWhenNothingSelected("Please select a device");
            devicelist2_combobox.setTextWhenNothingSelected("Please select a device");
        }
    }
}



void MainComponent::sliderValueChanged(juce::Slider* slider) {
    float value = slider->getValue();

    if (slider == &slider_mix_device1) {
        setMix(0, value);
    }
    else if (slider == &slider_mix_device2) {
        setMix(1, value);
    }
    else if (slider == &slider_pitch_device1) {
        setPitch(0, value);
    }
    else if (slider == &slider_pitch_device2) {
       setPitch(1, value);
    }
    else if (slider == &compression_slider_treshold) {
        setCompressionValues(0, value);
    }
    else if (slider == &compression_slider_ratio) {
        setCompressionValues(1, value);
    }
    else if (slider == &compression_slider_attacktime) {
        setCompressionValues(2, value);
    }
    else if (slider == &compression_slider_releasetime) {
        setCompressionValues(3, value);
    }

}




void MainComponent::init_asioDriver() {
    asioDriver = new AsioDriver;
    asioResult = (*asioDriver).getAggregateDeviceManager(&aggregateDevice);

    if (aggregateDevice->getFriendlyNameInputDevices(&friendlyNames) != AGGDEVMAN_SUCCES)
    {
        LOGD << "failed to get friendly names";
    }
    for (int i = 0; i < friendlyNames.size(); i++)
    {
        devicelist1_combobox.addItem(friendlyNames[i], i + 1);
        devicelist2_combobox.addItem(friendlyNames[i], i + 1);

        LOGD << "just added " << friendlyNames[i] << "to the AllDevicelist";
    }
}

void MainComponent::start_asioDriver() {
    LOGD << "Lets start the ASIO driver for this many devices: " << desiredIndices.size();
    (*asioDriver).init(desiredIndices);
    LOGD << "The DesiredIndices vector has values : " << desiredIndices[0] << desiredIndices[1];

    // Check sample rate & channels of complete aggregate device.
    ASIOSampleRate sampleRate{ 0 };
    if ((*asioDriver).getSampleRate(&sampleRate) != ASE_OK) {
        LOGD << "Failed to retrieve sample rate from ASIO Aggregate Device.";

    }
    LOGD << "The sample rate of ASIO aggregate device is: " << sampleRate;

    long numInputChannels{ 0 }, numOutputChannels{ 0 };
    if ((*asioDriver).getChannels(&numInputChannels, &numOutputChannels) != ASE_OK) {
        LOGD << "Failed to retrieve number of channels from ASIO Aggregate Device.";
    }

    LOGD << "The number of input channels are: " << numInputChannels;

    initBufferSwitchCallback(numInputChannels, desiredIndices.size(), 32000);


    // Altijd sampleSetten
    (*asioDriver).setSampleRate(32000);
    asioResult = (*asioDriver).createBuffers(&bufferInfoVector, 0);
    (*asioDriver).start(&bufferSwitchCallback, &bufferInfoVector);

}

void MainComponent::resized()
{
    // dont touch the volgorde aub, very confusing.
    auto padzone = 20;
    auto area = getLocalBounds();
    auto dsp_area = area.removeFromRight(area.getWidth() / 2);
    auto soundsettings = dsp_area.removeFromTop(dsp_area.getHeight() / 2);
    auto agg_area = area.removeFromTop(area.getHeight() / 2);
    auto wifi_area = area;
    auto combobox_height = 60;

    aggregator_label.setBounds(agg_area.removeFromTop(40).reduced(10));
    DSP_label.setBounds(soundsettings.removeFromTop(40).reduced(10));
    slider_mix_device1.setBounds(soundsettings.removeFromLeft(soundsettings.getWidth() / 4).reduced(10));
    slider_mix_device2.setBounds(soundsettings.removeFromLeft(soundsettings.getWidth() / 3).reduced(10));
    slider_pitch_device1.setBounds(soundsettings.removeFromLeft(soundsettings.getWidth() / 2).reduced(10));
    slider_pitch_device2.setBounds(soundsettings.reduced(10));

    soundeffect_label.setBounds(dsp_area.removeFromTop(40).reduced(10));
    soundeffect_combobox.setBounds(dsp_area.removeFromTop(combobox_height).reduced(padzone));
    compression_slider_treshold.setBounds(dsp_area.removeFromLeft(dsp_area.getWidth() / 4).reduced(10));
    compression_slider_ratio.setBounds(dsp_area.removeFromLeft(dsp_area.getWidth() / 3).reduced(10));
    compression_slider_attacktime.setBounds(dsp_area.removeFromLeft(dsp_area.getWidth() / 2).reduced(10));
    compression_slider_releasetime.setBounds(dsp_area.reduced(10));

    devicelist1_combobox.setBounds(agg_area.removeFromTop(combobox_height).reduced(padzone));
    devicelist2_combobox.setBounds(agg_area.removeFromTop(combobox_height).reduced(padzone));

    startBroadcasting.setBounds(wifi_area.removeFromBottom(wifi_area.getHeight() / 2).reduced(padzone));
    enter_ip.setBounds(wifi_area.removeFromBottom(40).reduced(10));
}



void MainComponent::init_juceGUIcomponents() {
    auto darkgrey = juce::Colour::Colour(39, 43, 52);

    //aggregator device selection
    addAndMakeVisible(devicelist1_header);
    addAndMakeVisible(devicelist2_header);
    addAndMakeVisible(devicelist1_combobox);
    addAndMakeVisible(devicelist2_combobox);
    devicelist1_header.setText("Select device 1: ", juce::NotificationType::dontSendNotification);
    devicelist1_combobox.setColour(juce::ComboBox::outlineColourId, juce::Colours::aqua);
    devicelist1_header.attachToComponent(&devicelist1_combobox, false);
    devicelist2_header.setText("Select device 2", juce::NotificationType::dontSendNotification);
    devicelist2_header.attachToComponent(&devicelist2_combobox, false);
    devicelist2_combobox.setColour(juce::ComboBox::outlineColourId, juce::Colours::aqua);

    // Network
    addAndMakeVisible(enter_ip);
    enter_ip.setTextToShowWhenEmpty("xxx.xxx.xxx.xxx", juce::Colours::lightgrey);
    enter_ip.setJustification(juce::Justification::centred);
    addAndMakeVisible(enter_ip_label);
    enter_ip_label.setText("Enter the server port:", juce::NotificationType::dontSendNotification);
    enter_ip_label.attachToComponent(&enter_ip, false);

    addAndMakeVisible(startBroadcasting);
    startBroadcasting.setToggleState(true, juce::NotificationType::dontSendNotification);
    startBroadcasting.setButtonText("Start Broadcasting");

    //Code for aggregator label
    addAndMakeVisible(aggregator_label);
    aggregator_label.setText("Available input devices", juce::NotificationType::dontSendNotification);
    aggregator_label.setJustificationType(juce::Justification::horizontallyCentred);
    aggregator_label.setColour(juce::Label::backgroundColourId, darkgrey);

    // Sound properties label
    addAndMakeVisible(DSP_label);
    DSP_label.setText("Sound Properties", juce::NotificationType::dontSendNotification);
    DSP_label.setJustificationType(juce::Justification::horizontallyCentred);
    DSP_label.setColour(juce::Label::backgroundColourId, darkgrey);

    // manual mixing slider device 1
    addAndMakeVisible(slider_mix_device1);
    slider_mix_device1.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider_mix_device1.setRange(0, 200, 5);
    slider_mix_device1.setValue(100, juce::NotificationType::dontSendNotification);
    slider_mix_device1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    slider_mix_device1.setTextValueSuffix("%");
    slider_mix_device1.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    slider_mix_device1.addListener(this);
    addAndMakeVisible(slider_mix_device1_label);
    slider_mix_device1_label.setText("Volume device 1:", juce::NotificationType::dontSendNotification);
    slider_mix_device1_label.attachToComponent(&slider_mix_device1, false);

    // manual mixing slider device 2
    addAndMakeVisible(slider_mix_device2);
    slider_mix_device2.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider_mix_device2.setRange(0, 200, 5);
    slider_mix_device2.setValue(100, juce::NotificationType::dontSendNotification);
    slider_mix_device2.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    slider_mix_device2.setTextValueSuffix("%");
    slider_mix_device2.addListener(this);
    slider_mix_device2.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    addAndMakeVisible(slider_mix_device2_label);
    slider_mix_device2_label.setText("Volume device 2:", juce::NotificationType::dontSendNotification);
    slider_mix_device2_label.attachToComponent(&slider_mix_device2, false);

    //Pitch shifting slider device 1
    addAndMakeVisible(slider_pitch_device1);
    slider_pitch_device1.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider_pitch_device1.setRange(0.5, 2, 0.05);
    slider_pitch_device1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    
    slider_pitch_device1.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    slider_pitch_device1.addListener(this);
    addAndMakeVisible(slider_pitch_device1_label);
    slider_pitch_device1_label.setText("¨Pitch device 1:", juce::NotificationType::dontSendNotification);
    slider_pitch_device1_label.attachToComponent(&slider_pitch_device1, false);
    slider_pitch_device1.setVisible(false);

    //Pitch shifting slider device 2
    addAndMakeVisible(slider_pitch_device2);
    slider_pitch_device2.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider_pitch_device2.setRange(0.5, 2, 0.05);
    slider_pitch_device2.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    slider_pitch_device2.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    slider_pitch_device2.addListener(this);
    addAndMakeVisible(slider_pitch_device2_label);
    slider_pitch_device2_label.setText("Pitch device 2:", juce::NotificationType::dontSendNotification);
    slider_pitch_device2_label.attachToComponent(&slider_pitch_device2, false);
    slider_pitch_device2.setVisible(false);

    //Code for sound effect label
    addAndMakeVisible(soundeffect_label);
    soundeffect_label.setText("Sound Effects", juce::NotificationType::dontSendNotification);
    soundeffect_label.setJustificationType(juce::Justification::horizontallyCentred);
    soundeffect_label.setColour(juce::Label::backgroundColourId, darkgrey);

    // code for sound effect combobox
    addAndMakeVisible(soundeffect_combobox_label);
    addAndMakeVisible(soundeffect_combobox);
    soundeffect_combobox_label.setText("Select a sound effect: ", juce::NotificationType::dontSendNotification);
    soundeffect_combobox.setColour(juce::ComboBox::outlineColourId, juce::Colours::aqua);
    soundeffect_combobox_label.attachToComponent(&devicelist1_combobox, false);
    soundeffect_combobox.setColour(juce::ComboBox::outlineColourId, juce::Colours::aqua);
    soundeffect_combobox_label.attachToComponent(&soundeffect_combobox, false);
    soundeffect_combobox.addItem("Pitch shift", 1);
    soundeffect_combobox.addItem("Autotune", 2);
    soundeffect_combobox.addItem("Compression", 3);
    soundeffect_combobox.addItem("Pitchshift and Compression", 4);


    //==============================================================

    // compression treshold slider
    addAndMakeVisible(compression_slider_treshold);
    compression_slider_treshold.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    compression_slider_treshold.setRange(-60, 20, 2);
    compression_slider_treshold.setValue(50, juce::NotificationType::dontSendNotification);
    compression_slider_treshold.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    compression_slider_treshold.setTextValueSuffix(" decibel");
    compression_slider_treshold.addListener(this);
    compression_slider_treshold.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    addAndMakeVisible(compression_slider_treshold_label);
    compression_slider_treshold_label.setText("treshold value:", juce::NotificationType::dontSendNotification);
    compression_slider_treshold_label.attachToComponent(&compression_slider_treshold, false);
    compression_slider_treshold.setVisible(false);

    // compression ratio slider
    // 2 5 10 20 50 100
    addAndMakeVisible(compression_slider_ratio);
    compression_slider_ratio.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    compression_slider_ratio.setRange(1, 6, 1);
    compression_slider_ratio.setValue(1, juce::NotificationType::dontSendNotification);
    compression_slider_ratio.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    compression_slider_ratio.setTextValueSuffix("/1");
    compression_slider_ratio.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    compression_slider_ratio.addListener(this);
    addAndMakeVisible(compression_slider_ratio_label);
    compression_slider_ratio_label.setText("compression:", juce::NotificationType::dontSendNotification);
    compression_slider_ratio_label.attachToComponent(&compression_slider_ratio, false);
    compression_slider_ratio.setVisible(false);

    // compression attacktime slider
    addAndMakeVisible(compression_slider_attacktime);
    compression_slider_attacktime.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    compression_slider_attacktime.setRange(0, 20, 1);
    compression_slider_attacktime.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 75, 25);
    compression_slider_attacktime.setTextValueSuffix("ms");
    compression_slider_attacktime.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    compression_slider_attacktime.addListener(this);
    addAndMakeVisible(compression_slider_attacktime_label);
    compression_slider_attacktime_label.setText("attacktime:", juce::NotificationType::dontSendNotification);
    compression_slider_attacktime_label.attachToComponent(&compression_slider_attacktime, false);
    compression_slider_attacktime.setVisible(false);

    // compression releasetime slider
    addAndMakeVisible(compression_slider_releasetime);
    compression_slider_releasetime.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    compression_slider_releasetime.setRange(0, 20, 1);
    compression_slider_releasetime.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 75, 25);
    compression_slider_releasetime.setTextValueSuffix("ms");
    compression_slider_releasetime.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aqua);
    compression_slider_releasetime.addListener(this);
    addAndMakeVisible(compression_slider_releasetime_label);
    compression_slider_releasetime_label.setText("releasetime:", juce::NotificationType::dontSendNotification);
    compression_slider_releasetime_label.attachToComponent(&compression_slider_releasetime, false);
    compression_slider_releasetime.setVisible(false);
}

void MainComponent::play()
{
    playState = PlayState::Play;
    startBroadcasting.setToggleState(false, juce::NotificationType::dontSendNotification);
    startBroadcasting.setButtonText("we're live");
    startBroadcasting.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
}

void MainComponent::stop()
{
    playState = PlayState::Stop;
    startBroadcasting.setToggleState(true, juce::NotificationType::dontSendNotification);
    startBroadcasting.setButtonText("Start Broadcasting");
    startBroadcasting.setColour(juce::TextButton::buttonColourId, juce::Colours::mediumaquamarine);
}
