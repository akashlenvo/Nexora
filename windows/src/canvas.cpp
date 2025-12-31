#include "canvas.h"

#include <wx/dcbuffer.h>

Canvas::Canvas(wxWindow* parent, wxPoint position, wxSize size) 
	: wxPanel(parent, wxID_ANY, position, size), 
	size(size), 
	bitmap(size.x, size.y)
{
	drawX = 0;
	drawY = 0;
	
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetDoubleBuffered(false);
	
	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &Canvas::OnEraseBackground, this);

	SetAspectRatio(WIDTH, HEIGHT);
}

void Canvas::Render(const wxImage& image)
{
	if (!image.IsOk())
		return;

	this->bitmap = wxBitmap(image.Scale(size.x, size.y));
	if (!bitmap.IsOk())
		return;

	shouldDraw = true;
	this->Refresh(false);
	this->Update();
}

void Canvas::SetAspectRatio(int w, int h)
{
	if (h == 0 || w == 0) return;

    double targetRatio = static_cast<double>(w) / h;
    double containerRatio = static_cast<double>(WIDTH) / HEIGHT;

    int finalW, finalH;

    // Aspect Fit Logic
    if (targetRatio > containerRatio) 
    {
        // Landscape mode (image si wider)
        finalW = WIDTH;
        finalH = static_cast<int>(WIDTH / targetRatio);
    } 
    else 
    {
		// Portrait mode (image is taller)
        finalH = HEIGHT;
        finalW = static_cast<int>(HEIGHT * targetRatio);
    }

    size = wxSize(finalW, finalH);
    bitmap = wxBitmap(finalW, finalH);

    // Calculate Centering Offsets
    // (Container Size - Content Size) / 2
    drawX = (WIDTH - finalW) / 2;
    drawY = (HEIGHT - finalH) / 2;
}

void Canvas::ClearBeforeNextRender()
{
	shouldClear = true;
}

void Canvas::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(this);
	
	if (shouldClear)
	{
		dc.Clear();
		shouldClear = false;
	}

	if (shouldDraw)
	{
		dc.DrawBitmap(bitmap, drawX, drawY, false);
	}
	else
	{
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawRectangle(0, 0, size.x, size.y);
	}
}

void Canvas::OnEraseBackground(wxEraseEvent& event)
{
	// Empty to remove flickering created by repainting the background
	// before actually rendering the video frame
}