/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
//#include "PluginProcessor.h"

struct Pfmproject0AudioProcessor;

//==============================================================================
/**
*/
class Pfmproject0AudioProcessorEditor  : public AudioProcessorEditor, public Timer
{
public:
    Pfmproject0AudioProcessorEditor (Pfmproject0AudioProcessor&);
    ~Pfmproject0AudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

	void mouseDown(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void timerCallback() override;

private:
    void update();
    Point<int> lastClickPos;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Pfmproject0AudioProcessor& processor;
    float cachedBgColor = 0.f;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pfmproject0AudioProcessorEditor)
};
