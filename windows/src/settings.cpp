#include "settings.h"

int Settings::Get(std::string name)
{
    auto res = settings.find(name);
    return (res != settings.end()) ? res->second : -1;
}

void Settings::Set(std::string name, int value)
{
    settings[name] = value;
}

State::Registry Settings::GetDeviceStates()
{
    // Only return loaded states if the feature is enabled
    if (Get("SAVE_DEVICE_STATES") == 1)
    {
        return loadedDeviceStates;
    }
    return {}; // Return empty if disabled (App will use defaults)
}

void Settings::UpdateDeviceStates(const std::map<std::string, State>& currentStates)
{
    if (Get("SAVE_DEVICE_STATES") == 1)
    {
        loadedDeviceStates = currentStates;
    }
}

void Settings::Load()
{
    std::string path = Dir() + "\\settings.cfg";
    std::ifstream f(path);
    if (!f.is_open()) return;

    settings.clear();
    loadedDeviceStates.clear();
    preservedDeviceLines.clear();

    std::string line;
    std::string currentSection = "";
    State* currentDevice = nullptr;

    while (std::getline(f, line))
    {
        if (line.empty()) continue;

        // 1. Check for Section Headers [DEVICE:Name]
        if (line.front() == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.size() - 2);

            if (currentSection.find("DEVICE:") == 0)
            {
                std::string devName = currentSection.substr(7);
                // Create new entry
                currentDevice = &loadedDeviceStates[devName];
            }
            else
            {
                currentDevice = nullptr;
            }
            continue;
        }

        // 2. Parse Key=Value
        size_t pos = line.find("=");
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string valStr = line.substr(pos + 1);

        // 3. Logic: Global vs Device
        if (currentDevice)
        {
            // We are inside a device section
            ParseDeviceLine(*currentDevice, key, valStr);
        }
        else
        {
            // We are in global settings
            settings[key] = std::atoi(valStr.c_str());
        }
    }
    f.close();

    // CACHE LOGIC:
    // If the feature is disabled, we must re-read the specific lines 
    // that belong to devices so we can preserve them blindly during save.
    if (Get("SAVE_DEVICE_STATES") != 1)
    {
        CachePreservedLines(path);
    }
}

void Settings::Save()
{
    std::ofstream f(Dir() + "\\settings.cfg");

    // 1. Save Global Settings
    for (auto& entry : settings)
    {
        f << entry.first << "=" << entry.second << std::endl;
    }

    f << std::endl;

    // 2. Save Device States
    if (Get("SAVE_DEVICE_STATES") == 1)
    {
        // Feature Enabled: Write the current active states
        for (const auto& [name, state] : loadedDeviceStates)
        {
            f << "[DEVICE:" << name << "]" << std::endl;
            f << "fps=" << state.fps << std::endl;
            f << "res=" << state.resolution.first << "x" << state.resolution.second << std::endl;
            f << "back_cam=" << (state.backCameraActive ? 1 : 0) << std::endl;

            f << "adaptive_br=" << (state.adaptiveBitrate ? 1 : 0) << std::endl;
            f << "bitrate=" << state.bitrate << std::endl;
            f << "min_br=" << state.minBitrate << std::endl;
            f << "max_br=" << state.maxBitrate << std::endl;
            f << "audio=" << (state.audioEnabled ? 1 : 0) << std::endl;
            f << "stab=" << (state.stabilizationEnabled ? 1 : 0) << std::endl;
            f << "flash=" << (state.flashEnabled ? 1 : 0) << std::endl;
            f << "h265=" << (state.h265Enabled ? 1 : 0) << std::endl;
            f << "focus=" << state.focusMode << std::endl;

            // Save Sliders (Prefix "S:")
            for (const auto& [filter, val] : state.filterSliderValues) 
            {
                f << "S:" << filter << "=" << val << std::endl;
            }

            // Save Dropdowns (Prefix "D:")
            for (const auto& [cat, filter] : state.activeFilters) 
            {
                f << "D:" << cat << "=" << filter << std::endl;
            }
            f << std::endl;
        }
    }
    else
    {
        // Feature Disabled: Write back the preserved lines exactly as they were
        for (const auto& line : preservedDeviceLines)
        {
            f << line << std::endl;
        }
    }

    f.close();
}

void Settings::ParseDeviceLine(State& state, const std::string& key, const std::string& val)
{
    if (key == "fps")
    {
        state.fps = std::atoi(val.c_str());
    }
    else if (key == "back_cam")
    {
         state.backCameraActive = (val == "1");
    }
    else if (key == "res") 
    {
        size_t xPos = val.find("x");
        if (xPos != std::string::npos) 
        {
            state.resolution.first = std::atoi(val.substr(0, xPos).c_str());
            state.resolution.second = std::atoi(val.substr(xPos + 1).c_str());
        }
    }
    else if (key == "adaptive_br") state.adaptiveBitrate = (val == "1");
    else if (key == "bitrate") state.bitrate = std::atoi(val.c_str());
    else if (key == "min_br") state.minBitrate = std::atoi(val.c_str());
    else if (key == "max_br") state.maxBitrate = std::atoi(val.c_str());
    else if (key == "audio") state.audioEnabled = (val == "1");
    else if (key == "stab") state.stabilizationEnabled = (val == "1");
    else if (key == "flash") state.flashEnabled = (val == "1");
    else if (key == "h265") state.h265Enabled = (val == "1");
    else if (key == "focus") state.focusMode = std::atoi(val.c_str());
    else if (key.rfind("S:", 0) == 0) 
    { 
        // Starts with S: (Slider)
        state.filterSliderValues[key.substr(2)] = std::atoi(val.c_str());
    }
    else if (key.rfind("D:", 0) == 0) 
    { 
        // Starts with D: (Dropdown)
        state.activeFilters[std::atoi(key.substr(2).c_str())] = val;
    }
}

void Settings::CachePreservedLines(const std::string& path)
{
    std::ifstream f(path);
    std::string line;
    bool insideDeviceSection = false;

    while (std::getline(f, line))
    {
        if (line.empty()) 
            continue;

        if (line.front() == '[' && line.find("DEVICE:") != std::string::npos) 
        {
            insideDeviceSection = true;
        }
        else if (line.front() == '[' && line.find("DEVICE:") == std::string::npos) 
        {
            insideDeviceSection = false; // Different section?
        }
        else if (!insideDeviceSection && line.find("=") != std::string::npos) 
        {
            // This is a global setting, don't preserve it here (we write global settings from map)
            continue;
        }

        if (insideDeviceSection) 
        {
            preservedDeviceLines.push_back(line);
        }
    }
}

std::string Settings::Dir()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    return path.substr(0, path.find_last_of("\\/"));
}