#include "Parameters.h"
#include "Functions.hpp"

template<typename T>
static auto castParameter(const AudioProcessorValueTreeState& tree, 
    const ParameterID* id, T*& dest) -> void {
    dest = dynamic_cast<T*>(tree.getParameter(id->getParamID()));
    jassert(dest != nullptr);
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterFloat* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = paramObj->getDefaultValue();
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterBool* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = paramObj->getDefaultValue();
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterChoice* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = static_cast<T>(paramObj->getDefaultValue());
}

ParameterIDs Parameters::paramIDs = ParameterIDs::loadFromJSON();

Parameters::Parameters(AudioProcessorValueTreeState& tree) : tree(tree) {
    using FloatPair = std::pair<AudioParameterFloat*&, const ParameterID*>;
    using BoolPair = std::pair<AudioParameterBool*&, const ParameterID*>;
    using ChoicePair = std::pair<AudioParameterChoice*&, const ParameterID*>;

    auto floatParameters = std::vector<FloatPair>{
        {mixParam, &paramIDs.mix},
        {sampleRateParam, &paramIDs.sampleRate},
        {bitDepthParam, &paramIDs.bitDepth},
        {crushLFOAmountParam, &paramIDs.crushLFOAmount},
        {crushLFORateParam, &paramIDs.crushLFORate},
    };

    auto boolParameters = std::vector<BoolPair>{
        {crushLFOInvertParam, &paramIDs.crushLFOInvert}
    };

    auto choiceParameters = std::vector<ChoicePair>{
        {crushLFOTypeParam, &paramIDs.crushLFOType}
    };

    for (const auto& [param, paramID] : floatParameters) {
        castParameter(tree, paramID, param);
    }

    for (auto& [param, paramID] : boolParameters) {
        castParameter(tree, paramID, param);
    }

    for (auto& [param, paramID] : choiceParameters) {
        castParameter(tree, paramID, param);
    }
}

auto Parameters::createParameterLayout() -> AudioProcessorValueTreeState::ParameterLayout {
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.mix, "Mix", NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 1.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.sampleRate, "Sample Rate", NormalisableRange<float>{100.0f, 44100.0f, 1.0f, 0.3f}, 44100.0f, 
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayHz)
        .withValueFromStringFunction(Functions::parseHz)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.bitDepth, "Bit Depth", NormalisableRange<float>{1.0f, 16.0f, 1.0f}, 16.0f, 
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayBits)
        .withValueFromStringFunction(Functions::parseBits)
    ));

    layout.add(std::make_unique<AudioParameterChoice>(
        paramIDs.crushLFOType, "Crush LFO Type", StringArray{"square", "saw", "triangle", "sine"}, 0
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.crushLFORate, "Crush LFO Rate", NormalisableRange<float>{0.03125f, 4.0f, 0.0001f}, 0.25f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayLFORate)
        .withValueFromStringFunction(Functions::parseLFORate)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.crushLFOAmount, "Crush LFO Amount", NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterBool>(
        paramIDs.crushLFOInvert, "Crush LFO Invert", false
    ));

    return layout;
}

auto Parameters::getDefaultParameter(const Array<var>& args,
    WebBrowserComponent::NativeFunctionCompletion completion) -> void {

    auto paramID = args[0].toString();
    auto* param = this->tree.getParameter(paramID);
    float defaultValue = param->convertFrom0to1(param->getDefaultValue());

    completion(defaultValue);
}

auto Parameters::prepareToPlay(double sampleRate, int blockSize) noexcept -> void {
    this->sampleRate = sampleRate;
    this->blockSize = blockSize;

    double duration = 0.001;

    auto smoothers = std::vector{
        &mixSmoother,
        &sampleRateSmoother,
        &bitDepthSmoother,
        &crushLFOAmountSmoother
    };

    for (const auto& smoother : smoothers) {
        smoother->reset(this->sampleRate, duration);
    }

    this->crushLFO.prepareToPlay(this->sampleRate);
}

auto Parameters::reset() noexcept -> void {
    auto paramFloats = std::vector{
        std::pair{mixParam, &mix},
        std::pair{sampleRateParam, &sampleRateReduction},
        std::pair{bitDepthParam, &bitDepthReduction}
    };

    for (auto& [param, value] : paramFloats) {
        resetParameter(tree, param, value);
    }
    
    auto smoothers = std::vector{
        std::pair{mixParam, &mixSmoother},
        std::pair{sampleRateParam, &sampleRateSmoother},
        std::pair{bitDepthParam, &bitDepthSmoother},
        std::pair{crushLFOAmountParam, &crushLFOAmountSmoother},
    };

    for (const auto& [param, smoother] : smoothers) {
        smoother->setCurrentAndTargetValue(param->get());
    }

    this->crushLFO.reset();
}

auto Parameters::setHostInfo(double bpm, double ppq, const AudioPlayHead::TimeSignature& timeSignature) noexcept -> void {
    this->bpm = bpm;
    this->ppq = ppq;
    this->timeSignature = timeSignature;

    if (ppq > 0.0) {
        this->ppq = ppq;
        this->internalPPQ = ppq;
    } else {
        double ppqPerSample = (this->bpm / 60.0) / this->sampleRate;
        this->internalPPQ += ppqPerSample * this->blockSize; 
        this->ppq = this->internalPPQ;
    }

    this->crushLFO.syncToHost(this->bpm, this->ppq, this->timeSignature);
}

auto Parameters::blockUpdate() noexcept -> void {
    auto smoothers = std::vector{
        std::pair{mixParam, &mixSmoother},
        std::pair{sampleRateParam, &sampleRateSmoother},
        std::pair{bitDepthParam, &bitDepthSmoother},
        std::pair{crushLFOAmountParam, &crushLFOAmountSmoother}
    };

    for (const auto& [param, smoother] : smoothers) {
        smoother->setTargetValue(param->get());
    }

    this->crushLFO.setType(this->crushLFOTypeParam->getCurrentChoiceName());
    this->crushLFO.setSyncedRate(this->crushLFORateParam->get());
    this->crushLFO.setPhaseInvert(this->crushLFOInvertParam->get());
}

auto Parameters::update() noexcept -> void {
    this->mix = mixSmoother.getNextValue();
    this->sampleRateReduction = sampleRateSmoother.getNextValue();
    this->bitDepthReduction = bitDepthSmoother.getNextValue();

    float crushLFOValue = this->crushLFO.getSample();
    float crushLFOAmount = this->crushLFOAmountSmoother.getNextValue();
    this->sampleRateReduction *= jmap(crushLFOValue, -1.0f, 1.0f, 1.0f - crushLFOAmount, 1.0f);
}