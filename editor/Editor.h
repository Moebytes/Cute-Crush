#pragma once
#pragma clang diagnostic ignored "-Wshadow-field"
#include <JuceHeader.h>
#include "Processor.h"
#include "EventEmitter.hpp"

class Editor : public AudioProcessorEditor, public EventEmitter::Listener {
public:
    Editor(Processor& p);
    ~Editor() override;
    
    auto resized() -> void override;

    auto getResource(const String& url) -> std::optional<WebBrowserComponent::Resource>;
    auto webviewOptions() -> WebBrowserComponent::Options;
    auto getWebviewFileBytes(const String& resourceStr) -> std::vector<std::byte>;

    auto handleEvent(const String& name, const var& payload) -> void override;
    auto handleThemeChange(const String& theme) -> void;
        
private:
    Processor& processor;
    ComponentBoundsConstrainer constrainer;

    WebSliderRelay mixRelay {Parameters::paramIDs.mix.getParamID()};
    WebSliderParameterAttachment mixAttachment {*this->processor.parameters.mixParam, mixRelay, nullptr};

    WebSliderRelay sampleRateRelay {Parameters::paramIDs.sampleRate.getParamID()};
    WebSliderParameterAttachment sampleRateAttachment {*this->processor.parameters.sampleRateParam, sampleRateRelay, nullptr};

    WebSliderRelay bitDepthRelay {Parameters::paramIDs.bitDepth.getParamID()};
    WebSliderParameterAttachment bitDeptheAttachment {*this->processor.parameters.bitDepthParam, bitDepthRelay, nullptr};

    WebComboBoxRelay crushLFOTypeRelay {Parameters::paramIDs.crushLFOType.getParamID()};
    WebComboBoxParameterAttachment crushLFOTypeAttachment {*this->processor.parameters.crushLFOTypeParam, crushLFOTypeRelay, nullptr};
    WebSliderRelay crushLFORateRelay {Parameters::paramIDs.crushLFORate.getParamID()};
    WebSliderParameterAttachment crushLFORateAttachment {*this->processor.parameters.crushLFORateParam, crushLFORateRelay, nullptr};
    WebSliderRelay crushLFOAmountRelay {Parameters::paramIDs.crushLFOAmount.getParamID()};
    WebSliderParameterAttachment crushLFOAmountAttachment {*this->processor.parameters.crushLFOAmountParam, crushLFOAmountRelay, nullptr};
    WebToggleButtonRelay crushLFOInvertRelay {Parameters::paramIDs.crushLFOInvert.getParamID()};
    WebToggleButtonParameterAttachment crushLFOInvertAttachment {*this->processor.parameters.crushLFOInvertParam, crushLFOInvertRelay, nullptr};

    WebBrowserComponent webview;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};