#include "gui/window.h"
#include "settings.h"

#include <wx/settings.h>
#include <wx/statline.h>

namespace {
const wxColour kInk(8, 13, 24);
const wxColour kSurface(18, 26, 43);
const wxColour kSurfaceRaised(27, 37, 58);
const wxColour kPrimary(108, 99, 255);
const wxColour kAccent(33, 212, 180);
const wxColour kText(242, 245, 251);
const wxColour kMuted(178, 188, 208);

wxButton* MakeToolButton(wxWindow* parent, const wxString& label, const wxString& tooltip)
{
	auto* button = new wxButton(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	button->SetBackgroundColour(kSurfaceRaised);
	button->SetForegroundColour(kText);
	button->SetMinSize(parent->FromDIP(wxSize(108, 38)));
	button->SetToolTip(tooltip);
	return button;
}
}

Window::Window(Server::HostInfo hostinfo)
	: wxFrame(nullptr, wxID_ANY, "Nexora Studio", wxDefaultPosition, wxSize(920, 720), wxDEFAULT_FRAME_STYLE)
{
	SetMinClientSize(FromDIP(wxSize(760, 600)));
	auto* panel = new wxPanel(this, wxID_ANY);
	panel->SetBackgroundColour(kInk);
	panel->SetForegroundColour(kText);
	auto* topsizer = new wxBoxSizer(wxVERTICAL);

	wxIcon icon("res/nexora.ico", wxBITMAP_TYPE_ICO);
	taskbarIcon = new wxTaskBarIcon();
	taskbarIcon->SetIcon(icon, "Nexora Studio");
	taskbarIcon->Bind(wxEVT_TASKBAR_LEFT_DCLICK, &Window::MaximizeFromTaskbar, this);
	Bind(wxEVT_ICONIZE, &Window::MinimizeToTaskbar, this);
	SetIcon(icon);

	InitializeMenu(hostinfo);
	InitializeHeader(panel, topsizer);
	InitializeTopBar(panel, topsizer);
	InitializeCanvasPanel(panel, topsizer);
	InitializeBottomBar(panel, topsizer);

	panel->SetSizer(topsizer);
	Center();
}

Window::~Window() { delete taskbarIcon; }

void Window::InitializeMenu(Server::HostInfo hostinfo)
{
	auto* menuBar = new wxMenuBar();
	auto* file = new wxMenu();
	auto* c1 = file->AppendCheckItem(MenuIDs::HIDE2TRAY, "Minimize to tray");
	c1->Check(Settings::Get("MINIMIZE_TASKBAR") == 1);
	auto* c2 = file->AppendCheckItem(MenuIDs::SHOWSTATS, "Show stream statistics");
	c2->Check(Settings::Get("SHOW_STATS") == 1);
	auto* c3 = file->AppendCheckItem(MenuIDs::SAVESTATE, "Remember device settings");
	c3->Check(Settings::Get("SAVE_DEVICE_STATE") == 1);

	auto* resolutions = new wxMenu();
	resolutions->AppendRadioItem(MenuIDs::DS_SD, "640 x 480", "Standard 4:3");
	resolutions->AppendRadioItem(MenuIDs::DS_HD, "1280 x 720", "HD");
	resolutions->AppendRadioItem(MenuIDs::DS_FHD, "1920 x 1080", "Full HD");
	resolutions->AppendRadioItem(MenuIDs::DS_QHD, "3840 x 2160", "4K UHD");
	auto selected = Settings::Get("DIRECTSHOW_RESOLUTION");
	resolutions->Check((selected != -1 ? selected : 0) + MenuIDs::DS_SD, true);
	file->AppendSubMenu(resolutions, "Virtual camera resolution", "Requires restart");
	file->AppendSeparator();
	file->Append(wxID_EXIT, "Exit");
	menuBar->Append(file, "Nexora");

	auto* connect = new wxMenu();
	connect->Append(MenuIDs::QR, "Show QR code");
	connect->Append(MenuIDs::DEVICES, "Connected devices");
	connect->AppendSeparator();
	connect->Append(wxID_ANY, "Address: " + std::get<1>(hostinfo));
	connect->Append(wxID_ANY, "Port: " + std::get<2>(hostinfo));
	menuBar->Append(connect, "Connection");
	SetMenuBar(menuBar);
}

void Window::InitializeHeader(wxPanel* parent, wxBoxSizer* topsizer)
{
	auto* header = new wxPanel(parent);
	header->SetBackgroundColour(kSurface);
	auto* row = new wxBoxSizer(wxHORIZONTAL);
	auto* brand = new wxBoxSizer(wxVERTICAL);
	auto* title = new wxStaticText(header, wxID_ANY, "NEXORA");
	auto titleFont = title->GetFont();
	titleFont.SetPointSize(titleFont.GetPointSize() + 8);
	titleFont.SetWeight(wxFONTWEIGHT_BOLD);
	title->SetFont(titleFont);
	title->SetForegroundColour(kText);
	auto* tagline = new wxStaticText(header, wxID_ANY, "Your phone. Your camera. Connected.");
	tagline->SetForegroundColour(kMuted);
	brand->Add(title);
	brand->Add(tagline, 0, wxTOP, FromDIP(2));
	row->Add(brand, 0, wxALIGN_CENTER_VERTICAL);
	row->AddStretchSpacer();
	statusText = new wxStaticText(header, wxID_ANY, "OFFLINE  |  Waiting for a device");
	statusText->SetForegroundColour(kMuted);
	row->Add(statusText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(18));
	auto* qrButton = new wxButton(header, MenuIDs::QR, "Connect with QR");
	qrButton->SetBackgroundColour(kPrimary);
	qrButton->SetForegroundColour(*wxWHITE);
	row->Add(qrButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(8));
	auto* devicesButton = new wxButton(header, MenuIDs::DEVICES, "Devices");
	row->Add(devicesButton, 0, wxALIGN_CENTER_VERTICAL);
	header->SetSizer(row);
	topsizer->Add(header, 0, wxEXPAND);
	row->SetMinSize(-1, FromDIP(86));
	row->Layout();
	header->SetSizerAndFit(row);
	row->InsertSpacer(0, FromDIP(24));
	row->AddSpacer(FromDIP(24));
}

void Window::InitializeTopBar(wxPanel* parent, wxBoxSizer* topsizer)
{
	auto* card = new wxPanel(parent);
	card->SetBackgroundColour(kSurface);
	auto* row = new wxBoxSizer(wxHORIZONTAL);
	auto* label = new wxStaticText(card, wxID_ANY, "VIDEO SOURCE");
	label->SetForegroundColour(kMuted);
	auto font = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	row->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(12));
	sourceChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(260, -1)), 1, new wxString[1]{ "No devices detected" });
	sourceChoice->SetSelection(0);
	row->Add(sourceChoice, 0, wxALIGN_CENTER_VERTICAL);
	row->AddStretchSpacer();
	statsText = new wxStaticText(card, wxID_ANY, "-- x --  |  -- fps  |  -- Mbps", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	statsText->SetForegroundColour(kAccent);
	statsText->Show(Settings::Get("SHOW_STATS") == 1);
	row->Add(statsText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(14));
	streamOptionsButton = new wxButton(card, wxID_ANY, "Stream settings");
	row->Add(streamOptionsButton, 0, wxALIGN_CENTER_VERTICAL);
	card->SetSizer(row);
	topsizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(18));
	row->SetMinSize(-1, FromDIP(58));
	row->InsertSpacer(0, FromDIP(16)); row->AddSpacer(FromDIP(16));
}

void Window::InitializeCanvasPanel(wxPanel* parent, wxBoxSizer* topsizer)
{
	canvas = new Canvas(parent, wxDefaultPosition, FromDIP(wxSize(768, 432)));
	canvas->SetBackgroundColour(wxColour(3, 6, 12));
	topsizer->Add(canvas, 1, wxEXPAND | wxALL, FromDIP(18));
}

void Window::InitializeBottomBar(wxPanel* parent, wxBoxSizer* topsizer)
{
	auto* bar = new wxPanel(parent);
	bar->SetBackgroundColour(kSurface);
	auto* stack = new wxBoxSizer(wxVERTICAL);
	auto* transformRow = new wxBoxSizer(wxHORIZONTAL);
	auto* deviceRow = new wxBoxSizer(wxHORIZONTAL);
	rotateLeftButton = MakeToolButton(bar, "Rotate left", "Rotate left");
	rotateRightButton = MakeToolButton(bar, "Rotate right", "Rotate right");
	flipButton = MakeToolButton(bar, "Mirror", "Flip horizontally");
	flipVerticalButton = MakeToolButton(bar, "Flip", "Flip vertically");
	zoomOutButton = MakeToolButton(bar, "-", "Zoom out"); zoomOutButton->SetMinSize(FromDIP(wxSize(40, 38)));
	zoomLevelLabel = new wxStaticText(bar, wxID_ANY, "1.0x", wxDefaultPosition, FromDIP(wxSize(42, -1)), wxALIGN_CENTER);
	zoomLevelLabel->SetForegroundColour(kText);
	zoomInButton = MakeToolButton(bar, "+", "Zoom in"); zoomInButton->SetMinSize(FromDIP(wxSize(40, 38)));
	torchButton = MakeToolButton(bar, "Flash", "Toggle flash");
	swapButton = MakeToolButton(bar, "Switch camera", "Switch front/back camera");
	snapshotButton = MakeToolButton(bar, "Snapshot", "Save a snapshot");
	adjustmentsButton = MakeToolButton(bar, "Adjust image", "Image adjustments and filters");

	for (auto* button : { rotateLeftButton, rotateRightButton, flipButton, flipVerticalButton }) transformRow->Add(button, 0, wxRIGHT, FromDIP(5));
	transformRow->AddStretchSpacer();
	auto* zoomLabel = new wxStaticText(bar, wxID_ANY, "ZOOM");
	zoomLabel->SetForegroundColour(kMuted);
	transformRow->Add(zoomLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(8));
	transformRow->Add(zoomOutButton, 0, wxALIGN_CENTER_VERTICAL);
	transformRow->Add(zoomLevelLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(4));
	transformRow->Add(zoomInButton, 0, wxALIGN_CENTER_VERTICAL);
	for (auto* button : { torchButton, swapButton, snapshotButton, adjustmentsButton }) deviceRow->Add(button, 0, wxRIGHT, FromDIP(5));
	deviceRow->AddStretchSpacer();
	auto* controlsLabel = new wxStaticText(bar, wxID_ANY, "Camera controls");
	controlsLabel->SetForegroundColour(kMuted);
	deviceRow->Add(controlsLabel, 0, wxALIGN_CENTER_VERTICAL);
	stack->Add(transformRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(14));
	stack->Add(deviceRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, FromDIP(14));
	bar->SetSizer(stack);
	topsizer->Add(bar, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(18));
}

void Window::SetConnectionStatus(bool connected, const wxString& deviceName)
{
	statusText->SetLabel(connected ? wxString("ONLINE  |  ") + deviceName : wxString("OFFLINE  |  Waiting for a device"));
	statusText->SetForegroundColour(connected ? kAccent : kMuted);
	statusText->GetParent()->Layout();
}

void Window::MinimizeToTaskbar(wxIconizeEvent& evt) { if (Settings::Get("MINIMIZE_TASKBAR") == 1) { Hide(); evt.Skip(); } }
void Window::MaximizeFromTaskbar(wxTaskBarIconEvent&) { Iconize(false); Show(); Raise(); SetFocus(); }

Canvas* Window::GetCanvas() { return canvas; }
wxChoice* Window::GetSourceChoice() { return sourceChoice; }
wxButton* Window::GetStreamOptionsButton() { return streamOptionsButton; }
wxButton* Window::GetRotateLeftButton() { return rotateLeftButton; }
wxButton* Window::GetRotateRightButton() { return rotateRightButton; }
wxButton* Window::GetFlipButton() { return flipButton; }
wxButton* Window::GetFlipVerticalButton() { return flipVerticalButton; }
wxButton* Window::GetZoomInButton() { return zoomInButton; }
wxButton* Window::GetZoomOutButton() { return zoomOutButton; }
wxStaticText* Window::GetZoomLevelLabel() { return zoomLevelLabel; }
wxButton* Window::GetTorchButton() { return torchButton; }
wxButton* Window::GetSwapButton() { return swapButton; }
wxButton* Window::GetAdjustmentsButton() { return adjustmentsButton; }
wxButton* Window::GetSnapshotButton() { return snapshotButton; }
wxStaticText* Window::GetStatsText() { return statsText; }
wxTaskBarIcon* Window::GetTaskbarIcon() { return taskbarIcon; }
