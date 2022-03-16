/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
//#include "PluginEditor.h"

//==============================================================================
Wind4Unity3AudioProcessor::Wind4Unity3AudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    )
{
    //    Global Parameters
    addParameter(gain = new juce::AudioParameterFloat(
        "Master Gain", "Master Gain", 0.0f, 1.0f, 0.5f));
    //  Wind Speed Parameters
    addParameter(dstAmplitude = new juce::AudioParameterFloat(
        "DstAmp", "Distant Gain", 0.0001f, 1.5f, 0.75f));
    addParameter(windSpeed = new juce::AudioParameterFloat(
        "windSpeed", "Wind Speed", 0.01f, 40.0f, 1.0f));
    addParameter(dstIntensity = new juce::AudioParameterFloat(
        "intensity", "dst Intensity", 1.0f, 50.0f, 30.0f));
    addParameter(dstResonance = new juce::AudioParameterFloat(
        "dstResonance", "dst Resonance", 0.1f, 50.0f, 1.0f));

}

Wind4Unity3AudioProcessor::~Wind4Unity3AudioProcessor()
{
}

//==============================================================================
const juce::String Wind4Unity3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Wind4Unity3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Wind4Unity3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Wind4Unity3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Wind4Unity3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Wind4Unity3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Wind4Unity3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Wind4Unity3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Wind4Unity3AudioProcessor::getProgramName (int index)
{
    return {};
}

void Wind4Unity3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Wind4Unity3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //    Create DSP Spec
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    currentSpec = spec;

    //    Prepare Distant Wind
    dstPrepare(spec);
}

void Wind4Unity3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Wind4Unity3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void Wind4Unity3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    dstUpdateSettings();
    dstProcess(buffer);

    buffer.applyGain(gain->get());


}

//==============================================================================
bool Wind4Unity3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Wind4Unity3AudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void Wind4Unity3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Wind4Unity3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void Wind4Unity3AudioProcessor::dstPrepare(const juce::dsp::ProcessSpec& spec)
{
    //    Prepare Filter
    dstBPF.prepare(spec);
    dstBPF.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    dstBPF.setCutoffFrequency(10.0f);
    dstBPF.setResonance(1.0f);
    dstBPF.reset();
}

void Wind4Unity3AudioProcessor::dstProcess(juce::AudioBuffer<float>& buffer)
{
    //    Get Buffer info
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    float dstFrameAmp = dstAmplitude->get();

    //    Distant Wind DSP Loop
    for (int ch = 0; ch < numChannels; ++ch)
    {
        for (int s = 0; s < numSamples; ++s)
        {
            float output = r.nextFloat() * 2.0f - 1.0f;
            output = dstBPF.processSample(ch, output);
            buffer.addSample(ch, s, output * dstFrameAmp);
        }
    }
    dstBPF.snapToZero();
}

void Wind4Unity3AudioProcessor::dstUpdateSettings()
{
    float currentWindSpeed = windSpeed->get();
    float currentDstIntensity = dstIntensity->get();
    float currentDstResonance = dstResonance->get();
    //    Update Filter Settings
    dstBPF.setCutoffFrequency(currentWindSpeed * currentDstIntensity);
    dstBPF.setResonance(currentDstResonance);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Wind4Unity3AudioProcessor();
}
