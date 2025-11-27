#include "pch.h"
#include "main.h"
#include "ConfigReader.h"
#include "SharedData.h"

// WICHTIG: Sherpa Header muss hier bekannt sein. 
// Falls er nicht in "main.h" ist, füge ihn hier hinzu:
#include "c-api.h" 
// Oder einfach: #include "c-api.h" (je nach deinem Include Pfad)

#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

#pragma comment(lib, "winmm.lib")

// Namespace Alias für C++17 Filesystem
namespace fs = std::filesystem;

// ------------------------------------------------------------
// Globale Variablen
// ------------------------------------------------------------
VoiceBridge* bridge = nullptr;
const SherpaOnnxOfflineTts* tts = nullptr;
SherpaOnnxOfflineTtsConfig sherpaConfig;
int fileCounter = 0;

struct SherpaVoiceParam {
    int speakerId;
};

static std::map<std::string, SherpaVoiceParam> g_SpeakerMap = {
    {"1", {0}}, {"2", {1}}, {"3", {2}}, {"4", {3}}
};

const std::string DEFAULT_REL_PATH = "models\\vits-piper-en_US-libritts_r-medium";
const std::string DEFAULT_MODEL_NAME = "en_US-libritts_r-medium.onnx";

// ------------------------------------------------------------
// Helper: WAV Header Writer
// ------------------------------------------------------------
void WriteWavFile(const std::string& filename, const float* samples, int sampleCount, int sampleRate) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return;

    std::vector<int16_t> pcm(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        float s = samples[i];
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
        pcm[i] = static_cast<int16_t>(s * 32767.0f);
    }

    int dataSize = sampleCount * 2;
    int headerSize = 44; // Variable nicht genutzt, aber für Lesbarkeit ok

    file.write("RIFF", 4);
    int chunkSize = 36 + dataSize;
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    int subchunk1Size = 16;
    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    int16_t audioFormat = 1;
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    int16_t numChannels = 1;
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    int byteRate = sampleRate * numChannels * 2;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    int16_t blockAlign = numChannels * 2;
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    int16_t bitsPerSample = 16;
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);
    file.write(reinterpret_cast<const char*>(pcm.data()), dataSize);
    file.close();
}

void LogAudio(const std::string& msg) {
    std::ofstream log("kkamel_audio.log", std::ios_base::app);
    if (log.is_open()) log << msg << "\n";
}

bool IsValidModelPath(const std::string& folder, const std::string& filename, std::string& outFullModelPath, std::string& outFullTokensPath) {
    try {
        if (folder.empty() || filename.empty()) return false;
        fs::path f = fs::path(folder);
        fs::path m = f / filename;
        fs::path t = f / "tokens.txt";
        if (fs::exists(m) && fs::exists(t)) {
            outFullModelPath = m.string();
            outFullTokensPath = t.string();
            return true;
        }
    }
    catch (...) {}
    return false;
}

// ------------------------------------------------------------
// Init
// ------------------------------------------------------------
void InitSherpa() {
    LogAudio("InitSherpa...");
    try { ConfigReader::LoadAllConfigs(); }
    catch (...) {}

    std::string iniFolder = ConfigReader::g_Settings.TTS_MODEL_PATH;
    std::string iniName = ConfigReader::g_Settings.TTS_MODEL_ALT_NAME;
    if (iniName.empty()) iniName = DEFAULT_MODEL_NAME;

    std::string finalModelPath, finalTokensPath, finalDataDir;
    bool found = false;

    if (!found && IsValidModelPath(iniFolder, iniName, finalModelPath, finalTokensPath)) found = true;
    if (!found && IsValidModelPath(DEFAULT_REL_PATH, DEFAULT_MODEL_NAME, finalModelPath, finalTokensPath)) found = true;
    if (!found && IsValidModelPath(".", iniName, finalModelPath, finalTokensPath)) found = true;

    if (!found) { LogAudio("FATAL: TTS Modell nicht gefunden!"); return; }

    fs::path p(finalModelPath);
    finalDataDir = (p.parent_path() / "espeak-ng-data").string();

    memset(&sherpaConfig, 0, sizeof(sherpaConfig));
    sherpaConfig.model.vits.model = _strdup(finalModelPath.c_str());
    sherpaConfig.model.vits.tokens = _strdup(finalTokensPath.c_str());
    sherpaConfig.model.vits.data_dir = _strdup(finalDataDir.c_str());
    sherpaConfig.model.num_threads = 2;
    sherpaConfig.model.provider = "cpu";
    sherpaConfig.model.debug = 0;

    tts = SherpaOnnxCreateOfflineTts(&sherpaConfig);
    if (tts) LogAudio("Sherpa gestartet.");
    else LogAudio("Sherpa Start fehlgeschlagen.");
}

// ------------------------------------------------------------
// Generate
// ------------------------------------------------------------
void GenerateAndPlayAudio(const std::string& text, const std::string& voiceId, float speed) {
    if (!tts) return;

    int speakerId = 0;
    if (g_SpeakerMap.count(voiceId)) speakerId = g_SpeakerMap[voiceId].speakerId;

    const SherpaOnnxGeneratedAudio* audio = SherpaOnnxOfflineTtsGenerate(tts, text.c_str(), speakerId, speed);

    if (audio && audio->n > 0) {
        std::string filename = "temp_tts.wav";
        fs::path outPath = fs::current_path() / filename;

        WriteWavFile(outPath.string(), audio->samples, audio->n, audio->sample_rate);
        PlaySoundA(outPath.string().c_str(), NULL, SND_FILENAME | SND_SYNC);

        // HIER WAR DER FEHLER: Sicherstellen, dass Sherpa C-API eingebunden ist
        SherpaOnnxDestroyOfflineTtsGeneratedAudio(audio);
    }
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
void ScriptMain() {
    bridge = new VoiceBridge(false);
    if (!bridge->IsConnected()) return;

    InitSherpa();

    while (true) {
        std::string text, voice;
        float speed;
        // CheckForJob gibt bool zurück
        if (bridge->CheckForJob(text, voice, speed)) {
            bridge->SetTalkingState(true);
            GenerateAndPlayAudio(text, voice, speed);
            bridge->SetTalkingState(false);
        }
        Sleep(10);
    }
    delete bridge;
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        scriptRegister(hInstance, ScriptMain);
        break;
    case DLL_PROCESS_DETACH:
        scriptUnregister(hInstance);
        if (bridge) delete bridge;
        if (tts) SherpaOnnxDestroyOfflineTts(tts);
        break;
    }
    return TRUE;
}