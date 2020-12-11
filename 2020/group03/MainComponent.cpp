#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() 
    : audioSetupComp(deviceManager,     // AudioAppComponent deviceManager
                        0,              // minimum input channels
                        4,              // maximum input channels
                        0,              // minimum output channels
                        256,            // maximum output channels
                        false,          // ability to select midi inputs
                        false,          // ability to select midi output device
                        false,          // treat channels as stereo pairs
                        false),          // hide advanced options)
    recording1(false), writeThread1("write thread"),
    recording2(false), writeThread2("write thread")
{
    // Add Audio Setup Component
    audioSetupComp.setBounds(0, 0, 800, 300);
    addAndMakeVisible(audioSetupComp);

    // Mute/Play toggle button
    readyToggle.setButtonText("Play!");
    readyToggle.setBounds(100, 300, 200, 30);
    addAndMakeVisible(readyToggle);

    // Configuration check text
    readyLabel.setBounds(300, 300, 200, 30);
    addAndMakeVisible(readyLabel);

    // Input attenuation sliders settings
    attSlider1.setRange(0.0f, 1.0f);
    attSlider2.setRange(0.0f, 1.0f);
    attSlider1.setValue(1.0f);
    attSlider2.setValue(1.0f);
    attSlider1.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    attSlider2.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    attSlider1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    attSlider2.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    attSlider1.setBounds(50, 400, 80, 80);
    attSlider2.setBounds(650, 400, 80, 80);
    attLabel1.setText("Input 1",juce::NotificationType::dontSendNotification);
    attLabel1.attachToComponent(&attSlider1,false);
    attLabel2.setText("Input 2",juce::NotificationType::dontSendNotification);
    attLabel2.attachToComponent(&attSlider2, false);
    addAndMakeVisible(attSlider1);
    addAndMakeVisible(attSlider2);
    
    // Mix slider settings
    mixSlider.setRange(0.0f, 1.0f);
    mixSlider.setBounds(200, 400, 400, 50);
    mixSlider.setValue(0.5f, juce::NotificationType::dontSendNotification);
    mixLabel.setText("Microphone Mix", juce::NotificationType::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider, false);
    addAndMakeVisible(mixSlider);

    // Pitch slider settings
    pitchSlider.setRange(0.1f, 5.0f);
    pitchSlider.setBounds(200, 500, 400, 50);
    pitchSlider.setValue(1.0f, juce::NotificationType::dontSendNotification);
    pitchLabel.setText("Pitch Shift", juce::NotificationType::dontSendNotification);
    pitchLabel.attachToComponent(&pitchSlider, false);
    addAndMakeVisible(pitchSlider);

    pitchToggle.setBounds(10, 500, 100, 50);
    pitchToggle.setButtonText("Enable");
    addAndMakeVisible(pitchToggle);

    // Audio FX slider settings
    fxSlider.setRange(0.0f, 10.0f); // TODO set range
    fxSlider.setBounds(50, 600, 80, 80);
    fxSlider.setValue(5.0f); // TODO set default value
    fxLevelLabel.setText("Audio FX", juce::NotificationType::dontSendNotification);
    fxLevelLabel.attachToComponent(&fxSlider, true);
    addAndMakeVisible(fxSlider);
    
    // Audio FX toggle
    fxToggle.setButtonText("Audio FX");
    fxToggle.setBounds(200, 600, 400, 50);
    addAndMakeVisible(fxToggle);

    // Output attenuation slider
    outputSlider.setRange(0.0f, 1.0f);
    outputSlider.setValue(1.0f);
    outputSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    outputSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    outputSlider.setBounds(360, 700, 80, 80);
    outputLabel.setText("Output",juce::NotificationType::dontSendNotification);
    outputLabel.attachToComponent(&outputSlider, false);
    addAndMakeVisible(outputSlider);

    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 800);

    // Set Audio Channels
    setAudioChannels(4, 2);

    //// Files
    //wavFile = juce::File("C:\Export\test.wav");
    //outStream = &juce::FileOutputStream(wavFile);
    //writer = wavFormat.createWriterFor(outStream, 48000, (unsigned int)2, 16, NULL, 0);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    if (readyToggle.getToggleStateValue() == true) {
        writeThread1.startThread();
        stream1 = new juce::FileOutputStream(juce::File::getCurrentWorkingDirectory().getChildFile("Part1_at_Input.wav"));
        juce::WavAudioFormat format1;
        writer1 = format1.createWriterFor(stream1, sampleRate, 4, 24, juce::StringPairArray(), 0);
        threaded1 = new juce::AudioFormatWriter::ThreadedWriter(writer1, writeThread1, 16384);

        writeThread2.startThread();
        stream2 = new juce::FileOutputStream(juce::File::getCurrentWorkingDirectory().getChildFile("Part1_at_Output.wav"));
        juce::WavAudioFormat format2;
        writer2 = format2.createWriterFor(stream2, sampleRate, 4, 24, juce::StringPairArray(), 0);
        threaded2 = new juce::AudioFormatWriter::ThreadedWriter(writer2, writeThread2, 16384);
    }
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Audio device pointer
    auto* device = deviceManager.getCurrentAudioDevice();
    
    // If Play button isn't on, mute everything and return!
    if (readyToggle.getToggleState() == false) {
        bufferToFill.buffer->clear();
        recording1 = false;
        recording2 = false;
        return;
    }
    else {
        recording1 = true;
        recording2 = true;
    }

    // Active in-/output channels
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();

    // Maximum in-/output channels
    auto maxInputChannels = activeInputChannels.countNumberOfSetBits();
    auto maxOutputChannels = activeOutputChannels.countNumberOfSetBits();

    // Check in-/output channel configuration
    if (maxInputChannels != 4 || maxOutputChannels != 2) {
        readyLabel.setText("Wrong device configuration!", juce::NotificationType::dontSendNotification);
        return;
    }
    else {
        readyLabel.setText("Ready!", juce::NotificationType::dontSendNotification);
    }

    /***** INPUT RECORDING (In1/In2) *****/

    // Export to Input_part1.wav
    if (threaded1 && recording1) {
        threaded1->write(bufferToFill.buffer->getArrayOfReadPointers(), bufferToFill.numSamples);
    }

    /***** MICROPHONE INPUT ATTENUATION *****/

    // Input 1
    auto gain1 = (float)attSlider1.getValue();
    bufferToFill.buffer->applyGain(0, bufferToFill.startSample, bufferToFill.numSamples, gain1);
    bufferToFill.buffer->applyGain(1, bufferToFill.startSample, bufferToFill.numSamples, gain1);
    
    // Input 2
    auto gain2 = (float)attSlider2.getValue();
    bufferToFill.buffer->applyGain(2, bufferToFill.startSample, bufferToFill.numSamples, gain2);
    bufferToFill.buffer->applyGain(3, bufferToFill.startSample, bufferToFill.numSamples, gain2);


    /***** CHANNEL MIXER *****/

    // Retrieve current Mix slider setting
    auto mix = (float)mixSlider.getValue(); // A float value between 0.0f (Input 1) and 1.0f (Input 2)

    // (1) Apply a weight of (1.0f - mix) to the first input buffer pair (Input 1)
    bufferToFill.buffer->applyGain(0, bufferToFill.startSample, bufferToFill.numSamples, (1.0f - mix));
    bufferToFill.buffer->applyGain(1, bufferToFill.startSample, bufferToFill.numSamples, (1.0f - mix));

    // (2) Add the samples from the second input buffer pair (Input 2) to the first buffer pair with a weight of (mix)
    bufferToFill.buffer->addFrom(0, bufferToFill.startSample, *(bufferToFill.buffer), 2, bufferToFill.startSample, bufferToFill.numSamples, mix);
    bufferToFill.buffer->addFrom(1, bufferToFill.startSample, *(bufferToFill.buffer), 3, bufferToFill.startSample, bufferToFill.numSamples, mix);
    
    // (3) Clear all but the buffers for the output
    //bufferToFill.buffer->clear(2, bufferToFill.startSample, bufferToFill.numSamples);
    //bufferToFill.buffer->clear(3, bufferToFill.startSample, bufferToFill.numSamples);

    /***** PITCH SHIFTER *****/

    //Only perform pitch shifting if toggle is on
    if (pitchToggle.getToggleState() == true) {

        // Perform pitch shifting on Left Channel
        pitchCoreLeft.smbPitchShift(pitchSlider.getValue(), bufferToFill.buffer->getNumSamples(), 1024, 32, device->getCurrentSampleRate(), bufferToFill.buffer->getWritePointer(0), bufferToFill.buffer->getWritePointer(0));

        // Perform pitch shifting on Right Channel
        pitchCoreRight.smbPitchShift(pitchSlider.getValue(), bufferToFill.buffer->getNumSamples(), 1024, 32, device->getCurrentSampleRate(), bufferToFill.buffer->getWritePointer(1), bufferToFill.buffer->getWritePointer(1));

    }
    else { // Reset pitch shift object, improvement possible by changing to ClickListener
        pitchCoreLeft.reset();
        pitchCoreRight.reset();
    }

    /***** AUDIO FX *****/

    // Only perform FX when toggle is on
    if (fxToggle.getToggleState() == true) {
        // Perform Audio FX processing on Left Channel
        fxCore.performFX(bufferToFill.buffer->getWritePointer(0), bufferToFill.numSamples, bufferToFill.buffer->getWritePointer(0), 0);

        // Perform Audio FX processing on Right Channel
        fxCore.performFX(bufferToFill.buffer->getWritePointer(1), bufferToFill.numSamples, bufferToFill.buffer->getWritePointer(1), 1);

        // Perform level compensation on both channels
        auto fxCompGain = (float)fxSlider.getValue();
        bufferToFill.buffer->applyGain(0, bufferToFill.startSample, bufferToFill.numSamples, fxCompGain);
        bufferToFill.buffer->applyGain(1, bufferToFill.startSample, bufferToFill.numSamples, fxCompGain);

    }

    /***** OUTPUT ATTENUATION *****/

    auto outputGain = (float)outputSlider.getValue();
    bufferToFill.buffer->applyGain(0, bufferToFill.startSample, bufferToFill.numSamples, outputGain);
    bufferToFill.buffer->applyGain(1, bufferToFill.startSample, bufferToFill.numSamples, outputGain);

    /***** OUTPUT RECORDING (Out1) *****/

    if (threaded2 && recording2) {
        threaded2->write(bufferToFill.buffer->getArrayOfReadPointers(), bufferToFill.numSamples);
    }

}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()

    recording1 = false;
    writeThread1.stopThread(1000);
    threaded1 = nullptr;
    writer1 = nullptr;

    recording2 = false;
    writeThread2.stopThread(1000);
    threaded2 = nullptr;
    writer2 = nullptr;

}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.    
}