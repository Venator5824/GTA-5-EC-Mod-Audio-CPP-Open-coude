#pragma once
#include "TTS_Interface.h" // <-- It includes the contract.
#include <onnxruntime_cxx_api.h>

class TTSTokenizer; // Forward declaration

class VITSEngine : public ITTSEngine { // <-- It "signs" the contract.
public:
    VITSEngine();
    ~VITSEngine();

    // These functions are the required promises from the contract.
    bool LoadModel(const std::string& modelPath, const std::string& configPath) override;
    std::vector<float> Synthesize(const std::string& text, const std::string& voiceId) override;

private:
    Ort::Env* env_ = nullptr;
    Ort::Session* session_ = nullptr;
    TTSTokenizer* tokenizer_ = nullptr;
};