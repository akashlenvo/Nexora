#include "canvas.h"

#include <wx/dcbuffer.h>
#include <wx/rawbmp.h>

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

void Canvas::Render(const uint8_t* bytes, int width, int height)
{
	// Create the bitmap if the resolution changed or if the bitmap cannot be used
	if (!bitmap.IsOk() || bitmap.GetWidth() != width || bitmap.GetHeight() != height)
	{
		bitmap.Create(width, height, 24);
		SetAspectRatio(width, height);
	}

	wxNativePixelData data(bitmap);
	if (!data)
		return;

	wxNativePixelData::Iterator p(data);
	const uint8_t* src = bytes;

	// Manually copy the source bytes in the bitmap object
	for (int y = 0; y < height; y++)
	{
		wxNativePixelData::Iterator rowStart = p;
		for (int x = 0; x < width; x++)
		{
			p.Red() = *src++;
			p.Green() = *src++;
			p.Blue() = *src++;

			++p;
		}

		p = rowStart;
		p.OffsetY(data, 1);
	}

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
    // bitmap = wxBitmap(finalW, finalH);

    // Calculate Centering Offsets
    // (Container Size - Content Size) / 2
    drawX = (WIDTH - finalW) / 2;
    drawY = (HEIGHT - finalH) / 2;

	shouldClear = true;
}

void Canvas::ProcessFrameAsync(const uint8_t* frame, int width, int height)
{
	if (isRendering)
		return;

	isRendering = true;

	size_t size = width * height * 3;
	std::vector<uint8_t> safeFrame(frame, frame + size);

	this->CallAfter([this, bytes = std::move(safeFrame), width, height]() {
		this->Render(bytes.data(), width, height);
		this->isRendering = false;
	});
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
		if (bitmap.IsOk())
		{
			wxMemoryDC memdc(bitmap);
			dc.StretchBlit(drawX, drawY, size.GetWidth(), size.GetHeight(), &memdc, 0, 0, bitmap.GetWidth(), bitmap.GetHeight());
		}
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