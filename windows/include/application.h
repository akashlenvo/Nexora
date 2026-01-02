#pragma once

#include <wx/wx.h>

#include "gui/window.h"
#include "net/server.h"
#include "rtsp/manager.h"
#include "directshowsource.h"

class Application : public wxApp, public Server::ConnectionListener
{
public:

	/*
		State store for a connected device
	*/
	struct State
	{
		// State registry mapping device name to its cached state store
		using Registry = std::map<std::string, State>;

		int fps = 30;
		std::pair<int, int> resolution = { 640, 480 };
		bool backCameraActive = false;

		// Cached values of adjustment filters
		std::map<std::string, int> filterSliderValues;
		// Cached selected filters per every effect category
		std::map<int, std::string> activeFilters;
	};

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

	void EnsureStateInitialized(std::string name, const DeviceDescriptor& descriptor);

	// void SetVideoOptions(int width, int height, int aspectRatioW, int aspectRatioH, bool portrait = false);
};