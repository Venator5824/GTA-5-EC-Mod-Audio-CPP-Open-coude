#pragma once
#include <string>
#include <vector>
#include "main.h"

class ITTSEngine {
public:
    virtual ~ITTSEngine() {}

    // Tries to load the model. Returns false on failure.
    // We pass both the model and config file paths.
    virtual bool LoadModel(const std::string& modelPath, const std::string& configPath) = 0;

    // Takes plain text and a voice ID, and returns a vector of audio samples.
    virtual std::vector<float> Synthesize(const std::string& text, const std::string& voiceId) = 0;
};