/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for cloused source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
StreamingDemoPluginAudioProcessor::StreamingDemoPluginAudioProcessor():
	backgroundThreadPool(new hise::SampleThreadPool()),
	temporaryVoiceBuffer(false, 2, 0) // we'll need an integer buffer for monolith samples with stereo channels
{
	

	

	// Parse the XML element from the embedded binary data
	MemoryBlock mb(BinaryData::MusicBoxSampleMap_xml, BinaryData::MusicBoxSampleMap_xmlSize);
	auto xmlText = mb.toString();
	ScopedPointer<XmlElement> xmlElement = XmlDocument::parse(xmlText);

	if (xmlElement != nullptr)
	{
		// In a real project, you might want to store the ValueTree directly...
		auto sampleData = ValueTree::fromXml(*xmlElement);

		// Load the sample data into the pool
		pool.loadSampleMap(sampleData);

		auto& soundsFromPool = pool.getSounds();

		if (soundsFromPool.size() == 0)
		{
			// The loading didn't work
			jassertfalse;
		}

		for (auto sound : soundsFromPool)
		{
			// Add the sound to the synthesiser
			streamingSampler.addSound(new ExampleStreamingSamplerSound(sound));
		}

		for (int i = 0; i < 128; i++)
		{
			streamingSampler.addVoice(new ExampleStreamingSamplerVoice(backgroundThreadPool, &temporaryVoiceBuffer));
		}

	}
	else
	{
		// huh?
		jassertfalse;
	}

	

}

StreamingDemoPluginAudioProcessor::~StreamingDemoPluginAudioProcessor()
{
}

//==============================================================================
const String StreamingDemoPluginAudioProcessor::getName() const { return JucePlugin_Name; }
bool StreamingDemoPluginAudioProcessor::acceptsMidi() const { return true; }
bool StreamingDemoPluginAudioProcessor::producesMidi() const { return false; }
double StreamingDemoPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int StreamingDemoPluginAudioProcessor::getNumPrograms() { return 1; }
int StreamingDemoPluginAudioProcessor::getCurrentProgram() { return 0; }
void StreamingDemoPluginAudioProcessor::setCurrentProgram (int /*index*/) { }
const String StreamingDemoPluginAudioProcessor::getProgramName (int /*index*/) { return String(); }
void StreamingDemoPluginAudioProcessor::changeProgramName (int /*index*/, const String& newName) {}

//==============================================================================
void StreamingDemoPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	// Initialise the synthesiser
	streamingSampler.setCurrentPlaybackSampleRate(sampleRate);

	// Initialise the temporary voice buffer. This will be a working buffer that holds temporary data from the streaming threads
	hise::StreamingSamplerVoice::initTemporaryVoiceBuffer(&temporaryVoiceBuffer, samplesPerBlock);

	// Initialise the streaming voices. This is necessary because they will create temporary buffers according
	// to the expected block size.
	for (int i = 0; i < streamingSampler.getNumVoices(); i++)
	{
		static_cast<ExampleStreamingSamplerVoice*>(streamingSampler.getVoice(i))->prepareToPlay(sampleRate, samplesPerBlock);
	}
}

void StreamingDemoPluginAudioProcessor::releaseResources()
{
	streamingSampler.clearSounds();
	streamingSampler.clearVoices();
}

void StreamingDemoPluginAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	state.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	// The default method for rendering JUCE synthesisers...
	streamingSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

	midiMessages.clear();

}

//==============================================================================
bool StreamingDemoPluginAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* StreamingDemoPluginAudioProcessor::createEditor()
{
    return new StreamingDemoPluginAudioProcessorEditor (*this);
}

void StreamingDemoPluginAudioProcessor::getStateInformation (MemoryBlock& /*destData*/) {}
void StreamingDemoPluginAudioProcessor::setStateInformation (const void* data, int /*sizeInBytes*/) {}


AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StreamingDemoPluginAudioProcessor();
}
