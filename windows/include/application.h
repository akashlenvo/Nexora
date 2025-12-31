#pragma once

#include <wx/wx.h>

#include "gui/window.h"
#include "net/server.h"
#include "rtsp/manager.h"

#include "softcam/softcam.h"

class Application : public wxApp, public Server::ConnectionListener
{
public:
	Application();

	virtual bool OnInit();
	
	virtual void OnDeviceConnected(DeviceDescriptor& descriptor) const override;
	virtual void OnDeviceDisconnected(DeviceDescriptor& descriptor) const override;

private:
	Window* mainWindow;

	scCamera camera;
	int cameraWidth, cameraHeight;
	int cameraAspectRatioW, cameraAspectRatioH;

	std::unique_ptr<Server> server;
	std::unique_ptr<RTSP::Manager> rtspManager;

	bool backCameraActive;

	void UpdateAvailableDevices() const;

	/*
		Update avaialble resolutions of the device with given id
		for the given camera (0 for back, 1 for front)

		Returns the id of the default initial resolution (640x480)
	*/
	int UpdateAvailableResolutions(int id, int camera);

	void OnMenuEvent(wxCommandEvent& event);
	void OnSourceChanged(wxEvent& event);
	void OnResolutionChanged(wxEvent& event);
	void ShowAdjustmentsDialog(wxCommandEvent& event);
	void OnWindowCloseEvent(wxCloseEvent& event);

	// void SetVideoOptions(int width, int height, int aspectRatioW, int aspectRatioH, bool portrait = false);
};