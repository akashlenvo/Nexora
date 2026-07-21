#pragma once

#include "softcam/softcam.h"
#include <atomic>
#include "virtualcamera.h"
#include "video/directshowscaler.h"

/*
	Wrapper around softcam library. 
	Manages the scCamera instance
*/
class DirectShowSource final : public VirtualCameraSink
{
public:
	DirectShowSource(int width, int height);
	~DirectShowSource() override;

	void SendRawFrame(const AVFrame* frame) override;
	bool IsReady() const override;
	std::string StatusMessage() const override;

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
