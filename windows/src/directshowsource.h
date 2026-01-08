#pragma once

#include "softcam/softcam.h"
#include <atomic>
#include "video/directshowscaler.h"

/*
	Wrapper around softcam library. 
	Manages the scCamera instance
*/
class DirectShowSource
{
public:
	DirectShowSource(int width, int height);

	void SendRawFrame(const AVFrame* frame);

	void WaitForConnection();
	void Pause();
	void Resume();

private:
	int width, height;

	scCamera camera = nullptr;
	Video::DirectShowScaler scaler;

	std::atomic<bool> streamingEnabled{ false };
	std::atomic<bool> streamingFrame{ false };
};