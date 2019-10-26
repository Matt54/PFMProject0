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
    cachedBgColor = processor.bgColor->get();
    
    addAndMakeVisible(processor.leftBufferAnalyzer);
    addAndMakeVisible(processor.rightBufferAnalyzer);
    
    processor.leftBufferAnalyzer.setInterceptsMouseClicks(false,false);
    processor.rightBufferAnalyzer.setInterceptsMouseClicks(false,false);
    
    setSize (400, 300);
    
    
    startTimerHz(20);
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

void Pfmproject0AudioProcessorEditor::timerCallback()
{
    update();
}

void Pfmproject0AudioProcessorEditor::update()
{
    cachedBgColor = processor.bgColor->get();
    repaint();
}

//==============================================================================
void Pfmproject0AudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).interpolatedWith(Colours::red,cachedBgColor));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void Pfmproject0AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    processor.leftBufferAnalyzer.setBounds(0,
                                           0,
                                           getWidth(),
                                           getHeight() * 0.5);
    processor.rightBufferAnalyzer.setBounds(0,
                                            getHeight() * 0.5,
                                            getWidth(),
                                            getHeight() * 0.5);
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
    
    lastClickPos = e.getPosition();
}

void Pfmproject0AudioProcessorEditor::mouseDrag(const MouseEvent &e)
{
    auto clickPos = e.getPosition();
    
    auto difY = jlimit(-1.0,1.0, -(clickPos.y-lastClickPos.y) / 200.0); //static_cast<double>(getHeight()));
    difY = jmap(difY,-1.0,1.0,0.0,1.0);
    
    //He thinks this would be easier in hindsight
    //auto difY = jmap((double)clickPos.y,0.0,(double)getHeight(),1.0,0.0);
    
    DBG("difY:" << difY);
    
    Pfmproject0AudioProcessor::UpdateAutomatableParameter(processor.bgColor, difY);
    update();
    //DBG(clickPos.toString());
}
