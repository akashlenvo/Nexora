#include "gui/devicesview.h"
#include <wx/listctrl.h>
#include <algorithm> // For std::max_element

DevicesView::DevicesView(wxWindow* parent, const std::vector<DeviceDescriptor>& devices)
    : wxDialog(parent, wxID_ANY, "Connected Devices", wxDefaultPosition, wxSize(600, 300))
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto list = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);

    // Define Columns
    list->AppendColumn("#", wxLIST_FORMAT_LEFT, 30);
    list->AppendColumn("Model Name", wxLIST_FORMAT_LEFT, 150);
    list->AppendColumn("RTSP URL", wxLIST_FORMAT_LEFT, 220);
    list->AppendColumn("Max Back", wxLIST_FORMAT_LEFT, 90);  // New Column
    list->AppendColumn("Max Front", wxLIST_FORMAT_LEFT, 90); // New Column

    // Helper lambda to format the maximum resolution string
    auto getMaxResString = [](const std::vector<DeviceDescriptor::Resolution>& resList) -> std::string {
        if (resList.empty()) return "-";

        // Find resolution with the highest pixel count (width * height)
        auto maxRes = *std::max_element(resList.begin(), resList.end(),
            [](const DeviceDescriptor::Resolution& a, const DeviceDescriptor::Resolution& b) {
                return (a.first * a.second) < (b.first * b.second);
            });

        return std::to_string(maxRes.first) + "x" + std::to_string(maxRes.second);
        };

    for (size_t i = 0; i < devices.size(); i++)
    {
        const auto& dev = devices[i];
        long index = list->InsertItem(i, std::to_string(i + 1));

        list->SetItem(index, 1, dev.name());
        list->SetItem(index, 2, dev.url());

        // 3. Max Back Resolution
        list->SetItem(index, 3, getMaxResString(dev.backResolutions()));

        // 4. Max Front Resolution
        list->SetItem(index, 4, getMaxResString(dev.frontResolutions()));
    }

    sizer->Add(list, 1, wxEXPAND | wxALL, 10);

    auto btnSizer = new wxBoxSizer(wxHORIZONTAL);
    auto closeBtn = new wxButton(this, wxID_OK, "Close");
    btnSizer->Add(closeBtn, 0);
    sizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    this->SetSizer(sizer);
    this->CenterOnParent();
}