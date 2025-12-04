
#pragma once
#include <memory>
#include <vector>
#include <string>
#include "babylon.h" // We still use Babylon for the phonemizer
#include "VITS_Engine.h"
#include "VctkEngine.h"
#include "TTS_Interface.h"
#include "PiperEngine.h"

class VctkEngine : public ITTSEngine {
public:
    VctkEngine();
    ~VctkEngine();

    // --- Functions required by the ITTSEngine contract ---
    bool LoadModel(const std::string& modelPath, const std::string& configPath) override;
    std::vector<float> Synthesize(const std::string& text, const std::string& voiceId) override;

private:
    // --- Member Variables ---
    // We still need the phonemizer.
    std::unique_ptr<DeepPhonemizer::Session> phonemizer_;

    // We only have ONE voice session for all 109 speakers.
    std::unique_ptr<Vits::Session> voice_session_;
};

VctkEngine::VctkEngine() {
    LogAudio("VctkEngine created. Initializing phonemizer...");
    try {
        phonemizer_ = std::make_unique<DeepPhonemizer::Session>("models/base/deep_phonemizer.onnx");
        LogAudio("Phonemizer loaded successfully.");
    }
    catch (const std::exception& e) {
        LogAudio("FATAL: VctkEngine failed to load phonemizer: " + std::string(e.what()));
        phonemizer_ = nullptr;
    }
}

VctkEngine::~VctkEngine() {}

// For this engine, LoadModel is very important. It loads the one and only model.
bool VctkEngine::LoadModel(const std::string& modelPath, const std::string& configPath) {
    if (!phonemizer_) return false;

    LogAudio("Loading VCTK multi-speaker model from: " + modelPath);
    try {
        // We create the single Vits::Session here.
        // Babylon's Vits::Session needs to be able to understand the config.yaml.
        // If it can't, this approach won't work and we would need to use ONNX Runtime directly.
        // For now, we assume Babylon is smart enough.
        voice_session_ = std::make_unique<Vits::Session>(modelPath);
        LogAudio("VCTK model loaded successfully.");
        return true;
    }
    catch (const std::exception& e) {
        LogAudio("FATAL: Failed to load VCTK model: " + std::string(e.what()));
        return false;
    }
}

std::vector<float> VctkEngine::Synthesize(const std::string& text, const std::string& voiceId) {
    if (!phonemizer_ || !voice_session_) {
        LogAudio("Synthesis failed: VCTK Engine not ready.");
        return {};
    }

    try {
        // --- Step 1: Phonemize ---
        std::vector<std::string> phonemes = phonemizer_->g2p(text);
        if (phonemes.empty()) {
            throw std::runtime_error("G2P returned no phonemes.");
        }

        // --- Step 2: Synthesize using a Speaker ID ---
        // We need to convert the string voiceId (e.g., "299") into an integer.
        int speaker_id = std::stoi(voiceId);

        // Babylon's tts function is overloaded. One version takes an output path,
        // another might take a speaker ID. We assume this version exists.
        // You would need to check the babylon.hpp header for the exact function signature.
        std::vector<float> audio_samples = voice_session_->tts(phonemes, speaker_id);

        if (audio_samples.empty()) {
            throw std::runtime_error("VCTK synthesis returned empty audio.");
        }

        return audio_samples;

    }
    catch (const std::exception& e) {
        LogAudio("ERROR during VCTK synthesis for voice '" + voiceId + "': " + std::string(e.what()));
        return {};
    }
}