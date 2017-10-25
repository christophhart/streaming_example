/*
Copyright 2017 Christoph Hart

HISE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HISE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HISE.  If not, see <http://www.gnu.org/licenses/>.

Commercial licenses for using HISE in an closed source project are
available on request. Please visit the project's website to get more
information about commercial licensing:

http://www.hise.audio/

HISE is based on the JUCE library,
which must be separately licensed for commercial applications:

http://www.juce.com
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
StreamingDemoPluginAudioProcessorEditor::StreamingDemoPluginAudioProcessorEditor (StreamingDemoPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	addAndMakeVisible(keyboard = new MidiKeyboardComponent(processor.state, MidiKeyboardComponent::horizontalKeyboard));
	keyboard->setKeyWidth(32);
    setSize (1024, 768);

}

StreamingDemoPluginAudioProcessorEditor::~StreamingDemoPluginAudioProcessorEditor()
{
}

//==============================================================================
void StreamingDemoPluginAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void StreamingDemoPluginAudioProcessorEditor::resized()
{
	keyboard->setBounds(getLocalBounds().removeFromBottom(120));
}
