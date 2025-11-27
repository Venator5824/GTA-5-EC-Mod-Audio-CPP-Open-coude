#pragma once

// --- A. SYSTEM INCLUDES ---
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

// ScriptHookV Basis-Typen
#include "types.h" 
#include "SharedData.h"

// ------------------------------------------------------------
// B. KERN-STRUKTUREN
// ------------------------------------------------------------
struct NpcPersona {
    Hash hash = 0;
    std::string modelName = "";
    bool isHuman = false;
    std::string inGameName = "";
    std::string type = "";
    std::string relationshipGroup = "";
    std::string subGroup = "";
    std::string gender = "";
    std::string behaviorTraits = "";
    std::vector<int> customVoiceIDs;
};

struct VoiceMap {
    std::vector<int> MaleIDs;
    std::vector<int> FemaleIDs;
};

// ------------------------------------------------------------
// C. MOD SETTINGS
// ------------------------------------------------------------
struct ModSettings {
    bool Enabled = false;
    int ActivationKey = 0x54;
    int ActivationDurationMs = 1000;
    int StopKey_Primary = 0x1B;
    int StopKey_Secondary = 0;
    int StopDurationMs = 3000;
    int MaxInputChars = 786;
    float MaxConversationRadius = 3.0f;
    int MaxOutputChars = 512;
    int MinResponseDelayMs = 750;
    int MaxHistoryTokens = 1024;
    int DeletionTimer = 120;
    int MaxNpcGetModel = 1;
    int MaxChatHistoryLines = 16;
    int USE_GPU_LAYERS = 33;
    bool USE_VRAM_PREFERED = true;
    int StT_Enabled = 1;
    int TtS_Enabled = 0;
    int AUDIO_USE_GPU_LAYERS = 0;
    std::string MODEL_PATH = "";
    std::string MODEL_ALT_NAME = "";
    int StTRB_Activation_Key = 0;
    int DEBUG_LEVEL = 2;
    std::string LOG_NAME = "kkamel.log";
    std::string LOG2_NAME = "kkamel_metrics.log";
    std::string LOG3_NAME = "kkamel_timers.log";
    std::string LOG4_NAME = "kkamel_loader.log";
    std::string STT_MODEL_PATH = "";
    std::string STT_MODEL_ALT_NAME = "";
    std::string TTS_MODEL_PATH = "";
    std::string TTS_MODEL_ALT_NAME = "";
};

// ------------------------------------------------------------
// D. GLOBALE DEKLARATIONEN
// ------------------------------------------------------------
extern ModSettings g_ModSettings;
extern VoiceMap g_CuratedVoiceMap;

// ------------------------------------------------------------
// E. CONFIG READER KLASSE
// ------------------------------------------------------------
class ConfigReader {
public:
    // Caches
    static ModSettings g_Settings;
    static std::map<Hash, NpcPersona> g_PersonaCache;
    static std::map<std::string, NpcPersona> g_DefaultTypeCache;
    static std::map<std::string, std::string> g_RelationshipMatrix;
    static std::map<std::string, std::string> g_ZoneContextCache;
    static std::map<std::string, std::string> g_OrgContextCache;
    static std::string g_GlobalContextStyle;
    static std::string g_GlobalContextTimeEra;
    static std::string g_GlobalContextLocation;
    static std::string g_ContentGuidelines;
    static std::vector<std::string> g_chat_history;

    // Public API
    static void LoadAllConfigs();
    static NpcPersona GetPersona(Ped npc);
    static std::string GetRelationship(const std::string& npcSubGroup, const std::string& playerSubGroup);
    static std::string GetZoneContext(const std::string& zoneName);
    static std::string GetOrgContext(const std::string& orgName);

    // Helper (Passend zur ConfigReader.cpp gemacht!)
    static std::string GetValueFromINI(const char* iniPath, const std::string& section, const std::string& key, const std::string& defaultValue = "");

    // KORREKTUR HIER: 3 Argumente, damit es zur CPP passt
    static void LoadINISectionToCache(const char* iniPath, const std::string& section, std::map<std::string, std::string>& cache);

    // KORREKTUR HIER: 2 Argumente
    static std::string GetSetting(const std::string& section, const std::string& key);

private:
    // Core Ladefunktionen
    static void LoadPersonaDatabase();
    static void LoadRelationshipDatabase();
    static void LoadWorldContextDatabase();
    static void LoadVoiceMap(); // Implementierung fehlt in deiner CPP, aber Deklaration ist ok

    // Weitere Helper
    static Hash GetHashFromHex(const std::string& hexString);
    static int KeyNameToVK(const std::string& keyName);
    static std::vector<std::string> SplitString(const std::string& str, char delimiter);
    static std::vector<int> ParseInts(const std::string& str);
};


//EOF