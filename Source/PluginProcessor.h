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
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //  Wind Methods
    void dstPrepare(const juce::dsp::ProcessSpec& spec);
    void dstProcess(juce::AudioBuffer<float>& buffer);
    void dstUpdateSettings();

    //  Global Parameters
    juce::AudioParameterFloat* gain;

    //  Wind Speed Parameters
    juce::AudioParameterFloat* windSpeed;

    // Distant Wind Parameters
    juce::AudioParameterFloat* dstAmplitude;
    juce::AudioParameterFloat* dstIntensity;
    juce::AudioParameterFloat* dstResonance;

    //  Distant Wind DSP Resources
    juce::Random r;
    juce::dsp::StateVariableTPTFilter<float> dstBPF;

    //  Internal Variables
    juce::dsp::ProcessSpec currentSpec;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Wind4Unity3AudioProcessor)
};
