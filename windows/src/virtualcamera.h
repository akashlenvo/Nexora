#pragma once

#include <memory>
#include <string>

#include "rtsp/ffmpeg.h"

class VirtualCameraSink
{
public:
	virtual ~VirtualCameraSink() = default;
	virtual void SendRawFrame(const AVFrame* frame) = 0;
	virtual bool IsReady() const = 0;
	virtual std::string StatusMessage() const = 0;
};

std::unique_ptr<VirtualCameraSink> CreateVirtualCameraSink(int width, int height);
