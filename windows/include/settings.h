#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <windows.h>

#include "rtsp/streamoptions.h"

class Settings
{
public:
    // --- Global Settings (Ints) ---
    static int Get(std::string name);
    static void Set(std::string name, int value);

    // --- Device State Management ---

    // Call this at startup to get the loaded states
    static StreamOptions::Registry GetDeviceStates();
    // Call this before saving to update the internal state from the App
    static void UpdateDeviceStates(const std::map<std::string, StreamOptions>& currentStates);

    static void Load();
    static void Save();

private:
    inline static std::map<std::string, int> settings;
    inline static StreamOptions::Registry loadedDeviceStates;
    inline static std::vector<std::string> preservedDeviceLines;

    // Helpers
    static void ParseDeviceLine(StreamOptions& state, const std::string& key, const std::string& val);
    
    // Reads the file again just to extract Device sections as raw text
    static void CachePreservedLines(const std::string& path);
    static std::string Dir();
};