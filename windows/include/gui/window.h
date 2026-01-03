#pragma once

#include <wx/wx.h>
#include <wx/taskbar.h>
#include <map>

#include "canvas.h"
#include "net/server.h"

class Window : public wxFrame
{
public:
	struct MenuIDs
	{
		static const int QR = 100;
		static const int DEVICES = 101;
		static const int HIDE2TRAY = 102;
		static const int SHOWSTATS = 103;
		static const int SAVESTATE = 104;
		static const int DS_SD = 105;
		static const int DS_HD = 106;
		static const int DS_FHD = 107;
		static const int DS_QHD = 108;
	};

	Window(Server::HostInfo hostInfo);
	~Window();

	Canvas* GetCanvas();
	wxChoice* GetSourceChoice();
	wxButton* GetStreamOptionsButton();

	// Bottom Bar Controls
	wxButton* GetRotateLeftButton();
	wxButton* GetRotateRightButton();
	wxButton* GetFlipButton();
	wxButton* GetFlipVerticalButton();

	wxButton* GetZoomInButton();
	wxButton* GetZoomOutButton();
	wxStaticText* GetZoomLevelLabel();

	wxButton* GetTorchButton();
	wxButton* GetSwapButton();

	wxButton* GetAdjustmentsButton();
	wxButton* GetSnapshotButton();

	wxStaticText* GetStatsText();
	wxTaskBarIcon* GetTaskbarIcon();

private:
	wxTaskBarIcon* taskbarIcon;
	Canvas* canvas;

	wxChoice* sourceChoice;
	wxButton* streamOptionsButton;

	// Bottom Bar
	wxButton* rotateLeftButton;
	wxButton* rotateRightButton;
	wxButton* flipButton;
	wxButton* flipVerticalButton;

	wxButton* zoomInButton;
	wxButton* zoomOutButton;
	wxStaticText* zoomLevelLabel;

	wxButton* torchButton;
	wxButton* swapButton;

	wxButton* adjustmentsButton;
	wxButton* snapshotButton;

	wxStaticText* statsText;

	void InitializeMenu(Server::HostInfo hostinfo);
	void InitializeTopBar(wxPanel* parent, wxBoxSizer* topsizer);
	void InitializeCanvasPanel(wxPanel* parent, wxBoxSizer* topsizer);
	void InitializeBottomBar(wxPanel* parent, wxBoxSizer* topsizer);

	void MinimizeToTaskbar(wxIconizeEvent& evt);
	void MaximizeFromTaskbar(wxTaskBarIconEvent& evt);
};