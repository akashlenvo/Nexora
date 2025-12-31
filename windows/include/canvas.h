#pragma once

#include <wx/wx.h>

class Canvas : public wxPanel
{
public:
	const int WIDTH = 400;
	const int HEIGHT = 300;

	Canvas(wxWindow* parent, wxPoint position, wxSize size);

	void ProcessFrameAsync(const uint8_t* rawData, int width, int height);

	void ClearBeforeNextRender();

private:
	wxSize size;
	wxBitmap bitmap;

	bool shouldDraw = false;
	bool shouldClear = false;
	int drawX, drawY;

	std::atomic<bool> isRendering{ false };

	void Render(const uint8_t* bytes, int width, int height);
	void SetAspectRatio(int w, int h);

	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
};