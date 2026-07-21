#pragma once

#if defined(__linux__)

#include <atomic>
#include <string>

#include "virtualcamera.h"
#include "video/directshowscaler.h"

class V4L2Source final : public VirtualCameraSink
{
public:
	V4L2Source(int width, int height);
	~V4L2Source() override;

	void SendRawFrame(const AVFrame* frame) override;
	bool IsReady() const override;
	std::string StatusMessage() const override;

private:
	int descriptor = -1;
	int width;
	int height;
	std::string devicePath;
	std::string status;
	Video::DirectShowScaler scaler;
	std::atomic<bool> writingFrame{ false };
};

#endif
