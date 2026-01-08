#pragma once

#include <wx/wx.h>
#include <vector>
#include "net/devicedescriptor.h"

class DevicesView : public wxDialog
{
public:
    DevicesView(wxWindow* parent, const std::vector<DeviceDescriptor>& devices);
};