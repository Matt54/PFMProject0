/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
// 1) copy audio thread buffer to analyzer buffer
// 2) background thread will copy analyzer buffer into fifo buffer
// 3) when the fifo buffer is full, copy to FFT Data buffer, signal GUI to repaint() from bkgd thread
// 4) GUI will compute FFT, generate a juce::Path from the data and draw it

BufferAnalyzer2::BufferAnalyzer2() : Thread("BufferAnalyzer")
{
    startThread();
    startTimerHz(20);
    fftCurve.preallocateSpace(3 * numPoints);
    
}

BufferAnalyzer2::~BufferAnalyzer2()
{
    notify();
    stopThread(100);
}

void BufferAnalyzer2::prepare(double sampleRate, int samplesPerBlock)
{
    firstBuffer = true;
    buffers[0].setSize(1, samplesPerBlock);
    buffers[1].setSize(1, samplesPerBlock);
    
    samplesCopied[0].set(0);
    samplesCopied[1].set(0);
    
    fifoIndex = 0;
    zeromem(fifoBuffer, sizeof(fifoBuffer));
    zeromem(fftData,sizeof(fftData));
    zeromem(curveData, sizeof(curveData));
}

void BufferAnalyzer2::cloneBuffer( const dsp::AudioBlock<float>& other )
{
    auto whichIndex = firstBuffer.get();
    auto index = whichIndex ? 0 : 1;
    firstBuffer.set( !whichIndex );
    
    jassert(other.getNumChannels() == buffers[index].getNumChannels() );
    jassert(other.getNumChannels() <= buffers[index].getNumSamples() );
    
    buffers[index].clear();
    
    dsp::AudioBlock<float> buffer(buffers[index]);
    buffer.copyFrom(other);
    
    samplesCopied[index].set( other.getNumSamples() );
    
    notify();
}

void BufferAnalyzer2::run()
{
    while(true)
    {
        wait(-1);
        
        DBG( "BufferAnalyzer::run() awake!!");
            
        if(threadShouldExit())
        {
            break;
        }
        
        auto index = !firstBuffer.get() ? 0 : 1;
        
        auto numSamples = samplesCopied[index].get();
        auto* readPtr = buffers[index].getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
        {
            //pushNextSampleIntoFifo(buffers[index].getSample(0,i));
            /*
             getSample is slow
             try using a pointer
             */
            pushNextSampleIntoFifo( *(readPtr+i));
        }
    }
}

void BufferAnalyzer2::pushNextSampleIntoFifo(float sample)
{
    if(fifoIndex == fftSize)
    {
        auto ready = nextFFTBlockReady.get();
        if(!ready)
        {
            zeromem(fftData, sizeof(fftData));
            memcpy(fftData, fifoBuffer, sizeof(fifoBuffer));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifoBuffer[fifoIndex++] = sample;
}

void BufferAnalyzer2::timerCallback()
{
    auto ready = nextFFTBlockReady.get();
    if(ready)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }
}

void BufferAnalyzer2::drawNextFrameOfSpectrum()
{
    window.multiplyWithWindowingTable (fftData, fftSize);      // [1]
    forwardFFT.performFrequencyOnlyForwardTransform (fftData); // [2]
    auto mindB = -100.0f;
    auto maxdB =    0.0f;
    for (int i = 0; i < numPoints; ++i)                        // [3]
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - i / (float) numPoints) * 0.2f);
        auto fftDataIndex = jlimit (0, fftSize / 2, (int) (skewedProportionX * fftSize / 2));
        auto level = jmap (jlimit (mindB, maxdB, Decibels::gainToDecibels (fftData[fftDataIndex])
                                               - Decibels::gainToDecibels ((float) fftSize)),
                           mindB, maxdB, 0.0f, 1.0f);
        curveData[i] = level;                                  // [4]
    }
}

void BufferAnalyzer2::paint(Graphics& g)
{
    fftCurve.clear();
    
    float w = getWidth();
    float h = getHeight();
    
    fftCurve.startNewSubPath(0,jmap(curveData[0],
                                    0.f,1.f, //from range
                                    h, 0.f));
    
    for( int i = 1; i < numPoints; ++i)
    {
        auto data = curveData[i];
        auto endX = jmap((float)i,
                         0.f,float(numPoints), //from range
                         0.f,w); //to range
        auto endY = jmap(data,
                         0.f,1.f, //from range
                         h, 0.f); //to range
        fftCurve.lineTo(endX,endY);
    }
    
    g.fillAll(Colours::black);
    
    //g.setColour(Colours::white);
    
    ColourGradient cg;
    
    /*
     white red orange yellow green blue violet
     */
    auto colors = std::vector<Colour>
    {
        Colours::violet,
        Colours::blue,
        Colours::green,
        Colours::yellow,
        Colours::orange,
        Colours::red,
        Colours::white
    };
    
    for(int i = 0; i < colors.size(); ++i)
    {
        cg.addColour( double(i) / double( colors.size() - 1 ),colors[i] );
    }
    
    cg.point1 = {0,h};
    cg.point2 = {0,0};
    
    g.setGradientFill(cg);
    
    g.strokePath( fftCurve, PathStrokeType(1) );
}

//==============================================================================
Pfmproject0AudioProcessor::Pfmproject0AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
apvts(*this, nullptr)
{
	//shouldPlaySound = new AudioParameterBool("ShouldPlaySoundParam", "shouldPlaySound", false);
	//addParameter(shouldPlaySound);
    
    auto shouldPlaySoundParam = std::make_unique<AudioParameterBool>("ShouldPlaySoundParam","shouldPlaySound",false);
    
    auto* param = apvts.createAndAddParameter(std::move(shouldPlaySoundParam) );
    
    shouldPlaySound = dynamic_cast<AudioParameterBool*>(param);
    
    auto bgColorParam = std::make_unique<AudioParameterFloat>("Background color","background color",0.f,1.f,0.5f);
    
    param = apvts.createAndAddParameter(std::move(bgColorParam));
    bgColor = dynamic_cast<AudioParameterFloat*>(param);
    
    apvts.state = ValueTree("PFMSynthValueTree");
    
	OwnedArray<SystemTrayIconComponent> comps;
	comps.add(new SystemTrayIconComponent());

	std::vector< std::unique_ptr<SystemTrayIconComponent> > unique_comps;

	ScopedPointer<SystemTrayIconComponent> sysTrayIcon;
	std::unique_ptr<SystemTrayIconComponent> uniqueSysTrayIcon;
}

Pfmproject0AudioProcessor::~Pfmproject0AudioProcessor()
{
}

//==============================================================================
const String Pfmproject0AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Pfmproject0AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Pfmproject0AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Pfmproject0AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Pfmproject0AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Pfmproject0AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Pfmproject0AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Pfmproject0AudioProcessor::setCurrentProgram (int index)
{
}

const String Pfmproject0AudioProcessor::getProgramName (int index)
{
    return {};
}

void Pfmproject0AudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Pfmproject0AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    leftBufferAnalyzer.prepare(sampleRate, samplesPerBlock);
    rightBufferAnalyzer.prepare(sampleRate, samplesPerBlock);
}

void Pfmproject0AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Pfmproject0AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Pfmproject0AudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    //Random r;
    for(int i = 0; i < buffer.getNumSamples(); ++i)
    {
        for(int channel=0; channel < buffer.getNumChannels(); ++channel)
        {
             //buffer.setSample(channel,i,r.nextFloat());
            
            if(shouldPlaySound->get())
			{
                buffer.setSample(channel,i,r.nextFloat());
            }
            else
			{
                buffer.setSample(channel,i,0);
            }
             
        }
    }
    
    dsp::AudioBlock<float> block(buffer);
    auto left = block.getSingleChannelBlock(0);
    leftBufferAnalyzer.cloneBuffer(left);
    
    if( buffer.getNumChannels() == 2 )
    {
        auto right = block.getSingleChannelBlock(1);
        rightBufferAnalyzer.cloneBuffer(right);
    }
    
    buffer.clear();
    
    /* HE WANTED US TO DELETE THIS
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
     */
}

//==============================================================================
bool Pfmproject0AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Pfmproject0AudioProcessor::createEditor()
{
    return new Pfmproject0AudioProcessorEditor (*this);
}

//==============================================================================
void Pfmproject0AudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    DBG(apvts.state.toXmlString());
    MemoryOutputStream mos(destData,false);
    apvts.state.writeToStream(mos);
}

void Pfmproject0AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    ValueTree tree = ValueTree::readFromData(data, sizeInBytes);
    if(tree.isValid() )
    {
        apvts.state = tree;
    }
    DBG(apvts.state.toXmlString());
    
    /*
    MemoryBlock mb(data, static_cast<size_t>(sizeInBytes));
    MemoryInputStream mis(mb,false);
    apvts.state.readFromStream(mis);
     */
}

void Pfmproject0AudioProcessor::UpdateAutomatableParameter(RangedAudioParameter* param, float value)
{
	param->beginChangeGesture();
	param->setValueNotifyingHost(value);
	param->endChangeGesture();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Pfmproject0AudioProcessor();
}
