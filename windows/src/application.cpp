#include "application.h"

#include "gui/imgadjdlg.h"
#include "gui/devicesview.h"
#include "gui/qrconview.h"
#include "settings.h"
#include "logger.h"
#include "video/guipreviewscaler.h"
#include "video/directshowscaler.h"

#include <qrcodegen.hpp>

Application::Application()
{
	SetAppearance(Appearance::System);

	Settings::load();
	wxInitAllImageHandlers();
	//wxImageHandler::

	dsSource = std::make_unique<DirectShowSource>(1280, 720);

	server = std::make_unique<Server>(6969, *this);
	server->Start();

	rtspManager = std::make_unique<RTSP::Manager>(*server, [&](AVFrame* frame) {
		mainWindow->GetCanvas()->ProcessRawFrameAsync(frame);
		dsSource->SendRawFrame(frame);
	});

	mainWindow = new Window(server->GetHostInfo());

	mainWindow->Bind(wxEVT_CLOSE_WINDOW, &Application::OnWindowCloseEvent, this);
	mainWindow->Bind(wxEVT_MENU, &Application::OnMenuEvent, this);

	mainWindow->GetSourceChoice()->Bind(wxEVT_CHOICE, &Application::OnSourceChanged, this);
	mainWindow->GetResolutionChoice()->Bind(wxEVT_CHOICE, &Application::OnResolutionChanged, this);
	mainWindow->GetAdjustmentsButton()->Bind(wxEVT_BUTTON, &Application::ShowAdjustmentsDialog, this);

	mainWindow->GetRotateLeftButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		//int rotation = stream->RotateLeft();
		/*SetVideoOptions(
			cameraWidth, 
			cameraHeight, 
			cameraAspectRatioW, 
			cameraAspectRatioH, 
			rotation == Stream::Transforms::ROTATE_90 || rotation == Stream::Transforms::ROTATE_270
		);*/
	});

	mainWindow->GetRotateRightButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		//int rotation = stream->RotateRight();
		/*SetVideoOptions(
			cameraWidth,
			cameraHeight,
			cameraAspectRatioW,
			cameraAspectRatioH,
			rotation == Stream::Transforms::ROTATE_90 || rotation == Stream::Transforms::ROTATE_270
		);*/
	});

	mainWindow->GetFlipButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		//stream->Mirror();
	});

	mainWindow->GetSwapButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		backCameraActive = !backCameraActive;
		auto deviceId = rtspManager->GetStreamingDevice();
		UpdateAvailableResolutions(deviceId, backCameraActive ? 0 : 1);
		rtspManager->SwapCamera();
	});

	// Create a camera handle to access the DirechShow Virtual Camera filter
	backCameraActive = true;
}

bool Application::OnInit()
{
	mainWindow->Show();
	return true;
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

int Application::UpdateAvailableResolutions(int id, int camera)
{
	mainWindow->GetResolutionChoice()->Clear();

	const auto& resolutions = camera == 0 ? rtspManager->GetDescriptors()[id].backResolutions() : rtspManager->GetDescriptors()[id].frontResolutions();
	int defaultResolutionPos = 0;

	for (int i = 0; i < resolutions.size(); i++)
	{
		const auto& resolution = resolutions[i];
		mainWindow->GetResolutionChoice()->Append(std::to_string(resolution.first) + " x " + std::to_string(resolution.second));

		if (resolution.first == 640 && resolution.second == 480)
			defaultResolutionPos = i;
	}

	mainWindow->GetResolutionChoice()->SetSelection(defaultResolutionPos);
	return defaultResolutionPos;
}

void Application::OnMenuEvent(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case Window::MenuIDs::DEVICES:
		{
			//DevicesView devlistview(server->GetConnectedDevicesInfo());
			//devlistview.ShowModal();
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
			Settings::set("MINIMIZE_TASKBAR", event.IsChecked() ? 1 : 0);
			break;
		}

		case Window::MenuIDs::SHOWSTATS:
		{
			Settings::set("SHOW_STATS", event.IsChecked() ? 1 : 0);
			mainWindow->GetStatsText()->Show(event.IsChecked());
			break;
		}
	}
}

void Application::OnSourceChanged(wxEvent& event)
{
	int selection = mainWindow->GetSourceChoice()->GetSelection();
	int resoltution = UpdateAvailableResolutions(selection, 0);

	rtspManager->Connect2Stream(selection, resoltution);
}

void Application::OnWindowCloseEvent(wxCloseEvent& event)
{
	// Hide window for reponsive UI 
	// Actual closing sequence can take longer (couple hundred milliseconds)
	mainWindow->Hide();

	rtspManager.reset();
	server->Close();

	Settings::save();
	event.Skip();
}

void Application::OnResolutionChanged(wxEvent& event)
{
	// Pause the stream so no more frames
	// are sent to the Softcam DirectShow filter
	// Do this in order to avoid deleting the current camera 
	// while still writing to the video buffer which will result
	// in a memory violation error
	//
	// The other functions before ScCameraInit should add enough
	// delay so the previous frame is done writing to the video buffer
	// 
	// (not ideal -> TODO: add a better synchronization mechanism)

	auto resolutionStr = mainWindow->GetResolutionChoice()->GetStringSelection();
	int width, height;
	std::sscanf(resolutionStr.c_str(), "%d x %d", &width, &height);

	// mainWindow->GetCanvas()->SetAspectRatio(width, height);

	cameraWidth = width;
	cameraHeight = height;

	rtspManager->SetStreamResolution(width, height);
}

void Application::ShowAdjustmentsDialog(wxCommandEvent& event)
{
	// ImgAdjDlg dialog(nullptr, stream->GetAdjustments());

	/*dialog.Bind(EVT_BRIGHTNESS_CHANGED, [&](const wxCommandEvent& event) {
		stream->SetBrightnessAdjustment(event.GetInt());f
	});

	dialog.Bind(EVT_SATURATION_CHANGED, [&](const wxCommandEvent& event) {
		stream->SetSaturationAdjustment(event.GetInt());
	});

	dialog.Bind(EVT_JPEGQUALITY_CHANGED, [&](const wxCommandEvent& event) {
		stream->SetQualityAdjustment(event.GetInt());
		server->SetStreamingQuality(event.GetInt());
	});

	dialog.Bind(EVT_WB_CHANGED, [&](const wxCommandEvent& event) {
		stream->SetWBAdjustment(event.GetInt());
		server->SetStreamingWB(event.GetInt());
	});

	dialog.Bind(EVT_EFFECT_CHANGED, [&](const wxCommandEvent& event) {
		stream->SetEffectAdjustment(event.GetInt());
		server->SetStreamingEffect(event.GetInt());
	});*/
	// dialog.ShowModal();
}