#pragma once

#include <wx/wx.h>
#include <map>
#include <vector>
#include <string>
#include "net/devicedescriptor.h"

// Events
wxDECLARE_EVENT(EVT_FILTER_PARAM_CHANGED, wxCommandEvent); // Int value
wxDECLARE_EVENT(EVT_FILTER_SWITCH_CHANGED, wxCommandEvent); // String name

class ImgAdjDlg : public wxDialog
{
public:
	ImgAdjDlg(wxWindow* parent, const DeviceDescriptor& deviceDesc);
	~ImgAdjDlg();

private:
	void AddSlider(wxWindow* parent, wxSizer* sizer, const std::string& label, int initialValue);

	void OnReset(wxCommandEvent& event);

	// Store controls for reset logic
	std::map<std::string, wxSlider*> sliders;
	std::map<std::string, wxStaticText*> sliderLabels;
	std::map<int, wxChoice*> categoryChoices;
};