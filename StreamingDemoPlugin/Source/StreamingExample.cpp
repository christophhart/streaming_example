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


#include "StreamingExample.h"

ExampleStreamingPool::ExampleStreamingPool() {}
ExampleStreamingPool::~ExampleStreamingPool() 
{
	info = nullptr;
}

void ExampleStreamingPool::loadSampleMap(const ValueTree& sampleMap)
{
	jassert(sampleMap.isValid());
	jassert(sampleMap.getType() == Identifier("samplemap"));

	// This example only supports loading monoliths
	jassert((int)sampleMap.getProperty("SaveMode") == 2);

	// This example only supports loading samplemaps with one mic position
	jassert(sampleMap.getProperty("MicPositions") == ";");

	// Create a list of monolith files that you want to load
	// For this simple example, it just assumes this file at the desktop
	const File monolithFile = File::getSpecialLocation(File::userDesktopDirectory).getChildFile("MusicBoxSampleMap.ch1");

	// Check that the files exists at the given location
	if (!monolithFile.existsAsFile())
	{
		jassertfalse;
		return;
	}

	Array<File> files;
	files.add(monolithFile);

	// Create a HLAC Monolith Info object that parses the sample offsets & handles the multimic position
	info = new hise::HlacMonolithInfo(files);

	// Give it the sample map and it will divide it according to the information in the samplemap.
	info->fillMetadataInfo(sampleMap);

	// Now we iterate over all samples in the samplemap and create a StreamingSamplerSound object
	for (int i = 0; i < sampleMap.getNumChildren(); i++)
	{
		auto sampleData = sampleMap.getChild(i);

		// Create a StreamingSamplerSound object and save it in the reference counted array
		auto sound = new hise::StreamingSamplerSound(info, 0, i);
		sounds.add(sound);

		// This helper function extracts the most important MIDI mapping informations from the sample map
		auto mappingData = hise::StreamingHelpers::getBasicMappingDataFromSample(sampleData);
		sound->setBasicMappingData(mappingData);
	}

	// Now we need to preload the samples. We'll be using another thread for this
	ExampleSampleLoadThread loadThread(this);

	if (loadThread.runThread())
		DBG("Sample loading successful.");
	else
		DBG("Sample loading wasn't successful.");
}

const ReferenceCountedArray<hise::StreamingSamplerSound>& ExampleStreamingPool::getSounds() const
{
	return sounds;

}

ExampleStreamingPool::ExampleSampleLoadThread::ExampleSampleLoadThread(ExampleStreamingPool* pool_) :
	ThreadWithProgressWindow("Example Sample Load Thread", true, true),
	pool(pool_)
{

}

void ExampleStreamingPool::ExampleSampleLoadThread::run()
{
	// Get the number of all samples to load
	const int numSamplesToLoad = pool->sounds.size();

	// Create a string object which will contain the error message
	String errorMessage;

	for (int i = 0; i < numSamplesToLoad; ++i)
	{
		// Check if the user pressed cancel
		if (threadShouldExit())
			break;

		// set the progress
		setProgress((double)i / (double)numSamplesToLoad);

		// preload the given sample with a default preload size of 8192 (multiples of 4096 are recommended for better HLAC performance)
		const bool ok = hise::StreamingHelpers::preloadSample(pool->sounds[i], 8192, errorMessage);

		if (!ok)
			break;
	}
}

bool ExampleStreamingSamplerSound::appliesToNote(int midiNoteNumber) noexcept
{
	return wrappedSound->appliesToNote(midiNoteNumber);
}

bool ExampleStreamingSamplerSound::appliesToChannel(int midiNoteNumber) noexcept
{
	return wrappedSound->appliesToChannel(midiNoteNumber);
}

hise::StreamingSamplerSound* ExampleStreamingSamplerSound::getWrappedSound() const
{
	return wrappedSound.get();
}

ExampleStreamingSamplerVoice::ExampleStreamingSamplerVoice(hise::NewSampleThreadPool* pool, hlac::HiseSampleBuffer* temporaryVoiceBuffer) :
	wrappedVoice(pool)
{
	// Give the voice a temporary scratch buffer
	wrappedVoice.setTemporaryVoiceBuffer(temporaryVoiceBuffer);

	// Tell the voice whether to use a integer buffer or a floating-point buffer.
	// In our case, we're using HLAC samples, so the integer buffer saves us 50% memory for the 
	// streaming buffers
	wrappedVoice.setStreamingBufferDataType(false);
}

void ExampleStreamingSamplerVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
	auto wrappedSound = getWrappedSound(sound);

	// We need to set the pitch factor before we start the note
	wrappedVoice.setPitchFactor(midiNoteNumber, wrappedSound->getRootNote(), wrappedSound, 1.0);

	// Just pass everything onto the wrapped voice
	wrappedVoice.startNote(midiNoteNumber, velocity, wrappedSound, currentPitchWheelPosition);

	// Save the velocity as gain (it will be applied manually during rendering
	gain = velocity;
}

void ExampleStreamingSamplerVoice::stopNote(float velocity, bool allowTailOff)
{
	// Nothing to do here. If you're adding envelopes, this would be different,
	// then you could go into release phase and call stopNote for the wrappedVoice after
	// it has ringed off.
	wrappedVoice.stopNote(velocity, allowTailOff);
}

void ExampleStreamingSamplerVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
	// Tell the streaming engine how much samples it must fetch.
	// See the example implementation
	updatePitchData(startSample, numSamples);

	// This creates a temporary buffer by using stack allocation
	// There are multitudes of smarter ways to handle this, but this
	// is the most easiest...
	float* left = (float*)alloca(sizeof(float)*numSamples);
	float* right = (float*)alloca(sizeof(float)*numSamples);
	float* channels[2] = { left, right };
	AudioSampleBuffer temp(channels, 2, numSamples);
	temp.clear();

	// Pass the temp buffer to the wrapped voice. We need the temp buffer
	// because the voice is replacing the content
	wrappedVoice.renderNextBlock(temp, 0, numSamples);

	// Here you could add your custom processing logic, like envelopes, FX, etc.
	// All on the temp buffer.

	// Now we copy the content of the temp buffer to the output
	outputBuffer.addFrom(0, startSample, temp, 0, 0, numSamples, gain);
	outputBuffer.addFrom(1, startSample, temp, 0, 0, numSamples, gain);
}

void ExampleStreamingSamplerVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	wrappedVoice.prepareToPlay(sampleRate, samplesPerBlock);
}

void ExampleStreamingSamplerVoice::updatePitchData(int startSample, int numSamples)
{
	// We need to calculate the amount of samples that this voice is going to
	// use for the current block.
	// If you have dynamic pitch values, you have to accumulate them,
	// but here, we'll just take the constant pitch factor and multiply it with the sample amount
	auto pitchCounterThisBlock = wrappedVoice.getUptimeDelta() * (double)numSamples;

	wrappedVoice.setPitchCounterForThisBlock(pitchCounterThisBlock);
}

hise::StreamingSamplerSound* ExampleStreamingSamplerVoice::getWrappedSound(SynthesiserSound* exampleSound)
{
	return static_cast<ExampleStreamingSamplerSound*>(exampleSound)->getWrappedSound();
}

const hise::StreamingSamplerSound* ExampleStreamingSamplerVoice::getWrappedSound(SynthesiserSound* exampleSound) const
{
	return static_cast<ExampleStreamingSamplerSound*>(exampleSound)->getWrappedSound();
}
