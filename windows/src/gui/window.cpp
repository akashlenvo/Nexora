#include "gui/window.h"
#include "settings.h"

#include <wx/settings.h>
#include <wx/statline.h>
#include <algorithm>

namespace {
const wxColour kInk(8, 13, 24);
const wxColour kSurface(18, 26, 43);
const wxColour kSurfaceRaised(27, 37, 58);
const wxColour kPrimary(108, 99, 255);
const wxColour kAccent(33, 212, 180);
const wxColour kText(242, 245, 251);
const wxColour kMuted(178, 188, 208);

wxButton* MakeIconButton(wxWindow* parent, const wxString& icon, const wxString& tooltip)
{
	auto* button = new wxBitmapButton(parent, wxID_ANY,
		wxBitmap(wxString("res/") + icon, wxBITMAP_TYPE_PNG), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	button->SetBackgroundColour(kSurfaceRaised);
	button->SetMinSize(parent->FromDIP(wxSize(40, 40)));
	button->SetToolTip(tooltip);
	return button;
}
}

Window::Window(Server::HostInfo hostinfo)
	: wxFrame(nullptr, wxID_ANY, "Nexora Studio", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
	SetClientSize(FromDIP(wxSize(900, 640)));
	SetMinClientSize(FromDIP(wxSize(620, 480)));
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
	titleFont.SetPointSize(titleFont.GetPointSize() + 5);
	titleFont.SetWeight(wxFONTWEIGHT_BOLD);
	title->SetFont(titleFont);
	title->SetForegroundColour(kText);
	auto* tagline = new wxStaticText(header, wxID_ANY, "Your phone. Your camera. Connected.");
	tagline->SetForegroundColour(kMuted);
	brand->Add(title);
	brand->Add(tagline, 0, wxTOP, FromDIP(1));
	row->Add(brand, 0, wxALIGN_CENTER_VERTICAL);
	row->AddStretchSpacer();
	statusText = new wxStaticText(header, wxID_ANY, "OFFLINE  |  Waiting for a device",
		wxDefaultPosition, FromDIP(wxSize(210, -1)), wxST_ELLIPSIZE_END | wxALIGN_RIGHT);
	statusText->SetForegroundColour(kMuted);
	row->Add(statusText, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(12));
	auto* qrButton = new wxButton(header, MenuIDs::QR, "QR code");
	qrButton->SetBackgroundColour(kPrimary);
	qrButton->SetForegroundColour(*wxWHITE);
	qrButton->SetToolTip("Show the Wi-Fi connection QR code");
	row->Add(qrButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(6));
	auto* devicesButton = new wxButton(header, MenuIDs::DEVICES, "Devices");
	row->Add(devicesButton, 0, wxALIGN_CENTER_VERTICAL);
	header->SetSizer(row);
	topsizer->Add(header, 0, wxEXPAND);
	row->SetMinSize(-1, FromDIP(68));
	row->InsertSpacer(0, FromDIP(18));
	row->AddSpacer(FromDIP(18));
}

void Window::InitializeTopBar(wxPanel* parent, wxBoxSizer* topsizer)
{
	auto* card = new wxPanel(parent);
	card->SetBackgroundColour(kSurface);
	auto* row = new wxBoxSizer(wxHORIZONTAL);
	auto* label = new wxStaticText(card, wxID_ANY, "SOURCE");
	label->SetForegroundColour(kMuted);
	auto font = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	row->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(12));
	sourceChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, new wxString[1]{ "No devices detected" });
	sourceChoice->SetMinSize(FromDIP(wxSize(170, -1)));
	sourceChoice->SetSelection(0);
	row->Add(sourceChoice, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(12));
	statsText = new wxStaticText(card, wxID_ANY, "-- x --  |  -- fps  |  -- Mbps", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	statsText->SetForegroundColour(kAccent);
	statsText->Show(Settings::Get("SHOW_STATS") == 1);
	row->Add(statsText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(10));
	streamOptionsButton = MakeIconButton(card, "setting.png", "Stream settings");
	row->Add(streamOptionsButton, 0, wxALIGN_CENTER_VERTICAL);
	card->SetSizer(row);
	topsizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(12));
	row->SetMinSize(-1, FromDIP(52));
	row->InsertSpacer(0, FromDIP(14)); row->AddSpacer(FromDIP(14));
}

void Window::InitializeCanvasPanel(wxPanel* parent, wxBoxSizer* topsizer)
{
	canvas = new Canvas(parent, wxDefaultPosition, FromDIP(wxSize(640, 360)));
	canvas->SetMinSize(FromDIP(wxSize(320, 180)));
	canvas->SetBackgroundColour(wxColour(3, 6, 12));
	topsizer->Add(canvas, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, FromDIP(12));
}

void Window::InitializeBottomBar(wxPanel* parent, wxBoxSizer* topsizer)
{
	auto* bar = new wxPanel(parent);
	bar->SetBackgroundColour(kSurface);
	auto* row = new wxBoxSizer(wxHORIZONTAL);
	rotateLeftButton = MakeIconButton(bar, "rotate-left.png", "Rotate left");
	rotateRightButton = MakeIconButton(bar, "rotate-right.png", "Rotate right");
	flipButton = MakeIconButton(bar, "flip.png", "Mirror horizontally");
	flipVerticalButton = MakeIconButton(bar, "flip-v.png", "Flip vertically");
	zoomOutButton = MakeIconButton(bar, "zoom-out.png", "Zoom out");
	zoomLevelLabel = new wxStaticText(bar, wxID_ANY, "1.0x", wxDefaultPosition, FromDIP(wxSize(42, -1)), wxALIGN_CENTER);
	zoomLevelLabel->SetForegroundColour(kText);
	zoomInButton = MakeIconButton(bar, "zoom-in.png", "Zoom in");
	torchButton = MakeIconButton(bar, "flash.png", "Toggle flash");
	swapButton = MakeIconButton(bar, "swap.png", "Switch front/back camera");
	snapshotButton = MakeIconButton(bar, "photo.png", "Save a snapshot");
	adjustmentsButton = MakeIconButton(bar, "settings.png", "Image adjustments and filters");

	for (auto* button : { rotateLeftButton, rotateRightButton, flipButton, flipVerticalButton })
		row->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(4));
	row->AddStretchSpacer();
	row->Add(zoomOutButton, 0, wxALIGN_CENTER_VERTICAL);
	row->Add(zoomLevelLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(5));
	row->Add(zoomInButton, 0, wxALIGN_CENTER_VERTICAL);
	row->AddStretchSpacer();
	for (auto* button : { torchButton, swapButton, snapshotButton, adjustmentsButton })
		row->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(4));

	bar->SetSizer(row);
	row->SetMinSize(-1, FromDIP(56));
	row->InsertSpacer(0, FromDIP(10));
	row->AddSpacer(FromDIP(10));
	topsizer->Add(bar, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(12));
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
