/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Pfmproject0AudioProcessorEditor::Pfmproject0AudioProcessorEditor (Pfmproject0AudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

Pfmproject0AudioProcessorEditor::~Pfmproject0AudioProcessorEditor()
{
	//processor.shouldPlaySound = false; //makes sure we don't play sound when the GUI is closed
	/*
	processor.shouldPlaySound->beginChangeGesture();
	processor.shouldPlaySound->setValueNotifyingHost(false);
	processor.shouldPlaySound->endChangeGesture();
	*/
	Pfmproject0AudioProcessor::UpdateAutomatableParameter(processor.shouldPlaySound, false);
}

//==============================================================================
void Pfmproject0AudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void Pfmproject0AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

}

void Pfmproject0AudioProcessorEditor::mouseUp(const MouseEvent &e)
{
	/*
	processor.shouldPlaySound->beginChangeGesture();
	processor.shouldPlaySound->setValueNotifyingHost(!processor.shouldPlaySound->get());
	processor.shouldPlaySound->endChangeGesture();
	*/

	Pfmproject0AudioProcessor::UpdateAutomatableParameter(processor.shouldPlaySound, !processor.shouldPlaySound->get());
}

void Pfmproject0AudioProcessorEditor::mouseDown(const MouseEvent &e)
{
	//processor.shouldPlaySound = true;
	//
}
