/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "StreamingExample.h"

//==============================================================================
/**
*/
class StreamingDemoPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    StreamingDemoPluginAudioProcessor();
    ~StreamingDemoPluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	

private:

	friend class StreamingDemoPluginAudioProcessorEditor;

	MidiKeyboardState state;

	// Working buffer for the streaming engine. You only need one per instance.
	hlac::HiseSampleBuffer temporaryVoiceBuffer;

	// This object holds information about the loaded streaming sounds
	ExampleStreamingPool pool;

	// This object handles the background threaded loading of the samples
	ScopedPointer<hise::NewSampleThreadPool> backgroundThreadPool;

	// Just use the default synthesiser from JUCE for the Streaming sampler demo...
	Synthesiser streamingSampler;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StreamingDemoPluginAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
