#include "gui/window.h"
#include "settings.h"
#include "icon.xpm"

#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/statline.h>

Window::Window(Server::HostInfo hostinfo)
// Adjusted size slightly to ensure top bar items fit comfortably
	: wxFrame(nullptr, wxID_ANY, "VCamdroid", wxDefaultPosition, wxSize(500, 480), wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)
{
	wxPanel* panel = nullptr;
	try {
		panel = new wxPanel(this, wxID_ANY);
	}
	catch (...) {
		wxMessageBox("Failed to create UI Panel.", "Error");
		return;
	}

	if (wxSystemSettings::GetAppearance().IsDark())
	{
		panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	}

	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

	taskbarIcon = new wxTaskBarIcon();
	taskbarIcon->SetIcon(icon, "VCamdroid");
	taskbarIcon->Bind(wxEVT_TASKBAR_LEFT_DCLICK, &Window::MaximizeFromTaskbar, this);
	Bind(wxEVT_ICONIZE, &Window::MinimizeToTaskbar, this);

	SetIcon(icon);
	InitializeMenu(hostinfo);

	InitializeTopBar(panel, topsizer);
	InitializeCanvasPanel(panel, topsizer);
	InitializeBottomBar(panel, topsizer);

	panel->SetSizer(topsizer);
	panel->Layout();

	Center();
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
	c1->Check(Settings::Get("MINIMIZE_TASKBAR") == 1);

	auto c2 = file->AppendCheckItem(MenuIDs::SHOWSTATS, "Show frame stats");
	c2->Check(Settings::Get("SHOW_STATS") == 1);

	auto c3 = file->AppendCheckItem(MenuIDs::SAVESTATE, "Save device presets");
	c3->Check(Settings::Get("SAVE_DEVICE_STATE") == 1);

	wxMenu* dsresolutions = new wxMenu();
	dsresolutions->AppendRadioItem(MenuIDs::DS_SD, "640 x 480", "Standard 4:3");
	dsresolutions->AppendRadioItem(MenuIDs::DS_HD, "1280 x 720", "HD");
	dsresolutions->AppendRadioItem(MenuIDs::DS_FHD, "1920 x 1080", "FHD");
	dsresolutions->AppendRadioItem(MenuIDs::DS_QHD, "3840 x 2160", "QHD");

	auto selected = Settings::Get("DIRECTSHOW_RESOLUTION");
	dsresolutions->Check((selected != -1 ? selected : 0) + 105, true);

	file->AppendSubMenu(dsresolutions, "DirectShow Resolution", "Requires restart");

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

void Window::InitializeTopBar(wxPanel* parent, wxBoxSizer* topsizer)
{
	wxBoxSizer* topBarSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* srcLabel = new wxStaticText(parent, wxID_ANY, "Source:");
	wxFont font = srcLabel->GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	srcLabel->SetFont(font);
	topBarSizer->Add(srcLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

	sourceChoice = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, new wxString[1]{ "No devices" });
	topBarSizer->Add(sourceChoice, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

	// Pushes everything after this to the right
	topBarSizer->AddStretchSpacer(1);

	// --- STATS TEXT (New Location) ---
	// Initial text with 2 lines as placeholder
	statsText = new wxStaticText(parent, wxID_ANY, "- @ -fps\n- mbps", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);

	// Slightly smaller font looks better for data display
	wxFont statsFont = statsText->GetFont();
	statsFont.SetPointSize(statsFont.GetPointSize() - 1);
	statsText->SetFont(statsFont);

	statsText->Show(Settings::Get("SHOW_STATS") == 1);

	// Add with some right padding to separate from the settings button
	topBarSizer->Add(statsText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

	// Settings Button
	streamOptionsButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/setting.png", wxBITMAP_TYPE_PNG));
	streamOptionsButton->SetToolTip("Stream Settings");
	topBarSizer->Add(streamOptionsButton, 0, wxALIGN_CENTER_VERTICAL);

	topsizer->Add(topBarSizer, 0, wxEXPAND | wxALL, 10);
}

void Window::InitializeCanvasPanel(wxPanel* parent, wxBoxSizer* topsizer)
{
	// Just the Canvas now
	canvas = new Canvas(parent, wxDefaultPosition, wxSize(400, 300));
	topsizer->Add(canvas, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxBOTTOM, 10);
}

void Window::InitializeBottomBar(wxPanel* parent, wxBoxSizer* topsizer)
{
	wxStaticLine* line = new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxLI_HORIZONTAL);
	topsizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

	wxBoxSizer* bottomBarSizer = new wxBoxSizer(wxHORIZONTAL);

	// --- GROUP 1: TRANSFORMS ---
	wxBoxSizer* groupTransform = new wxBoxSizer(wxHORIZONTAL);

	rotateLeftButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/rotate-left.png", wxBITMAP_TYPE_PNG));
	rotateLeftButton->SetToolTip("Rotate Left");
	groupTransform->Add(rotateLeftButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);

	rotateRightButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/rotate-right.png", wxBITMAP_TYPE_PNG));
	rotateRightButton->SetToolTip("Rotate Right");
	groupTransform->Add(rotateRightButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);

	flipButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/flip.png", wxBITMAP_TYPE_PNG));
	flipButton->SetToolTip("Flip Horizontally");
	groupTransform->Add(flipButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);

	flipVerticalButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/flip-v.png", wxBITMAP_TYPE_PNG));
	flipVerticalButton->SetToolTip("Flip Vertically");
	groupTransform->Add(flipVerticalButton, 0, wxALIGN_CENTER_VERTICAL);

	bottomBarSizer->Add(groupTransform, 0, wxALIGN_CENTER_VERTICAL);
	bottomBarSizer->AddStretchSpacer(1);


	// --- GROUP 2: ZOOM ---
	wxBoxSizer* groupZoom = new wxBoxSizer(wxHORIZONTAL);

	zoomOutButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/zoom-out.png", wxBITMAP_TYPE_PNG));
	zoomOutButton->SetToolTip("Zoom Out");
	groupZoom->Add(zoomOutButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

	zoomLevelLabel = new wxStaticText(parent, wxID_ANY, "1.0x", wxDefaultPosition, wxSize(32, -1), wxALIGN_CENTER);
	wxFont smallFont = zoomLevelLabel->GetFont();
	smallFont.SetPointSize(smallFont.GetPointSize() - 1);
	zoomLevelLabel->SetFont(smallFont);
	groupZoom->Add(zoomLevelLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

	zoomInButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/zoom-in.png", wxBITMAP_TYPE_PNG));
	zoomInButton->SetToolTip("Zoom In");
	groupZoom->Add(zoomInButton, 0, wxALIGN_CENTER_VERTICAL);

	bottomBarSizer->Add(groupZoom, 0, wxALIGN_CENTER_VERTICAL);
	bottomBarSizer->AddStretchSpacer(1);


	// --- GROUP 3: DEVICE ---
	wxBoxSizer* groupDevice = new wxBoxSizer(wxHORIZONTAL);

	torchButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/flash.png", wxBITMAP_TYPE_PNG));
	torchButton->SetToolTip("Toggle Flash");
	groupDevice->Add(torchButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	swapButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/swap.png", wxBITMAP_TYPE_PNG));
	swapButton->SetToolTip("Switch Camera");
	groupDevice->Add(swapButton, 0, wxALIGN_CENTER_VERTICAL);

	snapshotButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/photo.png", wxBITMAP_TYPE_PNG));
	snapshotButton->SetToolTip("Take Snapshot");
	groupDevice->Add(snapshotButton, 0, wxALIGN_CENTER_VERTICAL);

	bottomBarSizer->Add(groupDevice, 0, wxALIGN_CENTER_VERTICAL);
	bottomBarSizer->AddStretchSpacer(1);


	// --- GROUP 4: TOOLS ---
	wxBoxSizer* groupTools = new wxBoxSizer(wxHORIZONTAL);

	adjustmentsButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap("res/settings.png", wxBITMAP_TYPE_PNG));
	adjustmentsButton->SetToolTip("Image Adjustments");
	groupTools->Add(adjustmentsButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);

	bottomBarSizer->Add(groupTools, 0, wxALIGN_CENTER_VERTICAL);

	topsizer->Add(bottomBarSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
}

void Window::MinimizeToTaskbar(wxIconizeEvent& evt)
{
	if (Settings::Get("MINIMIZE_TASKBAR") == 1)
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