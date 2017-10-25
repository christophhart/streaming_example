# Sample Streaming Demo

This repository contains a JUCE project which demonstrates the usage of the HISE streaming engine in a pure C++ context. It's a simple audio plugin that loads a given samplemap (the Musicbox from the tutorial) and its HLAC compressed monolith file and plays it back using the default JUCE synthesiser class.

This is just a barebone-access to the streaming engine of HISE so you have to implement these things for yourself:

- envelopes, filters, effects
- MIDI processing
- file management (this project just uses one monolith and expects it to be on the desktop, so you need to add custom file handling for actual projects)
- multimic samplemaps are not supported
- RR group logic is not implemented (but you can take the `RRGroup` property from the samplemap to add your own handling).

It's currently tested only on Windows, but I'll check macOS and iOS later.

## How to use this project

1. Grab the latest HISE repository to make sure that you have the current code. Currently I am stuck on JUCE 4.3.1 for HISE, but the `hi_lac` and the `hi_streaming` module should also work with newer versions.
2. Open the .jucer file to create your IDE project files.
3. Download [this monolith sample file](https://github.com/christophhart/hise_tutorial/blob/master/MusicBox%20Tutorial/Samples/MusicBoxSampleMap.ch1?raw=true) and put it on your desktop.
4. Compile the project and load the plugin.
5. Play some notes. There are only ~2 octaves mapped, so don't freak out if you don't hear anything :)

## Where to go from here

If you want to use the HISE Streaming engine in your project, you most likely want to add more advanced features. Basically, you just need to create your own implementations of

1. A sample pool (derived from the `StreamingSamplerPool`) which manages the lifetime of your loaded samples and parses the sample map
2. A wrapper for the `StreamingSamplerSound` class, which adds additional logic to the selection of the samples (like Round Robin etc).
3. A wrapper for the `StreamingSamplerVoice` which renders your voices and applies envelopes, filters etc. on it.

You can take a look at the example implementations in the StreamingExample.cpp file, or even take a look at how HISE is using these classes for an advanced use case [here](https://github.com/christophhart/HISE/blob/master/hi_sampler/sampler/ModulatorSamplerVoice.h) and [here](https://github.com/christophhart/HISE/blob/master/hi_sampler/sampler/ModulatorSamplerSound.h).

## License

This example project has the same license as the HISE codebase (GPL v3). If you're interested in licensing just the streaming engine for your closed source JUCE project, get in touch with me, then we'll discuss the details.