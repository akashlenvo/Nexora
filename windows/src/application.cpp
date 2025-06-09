#include "application.h"

#include "gui/imgadjdlg.h"
#include "gui/devicesview.h"
#include "gui/qrconview.h"
#include "settings.h"
#include "logger.h"

#include <qrcodegen.hpp>

Application::Application()
{
	Settings::load();
	wxInitAllImageHandlers();
	//wxImageHandler::

	stream = std::make_unique<Stream>([&](const wxImage& image, Stream::FrameStats stats) {
		// Check the image dimensions
		// After changing the resolution there might still be incoming 
		// frames with the previous resolution and those need to be skipped
		// 
		// Also check the image width/height against the camera height/width
		// in case the image is rotated
		if (image.GetWidth() == cameraWidth && image.GetHeight() == cameraHeight || image.GetWidth() == cameraHeight && image.GetHeight() == cameraWidth)
		{
			mainWindow->GetCanvas()->Render(image);
			UpdateFrameStats(stats);

			// Send the current image frame to the DirechShow Virtual Camera filter
			scSendFrame(camera, stream->GetBGR(image));
		}
	});

	server = std::make_unique<Server>(6969, *this, *stream);
	server->Start();

	mainWindow = new Window(server->GetHostInfo());

	mainWindow->Bind(wxEVT_CLOSE_WINDOW, &Application::OnWindowCloseEvent, this);
	mainWindow->Bind(wxEVT_MENU, &Application::OnMenuEvent, this);

	mainWindow->GetResolutionChoice()->Bind(wxEVT_CHOICE, &Application::OnResolutionChanged, this);
	mainWindow->GetAdjustmentsButton()->Bind(wxEVT_BUTTON, &Application::ShowAdjustmentsDialog, this);

	mainWindow->GetSourceChoice()->Bind(wxEVT_CHOICE, [&](const wxEvent& arg) {
		int selection = mainWindow->GetSourceChoice()->GetSelection();
		server->SetStreamingDevice(selection);

		mainWindow->GetResolutionChoice()->SetSelection(0);
	});

	mainWindow->GetRotateLeftButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		int rotation = stream->RotateLeft();
		SetVideoOptions(
			cameraWidth, 
			cameraHeight, 
			cameraAspectRatioW, 
			cameraAspectRatioH, 
			rotation == Stream::Transforms::ROTATE_90 || rotation == Stream::Transforms::ROTATE_270
		);
	});

	mainWindow->GetRotateRightButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		int rotation = stream->RotateRight();
		SetVideoOptions(
			cameraWidth,
			cameraHeight,
			cameraAspectRatioW,
			cameraAspectRatioH,
			rotation == Stream::Transforms::ROTATE_90 || rotation == Stream::Transforms::ROTATE_270
		);
	});

	mainWindow->GetFlipButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		stream->Mirror();
	});

	mainWindow->GetSwapButton()->Bind(wxEVT_BUTTON, [&](const wxEvent& arg) {
		backCameraActive = !backCameraActive;
		server->SetStreamingCamera(backCameraActive);
	});

	// Create a camera handle to access the DirechShow Virtual Camera filter
	backCameraActive = true;
	camera = nullptr;
	SetVideoOptions(640, 480, 4, 3);
}

bool Application::OnInit()
{
	mainWindow->Show();
	return true;
}

void Application::OnDeviceConnected(std::string device) const
{
	mainWindow->GetTaskbarIcon()->ShowBalloon("Device connected", "Device " + device + " connected!", 10, wxICON_INFORMATION);
	UpdateAvailableDevices();
}

void Application::OnDeviceDisconnected(std::string device) const
{
	mainWindow->GetTaskbarIcon()->ShowBalloon("Device disconnected", "Device " + device + " disconnected!", 10, wxICON_INFORMATION);
	UpdateAvailableDevices();
}

void Application::UpdateAvailableDevices() const
{
	auto deviceInfo = server->GetConnectedDevicesInfo();
	
	mainWindow->GetSourceChoice()->Clear();
	for (auto& info : deviceInfo)
	{
		mainWindow->GetSourceChoice()->Append(info.name);
	}

	mainWindow->GetSourceChoice()->SetSelection(server->GetStreamingDevice());
}

void Application::OnMenuEvent(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case Window::MenuIDs::DEVICES:
		{
			DevicesView devlistview(server->GetConnectedDevicesInfo());
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

void Application::OnWindowCloseEvent(wxCloseEvent& event)
{
	stream->Close();
	server->Close();
	Settings::save();
	mainWindow->Destroy();
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
	stream->Pause();

	int selection = mainWindow->GetResolutionChoice()->GetSelection();

	if (selection == 0)
	{
		logger << "[STREAM] Set stream resolution to 640x480\n";
		SetVideoOptions(640, 480, 4, 3);
		//mainWindow->GetCanvas()->SetAspectRatio(4, 3);
		//server->SetStreamResolution(640, 480);

		// Update scCamera resolution
		//ScCameraInit(640, 480);
	}
	else
	{
		//mainWindow->GetCanvas()->SetAspectRatio(16, 9);
		if (selection == 1)
		{
			SetVideoOptions(1280, 720, 16, 9);
			logger << "[STREAM] Set stream resolution to 1280x720\n";
			//server->SetStreamResolution(1280, 720);
			// Update scCamera resolution
			//ScCameraInit(1280, 720);
		}
		else if (selection == 2)
		{
			SetVideoOptions(1920, 1080, 16, 9);
			logger << "[STREAM] Set stream resolution to 1920x1080\n";
			//server->SetStreamResolution(1920, 1080);
			// Update scCamera resolution
			//ScCameraInit(1920, 1080);
		}
	}

	stream->Unpause();
}

void Application::ShowAdjustmentsDialog(wxCommandEvent& event)
{
	ImgAdjDlg dialog(nullptr, stream->GetAdjustments());

	dialog.Bind(EVT_BRIGHTNESS_CHANGED, [&](const wxCommandEvent& event) {
		stream->SetBrightnessAdjustment(event.GetInt());
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
	});

	dialog.ShowModal();
}

void Application::SetVideoOptions(int width, int height, int aspectRatioW, int aspectRatioH, bool portrait)
{
	cameraWidth = width;
	cameraHeight = height;
	cameraAspectRatioW = aspectRatioW;
	cameraAspectRatioH = aspectRatioH;

	// The canvas and scCamera should have the correct dimensions and aspect ratios
	// corresponding to the indicated orientation to correctly display a rotated image
	if (portrait)
	{
		mainWindow->GetCanvas()->SetAspectRatio(cameraAspectRatioH, cameraAspectRatioW);
	}
	else
	{
		mainWindow->GetCanvas()->SetAspectRatio(cameraAspectRatioW, cameraAspectRatioH);
	}

	// But the client should still stream in the landscape (default) orientation
	server->SetStreamResolution(width, height);

	// Update scCamera resolution
	if (camera != nullptr)
	{
		scDeleteCamera(camera);
	}
	if (portrait)
	{
		camera = scCreateCamera(cameraHeight, cameraWidth, 0);
	}
	else
	{
		camera = scCreateCamera(cameraWidth, cameraHeight, 0);
	}
}

void Application::UpdateFrameStats(Stream::FrameStats stats)
{
	if (Settings::get("SHOW_STATS"))
	{
		mainWindow->GetStatsText()->SetLabel(wxString::Format("frame time: %lldms | frame size: %lldkb", stats.time, stats.size / 1024));
	}
}