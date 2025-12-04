
#include "main.h" // For logging, etc.
#include "SharedData.h"
#include <memory> // For std::unique_ptr

// Include your new engine
#include "TTS_Interface.h"
#include "VITS_Engine.h"

// --- GLOBALS ---
VoiceBridge* bridge = nullptr;
std::unique_ptr<ITTSEngine> g_tts_engine; // The smart pointer to our engine

// --- The factory that creates the correct engine ---
void InitInferenceEngine() {
    LogAudio("=== Init TTS Engine START ===");

    // For now, we only support VITS. This is where you'd read the INI in the future.
    g_tts_engine = std::make_unique<VITSEngine>();

    // TODO: Get these paths from your INI file instead of hardcoding
    std::string modelPath = "vctk-vits-onnx/model.onnx";
    std::string configPath = "vctk-vits-onnx/config.yaml";

    if (!g_tts_engine->LoadModel(modelPath, configPath)) {
        LogAudio("FATAL: Engine failed to load.");
        bridge->SetError("TTS Model Load Fail");
        g_tts_engine.reset(); // Destroy the failed engine
    }
    else {
        LogAudio("=== Engine initialized successfully ===");
    }
}

// --- Your Generate and Play function is now much simpler ---
bool GenerateAndPlayAudio(const std::string& text, const std::string& voiceId, float speed) {
    if (!g_tts_engine) return false;
    LogAudio("Processing: " + text);

    // 1. Synthesize
    std::vector<float> audio_samples = g_tts_engine->Synthesize(text, voiceId);
    if (audio_samples.empty()) {
        LogAudio("Synthesis returned no audio.");
        return false;
    }

    // 2. Play
    // Your existing code to save the vector<float> to a temp WAV and play it.
    std::string filename = "temp_tts.wav";
    WriteWavFile(filename, audio_samples.data(), audio_samples.size(), 22050); // Use correct sample rate

    bridge->SetState(AudioJobState::PLAYING);
    BOOL playResult = PlaySoundA(filename.c_str(), NULL, SND_FILENAME | SND_SYNC);

    return playResult != 0;
}

// --- Your ScriptMain is mostly unchanged, just calls the new Init ---
void ScriptMain() {
    LogAudio("ScriptMain started");
    bridge = new VoiceBridge(false);
    if (!bridge->IsConnected()) { /* ... */ return; }

    InitInferenceEngine(); // Call the new initialization function

    if (!g_tts_engine) { /* ... handle error ... */ return; }

    bridge->SetState(AudioJobState::IDLE);

    while (true) {
        std::string text, voice;
        float speed;
        if (bridge->CheckForJob(text, voice, speed)) {
            if (GenerateAndPlayAudio(text, voice, speed)) {
                bridge->CompleteJob();
            }
            else {
                bridge->SetError("Gen Failed");
            }
        }
        Sleep(50);
    }
}

// DllMain remains the same, but cleans up the smart pointer
BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        scriptRegister(hInstance, ScriptMain);
        break;
    case DLL_PROCESS_DETACH:
        scriptUnregister(hInstance);
        g_tts_engine.reset(); // Smart pointer handles cleanup
        if (bridge) delete bridge;
        break;
    }
    return TRUE;
}