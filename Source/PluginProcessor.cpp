/*
  ==============================================================================

    Copyright (c) 2022 - Gordon Webb

    This file is part of Wind4Unity3.

    Wind4Unity1 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wind4Unity1 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wind4Unity3.  If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================
*/
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
    addParameter(dstPan = new juce::AudioParameterFloat(
        "dstPan", "Distant Pan", 0.0f, 1.0f, 0.5f));
    addParameter(whsAmplitude = new juce::AudioParameterFloat(
        "WhsAmp", "Whistle Gain", 0.0001f, 1.5f, 0.75f));
    addParameter(whsPan1 = new juce::AudioParameterFloat(
        "WhsPan1", "Whistle Pan1", 0.0f, 1.0f, 0.5f));
    addParameter(whsPan2 = new juce::AudioParameterFloat(
        "WhsPan2", "Whistle Pan2", 0.0f, 1.0f, 0.5f));
    addParameter(howlAmplitude = new juce::AudioParameterFloat(
        "HowlAmp", "Howl Gain", 0.0001f, 1.5f, 0.75f));
    addParameter(howlPan1 = new juce::AudioParameterFloat(
        "HowlPan1", "Howl Pan1", 0.0f, 1.0f, 0.5f));
    addParameter(howlPan2 = new juce::AudioParameterFloat(
        "HowlPan2", "Howl Pan2", 0.0f, 1.0f, 0.5f));
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
    spec.numChannels = 1;
    // spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    currentSpec = spec;

    //    Prepare DSP
    Prepare(spec);
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

    updateSettings();
    dstProcess(buffer);
    whsProcess(buffer);
    howlProcess(buffer);

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

void Wind4Unity3AudioProcessor::Prepare(const juce::dsp::ProcessSpec& spec)
{
    //    Prepare DST
    dstBPF.prepare(spec);
    dstBPF.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    dstBPF.setCutoffFrequency(10.0f);
    dstBPF.setResonance(1.0f);
    dstBPF.reset();

    //    Prepare Whistle
    whsBPF1.prepare(spec);
    whsBPF1.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    whsBPF1.setCutoffFrequency(1000.0f);
    whsBPF1.setResonance(60.0f);
    whsBPF1.reset();

    whsBPF2.prepare(spec);
    whsBPF2.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    whsBPF2.setCutoffFrequency(1000.0f);
    whsBPF2.setResonance(60.0f);
    whsBPF2.reset();

    //    Prepare Howl
    howlBPF1.prepare(spec);
    howlBPF1.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    howlBPF1.setCutoffFrequency(400.0f);
    howlBPF1.setResonance(40.0f);
    howlBPF1.reset();

    howlBPF2.prepare(spec);
    howlBPF2.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    howlBPF2.setCutoffFrequency(200.0f);
    howlBPF2.setResonance(40.0f);
    howlBPF2.reset();

    howlOsc1.initialise([](float x) { return std::sin(x); }, 128);
    howlOsc2.initialise([](float x) { return std::sin(x); }, 128);

    howlBlockLPF1.prepare(0.5f, spec.maximumBlockSize, spec.sampleRate);
    howlBlockLPF2.prepare(0.4f, spec.maximumBlockSize, spec.sampleRate);
}

void Wind4Unity3AudioProcessor::dstProcess(juce::AudioBuffer<float>& buffer)
{
    //    Get Buffer info
    int numSamples = buffer.getNumSamples();
    float FrameAmp = dstAmplitude->get();

    //    Distant Wind DSP Loop
    float pan[2];
    cosPan(pan, dstPan->get());

    for (int s = 0; s < numSamples; ++s)
    {
        float output = dstBPF.processSample(0, r.nextFloat() * 2.0f - 1.0f) * FrameAmp;
        buffer.addSample(0, s, output * pan[0]);
        buffer.addSample(1, s, output * pan[1]);
    }
  
    dstBPF.snapToZero();
}

void Wind4Unity3AudioProcessor::whsProcess(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    float FrameAmp = whsAmplitude->get();

    //    Whistle DSP Loop
    float pan1[2];
    cosPan(pan1, whsPan1->get());
    float pan2[2];
    cosPan(pan2, whsPan2->get());
    float AmpMod1 = juce::square(juce::jmax(0.0f, wd.whsWindSpeed1 * 0.02f - 0.1f));
    float AmpMod2 = juce::square(juce::jmax(0.0f, wd.whsWindSpeed2 * 0.02f - 0.1f));

    for (int s = 0; s < numSamples; ++s)
    {
        float noiseOutput = r.nextFloat() * 2.0f - 1.0f;
        float output1 = whsBPF1.processSample(0, noiseOutput) * FrameAmp * AmpMod1;
        float output2 = whsBPF2.processSample(0, noiseOutput) * FrameAmp * AmpMod2;
        buffer.addSample(0, s, output1 * pan1[0] + output2 * pan2[0]);
        buffer.addSample(1, s, output1 * pan1[1] + output2 * pan2[1]);
    }

    whsBPF1.snapToZero();
    whsBPF2.snapToZero();
}

void Wind4Unity3AudioProcessor::howlProcess(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    float FrameAmp = howlAmplitude->get();

    //    Whistle DSP Loop
    float pan1[2];
    cosPan(pan1, howlPan1->get());
    float pan2[2];
    cosPan(pan2, howlPan2->get());
    float AmpMod1 = howlBlockLPF1.processSample(juce::dsp::FastMathApproximations::cos(
        ((juce::jlimit(0.35f, 0.6f, hd.howlWindSpeed1 * 0.02f)
         - 0.35f) * 2.0f - 0.25f) * juce::MathConstants<float>::twoPi));
    float AmpMod2 = howlBlockLPF2.processSample( juce::dsp::FastMathApproximations::cos(
        ((juce::jlimit(0.25f, 0.5f, hd.howlWindSpeed2 * 0.02f) 
         - 0.25f) * 2.0f - 0.25f) * juce::MathConstants<float>::twoPi));
    howlOsc1.setFrequency(AmpMod1 * 200.0f + 30.0f);
    howlOsc2.setFrequency(AmpMod2 * 100.0f + 20.0f);
    for (int s = 0; s < numSamples; ++s)
    {
        float noiseOutput = r.nextFloat() * 2.0f - 1.0f;
        float output1 = howlBPF1.processSample(0, noiseOutput) * FrameAmp * AmpMod1 * howlOsc1.processSample(0.0f);
        float output2 = howlBPF2.processSample(0, noiseOutput) * FrameAmp * AmpMod2 * howlOsc2.processSample(0.0f);
        buffer.addSample(0, s, output1 * pan1[0] + output2 * pan2[0]);
        buffer.addSample(1, s, output1 * pan1[1] + output2 * pan2[1]);
    }

    howlBPF1.snapToZero();
    howlBPF2.snapToZero();
}

void Wind4Unity3AudioProcessor::updateSettings()
{
    //  UpdateWSCircularBuffer;
    float currentWindSpeed = windSpeed->get();
    gd.windSpeedCircularBuffer[gd.wSCBWriteIndex] = currentWindSpeed;
    ++gd.wSCBWriteIndex;
    gd.wSCBWriteIndex = (gd.wSCBWriteIndex < wSCBSize) ? gd.wSCBWriteIndex : 0;
    
    //  Update DST
    float currentDstIntensity = dstIntensity->get();
    float currentDstResonance = dstResonance->get();
    //    Update DST Filter Settings
    dstBPF.setCutoffFrequency(currentWindSpeed * currentDstIntensity);
    dstBPF.setResonance(currentDstResonance);
    //    Update Whistle
    wd.whsWindSpeed1 = gd.windSpeedCircularBuffer[wd.whsWSCBReadIndex1];
    wd.whsWindSpeed2 = gd.windSpeedCircularBuffer[wd.whsWSCBReadIndex2];
    whsBPF1.setCutoffFrequency(wd.whsWindSpeed1 * 8.0f + 600.0f);
    whsBPF2.setCutoffFrequency(wd.whsWindSpeed2 * 20.0f + 1000.0f);
    ++wd.whsWSCBReadIndex1;
    ++wd.whsWSCBReadIndex2;
    wd.whsWSCBReadIndex1 = (wd.whsWSCBReadIndex1 < wSCBSize) ? wd.whsWSCBReadIndex1 : 0;
    wd.whsWSCBReadIndex2 = (wd.whsWSCBReadIndex2 < wSCBSize) ? wd.whsWSCBReadIndex2 : 0;

    //    Update Howl
    hd.howlWindSpeed1 = gd.windSpeedCircularBuffer[hd.howlWSCBReadIndex1];
    hd.howlWindSpeed2 = gd.windSpeedCircularBuffer[hd.howlWSCBReadIndex2];
    ++hd.howlWSCBReadIndex1;
    ++hd.howlWSCBReadIndex2;
    hd.howlWSCBReadIndex1 = (hd.howlWSCBReadIndex1 < wSCBSize) ? hd.howlWSCBReadIndex1 : 0;
    hd.howlWSCBReadIndex2 = (hd.howlWSCBReadIndex2 < wSCBSize) ? hd.howlWSCBReadIndex2 : 0;
}

void Wind4Unity3AudioProcessor::cosPan(float* output, float pan)
{
    output[0] = std::cosf((pan * 0.25f - 0.5f) * juce::MathConstants<float>::twoPi);
    output[1] = std::cosf((pan * 0.25f - 0.25f) * juce::MathConstants<float>::twoPi);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Wind4Unity3AudioProcessor();
}
