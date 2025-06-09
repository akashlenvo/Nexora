#pragma once

#include <wx/wx.h>

#include "gui/window.h"
#include "net/server.h"
#include "stream.h"

#include "softcam/softcam.h"

class Application : public wxApp, public Server::ConnectionListener
{
public:
	Application();

	virtual bool OnInit();
	
	virtual void OnDeviceConnected(std::string device) const override;
	virtual void OnDeviceDisconnected(std::string device) const override;

private:
	Window* mainWindow;

	scCamera camera;
	int cameraWidth, cameraHeight;
	int cameraAspectRatioW, cameraAspectRatioH;
	std::unique_ptr<Server> server;
	std::unique_ptr<Stream> stream;
	bool backCameraActive;

	void UpdateAvailableDevices() const;

	void OnMenuEvent(wxCommandEvent& event);
	void OnResolutionChanged(wxEvent& event);
	void ShowAdjustmentsDialog(wxCommandEvent& event);
	void OnWindowCloseEvent(wxCloseEvent& event);

	void SetVideoOptions(int width, int height, int aspectRatioW, int aspectRatioH, bool portrait = false);

	void UpdateFrameStats(Stream::FrameStats stats);
};