/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

/*
 TODO:
 click anywhere on the window, and play a note
 automation should update window display
 if you click and drag, it'll change the pitch of the note
 (CHECK) - save plugin state when exiting DAW
 (CHECK) - load plugin state when loading a session
 (CHECK) - Should we play a sound?
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

#include <array>
template<typename T>
struct Fifo
{
    bool push( const T& itemToAdd )
    {
        auto write = fifo.write(1);
        if(write.blockSize1 >= 1)
        {
            buffer[write.startIndex1] = itemToAdd;
            return true;
        }
        return false;
    }
    bool pull( T& itemToUpdate )
    {
        auto read = fifo.read(1);
        if(read.blockSize1 >= 1)
        {
            itemToUpdate = buffer[read.startIndex1];
            return true;
        }
        return false;
    }
private:
    static constexpr int Capacity = 5;
    std::array<T, Capacity> buffer;
    AbstractFifo fifo{Capacity};
};

//==============================================================================


// 1) copy audio thread buffer to analyzer buffer
// 2) background thread will copy analyzer buffer into fifo buffer
// 3) when the fifo buffer is full, copy to FFT Data buffer, signal GUI to repaint() from bkgd thread
// 4) GUI will compute FFT, generate a juce::Path from the data and draw it
struct BufferAnalyzer : Thread, Timer, Component
{
    BufferAnalyzer();
    ~BufferAnalyzer();
    
    void prepare(double sampleRate, int samplesPerBlock);
    void cloneBuffer( const dsp::AudioBlock<float>& other );
    void run() override;
    void timerCallback() override;
    void paint(Graphics& g) override;
private:
    std::array<AudioBuffer<float>, 2> buffers;
    Atomic<bool> firstBuffer {true};
    std::array<Atomic<size_t>, 2> samplesCopied;
    
    //==============================
    enum
    {
        fftOrder  = 11,
        fftSize   = 1 << fftOrder,
        numPoints = 512
    };
    
    float fifoBuffer [fftSize];
    float fftData [2 * fftSize];
    int fifoIndex = 0;
    
    void pushNextSampleIntoFifo(float sample);
    
    Atomic<bool> nextFFTBlockReady{false};
    float curveData [numPoints];
    
    dsp::FFT forwardFFT{fftOrder};
    dsp::WindowingFunction<float> window{fftSize, dsp::WindowingFunction<float>::hann};
    
    void drawNextFrameOfSpectrum();
    Path fftCurve;
};

//==============================================================================
/**
*/
class Pfmproject0AudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Pfmproject0AudioProcessor();
    ~Pfmproject0AudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
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
	AudioParameterBool* shouldPlaySound = nullptr; // = false;
    AudioParameterFloat* bgColor = nullptr;

	static void UpdateAutomatableParameter(RangedAudioParameter*, float value);

    BufferAnalyzer leftBufferAnalyzer;
    BufferAnalyzer rightBufferAnalyzer;
private:
    AudioProcessorValueTreeState apvts;
	Random r;
    
    
	//==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pfmproject0AudioProcessor)
};
