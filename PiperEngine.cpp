
#include "PiperEngine.h"
#include "TTS_Interface.h"
#include "VITS_Engine.h"
#include "SharedData.h"
#include "main.h" // Assuming LogAudio is declared here

PiperEngine::PiperEngine() {
    LogAudio("PiperEngine created. Initializing phonemizer...");
    try {
        // We assume the phonemizer model is always in this relative path
        const char* phonemizer_path = "models/base/deep_phonemizer.onnx";
        phonemizer_ = std::make_unique<DeepPhonemizer::Session>(phonemizer_path);
        LogAudio("Phonemizer loaded successfully.");
    }
    catch (const std::exception& e) {
        LogAudio("FATAL: PiperEngine failed to load phonemizer: " + std::string(e.what()));
        phonemizer_ = nullptr; // Ensure it's null on failure
    }
}

PiperEngine::~PiperEngine() {} // std::unique_ptr handles all cleanup automatically

// This function is not really needed for this engine type, since voices are loaded on-demand.
// We implement it to satisfy the contract.
bool PiperEngine::LoadModel(const std::string& modelPath, const std::string& configPath) {
    // We can use this to preload a default voice if we want, but it's optional.
    // For now, we just check if the phonemizer is ready.
    return phonemizer_ != nullptr;
}

std::vector<float> PiperEngine::Synthesize(const std::string& text, const std::string& voiceId) {
    if (!phonemizer_) {
        LogAudio("Synthesis failed: Phonemizer is not loaded.");
        return {};
    }

    try {
        // --- Step 1: Load the requested voice if it's the first time we've seen it ---
        if (voices_.find(voiceId) == voices_.end()) {
            LogAudio("Loading new Piper voice on-demand: " + voiceId);
            std::string modelPath = "models/" + voiceId + "/" + voiceId + ".onnx";

            // This is where the magic happens. We create a new Vits::Session for this voice.
            // Babylon's Vits::Session class knows how to find and read the associated .json file.
            voices_[voiceId] = std::make_unique<Vits::Session>(modelPath);
            LogAudio("Voice '" + voiceId + "' loaded and cached.");
        }

        // --- Step 2: Use the Phonemizer (Machine #1) ---
        std::vector<std::string> phonemes = phonemizer_->g2p(text);
        if (phonemes.empty()) {
            throw std::runtime_error("G2P returned no phonemes for text: " + text);
        }

        // Babylon may return phonemes as strings. The Vits::Session expects them this way.
        // We don't need to convert them to IDs ourselves.

        // --- Step 3: Use the Voice Synthesizer (Machine #2) ---
        Vits::Session* voice_session = voices_[voiceId].get();

        // This is the core synthesis call. Babylon handles everything internally:
        // running the ONNX model and returning the audio data.
        std::vector<float> audio_samples = voice_session->tts(phonemes);

        if (audio_samples.empty()) {
            throw std::runtime_error("VITS synthesis returned empty audio.");
        }

        LogAudio("Successfully synthesized " + std::to_string(audio_samples.size()) + " audio samples.");
        return audio_samples;

    }
    catch (const std::exception& e) {
        LogAudio("ERROR during Piper synthesis for voice '" + voiceId + "': " + std::string(e.what()));
        return {};
    }
}