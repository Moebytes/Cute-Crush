#pragma once
#include <cmath>
#include <JuceHeader.h>

class Bitcrusher {
public:
    Bitcrusher() = default;
    virtual ~Bitcrusher() = default;

    auto prepareToPlay(double sampleRate) -> void {
        this->sampleRate = sampleRate;
        reset();
    }

    auto reset() -> void {
        this->phase = 0.0f;
        this->heldL = 0.0f;
        this->heldR = 0.0f;
    }

    auto sampleAndHold(float inputL, float inputR, float targetRate) -> void {
        targetRate = juce::jlimit(1.0f, static_cast<float>(this->sampleRate), targetRate);

        this->phase += targetRate / static_cast<float>(this->sampleRate);

        if (this->phase >= 1.0f) {
            this->phase -= 1.0f;

            this->heldL = inputL;
            this->heldR = inputR;
        }
    }

    auto quantize(float sample, float bitDepth) -> float {
        bitDepth = juce::jmax(1.0f, bitDepth);

        float levels = std::pow(2.0f, std::round(bitDepth));

        return std::round(sample * levels) / levels;
    }

    auto processSample(float& left, float& right, float rateReduction, float bitReduction) -> void {
        this->sampleAndHold(left, right, rateReduction);
        left = this->quantize(heldL, bitReduction);
        right = this->quantize(heldR, bitReduction);
    }

private:
    double sampleRate = 44100.0;

    float phase = 0.0f;
    float heldL = 0.0f;
    float heldR = 0.0f;
};