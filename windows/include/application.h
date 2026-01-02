#pragma once

#include <wx/wx.h>

#include "gui/window.h"
#include "net/server.h"
#include "rtsp/manager.h"
#include "directshowsource.h"
#include "state.h"

class Application : public wxApp, public Server::ConnectionListener
{
public:
	Application();

	virtual bool OnInit();
	
	virtual void OnDeviceConnected(DeviceDescriptor& descriptor) const override;
	virtual void OnDeviceDisconnected(DeviceDescriptor& descriptor) const override;

private:
	Window* mainWindow;

	State::Registry stateRegistry;

	std::unique_ptr<Server> server;
	std::unique_ptr<RTSP::Manager> rtspManager;
	std::unique_ptr<DirectShowSource> dsSource;

	void UpdateAvailableDevices() const;

	void OnSourceChanged(wxEvent& event);

	void ShowAdjustmentsDialog(wxCommandEvent& event);
	void ShowStreamConfigDialog(wxCommandEvent& event);
	
	void OnMenuEvent(wxCommandEvent& event);
	void OnWindowCloseEvent(wxCloseEvent& event);

	void EnsureStateInitialized(std::string name, const DeviceDescriptor& descriptor);

	// void SetVideoOptions(int width, int height, int aspectRatioW, int aspectRatioH, bool portrait = false);
};