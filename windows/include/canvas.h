#pragma once

#include <wx/wx.h>

class Canvas : public wxPanel
{
public:
	Canvas(wxWindow* parent, wxPoint position, wxSize size);

	void Render(const wxImage& image);
	void SetAspectRatio(int w, int h);

	void ClearBeforeNextRender();

private:
	wxSize size;
	wxBitmap bitmap;

	bool shouldDraw = false;
	bool shouldClear = false;
	int drawX, drawY;
	
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
};