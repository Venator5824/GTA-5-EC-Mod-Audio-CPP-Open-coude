// ConfigReader.cpp v1.0.14
// BASIERT AUF v1.0.4/1.0.8
// FIX: GetValueFromINI bereinigt jetzt Kommentare
#define _CRT_SECURE_NO_WARNINGS
#include "ConfigReader.h"
#include "main.h"




// const std::string LOG_FILE_NAME4 = ConfigReader::g_Settings.LOG4_NAME;

// --- LOGGING ---
void LogConfig(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ofstream log("kkamel_loader.log", std::ios_base::app);
    if (log.is_open()) {
        log << "[" << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << "] [ConfigReader] " << message << "\n";
        log.flush();
    }
}

// --- INITIALIZATION OF STATIC CACHES AND PATHS ---
const char* SETTINGS_INI_PATH = ".\\GTA_LLM_Settings.ini";
const char* RELATIONSHIPS_INI_PATH = ".\\GTA_LLM_Relationships.ini";
const char* PERSONAS_INI_PATH = ".\\GTA_LLM_Personas.ini";

// Initialization of static members
ModSettings ConfigReader::g_Settings;
std::map<Hash, NpcPersona> ConfigReader::g_PersonaCache;
std::map<std::string, NpcPersona> ConfigReader::g_DefaultTypeCache;
std::map<std::string, std::string> ConfigReader::g_RelationshipMatrix;
std::map<std::string, std::string> ConfigReader::g_ZoneContextCache;
std::map<std::string, std::string> ConfigReader::g_OrgContextCache;
std::string ConfigReader::g_GlobalContextStyle;
std::string ConfigReader::g_GlobalContextTimeEra;
std::string ConfigReader::g_GlobalContextLocation;
std::string ConfigReader::g_ContentGuidelines;
//std::vector<std::string> ConfigReader::g_chat_history;

// --- HELPER FUNCTION IMPLEMENTATIONS ---
std::string ConfigReader::GetValueFromINI(const char* iniPath, const std::string& section, const std::string& key, const std::string& defaultValue) {
    char temp[2048];
    DWORD result = GetPrivateProfileStringA(section.c_str(), key.c_str(), defaultValue.c_str(), temp, sizeof(temp), iniPath);
    std::string value = (result == 0) ? defaultValue : std::string(temp);
    size_t commentPos = value.find(';');
    if (commentPos != std::string::npos) {
        value = value.substr(0, commentPos);
    }
    value.erase(0, value.find_first_not_of(" \t\r\n"));
    value.erase(value.find_last_not_of(" \t\r\n") + 1);
    return value;
}

std::vector<std::string> ConfigReader::SplitString(const std::string& str, char delimiter) {
    LogConfig("SplitString called with string: " + str + ", delimiter: " + delimiter);
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    LogConfig("SplitString returned " + std::to_string(tokens.size()) + " tokens");
    return tokens;
}

int ConfigReader::KeyNameToVK(const std::string& keyName) {
    LogConfig("KeyNameToVK called with keyName: " + keyName);
    std::string upperKey = keyName;
    std::transform(upperKey.begin(), upperKey.end(), upperKey.begin(), ::toupper);
    if (upperKey.length() == 1 && upperKey[0] >= 'A' && upperKey[0] <= 'Z') {
        int vk_code = upperKey[0];
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_" + upperKey + " (" + std::to_string(vk_code) + ")");
        return vk_code;
    }
    if (upperKey.length() == 1 && upperKey[0] >= '0' && upperKey[0] <= '9') {
        int vk_code = upperKey[0];
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_" + upperKey + " (" + std::to_string(vk_code) + ")");
        return vk_code;
    }
    if (upperKey == "ESC" || upperKey == "VK_ESCAPE") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_ESCAPE");
        return VK_ESCAPE;
    }
    if (upperKey == "F1" || upperKey == "VK_F1") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F1");
        return VK_F1;
    }
    if (upperKey == "F2" || upperKey == "VK_F2") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F2");
        return VK_F2;
    }
    if (upperKey == "F3" || upperKey == "VK_F3") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F3");
        return VK_F3;
    }
    if (upperKey == "F4" || upperKey == "VK_F4") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F4");
        return VK_F4;
    }
    if (upperKey == "F5" || upperKey == "VK_F5") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F5");
        return VK_F5;
    }
    if (upperKey == "F6" || upperKey == "VK_F6") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F6");
        return VK_F6;
    }
    if (upperKey == "F7" || upperKey == "VK_F7") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F7");
        return VK_F7;
    }
    if (upperKey == "F8" || upperKey == "VK_F8") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F8");
        return VK_F8;
    }
    if (upperKey == "F9" || upperKey == "VK_F9") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F9");
        return VK_F9;
    }
    if (upperKey == "F10" || upperKey == "VK_F10") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F10");
        return VK_F10;
    }
    if (upperKey == "F11" || upperKey == "VK_F11") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F11");
        return VK_F11;
    }
    if (upperKey == "F12" || upperKey == "VK_F12") {
        LogConfig("KeyNameToVK mapped " + keyName + " to VK_F12");
        return VK_F12;
    }
    LogConfig("KeyNameToVK FAILED. Unmapped key: " + keyName + ". Returning 0.");
    return 0;
}

Hash ConfigReader::GetHashFromHex(const std::string& hexString) {
    if (hexString.empty() || (hexString.rfind("0x", 0) != 0 && hexString.rfind("0X", 0) != 0)) {
        LogConfig("GetHashFromHex returned 0 (invalid hex string format)");
        return 0;
    }
    try {
        unsigned long hash = std::stoul(hexString, nullptr, 16);
        return static_cast<Hash>(hash);
    }
    catch (const std::exception& e) {
        LogConfig("GetHashFromHex caught exception: " + std::string(e.what()) + ", returning 0");
        return 0;
    }
}

void ConfigReader::LoadINISectionToCache(const char* iniPath, const std::string& section, std::map<std::string, std::string>& cache) {
    LogConfig("LoadINISectionToCache called for INI: " + std::string(iniPath) + ", section: " + section);
    const int bufferSize = 32767;
    std::vector<char> buffer(bufferSize);
    DWORD bytesRead = GetPrivateProfileSectionA(section.c_str(), buffer.data(), bufferSize, iniPath);
    if (bytesRead == 0 || bytesRead >= bufferSize - 2) {
        LogConfig("LoadINISectionToCache: No data read or buffer too small for section " + section);
        return;
    }
    int entries = 0;
    for (char* pKey = buffer.data(); *pKey; ) {
        std::string keyValue(pKey);
        size_t separatorPos = keyValue.find('=');
        if (separatorPos == std::string::npos) {
            separatorPos = keyValue.find(':');
        }
        if (separatorPos != std::string::npos) {
            std::string key = keyValue.substr(0, separatorPos);
            std::string value = keyValue.substr(separatorPos + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (!key.empty() && !value.empty()) {
                cache[key] = value;
                entries++;
            }
        }
        pKey += (keyValue.length() + 1);
    }
    LogConfig("LoadINISectionToCache: Loaded " + std::to_string(entries) + " entries for section " + section);
}

// --- CORE LOADING FUNCTIONS ---
void ConfigReader::LoadWorldContextDatabase() {
    LogConfig("LoadWorldContextDatabase started");
    g_GlobalContextStyle = GetValueFromINI(SETTINGS_INI_PATH, "GLOBAL_CONTEXT", "STYLE");
    g_GlobalContextTimeEra = GetValueFromINI(SETTINGS_INI_PATH, "GLOBAL_CONTEXT", "TIME_ERA");
    g_GlobalContextLocation = GetValueFromINI(SETTINGS_INI_PATH, "GLOBAL_CONTEXT", "LOCATION");
    LoadINISectionToCache(SETTINGS_INI_PATH, "KEY_ORGANIZATIONS", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "CITY_CONTEXT", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "MEDIA_AND_CULTURE", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "POLITICS", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "ECONOMY_AND_BRANDS", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "ENTERTAINMENT", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "HEALTH_AND_ISSUES", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "COMPANIES", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "GANGS", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "CELEBRITIES", g_ZoneContextCache);
    LoadINISectionToCache(SETTINGS_INI_PATH, "NORTH_YANKTON", g_ZoneContextCache);
    LogConfig("LoadWorldContextDatabase completed");
}

void ConfigReader::LoadRelationshipDatabase() {
    LogConfig("LoadRelationshipDatabase started");
    const int bufferSize = 65536;
    std::vector<char> sectionNamesBuffer(bufferSize);
    DWORD bytesRead = GetPrivateProfileSectionNamesA(sectionNamesBuffer.data(), bufferSize, RELATIONSHIPS_INI_PATH);
    if (bytesRead == 0 || bytesRead >= bufferSize - 2) {
        LogConfig("LoadRelationshipDatabase: No sections found or buffer too small for " + std::string(RELATIONSHIPS_INI_PATH));
        return;
    }
    int sectionCount = 0;
    for (char* pSection = sectionNamesBuffer.data(); *pSection; ) {
        std::string sectionName(pSection);
        if (sectionName == "RELATIONSHIPS" || sectionName == "TYPES" || sectionName == "GENDERS" ||
            sectionName == "GANG_SUBGROUPS" || sectionName == "LAW_SUBGROUPS" ||
            sectionName == "PRIVATE_SUBGROUPS" || sectionName == "BUSINESS_SUBGROUPS") {
            pSection += (sectionName.length() + 1);
            continue;
        }
        std::map<std::string, std::string> sectionCache;
        LoadINISectionToCache(RELATIONSHIPS_INI_PATH, sectionName, sectionCache);
        for (const auto& pair : sectionCache) {
            std::string key = sectionName + ":" + pair.first;
            g_RelationshipMatrix[key] = pair.second;
        }
        sectionCount++;
        pSection += (sectionName.length() + 1);
    }
    // LogConfig("LoadRelationshipDatabase: Loaded " + std::to_string(sectionCount) + " character/group sections");
}

std::string ConfigReader::GetSetting(const std::string& section, const std::string& key) {
    std::string value = GetValueFromINI(SETTINGS_INI_PATH, section, key);
    return value;
}

void ConfigReader::LoadPersonaDatabase() {
    LogConfig("LoadPersonaDatabase started");
    const int bufferSize = 65536;
    std::vector<char> sectionNamesBuffer(bufferSize);
    DWORD bytesRead = GetPrivateProfileSectionNamesA(sectionNamesBuffer.data(), bufferSize, PERSONAS_INI_PATH);
    if (bytesRead == 0 || bytesRead >= bufferSize - 2) {
        LogConfig("LoadPersonaDatabase: No sections found or buffer too small in " + std::string(PERSONAS_INI_PATH));
        return;
    }
    int personaCount = 0;
    for (char* pSection = sectionNamesBuffer.data(); *pSection; ) {
        std::string sectionName(pSection);
        NpcPersona persona;
        persona.modelName = sectionName;
        persona.hash = GetHashFromHex(GetValueFromINI(PERSONAS_INI_PATH, sectionName, "Hash"));
        persona.isHuman = (GetValueFromINI(PERSONAS_INI_PATH, sectionName, "IsHuman") == "1");
        persona.inGameName = GetValueFromINI(PERSONAS_INI_PATH, sectionName, "InGameName");
        persona.type = GetValueFromINI(PERSONAS_INI_PATH, sectionName, "Type");
        persona.relationshipGroup = GetValueFromINI(PERSONAS_INI_PATH, sectionName, "Relationship");
        persona.subGroup = GetValueFromINI(PERSONAS_INI_PATH, sectionName, "SubGroup");
        persona.gender = GetValueFromINI(PERSONAS_INI_PATH, sectionName, "Gender");
        persona.behaviorTraits = GetValueFromINI(PERSONAS_INI_PATH, sectionName, "Behavior");
        if (sectionName.rfind("DEFAULT_", 0) == 0) {
            g_DefaultTypeCache[persona.type] = persona;
        }
        else if (persona.hash != 0) {
            g_PersonaCache[persona.hash] = persona;
        }
        personaCount++;
        pSection += (sectionName.length() + 1);
    }
    LogConfig("LoadPersonaDatabase: Loaded " + std::to_string(personaCount) + " personas");
}

// --- CORE PUBLIC FUNCTIONS ---
void ConfigReader::LoadAllConfigs() {
    LogConfig("LoadAllConfigs started");
    try {
        g_Settings.Enabled = (GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "Enabled", "1") == "1");
        LogConfig("LoadAllConfigs: Enabled=" + std::to_string(g_Settings.Enabled));
        g_Settings.ActivationKey = KeyNameToVK(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "ACTIVATION_KEY", "T"));
        LogConfig("LoadAllConfigs: ActivationKey=" + std::to_string(g_Settings.ActivationKey));
        g_Settings.ActivationDurationMs = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "ACTIVATION_DURATION", "1000"));
        LogConfig("LoadAllConfigs: ActivationDurationMs=" + std::to_string(g_Settings.ActivationDurationMs));
        auto stopKeys = SplitString(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "STOP_KEY", "U"), ',');
        g_Settings.StopKey_Primary = KeyNameToVK(stopKeys[0]);
        LogConfig("LoadAllConfigs: StopKey_Primary=" + std::to_string(g_Settings.StopKey_Primary));
        g_Settings.StopKey_Secondary = stopKeys.size() > 1 ? KeyNameToVK(stopKeys[1]) : 0;
        LogConfig("LoadAllConfigs: StopKey_Secondary=" + std::to_string(g_Settings.StopKey_Secondary));
        g_Settings.StopDurationMs = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "STOP_DURATION", "4000"));
        LogConfig("LoadAllConfigs: StopDurationMs=" + std::to_string(g_Settings.StopDurationMs));
        g_Settings.MaxInputChars = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MAX_INPUT_CHARS", "100"));
        LogConfig("LoadAllConfigs: MaxInputChars=" + std::to_string(g_Settings.MaxInputChars));
        g_Settings.MaxOutputChars = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MAX_OUTPUT_CHARS", "100"));
        LogConfig("LoadAllConfigs: MaxOutputChars=" + std::to_string(g_Settings.MaxOutputChars));
        g_Settings.MaxConversationRadius = std::stof(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MAX_CONVERSATION_RADIUS", "3.0"));
        LogConfig("LoadAllConfigs: MaxConversationRadius=" + std::to_string(g_Settings.MaxConversationRadius));
        g_Settings.MinResponseDelayMs = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MIN_RESPONSE_DELAY_MS", "1500"));
        LogConfig("LoadAllConfigs: MinResponseDelayMs=" + std::to_string(g_Settings.MinResponseDelayMs));
        g_Settings.MaxHistoryTokens = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MAX_REMEMBER_HISTORY", "1900"));
        LogConfig("LoadAllConfigs: MaxHistoryTokens=" + std::to_string(g_Settings.MaxHistoryTokens));
        g_Settings.DeletionTimer = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "DELETION_TIMER", "180"));
        LogConfig("LoadAllConfigs: DeletionTimer=" + std::to_string(g_Settings.DeletionTimer));
        g_Settings.MaxNpcGetModel = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MAX_NPC_GET_MODEL", "1"));
        LogConfig("LoadAllConfigs: MaxNpcGetModel=" + std::to_string(g_Settings.MaxNpcGetModel));
        g_Settings.MaxChatHistoryLines = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MAX_PROMPT_MEMORY_HALFED", "10"));
        LogConfig("LoadAllConfigs: MaxChatHistoryLines=" + std::to_string(g_Settings.MaxChatHistoryLines));
        g_ContentGuidelines = GetValueFromINI(SETTINGS_INI_PATH, "CONTENT_GUIDELINES", "PROMPT_INJECTION", "You are a helpful assistant.");
        g_Settings.USE_GPU_LAYERS = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "USE_GPU_LAYERS", "0"));
        g_Settings.USE_VRAM_PREFERED = (GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "USE_VRAM_PREFERED", "0") == "1");
        g_Settings.DEBUG_LEVEL = std::stoi(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "DEBUG_LEVEL", "0"));
        g_Settings.LOG_NAME = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "LOG_NAME", "kkamel.log");
        g_Settings.LOG2_NAME = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "LOG2_NAME", "kkamel_metrics.log");
        g_Settings.LOG3_NAME = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "LOG3_NAME", "kkamel_timers.log");
        g_Settings.MODEL_PATH = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MODEL_PATH", "");
        g_Settings.MODEL_ALT_NAME = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "MODEL_ALT_NAME", "");
        g_Settings.StT_Enabled = (GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "SPEECH_TO_TEXT", "0") == "1");

        // FIX: Must use KeyNameToVK to convert the key (e.g., "N") to a keycode (e.g., 78)
        g_Settings.StTRB_Activation_Key = KeyNameToVK(GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "SPEECH_TO_TEXT_RECORDING_BUTTON", "N"));

        // Reads TEXT_TO_SPEECH = 1
        g_Settings.TtS_Enabled = (GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "TEXT_TO_SPEECH", "0") == "1");


        g_Settings.STT_MODEL_PATH = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "STT_MODEL_PATH", "");
        g_Settings.STT_MODEL_ALT_NAME = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "STT_MODEL_ALT_NAME", "ggml-base.en.bin");



        g_Settings.TTS_MODEL_PATH = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "TTS_MODEL_PATH", "");
        g_Settings.TTS_MODEL_ALT_NAME = GetValueFromINI(SETTINGS_INI_PATH, "SETTINGS", "TTS_MODEL_ALT_NAME", ""); // Add a default TTS model name later

        LogConfig("AllLoadConfigs: Logging file, model path, VRAM and GPU settings loaded.");
        LogConfig("LoadAllConfigs: ContentGuidelines loaded.");
        LoadWorldContextDatabase();
        LogConfig("LoadAllConfigs: WorldContextDatabase loaded.");
        LoadRelationshipDatabase();
        LogConfig("LoadAllConfigs: RelationshipDatabase loaded.");
        LoadPersonaDatabase();
        LogConfig("LoadAllConfigs: PersonaDatabase loaded.");
        LogConfig("LoadAllConfigs completed successfully");
    }
    catch (const std::exception& e) {
        LogConfig("LoadAllConfigs: FATAL Exception caught - " + std::string(e.what()));
        throw;
    }
}

NpcPersona ConfigReader::GetPersona(Ped ped) {
    if (!ENTITY::DOES_ENTITY_EXIST(ped)) return NpcPersona();
    Hash entityHash = ENTITY::GET_ENTITY_MODEL(ped);
    LogConfig("GetPersona: Entity model hash=" + std::to_string(entityHash));
    auto it = g_PersonaCache.find(entityHash);
    if (it != g_PersonaCache.end()) {
        LogConfig("Persona found in .ini");
        return it->second;
    }
    NpcPersona p;
    p.hash = entityHash;
    p.modelName = "UNKNOWN_MODEL";
    p.isHuman = PED::IS_PED_HUMAN(ped);
    p.inGameName = "";
    p.type = "Stranger";
    p.relationshipGroup = "Ambient";
    p.subGroup = "None";
    p.behaviorTraits = "Unknown";
    if (p.isHuman) {
        p.gender = PED::IS_PED_MALE(ped) ? "Male" : "Female";
    }
    else {
        p.gender = "Neutral";
        p.type = "ANIMAL";
    }
    LogConfig("Persona loaded successfully");
    g_PersonaCache[entityHash] = p;
    return p;
}

std::string ConfigReader::GetRelationship(const std::string& npcSubGroup, const std::string& playerSubGroup) {
    LogConfig("GetRelationship called for npcSubGroup: " + npcSubGroup + ", playerSubGroup: " + playerSubGroup);
    if (npcSubGroup.empty() || playerSubGroup.empty()) {
        LogConfig("GetRelationship: SubGroup is empty, fallback to neutral");
        return "neutral, stranger";
    }
    std::string key = npcSubGroup + ":" + playerSubGroup;
    auto it = g_RelationshipMatrix.find(key);
    if (it != g_RelationshipMatrix.end()) {
        LogConfig("GetRelationship: Found direct relationship: " + it->second);
        return it->second;
    }
    key = playerSubGroup + ":" + npcSubGroup;
    it = g_RelationshipMatrix.find(key);
    if (it != g_RelationshipMatrix.end()) {
        LogConfig("GetRelationship: Found reverse relationship: " + it->second);
        return it->second;
    }
    if (npcSubGroup == "Ambient" || playerSubGroup == "Ambient") {
        LogConfig("GetRelationship: Fallback to neutral, stranger for Ambient group");
        return "neutral, stranger";
    }
    if ((npcSubGroup == "Law" || npcSubGroup == "LSPD" || npcSubGroup == "FIB") &&
        (playerSubGroup == "Gang" || playerSubGroup == "Families" || playerSubGroup == "Ballas")) {
        LogConfig("GetRelationship: Fallback to adversary, distrusts, pursues for Law vs Gang");
        return "adversary, distrusts, pursues";
    }
    if ((npcSubGroup == "Gang" || npcSubGroup == "Families" || playerSubGroup == "Ballas") &&
        (playerSubGroup == "Law" || playerSubGroup == "LSPD" || playerSubGroup == "FIB")) {
        LogConfig("GetRelationship: Fallback to adversary, distrusts, pursued for Gang vs Law");
        return "adversary, distrusts, pursued";
    }
    LogConfig("GetRelationship: Fallback to neutral, unknown");
    return "neutral, unknown";
}

std::string ConfigReader::GetZoneContext(const std::string& zoneName) {
    LogConfig("GetZoneContext called for zoneName: " + zoneName);
    auto it = g_ZoneContextCache.find(zoneName);
    if (it != g_ZoneContextCache.end()) {
        LogConfig("GetZoneContext: Found context: " + it->second);
        return it->second;
    }
    LogConfig("GetZoneContext: No context found, returning default");
    return "Location: Unknown";
}

std::string ConfigReader::GetOrgContext(const std::string& orgName) {
    LogConfig("GetOrgContext called for orgName: " + orgName);
    auto it = g_OrgContextCache.find(orgName);
    if (it != g_OrgContextCache.end()) {
        LogConfig("GetOrgContext: Found context: " + it->second);
        return it->second;
    }
    LogConfig("GetOrgContext: No context found, returning empty string");
    return "";
}
// End of File