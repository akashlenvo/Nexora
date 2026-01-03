#include "application.h"

#include "gui/imgadjdlg.h"
#include "gui/streamconfigdlg.h"
#include "gui/devicesview.h"
#include "gui/qrconview.h"
#include "settings.h"
#include "video/guipreviewscaler.h"

Application::Application()
{
	SetAppearance(Appearance::System);
	wxInitAllImageHandlers();
	//wxImageHandler::

	Settings::Load();
	stateRegistry = Settings::GetDeviceStates();

	switch (Settings::Get("DIRECTSHOW_RESOLUTION") + Window::MenuIDs::DS_SD)
	{
		case Window::MenuIDs::DS_SD: dsSource = std::make_unique<DirectShowSource>(640, 480); break;
		case Window::MenuIDs::DS_HD: dsSource = std::make_unique<DirectShowSource>(1280, 720); break;
		case Window::MenuIDs::DS_FHD: dsSource = std::make_unique<DirectShowSource>(1920, 1080); break;
		case Window::MenuIDs::DS_QHD: dsSource = std::make_unique<DirectShowSource>(3840, 2160); break;
		default: dsSource = std::make_unique<DirectShowSource>(1280, 720);
	}

	server = std::make_unique<Server>(6969, *this);
	server->Start();

	rtspManager = std::make_unique<RTSP::Manager>(*server, [&](AVFrame* frame) {
		mainWindow->GetCanvas()->ProcessRawFrameAsync(frame);
		dsSource->SendRawFrame(frame);
	});

	mainWindow = new Window(server->GetHostInfo());
	BindEventListeners();
}

bool Application::OnInit()
{
	mainWindow->Show();
	return true;
}

StreamOptions& Application::GetCurrentDeviceStreamOptions()
{
	auto deviceId = rtspManager->GetStreamingDevice();
	const auto& desc = rtspManager->GetDescriptors()[deviceId];

	return stateRegistry[desc.name()];
}

void Application::BindEventListeners()
{
	mainWindow->Bind(wxEVT_CLOSE_WINDOW, &Application::OnWindowCloseEvent, this);
	mainWindow->Bind(wxEVT_MENU, &Application::OnMenuEvent, this);

	mainWindow->GetSourceChoice()->Bind(wxEVT_CHOICE, &Application::OnSourceChanged, this);
	mainWindow->GetAdjustmentsButton()->Bind(wxEVT_BUTTON, &Application::ShowAdjustmentsDialog, this);
	mainWindow->GetStreamOptionsButton()->Bind(wxEVT_BUTTON, &Application::ShowStreamConfigDialog, this);

	mainWindow->GetRotateLeftButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		rtspManager->Rotate(-90);
	});

	mainWindow->GetRotateRightButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		rtspManager->Rotate(90);
	});

	mainWindow->GetFlipButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		rtspManager->FlipHorizontally();
	});

	mainWindow->GetFlipVerticalButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		rtspManager->FlipVertically();
	});

	mainWindow->GetTorchButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		auto& options = GetCurrentDeviceStreamOptions();
		auto flash = options.flashEnabled = !options.flashEnabled;

		rtspManager->SetFlash(flash);
	});

	mainWindow->GetSwapButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		auto& options = GetCurrentDeviceStreamOptions();
		options.backCameraActive = !options.backCameraActive;

		rtspManager->SwapCamera();
	});

	mainWindow->GetZoomInButton()->Bind(wxEVT_BUTTON, [&](const wxEvent & arg) {
		auto& options = GetCurrentDeviceStreamOptions();
		options.zoom = std::min(10.0f, options.zoom - 0.5f); // Don't go higher than 10x
		mainWindow->GetZoomLevelLabel()->SetLabelText(wxString::Format("%.1fx", options.zoom));

		rtspManager->Zoom(options.zoom);
	});

	mainWindow->GetZoomOutButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		auto& options = GetCurrentDeviceStreamOptions();
		options.zoom = std::max(1.0f, options.zoom - 0.5f); // Don't go lower than 1x
		mainWindow->GetZoomLevelLabel()->SetLabelText(wxString::Format("%.1fx", options.zoom));
		
		rtspManager->Zoom(options.zoom);
	});
}

void Application::OnDeviceConnected(DeviceDescriptor& descriptor) const
{
	mainWindow->GetTaskbarIcon()->ShowBalloon("New stream available", "Streaming device " + descriptor.name() + " available!", 10, wxICON_INFORMATION);
	rtspManager->AddDescriptor(descriptor);
	UpdateAvailableDevices();
}

void Application::OnDeviceDisconnected(DeviceDescriptor& descriptor) const
{
	mainWindow->GetTaskbarIcon()->ShowBalloon("Stream eneded", "Streaming device " + descriptor.name() + " disconnected!", 10, wxICON_INFORMATION);
	rtspManager->RemoveDescriptor(descriptor);
	UpdateAvailableDevices();
}

void Application::UpdateAvailableDevices() const
{	
	mainWindow->GetSourceChoice()->Clear();
	for (auto& desc : rtspManager->GetDescriptors())
	{
		mainWindow->GetSourceChoice()->Append(desc.name());
	}

	mainWindow->GetSourceChoice()->SetSelection(rtspManager->GetStreamingDevice());
}

void Application::OnMenuEvent(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case Window::MenuIDs::DEVICES:
		{
			DevicesView devlistview(mainWindow, rtspManager->GetDescriptors());
			devlistview.ShowModal();
			break;
		}

		case Window::MenuIDs::QR:
		{
			auto info = server->GetHostInfo();
			QrconView qrview(std::get<0>(info), std::get<1>(info), std::get<2>(info), wxSize(150, 150));
			qrview.ShowModal();
			
			break;
		}
		
		case Window::MenuIDs::HIDE2TRAY:
		{
			Settings::Set("MINIMIZE_TASKBAR", event.IsChecked() ? 1 : 0);
			break;
		}

		case Window::MenuIDs::SHOWSTATS:
		{
			Settings::Set("SHOW_STATS", event.IsChecked() ? 1 : 0);
			mainWindow->GetStatsText()->Show(event.IsChecked());
			break;
		}

		case Window::MenuIDs::SAVESTATE:
		{
			Settings::Set("SAVE_DEVICE_STATES", event.IsChecked() ? 1 : 0);
			break;
		}

		case Window::MenuIDs::DS_SD:
		case Window::MenuIDs::DS_HD:
		case Window::MenuIDs::DS_FHD:
		case Window::MenuIDs::DS_QHD:
		{
			Settings::Set("DIRECTSHOW_RESOLUTION", event.GetId() - Window::MenuIDs::DS_SD);
			break;
		}
	}
}

void Application::OnSourceChanged(wxEvent& event)
{
	int deviceId = mainWindow->GetSourceChoice()->GetSelection();
	const auto& descriptor = rtspManager->GetDescriptors()[deviceId];

	EnsureStateInitialized(descriptor.name(), descriptor);
	auto& state = stateRegistry[descriptor.name()];
	
	// Make sure the resolution stored in state is actually avaialble
	// and if it's not default to the first resolution
	bool resFound = false;
	const auto& resList = state.backCameraActive ? descriptor.backResolutions() : descriptor.frontResolutions();
	for (size_t i = 0; i < resList.size(); i++) 
	{
		if (resList[i] == state.resolution) 
		{
			resFound = true;
			break;
		}
	}
	if (!resFound)
		state.resolution = resList[0];
	
	rtspManager->Connect2Stream(deviceId, state);
}

void Application::OnWindowCloseEvent(wxCloseEvent& event)
{
	// Hide window for reponsive UI 
	// Actual closing sequence can take longer (couple hundred milliseconds)
	mainWindow->Hide();

	rtspManager.reset();
	server->Close();

	Settings::UpdateDeviceStates(stateRegistry);
	Settings::Save();

	event.Skip();
}

void Application::EnsureStateInitialized(std::string name, const DeviceDescriptor& descriptor)
{
	auto& state = stateRegistry[name];

	state.backCameraActive = true;

	// 1. Initialize Sliders (Default 50 if empty)
	if (descriptor.filters().count(Video::Filter::Category::CORRECTION)) 
	{
		for (const auto& name : descriptor.filters().at(Video::Filter::Category::CORRECTION)) 
		{
			// Only add if missing (preserve user changes)
			if (state.filterSliderValues.find(name) == state.filterSliderValues.end()) 
			{
				state.filterSliderValues[name] = 50;
			}
		}
	}

	// 2. Initialize Dropdowns (Default "None")
	/*for (const auto& [cat, list] : descriptor.filters()) {
		if (cat == Video::Filter::Category::CORRECTION || cat == Video::Filter::Category::NONE) 
			continue;

		int catId = static_cast<int>(cat);
		if (state.activeFilters.find(catId) == state.activeFilters.end()) 
			state.activeFilters[catId] = "None";
	}*/
}

void Application::ShowAdjustmentsDialog(wxCommandEvent& event)
{
	if (rtspManager->GetDescriptors().empty())
		return;

	int currentDeviceId = rtspManager->GetStreamingDevice();
	if (currentDeviceId < 0)
		return;

	const auto& desc = rtspManager->GetDescriptors()[currentDeviceId];
	
	EnsureStateInitialized(desc.name(), desc);
	auto& state = stateRegistry[desc.name()];

	ImgAdjDlg dialog(nullptr, desc, state.filterSliderValues, state.activeEffectFilter);

	dialog.Bind(EVT_FILTER_PARAM_CHANGED, [&](const wxCommandEvent& event) {		
		auto name = event.GetString().ToStdString();
		auto value = event.GetInt();

		rtspManager->ApplyCorrectionFilter(name, value);
		state.filterSliderValues[name] = value;
	});

	dialog.Bind(EVT_FILTER_SWITCH_CHANGED, [&](const wxCommandEvent& event) {
		auto name = event.GetString().ToStdString();
		auto category = event.GetInt();

		rtspManager->ApplyEffectFilter(name);
		state.activeEffectFilter = name;
	});

	dialog.ShowModal();
}

void Application::ShowStreamConfigDialog(wxCommandEvent& event)
{
	int deviceId = rtspManager->GetStreamingDevice();
	if (deviceId < 0 || rtspManager->GetDescriptors().empty())
		return;

	const auto& desc = rtspManager->GetDescriptors()[deviceId];
	std::string deviceName = desc.name();

	EnsureStateInitialized(deviceName, desc);
	auto& state = stateRegistry[deviceName];

	// Prepare Config Object for Dialog
	StreamConfigDlg::Config config;

	config.resIndex = 0;
	const auto& resList = state.backCameraActive ? desc.backResolutions() : desc.frontResolutions();
	for (size_t i = 0; i < resList.size(); i++) 
	{
		if (resList[i] == state.resolution) 
		{
			config.resIndex = i;
			break;
		}
	}

	config.fps = state.fps;

	config.adaptiveBitrate = state.adaptiveBitrate;
	config.bitrate = state.bitrate;
	config.minBitrate = state.minBitrate;
	config.maxBitrate = state.maxBitrate;

	config.stabilizationEnabled = state.stabilizationEnabled;
	config.flashEnabled = state.flashEnabled;
	config.focusMode = state.focusMode;
	config.h265Enabled = state.h265Enabled;

	StreamConfigDlg dlg(nullptr, desc, state.backCameraActive, config);

	dlg.Bind(EVT_STREAM_RESOLUTION_CHANGED, [this, deviceName](wxCommandEvent& e) {
		wxString resStr = e.GetString(); // "1920 x 1080"

		long w = 0, h = 0;
		resStr.BeforeFirst('x').ToLong(&w);
		resStr.AfterFirst('x').ToLong(&h);

		rtspManager->SetResolution((uint16_t)w, (uint16_t)h);
		stateRegistry[deviceName].resolution = { (int)w, (int)h };
	});

	dlg.Bind(EVT_STREAM_FPS_CHANGED, [this, deviceName](wxCommandEvent& e) {
		int fps = e.GetInt();
		rtspManager->SetFPS(fps);
		stateRegistry[deviceName].fps = fps;
	});

	// Bitrate Changed (Handles both Static and Adaptive)
	dlg.Bind(EVT_STREAM_BITRATE_CHANGED, [this, deviceName, &dlg](wxCommandEvent& e) {
		auto& regState = stateRegistry[deviceName];

		// Read all values from dialog accessors
		regState.adaptiveBitrate = dlg.IsAdaptiveBitrate();
		regState.bitrate = dlg.GetStaticBitrate();
		regState.minBitrate = dlg.GetMinBitrate();
		regState.maxBitrate = dlg.GetMaxBitrate();

		if (regState.adaptiveBitrate) 
		{
			rtspManager->SetAdaptiveBitrate(regState.minBitrate, regState.maxBitrate);
		}
		else 
		{
			rtspManager->SetBitrate(regState.bitrate);
		}
	});

	// Hardware Toggles
	dlg.Bind(EVT_STREAM_CONFIG_CHANGED, [this, deviceName, &dlg](wxCommandEvent& e) {
		auto& regState = stateRegistry[deviceName];
		
		if (regState.stabilizationEnabled != dlg.IsStabilizationEnabled()) 
		{
			regState.stabilizationEnabled = dlg.IsStabilizationEnabled();
			rtspManager->SetStabilization(regState.stabilizationEnabled);
		}

		if (regState.flashEnabled != dlg.IsFlashEnabled()) 
		{
			regState.flashEnabled = dlg.IsFlashEnabled();
			rtspManager->SetFlash(regState.flashEnabled);
		}

		if (regState.focusMode != dlg.GetFocusMode()) 
		{
			regState.focusMode = dlg.GetFocusMode();
			rtspManager->SetFocusMode(regState.focusMode);
		}

		if (regState.h265Enabled != dlg.IsH265Enabled()) 
		{
			regState.h265Enabled = dlg.IsH265Enabled();
			rtspManager->SetH265Codec(regState.h265Enabled);
		}
	});

	dlg.ShowModal();
}