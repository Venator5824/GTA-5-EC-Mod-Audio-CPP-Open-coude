#pragma once
#include <windows.h>
#include <string>
#include <algorithm> // Für std::min
#include <vector>

// Versionierung sicherstellen
#define BRIDGE_VERSION "v1"

// Name des Shared Memory Bereichs
#define SHARED_MEM_NAME "Local\\EC_DATA_POOL_01_" BRIDGE_VERSION

// Puffergrößen
#define BUFFER_SIZE 8192 
#define VOICE_ID_SIZE 64 

// Byte-Alignment sicherstellen
#pragma pack(push, 1)

struct SharedVoiceData {
    // Steuer-Flags (volatile für Thread-Sicherheit)
    volatile bool hasNewJob;
    volatile bool isAudioPlaying;

    // Nutzdaten
    char text[BUFFER_SIZE];        // Der zu sprechende Text
    char voiceId[VOICE_ID_SIZE];   // Die ID der Stimme (z.B. "1", "20", "special_cop")
    float speed;                   // Sprechgeschwindigkeit (1.0 = normal)
};

#pragma pack(pop)

class VoiceBridge {
private:
    HANDLE hMapFile;
    SharedVoiceData* pData;
    bool isHost;

public:
    // Konstruktor
    VoiceBridge(bool host) : isHost(host), hMapFile(NULL), pData(NULL) {
        if (isHost) {
            // HOST (Main Mod): Erstellt den Speicher
            hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedVoiceData), SHARED_MEM_NAME);

            // Falls er schon existiert (z.B. durch alten Crash), öffnen wir ihn einfach
            if (hMapFile && GetLastError() == ERROR_ALREADY_EXISTS) {
                CloseHandle(hMapFile);
                hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
            }
        }
        else {
            // CLIENT (Audio DLL): Öffnet den vorhandenen Speicher
            hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        }

        if (hMapFile) {
            pData = (SharedVoiceData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedVoiceData));

            // Nur der Host initialisiert die Werte beim Start
            if (isHost && pData) {
                pData->hasNewJob = false;
                pData->isAudioPlaying = false;
                pData->text[0] = '\0';
                pData->voiceId[0] = '\0';
                pData->speed = 1.0f;
            }
        }
    }

    // Destruktor
    ~VoiceBridge() {
        if (pData) UnmapViewOfFile(pData);
        if (hMapFile) CloseHandle(hMapFile);
    }

    // Verbindung prüfen
    bool IsConnected() const { return pData != nullptr; }

    // HOST: Sendet einen Auftrag
    // voiceId: Die ID aus der INI (z.B. "1", "5")
    void Send(const std::string& text, const std::string& voiceId, float speed = 1.0f) {
        if (!pData) return;
        if (pData->hasNewJob) return; // Warten, wenn noch beschäftigt

        // Text kopieren
        size_t lenText = (std::min)(text.length(), (size_t)(BUFFER_SIZE - 1));
        memcpy(pData->text, text.c_str(), lenText);
        pData->text[lenText] = '\0';

        // Voice ID kopieren
        size_t lenVoice = (std::min)(voiceId.length(), (size_t)(VOICE_ID_SIZE - 1));
        memcpy(pData->voiceId, voiceId.c_str(), lenVoice);
        pData->voiceId[lenVoice] = '\0';

        pData->speed = speed;

        // Job freigeben
        pData->hasNewJob = true;
    }

    // CLIENT: Prüft auf neue Aufträge
    bool CheckForJob(std::string& outText, std::string& outVoiceId, float& outSpeed) {
        if (!pData || !pData->hasNewJob) return false;

        // Daten auslesen
        outText = pData->text;
        outVoiceId = pData->voiceId;
        outSpeed = pData->speed;

        // Job als gelesen markieren
        pData->hasNewJob = false;
        return true;
    }

    // CLIENT: Setzt Status (Spreche ich gerade?)
    void SetTalkingState(bool state) {
        if (pData) pData->isAudioPlaying = state;
    }

    // HOST: Liest Status (Spricht er gerade?)
    bool IsTalking() const {
        if (!pData) return false;
        return pData->isAudioPlaying;
    }
};