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
        fft_j(fftOrder), delayBuffer(4, 16384),
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
    attSlider1.setDoubleClickReturnValue(true, 1.0f);
    attSlider2.setDoubleClickReturnValue(true, 1.0f);
    attSlider1.setBounds(160, 400, 80, 80);
    attSlider2.setBounds(560, 400, 80, 80);
    attLabel1.setText("Input 1",juce::NotificationType::dontSendNotification);
    attLabel1.attachToComponent(&attSlider1,false);
    attLabel2.setText("Input 2",juce::NotificationType::dontSendNotification);
    attLabel2.attachToComponent(&attSlider2, false);
    addAndMakeVisible(attSlider1);
    addAndMakeVisible(attSlider2);

    // Time alignment
    delayLabel.setBounds(300, 400, 200, 25);
    addAndMakeVisible(delayLabel);
    delayToggle.setBounds(325, 375, 150, 25);
    delayToggle.setButtonText("Latency compensation");
    addAndMakeVisible(delayToggle);
    delayBacklogSize.setSliderStyle(juce::Slider::SliderStyle::IncDecButtons);
    delayBacklogSize.setRange(1, 1000, 1);
    delayBacklogSize.setValue(20);
    delayBacklogSize.setBounds(350, 425, 100, 50);
    addAndMakeVisible(delayBacklogSize);
    delayTresholdLevel.setRange(0.0f, 1.0f);
    delayTresholdLevel.setBounds(500, 700, 150, 50);
    delayTresholdLevel.setValue(1.0f, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(delayTresholdLevel);
    
    // Pitch slider settings
    pitchSlider1.setRange(0.1f, 5.0f);
    pitchSlider1.setSkewFactorFromMidPoint(1.0f);
    pitchSlider1.setDoubleClickReturnValue(true, 1.0f);
    pitchSlider1.setBounds(10, 500, 380, 50);
    pitchSlider1.setValue(1.0f, juce::NotificationType::dontSendNotification);
    pitchLabel1.setText("Pitch Shift", juce::NotificationType::dontSendNotification);
    pitchLabel1.attachToComponent(&pitchSlider1, false);
    addAndMakeVisible(pitchSlider1);

    pitchSlider2.setRange(0.1f, 5.0f);
    pitchSlider2.setSkewFactorFromMidPoint(1.0f);
    pitchSlider2.setDoubleClickReturnValue(true, 1.0f);
    pitchSlider2.setBounds(410, 500, 380, 50);
    pitchSlider2.setValue(1.0f, juce::NotificationType::dontSendNotification);
    pitchLabel2.setText("Pitch Shift", juce::NotificationType::dontSendNotification);
    pitchLabel2.attachToComponent(&pitchSlider2, false);
    addAndMakeVisible(pitchSlider2);

    pitchToggle1.setBounds(10, 550, 100, 50);
    pitchToggle2.setBounds(690, 550, 100, 50);
    pitchToggle1.setButtonText("Enable");
    pitchToggle2.setButtonText("Enable");
    addAndMakeVisible(pitchToggle1);
    addAndMakeVisible(pitchToggle2);

    // Mix slider settings
    mixSlider.setRange(0.0f, 1.0f);
    mixSlider.setDoubleClickReturnValue(true, 0.5f);
    mixSlider.setBounds(200, 600, 400, 50);
    mixSlider.setValue(0.5f, juce::NotificationType::dontSendNotification);
    mixLabel.setText("Microphone Mix", juce::NotificationType::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider, false);
    addAndMakeVisible(mixSlider);

    // Audio FX slider settings
    fxSlider.setRange(0.0f, 10.0f);
    fxSlider.setDoubleClickReturnValue(true, 5.0f);
    fxSlider.setBounds(50, 700, 80, 80);
    fxSlider.setValue(5.0f); // TODO set default value
    fxLevelLabel.setText("Audio FX", juce::NotificationType::dontSendNotification);
    fxLevelLabel.attachToComponent(&fxSlider, true);
    addAndMakeVisible(fxSlider);
    
    // Audio FX toggle
    fxToggle.setButtonText("Audio FX");
    fxToggle.setBounds(200, 700, 100, 50);
    addAndMakeVisible(fxToggle);

    // Output attenuation slider
    outputSlider.setRange(0.0f, 1.0f);
    outputSlider.setValue(1.0f);
    outputSlider.setDoubleClickReturnValue(true, 1.0f);
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

    // For more details, see the help for AudioProcessor::prepareToPlay()

    if (readyToggle.getToggleState() == true) {
        writeThread1.startThread();
        stream1 = new juce::FileOutputStream(juce::File::getCurrentWorkingDirectory().getChildFile("Part2_at_Input.wav"));
        juce::WavAudioFormat format1;
        writer1 = format1.createWriterFor(stream1, sampleRate, 4, 24, juce::StringPairArray(), 0);
        threaded1 = new juce::AudioFormatWriter::ThreadedWriter(writer1, writeThread1, 16384);

        writeThread2.startThread();
        stream2 = new juce::FileOutputStream(juce::File::getCurrentWorkingDirectory().getChildFile("Part2_at_Output.wav"));
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

    // Export to Input_part2.wav
    if (threaded1 && recording1) {
        threaded1->write(bufferToFill.buffer->getArrayOfReadPointers(), bufferToFill.numSamples);
    }

    /***** CHANNEL BUFFERS *****/

    // Input 1 Left - Output Left
    auto* buffer0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    // Input 1 Right - Output Right
    auto* buffer1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    
    // Input 2 Left
    auto* buffer2 = bufferToFill.buffer->getWritePointer(2, bufferToFill.startSample);
    // Input 2 Right
    auto* buffer3 = bufferToFill.buffer->getWritePointer(3, bufferToFill.startSample);

    /***** MICROPHONE INPUT ATTENUATION *****/

    // Input 1
    auto gain1 = (float)attSlider1.getValue();
    bufferToFill.buffer->applyGain(0, bufferToFill.startSample, bufferToFill.numSamples, gain1);
    bufferToFill.buffer->applyGain(1, bufferToFill.startSample, bufferToFill.numSamples, gain1);
    
    // Input 2
    auto gain2 = (float)attSlider2.getValue();
    bufferToFill.buffer->applyGain(2, bufferToFill.startSample, bufferToFill.numSamples, gain2);
    bufferToFill.buffer->applyGain(3, bufferToFill.startSample, bufferToFill.numSamples, gain2);

    /***** SIGNAL ALIGNMENT *****/

    if (delayToggle.getToggleState() == true) {

        /*for (auto channel = 0; channel < 4; channel++) {
            delayBuffer.copyFrom(channel, delayIndex, *(bufferToFill.buffer), channel, bufferToFill.startSample, bufferToFill.numSamples);
        }*/

        for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {

            delayBuffer.setSample(0, wrap_index(delayIndex + sample, 16384), bufferToFill.buffer->getSample(0, sample));
            delayBuffer.setSample(1, wrap_index(delayIndex + sample, 16384), bufferToFill.buffer->getSample(1, sample));
            delayBuffer.setSample(2, wrap_index(delayIndex + sample, 16384), bufferToFill.buffer->getSample(2, sample));
            delayBuffer.setSample(3, wrap_index(delayIndex + sample, 16384), bufferToFill.buffer->getSample(3, sample));

        }

        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            input_buffer[0][wrap_index(input_index + sample)] = buffer0[sample];
            input_buffer[1][wrap_index(input_index + sample)] = buffer1[sample];
            input_buffer[2][wrap_index(input_index + sample)] = buffer2[sample];
            input_buffer[3][wrap_index(input_index + sample)] = buffer3[sample];
        }

        if (bufferToFill.buffer->getRMSLevel(0, bufferToFill.startSample, bufferToFill.numSamples) > 0.1*delayTresholdLevel.getValue()) {
            int delayBacklog = (int)delayBacklogSize.getValue();
            offset_history[offset_history_index] = gccphat(input_buffer[2], input_buffer[0], input_index);
            offset = get_most_occuring(offset_history, delayBacklog);
            if (offset < 2048) { // Input 2 arrives after Input1
                offset1 = offset;
                offset2 = 0;
            }
            else { // Input1 arrives after Input2
                offset1 = 0;
                offset2 = 4096 - offset;
            }
            offset_history_index = (offset_history_index + 1) % delayBacklog;
        }

        /*for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            buffer0[sample] = input_buffer[0][wrap_index(input_index + sample - offset2)];
            buffer1[sample] = input_buffer[1][wrap_index(input_index + sample - offset2)];
            buffer2[sample] = input_buffer[2][wrap_index(input_index + sample - offset1)];
            buffer3[sample] = input_buffer[3][wrap_index(input_index + sample - offset1)];
        }*/

        for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
          
            bufferToFill.buffer->setSample(0, sample, delayBuffer.getSample(0, wrap_index(delayIndex + sample - offset1, 16384)));
            bufferToFill.buffer->setSample(1, sample, delayBuffer.getSample(1, wrap_index(delayIndex + sample - offset1, 16384)));
            bufferToFill.buffer->setSample(2, sample, delayBuffer.getSample(2, wrap_index(delayIndex + sample - offset2, 16384)));
            bufferToFill.buffer->setSample(3, sample, delayBuffer.getSample(3, wrap_index(delayIndex + sample - offset2, 16384)));

        }
        
        delayIndex = (delayIndex + bufferToFill.numSamples) % 16384;
        input_index = (input_index + bufferToFill.numSamples) % fftSize;

        // Update GUI delay display
        if (offset1 == 0) {
            delayString = juce::String("Input 2 delayed by ");
            delayString.append(juce::String(offset2), 4);
        }
        else {
            delayString = juce::String("Input 1 delayed by ");
            delayString.append(juce::String(offset1), 4);
        }
        delayString.append(" samples", 8);
        delayLabel.setText(delayString, juce::NotificationType::dontSendNotification);
    }
    else { delayLabel.setText("", juce::NotificationType::dontSendNotification); }

    /***** PITCH SHIFTER *****/
    
    // Only perform pitch shifting if toggle is on
    if (pitchToggle1.getToggleState() == true) {
        // Perform pitch shifting on Input 1
        pitchCoreLeft1.smbPitchShift(pitchSlider1.getValue(), bufferToFill.buffer->getNumSamples(), 1024, 32, device->getCurrentSampleRate(), bufferToFill.buffer->getWritePointer(0), bufferToFill.buffer->getWritePointer(0));
        pitchCoreRight1.smbPitchShift(pitchSlider1.getValue(), bufferToFill.buffer->getNumSamples(), 1024, 32, device->getCurrentSampleRate(), bufferToFill.buffer->getWritePointer(1), bufferToFill.buffer->getWritePointer(1));
    }
    else { // Reset pitch shift object, improvement possible by changing to ClickListener
        pitchCoreLeft1.reset();
        pitchCoreRight1.reset();
    }
    
    // Only perform pitch shifting if toggle is on
    if (pitchToggle2.getToggleState() == true) {
        // Perform pitch shifting on Input 2
        pitchCoreLeft2.smbPitchShift(pitchSlider2.getValue(), bufferToFill.buffer->getNumSamples(), 1024, 32, device->getCurrentSampleRate(), bufferToFill.buffer->getWritePointer(2), bufferToFill.buffer->getWritePointer(2));
        pitchCoreRight2.smbPitchShift(pitchSlider2.getValue(), bufferToFill.buffer->getNumSamples(), 1024, 32, device->getCurrentSampleRate(), bufferToFill.buffer->getWritePointer(3), bufferToFill.buffer->getWritePointer(3));
    }
    else { // Reset pitch shift object, improvement possible by changing to ClickListener
        pitchCoreLeft2.reset();
        pitchCoreRight2.reset();
    }

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

    // Written to 'Output_Part2.wav'
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

int MainComponent::gccphat(float data[], float data2[], int index)
{
    // doesn't give 0 if exactyly the same waveform, only if there is a delay
    float max_value = -1;
    int max_index = -1;
    DBG("called gccphat method\n");
    for (auto i = 0; i < fftSize; i++)
    {
        fftin[i] = data[(index + i) % fftSize], 0;
    }

    fft_j.perform(fftin.data(), data_spec.data(), false);

    // move 10 samples right
    for (auto i = 1; i < fftSize; i++)
    {
        fftin[i] = data2[(index + i) % fftSize], 0;
    }
    fft_j.perform(fftin.data(), fftout.data(), false);
    // normalize
    /*
    for (auto& elem : fftout)
        elem /= fftSize;
    for (auto& elem : fftout2)
        elem /= fftSize;
        */
    for (auto i = 1; i < fftSize; i++)
    {
        fftout[i] = std::conj(fftout[i]);
    }
    for (auto i = 1; i < fftSize; i++)
    {
        fftin[i] = fftout[i] * data_spec[i];
        fftin[i] = fftin[i] / abs(fftin[i]);
    }
    fft_j.perform(fftin.data(), fftout.data(), true);

    for (auto i = 1; i < fftSize; i++)
    {
        if (std::real(fftout[i]) > max_value)
        {
            max_value = std::real(fftout[i]);
            max_index = i;
        }
    }
    DBG("value: %f\n", max_value);
    DBG("value at 0: %f\n", real(fftout[0]));
    DBG("index offset: %f\n", max_index);
    return max_index;
}

int MainComponent::get_most_occuring(int offsets[1000], int backlogSize)
{
    int highest_count = 0;
    int highest_value = 0;
    int count = 0;
    int value = 0;
    for (auto i = 0; i < backlogSize; i++)
    {
        value = offsets[i];
        for (auto j = 0; j < backlogSize; j++)
        {
            if (offsets[i] == offsets[j])
            {
                count++;
            }
        }
        if (count > highest_count)
        {
            highest_count = count;
            highest_value = value;
        }
        count = 0;
    }
    return value;
}

int MainComponent::wrap_index(int ind)
{
    if (ind < 0)
    {
        ind = ind + fftSize;
    }
    ind = ind % fftSize;
    return ind;
}

int MainComponent::wrap_index(int ind, int wrapValue) {
    if (ind < 0)
    {
        ind = ind + wrapValue;
    }
    ind = ind % wrapValue;
    return ind;
}