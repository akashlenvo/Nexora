#pragma once

#include <wx/wx.h>
#include <vector>
#include <string>

#include "net/devicedescriptor.h"

wxDECLARE_EVENT(EVT_STREAM_CONFIG_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_STREAM_RESOLUTION_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_STREAM_FPS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_STREAM_BITRATE_CHANGED, wxCommandEvent);

class StreamConfigDlg : public wxDialog
{
public:
    struct Config
    {
        int resIndex;
        int fps;

        // Bitrate Settings
        bool adaptiveBitrate;
        int bitrate;
        int minBitrate;
        int maxBitrate;

        bool stabilizationEnabled;
        bool h265Enabled;
        int focusMode;
        bool flashEnabled;
    };

    StreamConfigDlg(wxWindow* parent, const DeviceDescriptor& device, bool isBackCamera, const Config& currentConfig);
    ~StreamConfigDlg();

    bool IsAdaptiveBitrate() const;
    int GetStaticBitrate() const;
    int GetMinBitrate() const;
    int GetMaxBitrate() const;

    bool IsStabilizationEnabled() const { return stabilizationCheck->GetValue(); }
    bool IsFlashEnabled() const { return flashCheck->GetValue(); }
    int GetFocusMode() const { return focusChoice->GetSelection(); }
    bool IsH265Enabled() const { return codecChoice->GetSelection() == 1; }

private:
    // Controls
    wxChoice* resChoice;
    wxChoice* fpsChoice;

    // -- New Bitrate Controls --
    wxRadioButton* rbStatic;
    wxRadioButton* rbAdaptive;

    wxPanel* staticBitratePanel;
    wxSlider* staticSlider;
    wxStaticText* staticLabel;

    wxPanel* adaptiveBitratePanel;
    wxSlider* minSlider;
    wxStaticText* minLabel;
    wxSlider* maxSlider;
    wxStaticText* maxLabel;
    // --------------------------

    wxCheckBox* stabilizationCheck;
    wxChoice* codecChoice;
    wxChoice* focusChoice;
    wxCheckBox* flashCheck;

    // Handlers
    void OnResolutionChanged(wxCommandEvent& event);
    void OnFpsChanged(wxCommandEvent& event);

    void OnBitrateModeChanged(wxCommandEvent& event);
    void OnBitrateSliderChanged(wxCommandEvent& event); // Handles all 3 sliders

    void OnSimpleSettingChanged(wxCommandEvent& event);

    void UpdateBitrateUI(); // Helper to hide/show panels
    wxString FormatBitrate(int kbps);
};