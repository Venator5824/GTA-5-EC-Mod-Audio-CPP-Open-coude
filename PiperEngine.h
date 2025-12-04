
#pragma once
#include "TTS_Interface.h" // The "contract" for all engines
#include <memory> // for std::unique_ptr
#include <map>
#include "babylon.h" // The main header from the babylon.lib you built

class PiperEngine : public ITTSEngine {
public:
    PiperEngine();
    ~PiperEngine();

    // --- Functions required by the ITTSEngine contract ---
    bool LoadModel(const std::string& modelPath, const std::string& configPath) override;
    std::vector<float> Synthesize(const std::string& text, const std::string& voiceId) override;

private:
    // --- Member Variables ---
    // Machine #1: The universal text-to-phoneme converter
    std::unique_ptr<DeepPhonemizer::Session> phonemizer_;

    // Machine #2: A collection of loaded voices
    // We map a voice name (e.g., "amy") to its loaded VITS session
    std::map<std::string, std::unique_ptr<Vits::Session>> voices_;
};