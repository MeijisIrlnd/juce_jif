// This is a simple example JUCE component that uses gif_load (https://github.com/hidefromkgb/gif_load) to load a GIF file 
// and play it back. This does not handle all the cases that the example demonstrates, but it's good enough for most GIFs,
// and can be easily extended to handle all types of frames.
// JUCE already has some classes for loading GIFs, but it just loads the first frame, and is not able to animate.
// There are examples of how the JUCE components can be extended to achieve this as well: https://github.com/Mrugalla/GIFLoad

#pragma once

#include <juce_graphics/juce_graphics.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4701)
#endif

#include "gif_load.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

class AnimatedGIF : public juce::Component,
    private juce::Timer
{
public:
    struct Listener
    {
        virtual ~Listener() { }
        virtual void gifPlaybackFinished() = 0;
    };

    explicit AnimatedGIF(juce::MemoryBlock gifData)
    {
        GIF_Load(gifData.getData(), (long)gifData.getSize(), Frame, nullptr, (void*)this, 0);

        const auto& [img, _] = frames.front();
        setSize(img.getWidth(), img.getHeight());
    }

    void addListener(Listener* newListener) {
        m_listener = newListener;
    }

    void paint(juce::Graphics& g) override
    {
        auto imgToDraw = frames[curIdx].first.rescaled(getWidth(), getHeight(), juce::Graphics::ResamplingQuality::highResamplingQuality);
       
    //    g.drawImage(imgToDraw, getLocalBounds().toFloat(), juce::RectanglePlacement::centred);
        g.drawImage(imgToDraw, getBounds().toFloat(), juce::RectanglePlacement::centred);
    }

    void start()
    {
        m_passedFirstFrame = false;
        if (!isTimerRunning())
        {
            showNextFrame();
        }
    }

    void stop()
    {
        stopTimer();
        curIdx = 0;
    }

private:
    static void Frame(void* data, struct GIF_WHDR* whdr) { reinterpret_cast<AnimatedGIF*>(data)->frameCallback(*whdr); }

    void frameCallback(const GIF_WHDR& whdr)
    {
        // Simplified version of this: https://github.com/hidefromkgb/gif_load#c--c-usage-example
        auto [xdim, ydim, clrs, bkgd, tran, intr, mode, frxd, fryd, frxo, fryo, time, ifrm, nfrm, bptr, cpal] = whdr;

        //fmt::print("GIF frame #{}: global size: ({}, {}), mode: {} - frame size: ({} {}), x: {}, y: {}, next delay: {}\n",
        //    ifrm, xdim, ydim, mode, frxd, fryd, frxo, fryo, time);

        juce::Image img = frames.empty()
            ? juce::Image(juce::Image::ARGB, xdim, ydim, true)
            : frames.back().first.createCopy();

        for (int y = 0; y < fryd; ++y)
        {
            for (int x = 0; x < frxd; ++x)
            {
                const auto idx = bptr[y * frxd + x];

                if (tran != (long)idx)
                {
                    const auto [r, g, b] = cpal[idx];
                    img.setPixelAt(x + frxo, y + fryo, juce::PixelARGB(0xFF, r, g, b));
                }
            }
        }

        frames.emplace_back(std::move(img), time);
    }

    void showNextFrame()
    {
        repaint();

        startTimer(frames[curIdx].second * 10);

        curIdx = (curIdx + 1) % frames.size();
        if (curIdx == 0 && m_passedFirstFrame) {
            if (m_listener != nullptr) {
                m_listener->gifPlaybackFinished();
            }
        }
        m_passedFirstFrame = true;
        

    }

    void timerCallback() override { showNextFrame(); }

    //==================================================================================================================
    size_t                             curIdx = 0;
    std::vector<std::pair<juce::Image, int>> frames;
    bool m_passedFirstFrame{ false };
    Listener* m_listener{ nullptr };
};
