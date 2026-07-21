#include "canvas.h"

#include <wx/dcbuffer.h>
#include <wx/rawbmp.h>
#include <algorithm>

Canvas::Canvas(wxWindow* parent, wxPoint position, wxSize size) 
	: wxPanel(parent, wxID_ANY, position, size), 
	scaler(WIDTH, HEIGHT),
	bitmap(WIDTH, HEIGHT)
{
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetDoubleBuffered(false);
	
	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &Canvas::OnEraseBackground, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
}

void Canvas::Render(const uint8_t* bytes, int width, int height)
{
	if (!bytes || width <= 0 || height <= 0)
		return;

	if (!bitmap.IsOk() || bitmap.GetWidth() != width || bitmap.GetHeight() != height) 
	{
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

	// If scaler failed (nullptr) or returned invalid dimensions, abort immediately.
    // Do NOT try to construct the vector below, or it will segfault.
	if (!data || width <= 0 || height <= 0)
	{
		isRendering = false;
		return;
	}

	size_t size = width * height * 3;
	std::vector<uint8_t> safeFrame;
	try {
		safeFrame.assign(data, data + size);
	}
	catch (const std::exception& e) {
		// Allocation failed (out of memory?)
		isRendering = false;
		return;
	}

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
	dc.SetBackground(wxBrush(wxColour(3, 6, 12)));
	dc.Clear();
	shouldClear = false;

	if (shouldDraw && bitmap.IsOk())
	{
		const auto client = GetClientSize();
		if (client.x <= 0 || client.y <= 0)
			return;

		const double scale = std::min(
			static_cast<double>(client.x) / bitmap.GetWidth(),
			static_cast<double>(client.y) / bitmap.GetHeight());
		const int targetWidth = std::max(1, static_cast<int>(bitmap.GetWidth() * scale));
		const int targetHeight = std::max(1, static_cast<int>(bitmap.GetHeight() * scale));
		const int targetX = (client.x - targetWidth) / 2;
		const int targetY = (client.y - targetHeight) / 2;

		wxMemoryDC source;
		source.SelectObject(bitmap);
		dc.StretchBlit(targetX, targetY, targetWidth, targetHeight,
			&source, 0, 0, bitmap.GetWidth(), bitmap.GetHeight(), wxCOPY, true);
		source.SelectObject(wxNullBitmap);
	}
}

void Canvas::OnEraseBackground(wxEraseEvent& event)
{
	// Empty to remove flickering created by repainting the background
	// before actually rendering the video frame
}

void Canvas::OnSize(wxSizeEvent& event)
{
	Refresh(false);
	event.Skip();
}
