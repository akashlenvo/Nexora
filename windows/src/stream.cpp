#include "stream.h"

#include <wx/mstream.h>
#include "logger.h"

Stream::Stream(OnFrameReadyCallback fn) : onFrameReady(fn)
{
	closed = false;
	paused = false;
	transforms = { 0, 0 };
	adjustments = { 0, 0, 80 };
}

void Stream::OnFrameReceived(unsigned char* bytes, int width, int height) const
{
	if (closed || paused)
		return;

	// Calculate the time between 2 consecutive received frames
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTimepoint);
	previousTimepoint = now;

	// Disable error message box
	wxLogNull noLog;
	
	wxImage img(width, height);
	if (!img.IsOk())
		return;

	// *3 because the image format is always 24bit RGB
	std::memcpy(img.GetData(), bytes, width * height * 3);

	if (onFrameReady)
	{
		onFrameReady(img, { duration.count(), (size_t)(width * height * 3)});
	}
}

int Stream::RotateLeft()
{
	transforms.rotation += 1;
	transforms.rotation %= 4;
	return transforms.rotation;
}

int Stream::RotateRight()
{
	transforms.rotation -= 1;
	transforms.rotation %= 4;
	
	// Required because of how % operator works in c++, 4%-1=-1 when we need it to be 3
	if (transforms.rotation < 0)
		transforms.rotation = 4 + transforms.rotation;

	return transforms.rotation;
}

void Stream::Mirror()
{
	transforms.mirror = !transforms.mirror;
}

void Stream::SetBrightnessAdjustment(int value)
{
	adjustments.brightness = value;
}

void Stream::SetSaturationAdjustment(int value)
{
	adjustments.saturation = value;
}

void Stream::SetQualityAdjustment(int value)
{
	adjustments.quality = value;
}

void Stream::SetWBAdjustment(int value)
{
	adjustments.whitebalance = value;
}

void Stream::SetEffectAdjustment(int value)
{
	adjustments.effect = value;
}

const Stream::Adjustments& Stream::GetAdjustments()
{
	return adjustments;
}

wxImage Stream::ApplyFrameTransformsAndAdjustments() const
{
	wxImage result = image;

	switch (transforms.rotation)
	{
	case Transforms::ROTATE_90:
		result = image.Rotate90(false);
		break;

	case Transforms::ROTATE_180:
		result = image.Rotate180();
		break;

	case Transforms::ROTATE_270:
		result = image.Rotate90();
		break;
	}
	
	if (transforms.mirror)
	{
		result = result.Mirror();
	}

	result.ChangeHSV(0, adjustments.saturation / 100.0, adjustments.brightness / 100.0);

	return result;
}

unsigned char* Stream::GetBGR(const wxImage& image)
{
	int w = image.GetWidth();
	int h = image.GetHeight();

	int size = w * h * 3;
	if (bgrData == nullptr || bgrSize != size)
	{
		if (bgrData != nullptr)
		{
			delete[] bgrData;
		}
		bgrSize = size;
		bgrData = new unsigned char[bgrSize];
	}

	auto data = image.GetData();
	for (int i = 0; i < w * h; i++)
	{
		bgrData[i * 3] = data[(i * 3) + 2];
		bgrData[(i * 3) + 1] = data[(i * 3) + 1];
		bgrData[(i * 3) + 2] = data[i * 3];
	}

	return bgrData;
}

void Stream::Pause()
{
	paused = true;
}

void Stream::Unpause()
{
	paused = false;
}

void Stream::Close()
{
	closed = true;
}
