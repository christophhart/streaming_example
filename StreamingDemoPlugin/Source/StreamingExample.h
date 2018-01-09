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


#ifndef STREAMINGEXAMPLE_H_INCLUDED
#define STREAMINGEXAMPLE_H_INCLUDED

#include <JuceHeader.h>

/** A Example implementation of a StreamingSamplerSoundPool. 
*
*	This just parses the given sample map and loads the monolith file from the desktop
*/
class ExampleStreamingPool : public hise::StreamingSamplerSoundPool
{
public:

	ExampleStreamingPool();
	~ExampleStreamingPool();

	void loadSampleMap(const ValueTree& sampleMap);

	const ReferenceCountedArray<hise::StreamingSamplerSound>& getSounds() const;;

	class ExampleSampleLoadThread : public ThreadWithProgressWindow
	{
	public:
		ExampleSampleLoadThread(ExampleStreamingPool* pool_);

		void run();

	private:

		ExampleStreamingPool* pool;
	};

private:

	ReferenceCountedArray<hise::StreamingSamplerSound> sounds;
	hise::HlacMonolithInfo::Ptr info;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExampleStreamingPool);
};

/** Although the StreamingSamplerSound is derived from the SynthesiserSound,
*	you will most likely and up writing a wrapper around it in order to customize the behaviour. 
*
*	If you want to eg. add Round Robin handling to your samples, this would be the place to implement the logic
*	
*/
class ExampleStreamingSamplerSound : public SynthesiserSound
{
public:

	ExampleStreamingSamplerSound(hise::StreamingSamplerSound* soundFromPool):
		wrappedSound(soundFromPool)
	{}

	bool appliesToNote(int midiNoteNumber) noexcept override;
	bool appliesToChannel(int midiNoteNumber) noexcept override;

	hise::StreamingSamplerSound* getWrappedSound() const;

private:

	hise::StreamingSamplerSound::Ptr wrappedSound;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExampleStreamingSamplerSound)
};

/** This is a simple wrapper around a StreamingSamplerVoice which allows to add custom processing etc. */
class ExampleStreamingSamplerVoice : public SynthesiserVoice
{
public:

	ExampleStreamingSamplerVoice(hise::SampleThreadPool* pool, hlac::HiseSampleBuffer* temporaryVoiceBuffer);

	bool canPlaySound(SynthesiserSound*) { return true; };
	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition);
	void stopNote(float velocity, bool allowTailOff);

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
	void prepareToPlay(double sampleRate, int samplesPerBlock);

	void pitchWheelMoved(int newPitchWheelValue) override {};
	void controllerMoved(int controllerNumber, int newControllerValue) override {};

	void updatePitchData(int startSample, int numSamples);

private:

	hise::StreamingSamplerSound* getWrappedSound(SynthesiserSound* exampleSound);
	const hise::StreamingSamplerSound* getWrappedSound(SynthesiserSound* exampleSound) const;

	float gain = 1.0f;
	hise::StreamingSamplerVoice wrappedVoice;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExampleStreamingSamplerVoice)
};

#endif  // STREAMINGEXAMPLE_H_INCLUDED
