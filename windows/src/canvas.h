#pragma once

#include <wx/wx.h>
#include "video/guipreviewscaler.h"

class Canvas : public wxPanel
{
public:
	static const int WIDTH = 400;
	static const int HEIGHT = 300;

	Canvas(wxWindow* parent, wxPoint position, wxSize size);

	void ProcessRawFrameAsync(const AVFrame* frame);

	void ClearBeforeNextRender();

private:
	wxSize size;
	wxBitmap bitmap;

	bool shouldDraw = false;
	bool shouldClear = false;
	int drawX, drawY;

	std::atomic<bool> isRendering{ false };

	Video::GuiPreviewScaler scaler;

	void Render(const uint8_t* bytes, int width, int height);

	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
};