#include "directshowsource.h"

DirectShowSource::DirectShowSource(int width, int height)
	: scaler(width, height),
	width(width),
	height(height)
{
	camera = scCreateCamera(width, height, 0);
	streamingEnabled = true;
}

void DirectShowSource::SendRawFrame(const AVFrame* frame)
{
	if (streamingEnabled)
	{
		int w, h;
		const uint8_t* bytes = scaler.Process(frame, w, h);
		scSendFrame(camera, bytes);
	}
}

void DirectShowSource::WaitForConnection()
{
	scWaitForConnection(camera);
}

void DirectShowSource::Pause()
{
	streamingEnabled = false;
}

void DirectShowSource::Resume()
{
	streamingEnabled = true;
}