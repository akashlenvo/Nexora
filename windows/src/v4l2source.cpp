#include "v4l2source.h"

#if defined(__linux__)

#include "logger.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace
{
bool SetVideoFormat(int descriptor, int width, int height, std::string& error)
{
	v4l2_format format{};
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	format.fmt.pix.width = width;
	format.fmt.pix.height = height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	format.fmt.pix.field = V4L2_FIELD_NONE;
	format.fmt.pix.bytesperline = width * 2;
	format.fmt.pix.sizeimage = width * height * 2;

	if (ioctl(descriptor, VIDIOC_S_FMT, &format) == -1)
	{
		error = std::string("Could not configure the V4L2 device: ") + std::strerror(errno);
		return false;
	}

	if (format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
	{
		error = "The V4L2 device rejected the YUYV pixel format";
		return false;
	}

	return true;
}
}

V4L2Source::V4L2Source(int width, int height)
	: width(width),
	height(height),
	scaler(width, height, AV_PIX_FMT_YUYV422)
{
	const char* configuredDevice = std::getenv("NEXORA_VIDEO_DEVICE");
	devicePath = configuredDevice && *configuredDevice ? configuredDevice : "/dev/video10";

	descriptor = open(devicePath.c_str(), O_WRONLY | O_NONBLOCK);
	if (descriptor == -1)
	{
		status = "Could not open " + devicePath + ": " + std::strerror(errno)
			+ ". Install and load v4l2loopback, or set NEXORA_VIDEO_DEVICE.";
		logger << "[V4L2] " << status << std::endl;
		return;
	}

	if (!SetVideoFormat(descriptor, width, height, status))
	{
		logger << "[V4L2] " << status << std::endl;
		close(descriptor);
		descriptor = -1;
		return;
	}

	status = "Streaming to " + devicePath;
	logger << "[V4L2] " << status << " at " << width << "x" << height << std::endl;
}

V4L2Source::~V4L2Source()
{
	if (descriptor != -1)
		close(descriptor);
}

void V4L2Source::SendRawFrame(const AVFrame* frame)
{
	if (descriptor == -1 || writingFrame.exchange(true))
		return;

	int outputWidth = 0;
	int outputHeight = 0;
	const uint8_t* bytes = scaler.Process(frame, outputWidth, outputHeight);
	if (bytes && outputWidth == width && outputHeight == height)
	{
		const size_t frameSize = static_cast<size_t>(width) * height * 2;
		const auto written = write(descriptor, bytes, frameSize);
		if (written == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
			logger << "[V4L2] Frame write failed: " << std::strerror(errno) << std::endl;
	}

	writingFrame = false;
}

bool V4L2Source::IsReady() const
{
	return descriptor != -1;
}

std::string V4L2Source::StatusMessage() const
{
	return status;
}

#endif
