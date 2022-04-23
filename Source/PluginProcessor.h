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

#pragma once

#include <JuceHeader.h>
#include <random>

//==============================================================================
/**
*/
struct BlockLPF
{
    BlockLPF()
    {

    }
    ~BlockLPF()
    {

    }

public:

    void prepare(float cutoff, int samplesPerBlock, double sampleRate)
    {
        float c0 = std::tan(juce::MathConstants<double>::pi * cutoff / (sampleRate / samplesPerBlock));
        coeff = c0 / (1 + c0);
    }

    float processSample(float input)
    {
        lastOutput = (1.0f - coeff) * lastOutput + coeff * input;
        lastOutput = lastOutput < 0.00001f ? 0.0f : lastOutput;
        return lastOutput;
    }

private:

    float coeff;
    float lastOutput;

};

class Wind4Unity3AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Wind4Unity3AudioProcessor();
    ~Wind4Unity3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override{ return true; }
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override{ return JucePlugin_Name; }

    bool acceptsMidi() const override{ return false; }
    bool producesMidi() const override{ return false; }
    bool isMidiEffect() const override{ return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override{}
    void setStateInformation (const void* data, int sizeInBytes) override{}

    // Data Structs

    // Constants
    static const int wSCBSize = 500;
    static const int numOutputChannels = 2;
    static const int maxPanFrames = 20;

    struct GlobalData
    {
        float windSpeedCircularBuffer[wSCBSize];
        int wSCBWriteIndex{ 0 };
    };

    struct WhistleData
    {
        int whsWSCBReadIndex1 = wSCBSize - 6;
        int whsWSCBReadIndex2 = wSCBSize - 16;
        float whsWindSpeed1;
        float whsWindSpeed2;
    };

    struct HowlData
    {
        int howlWSCBReadIndex1 = wSCBSize - 6;
        int howlWSCBReadIndex2 = wSCBSize - 51;
        float howlWindSpeed1;
        float howlWindSpeed2;
    };

    struct PanData
    {
        float whistlePan1;
        float whistlePan2;
        float howlPan1;
        float howlPan2;
    };

private:

    //  Wind Methods
    void Prepare(const juce::dsp::ProcessSpec& spec);
    void dstProcess(juce::AudioBuffer<float>& buffer);
    void whsProcess(juce::AudioBuffer<float>& buffer);
    void howlProcess(juce::AudioBuffer<float>& buffer);
    void updateSettings();
    void cosPan(float* output, float pan);  

    //  Global Parameters
    juce::AudioParameterFloat* gain;

    //  Wind Speed Parameters
    juce::AudioParameterFloat* windSpeed;

    // Distant Wind Parameters
    juce::AudioParameterFloat* dstAmplitude;
    juce::AudioParameterFloat* dstIntensity;
    juce::AudioParameterFloat* dstResonance;
    juce::AudioParameterFloat* dstPan;

    // Whistle Parameters

    juce::AudioParameterFloat* whsPan1;
    juce::AudioParameterFloat* whsPan2;
    juce::AudioParameterFloat* whsAmplitude;
    WhistleData wd;

    // Howl Parameters

    juce::AudioParameterFloat* howlPan1;
    juce::AudioParameterFloat* howlPan2;
    juce::AudioParameterFloat* howlAmplitude;
    HowlData hd;

    //  Distant Wind DSP Resources
    juce::Random r;
    juce::dsp::StateVariableTPTFilter<float> dstBPF;

    // Whistle DSP Resources
    juce::dsp::StateVariableTPTFilter<float> whsBPF1;
    juce::dsp::StateVariableTPTFilter<float> whsBPF2;

    // Howl DSP Resources
    juce::dsp::StateVariableTPTFilter<float> howlBPF1;
    juce::dsp::StateVariableTPTFilter<float> howlBPF2;
    juce::dsp::Oscillator<float> howlOsc1;
    juce::dsp::Oscillator<float> howlOsc2;
    BlockLPF howlBlockLPF1;
    BlockLPF howlBlockLPF2;

    //  Internal Variables
    juce::dsp::ProcessSpec currentSpec;
    GlobalData gd;
    PanData pd;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Wind4Unity3AudioProcessor)
};
