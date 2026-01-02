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
	};

	Window(Server::HostInfo hostInfo);
	~Window();

	Canvas* GetCanvas();

	wxChoice* GetSourceChoice();

	wxButton* GetRotateLeftButton();
	wxButton* GetRotateRightButton();
	wxButton* GetFlipButton();
	wxButton* GetAdjustmentsButton();
	wxButton* GetSwapButton();
	wxButton* GetStreamOptionsButton();
	wxStaticText* GetStatsText();
	wxTaskBarIcon* GetTaskbarIcon();
private:
	wxTaskBarIcon* taskbarIcon;
	Canvas* canvas;

	wxChoice* sourceChoice;

	wxButton* rotateLeftButton;
	wxButton* rotateRightButton;
	wxButton* flipButton;
	wxButton* streamOptionsButton;
	wxButton* adjustmentsButton;
	wxButton* swapButton;
	wxStaticText* statsText;

	void InitializeMenu(Server::HostInfo hostinfo);
	void InitializeCanvasPanel(wxPanel* parent, wxBoxSizer* topsizer);
	void InitializeControlPanel(wxPanel* parent, wxBoxSizer* topsizer);
	
	void MinimizeToTaskbar(wxIconizeEvent& evt);
	void MaximizeFromTaskbar(wxTaskBarIconEvent& evt);
};