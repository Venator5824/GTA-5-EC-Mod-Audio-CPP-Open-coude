#pragma once



#include <algorithm>
#include <string>

#include "ConfigReader.h"
#include "main.h"

#define NOMINMAX
#include <windows.h>

// Versioning
#define BRIDGE_VERSION "v2"
#define SHARED_MEM_NAME "Local\\EC_DATA_POOL_01_" BRIDGE_VERSION

// Buffer Sizes
#define BUFFER_SIZE 8192
#define VOICE_ID_SIZE 64


enum class AudioJobState : int32_t {
    IDLE = 0,
    PENDING = 1,
    PROCESSING = 2,
    PLAYING = 3,
    COMPLETED = 4,
    FAILED = 5
};

#pragma pack(push, 1)
struct SharedVoiceData {
    // Status (4 bytes, aligned)
    volatile AudioJobState jobState;

    // Timestamp for timeout detection (8 bytes)
    volatile ULONGLONG lastUpdateTime;

    // Job Data
    char text[BUFFER_SIZE];
    char voiceId[VOICE_ID_SIZE];
    float speed;

    // Error Info
    char errorMsg[256];
};
#pragma pack(pop)

class VoiceBridge {
private:
    HANDLE hMapFile;
    SharedVoiceData* pData;
    bool isHost;

    // Timeout in milliseconds
    static const ULONGLONG TIMEOUT_MS = 10000;  // 10 seconds

public:
    VoiceBridge(bool host) : isHost(host), hMapFile(NULL), pData(NULL) {
        if (isHost) {
            // HOST: Create Shared Memory
            hMapFile =
                CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                    sizeof(SharedVoiceData), SHARED_MEM_NAME);

            if (hMapFile && GetLastError() == ERROR_ALREADY_EXISTS) {
                // Old memory exists, try to open it again
                CloseHandle(hMapFile);
                hMapFile =
                    OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
            }
        }
        else {
            // CLIENT: Open existing memory
            hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        }

        if (hMapFile) {
            pData = (SharedVoiceData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0,
                0, sizeof(SharedVoiceData));

            // Only Host initializes at startup
            if (isHost && pData) {
                pData->jobState = AudioJobState::IDLE;
                pData->lastUpdateTime = GetTickCount64();
                pData->text[0] = '\0';
                pData->voiceId[0] = '\0';
                pData->speed = 1.0f;
                pData->errorMsg[0] = '\0';
            }
        }
    }

    ~VoiceBridge() {
        if (pData) UnmapViewOfFile(pData);
        if (hMapFile) CloseHandle(hMapFile);
    }

    bool IsConnected() const { return pData != nullptr; }

    // ========================================
    // HOST FUNCTIONS (Main Mod)
    // ========================================

    // Sends a new audio job (non-blocking)
    // Returns true on success, false if busy
    bool Send(const std::string& text, const std::string& voiceId,
        float speed = 1.0f) {
        if (!pData) return false;

        // Check current status
        AudioJobState currentState = pData->jobState;

        // Only IDLE or COMPLETED allow new jobs
        if (currentState != AudioJobState::IDLE &&
            currentState != AudioJobState::COMPLETED) {
            // Timeout check: Has the Audio DLL crashed?
            ULONGLONG now = GetTickCount64();
            ULONGLONG elapsed = now - pData->lastUpdateTime;

            if (elapsed > TIMEOUT_MS) {
                // Timeout! Reset and try again
                pData->jobState = AudioJobState::IDLE;
                pData->lastUpdateTime = now;
            }
            else {
                // Normally busy, wait
                return false;
            }
        }


        
        size_t lenText = std::min<size_t>(text.length(), BUFFER_SIZE - 1);
        memcpy(pData->text, text.c_str(), lenText);
        pData->text[lenText] = '\0';

        size_t lenVoice = std::min<size_t>(voiceId.length(), VOICE_ID_SIZE - 1);
        memcpy(pData->voiceId, voiceId.c_str(), lenVoice);
        pData->voiceId[lenVoice] = '\0';

        pData->speed = speed;
        pData->errorMsg[0] = '\0';

        // Atomic status change
        pData->lastUpdateTime = GetTickCount64();
        pData->jobState = AudioJobState::PENDING;

        return true;
    }

    // Checks if the audio system is ready
    bool IsReady() const {
        if (!pData) return false;

        AudioJobState state = pData->jobState;
        return (state == AudioJobState::IDLE || state == AudioJobState::COMPLETED);
    }

    // Checks if audio is currently playing
    bool IsTalking() const {
        if (!pData) return false;

        AudioJobState state = pData->jobState;
        return (state == AudioJobState::PROCESSING ||
            state == AudioJobState::PLAYING);
    }

    // Gets status info (for debug)
    std::string GetStatusString() const {
        if (!pData) return "DISCONNECTED";

        switch (pData->jobState) {
        case AudioJobState::IDLE:
            return "IDLE";
        case AudioJobState::PENDING:
            return "PENDING";
        case AudioJobState::PROCESSING:
            return "PROCESSING";
        case AudioJobState::PLAYING:
            return "PLAYING";
        case AudioJobState::COMPLETED:
            return "COMPLETED";
       case AudioJobState::FAILED:
             return "FAILED: " + std::string(pData->errorMsg);
        default:
            return "UNKNOWN";
        }
    }


    // ========================================
    // CLIENT FUNCTIONS (Audio DLL)
    // ========================================

    // Checks for a new job
    bool CheckForJob(std::string& outText, std::string& outVoiceId,
        float& outSpeed) {
        if (!pData) return false;

        // Only fetch PENDING jobs
        if (pData->jobState != AudioJobState::PENDING) {
            return false;
        }

        // Read data
        outText = pData->text;
        outVoiceId = pData->voiceId;
        outSpeed = pData->speed;

        // Mark as "in progress"
        pData->lastUpdateTime = GetTickCount64();
        pData->jobState = AudioJobState::PROCESSING;

        return true;
    }

    // Sets state during processing
    void SetState(AudioJobState newState) {
        if (!pData) return;

        pData->lastUpdateTime = GetTickCount64();
        pData->jobState = newState;
    }


    void SetError(const std::string& errorMsg) {
        if (!pData) return;

        size_t len = std::min<size_t>(errorMsg.length(), 255);
        memcpy(pData->errorMsg, errorMsg.c_str(), len);
        pData->errorMsg[len] = '\0';

        pData->lastUpdateTime = GetTickCount64();
        pData->jobState = AudioJobState::FAILED;
    } 



    // Reports job as completed
    void CompleteJob() {
        if (!pData) return;

        pData->lastUpdateTime = GetTickCount64();
        pData->jobState = AudioJobState::COMPLETED;
    }
};