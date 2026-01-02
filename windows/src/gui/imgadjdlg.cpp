#include "gui/imgadjdlg.h"
#include <wx/statline.h>

wxDEFINE_EVENT(EVT_FILTER_PARAM_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_FILTER_SWITCH_CHANGED, wxCommandEvent);

ImgAdjDlg::ImgAdjDlg(wxWindow* parent,
    const DeviceDescriptor& deviceDesc,
    const std::map<std::string, int>& initialSliderValues,
    const std::map<int, std::string>& initialActiveFilters)
    : wxDialog(parent, wxID_ANY, "Image Adjustments", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    // ---------------------------------------------------------
    // PREPARE CONTAINERS
    // ---------------------------------------------------------

    // 1. Sliders Container (Color Corrections)
    auto adjustmentsSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Color Corrections");
    auto adjustmentsGrid = new wxFlexGridSizer(3, 5, 10);
    adjustmentsGrid->AddGrowableCol(1, 1);

    // 2. Dropdowns Container (Other Effects)
    auto filtersSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Active Filters");
    auto filtersGrid = new wxFlexGridSizer(2, 5, 10);
    filtersGrid->AddGrowableCol(1, 1);

    bool hasSliders = false;
    bool hasDropdowns = false;

    // ---------------------------------------------------------
    // DYNAMIC GENERATION LOOP
    // ---------------------------------------------------------
    // Iterate through the filters provided by the device
    for (const auto& [category, filterList] : deviceDesc.filters())
    {
        // Skip the "None" category
        if (category == Video::Filter::Category::NONE)
            continue;

        // CASE A: Color Category -> Create Sliders
        if (category == Video::Filter::Category::CORRECTION)
        {
            for (const auto& filterName : filterList)
            {
                // STATE CHECK: Do we have a saved value for this slider?
                int startValue = 50; // Default
                if (initialSliderValues.count(filterName)) {
                    startValue = initialSliderValues.at(filterName);
                }

                // Create the slider with the RESTORED value
                AddSlider(adjustmentsSizer->GetStaticBox(), adjustmentsGrid, filterName, startValue);
                hasSliders = true;
            }
        }
        // CASE B: All Other Categories -> Create Dropdowns
        else
        {
            // Determine Label based on Category Enum
            std::string catName;
            switch (category) {
            case Video::Filter::Category::DISTORTION: catName = "Distortion"; break;
            case Video::Filter::Category::ARTISTIC: catName = "Artistic"; break;
            case Video::Filter::Category::EFFECT: catName = "Effects"; break;
            default: catName = "Other"; break;
            }

            auto label = new wxStaticText(filtersSizer->GetStaticBox(), wxID_ANY, catName + ":");

            wxArrayString choices;
            choices.Add("None"); // Index 0 is always "None"

            // STATE CHECK: What was the previously selected filter for this category?
            std::string activeName = "None";
            int catId = static_cast<int>(category);
            if (initialActiveFilters.count(catId)) {
                activeName = initialActiveFilters.at(catId);
            }

            int selectionIndex = 0; // Default to "None"

            // Populate choices and find the index of the active one
            for (size_t i = 0; i < filterList.size(); i++) 
            {
                std::string fName = filterList[i];
                choices.Add(fName);

                if (fName == activeName) 
                {
                    selectionIndex = i + 1; // +1 because we added "None" at the start
                }
            }

            auto choice = new wxChoice(filtersSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
            choice->SetSelection(selectionIndex); // Restore Selection

            // Bind Event
            choice->Bind(wxEVT_CHOICE, [this, category, choice](wxCommandEvent& e) {
                wxCommandEvent evt(EVT_FILTER_SWITCH_CHANGED);
                evt.SetEventObject(this);
                evt.SetInt(static_cast<int>(category));
                evt.SetString(choice->GetStringSelection());
                ProcessWindowEvent(evt);
            });

            categoryChoices[static_cast<int>(category)] = choice;

            filtersGrid->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            filtersGrid->Add(choice, 1, wxEXPAND);
            hasDropdowns = true;
        }
    }

    // ---------------------------------------------------------
    // LAYOUT ASSEMBLY
    // ---------------------------------------------------------

    if (hasSliders) {
        adjustmentsSizer->Add(adjustmentsGrid, 1, wxEXPAND | wxALL, 10);
        mainSizer->Add(adjustmentsSizer, 0, wxEXPAND | wxALL, 10);
    }
    else {
        adjustmentsSizer->DeleteWindows();
        delete adjustmentsSizer;
    }

    if (hasDropdowns) {
        filtersSizer->Add(filtersGrid, 1, wxEXPAND | wxALL, 10);
        mainSizer->Add(filtersSizer, 0, wxEXPAND | wxALL, 10);
    }
    else {
        filtersSizer->DeleteWindows();
        delete filtersSizer;
    }

    // Buttons
    auto btnSizer = new wxBoxSizer(wxHORIZONTAL);
    auto resetBtn = new wxButton(this, wxID_ANY, "Reset All");
    resetBtn->Bind(wxEVT_BUTTON, &ImgAdjDlg::OnReset, this);
    auto okBtn = new wxButton(this, wxID_OK, "Close");

    btnSizer->Add(resetBtn, 0, wxRIGHT, 10);
    btnSizer->Add(okBtn, 0);
    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);
}

ImgAdjDlg::~ImgAdjDlg()
{
}

void ImgAdjDlg::AddSlider(wxWindow* parent, wxSizer* sizer, const std::string& label, int initialValue)
{
    // Label
    auto text = new wxStaticText(parent, wxID_ANY, label);

    // Slider (0-100)
    auto slider = new wxSlider(parent, wxID_ANY, initialValue, 0, 100, wxDefaultPosition, wxSize(150, -1));

    // Percentage Label
    auto valLabel = new wxStaticText(parent, wxID_ANY, wxString::Format("%d%%", initialValue));

    // Store references
    sliders[label] = slider;
    sliderLabels[label] = valLabel;

    // Bind
    slider->Bind(wxEVT_SLIDER, [this, label, slider, valLabel](wxCommandEvent& e) {
        valLabel->SetLabel(wxString::Format("%d%%", slider->GetValue()));

        wxCommandEvent evt(EVT_FILTER_PARAM_CHANGED);
        evt.SetEventObject(this);
        evt.SetString(label);        // Filter Name
        evt.SetInt(slider->GetValue()); // 0-100
        ProcessWindowEvent(evt);
        });

    sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
    sizer->Add(slider, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);
    sizer->Add(valLabel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
}

void ImgAdjDlg::OnReset(wxCommandEvent& event)
{
    // 1. Reset Sliders to 50%
    for (auto& [name, slider] : sliders) {
        slider->SetValue(50);
        sliderLabels[name]->SetLabel("50%");

        // Notify Application to update State & Stream
        wxCommandEvent evt(EVT_FILTER_PARAM_CHANGED);
        evt.SetString(name);
        evt.SetInt(50);
        ProcessWindowEvent(evt);
    }

    // 2. Reset Dropdowns to "None"
    for (auto& [cat, choice] : categoryChoices) {
        if (choice->GetCount() > 0) {
            choice->SetSelection(0); // "None"

            // Notify Application
            wxCommandEvent evt(EVT_FILTER_SWITCH_CHANGED);
            evt.SetInt(cat);
            evt.SetString("None");
            ProcessWindowEvent(evt);
        }
    }
}