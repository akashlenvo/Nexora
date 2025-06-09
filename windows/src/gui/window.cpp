#include "gui/window.h"
#include "settings.h"
#include "icon.xpm"

#include <iostream>
#include <thread>
#include <wx/msw/private.h>
#include <wx/dc.h>
#include <wx/artprov.h>

Window::Window(Server::HostInfo hostinfo)
	: wxFrame(nullptr, wxID_ANY, "VCamdroid", wxDefaultPosition, wxSize(500, 450), wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

	taskbarIcon = new wxTaskBarIcon();
	taskbarIcon->SetIcon(icon, "VCamdroid");
	taskbarIcon->Bind(wxEVT_TASKBAR_LEFT_DCLICK, &Window::MaximizeFromTaskbar, this);
	Bind(wxEVT_ICONIZE, &Window::MinimizeToTaskbar, this);

	SetIcon(icon);
	InitializeMenu(hostinfo);
	InitializeCanvasPanel(panel, topsizer);
	InitializeControlPanel(panel, topsizer);

	panel->SetSizerAndFit(topsizer);
}

Window::~Window()
{
	delete taskbarIcon;
}

void Window::InitializeMenu(Server::HostInfo hostinfo)
{
	wxMenuBar* menuBar = new wxMenuBar();

	wxMenu* file = new wxMenu();
	
	auto c1 = file->AppendCheckItem(MenuIDs::HIDE2TRAY, "Hide to tray");
	c1->Check(Settings::get("MINIMIZE_TASKBAR") == 1);

	auto c2 = file->AppendCheckItem(MenuIDs::SHOWSTATS, "Show frame stats");
	c2->Check(Settings::get("SHOW_STATS") == 1);

	file->Append(wxID_ANY, "About");
	file->AppendSeparator();
	file->Append(wxID_ANY, "Exit");
	menuBar->Append(file, "File");

	wxMenu* connect = new wxMenu();
	connect->Append(MenuIDs::QR, "QR Code");
	connect->AppendSeparator();
	connect->Append(wxID_ANY, "IP:\t" + std::get<1>(hostinfo));
	connect->Append(wxID_ANY, "Port:\t" + std::get<2>(hostinfo));
	menuBar->Append(connect, "Connect");

	wxMenu* devices = new wxMenu();
	devices->Append(MenuIDs::DEVICES, "See connected devices");
	menuBar->Append(devices, "Devices");

	SetMenuBar(menuBar);
}

void Window::InitializeCanvasPanel(wxPanel* parent, wxBoxSizer* topsizer)
{
	canvas = new Canvas(parent, wxDefaultPosition, wxSize(400, 300));
	topsizer->Add(canvas, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);
}

void Window::InitializeControlPanel(wxPanel* parent, wxBoxSizer* topsizer)
{
	wxGridSizer* sizer = new wxGridSizer(2, 2, 20, 5);
	
	wxFlexGridSizer* settingsSizer = new wxFlexGridSizer(2, 2, 5, 5);
	                                                                                                        
	sourceChoice = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, new wxString[1]{ "No devices" });
	settingsSizer->Add(new wxStaticText(parent, wxID_ANY, "Source"), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	settingsSizer->Add(sourceChoice);

	resolutionChoice = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, new wxString[3] {"640x480", "(HD) 1280x720", "(FHD) 1920x1080"});
	settingsSizer->Add(new wxStaticText(parent, wxID_ANY, "Resolution"), 0, wxALL | wxALIGN_CENTER_VERTICAL);
	settingsSizer->Add(resolutionChoice);



	wxStaticBoxSizer* controlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Controls");

	rotateLeftButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/rotate-left.png", wxBITMAP_TYPE_PNG));
	controlsSizer->Add(rotateLeftButton, 0, wxRIGHT, 5);

	rotateRightButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/rotate-right.png", wxBITMAP_TYPE_PNG));
	controlsSizer->Add(rotateRightButton, 0, wxRIGHT, 5);

	flipButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/flip.png", wxBITMAP_TYPE_PNG));
	controlsSizer->Add(flipButton, 0, wxRIGHT, 5);

	adjustmentsButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/settings.png", wxBITMAP_TYPE_PNG));
	controlsSizer->Add(adjustmentsButton, 0, wxRIGHT, 10);

	swapButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/swap.png", wxBITMAP_TYPE_PNG));
	controlsSizer->Add(swapButton);

	sizer->Add(settingsSizer, 0, wxALIGN_CENTER_VERTICAL);
	sizer->Add(controlsSizer, 0, wxALIGN_CENTER_VERTICAL);

	statsText = new wxStaticText(parent, wxID_ANY, "frame time: -ms | frame size: -kb");
	statsText->Show(Settings::get("SHOW_STATS") == 1);
	sizer->AddSpacer(0);
	sizer->Add(statsText, 0, wxALIGN_RIGHT);

	topsizer->Add(sizer, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 15);
}

void Window::MinimizeToTaskbar(wxIconizeEvent& evt)
{
	if (Settings::get("MINIMIZE_TASKBAR") == 1)
	{
		this->Hide();
		evt.Skip();
	}
}

void Window::MaximizeFromTaskbar(wxTaskBarIconEvent& evt)
{
	this->Iconize(false);
	this->SetFocus();
	this->Raise();
	this->Show();
}

Canvas* Window::GetCanvas()
{
	return canvas;
}

wxChoice* Window::GetSourceChoice()
{
	return sourceChoice;
}

wxChoice* Window::GetResolutionChoice()
{
	return resolutionChoice;
}

wxButton* Window::GetRotateLeftButton()
{
	return rotateLeftButton;
}

wxButton* Window::GetRotateRightButton()
{
	return rotateRightButton;
}

wxButton* Window::GetFlipButton()
{
	return flipButton;
}

wxButton* Window::GetAdjustmentsButton()
{
	return adjustmentsButton;
}

wxButton* Window::GetSwapButton()
{
	return swapButton;
}

wxStaticText* Window::GetStatsText()
{
	return statsText;
}

wxTaskBarIcon* Window::GetTaskbarIcon()
{
	return taskbarIcon;
}
