#include "canvas.h"

#include <wx/dcbuffer.h>
#include <wx/rawbmp.h>

Canvas::Canvas(wxWindow* parent, wxPoint position, wxSize size) 
	: wxPanel(parent, wxID_ANY, position, size), 
	scaler(WIDTH, HEIGHT),
	size(size), 
	bitmap(size.x, size.y)
{
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetDoubleBuffered(false);

	drawX = 0;
	drawY = 0;
	
	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &Canvas::OnEraseBackground, this);
}

void Canvas::Render(const uint8_t* bytes, int width, int height)
{
	if (!bytes || width <= 0 || height <= 0)
		return;

	if (!bitmap.IsOk() || bitmap.GetWidth() != width || bitmap.GetHeight() != height) 
	{
		drawX = (WIDTH - width) / 2;
		drawY = (HEIGHT - height) / 2;
		bitmap.Create(width, height, 24);
		shouldClear = true;
	}

	wxNativePixelData data(bitmap);
	if (!data) 
		return;

	wxNativePixelData::Iterator p(data);
	int stride = width * 3;

	for (int y = 0; y < height; y++)
	{
		uint8_t* dstrow = (uint8_t*)p.m_ptr;
		std::memcpy(dstrow, bytes + (y * stride), stride);

		p.OffsetY(data, 1);
	}

	shouldDraw = true;
	this->Refresh(false);
	this->Update();
}

void Canvas::ProcessRawFrameAsync(const AVFrame* frame)
{
	if (isRendering)
		return;

	isRendering = true;

	int width, height;
	const uint8_t* data = scaler.Process(frame, width, height);

	size_t size = width * height * 3;
	std::vector<uint8_t> safeFrame(data, data + size);

	this->CallAfter([this, bytes = std::move(safeFrame), width, height]() {
		this->Render(bytes.data(), width, height);
		this->isRendering = false;
	});
}

void Canvas::ClearBeforeNextRender()
{
	shouldClear = true;
}

void Canvas::Clear()
{
	shouldDraw = false;
	isRendering = false;
	Refresh(false);
	Update();
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
			dc.DrawBitmap(bitmap, drawX, drawY, 0);
		}
	}
	else
	{
		dc.SetBrush(*wxBLACK_BRUSH);
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawRectangle(0, 0, size.x, size.y);
	}
}

void Canvas::OnEraseBackground(wxEraseEvent& event)
{
	// Empty to remove flickering created by repainting the background
	// before actually rendering the video frame
}