/*
  ==============================================================================

    GifLooper.h
    Created: 4 Dec 2022 5:27:09pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include "AnimatedGIF.h"
#include "Macros.h"
namespace Puritan::UI
{
    class GIFLooper : public juce::Component, public AnimatedGIF::Listener
    {
    public: 
        GIFLooper(const juce::MemoryBlock& gifData) : m_gif(gifData) {
            m_gif.addListener(this);
            addAndMakeVisible(&m_gif);
            setInterceptsMouseClicks(false, false);
        }
        
        ~GIFLooper() override {} 

        void gifPlaybackFinished() override
        {
            m_gif.start();
        }

        void paint(JIF_UNUSED juce::Graphics& g) override {}

        void resized() override
        {
            m_gif.setBounds(getLocalBounds());
        }

        void start()
        {
            m_gif.start();
        }

        void stop()
        {
            m_gif.stop();
        }

        void setAlphaMultiplier(double newMultiplier) {
            m_gif.setAlpha(newMultiplier);
        }
    private: 
        AnimatedGIF m_gif;
    };
}