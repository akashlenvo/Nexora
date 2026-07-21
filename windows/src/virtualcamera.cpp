#include "virtualcamera.h"

#if defined(_WIN32)
#include "directshowsource.h"
#elif defined(__linux__)
#include "v4l2source.h"
#endif

std::unique_ptr<VirtualCameraSink> CreateVirtualCameraSink(int width, int height)
{
#if defined(_WIN32)
	return std::make_unique<DirectShowSource>(width, height);
#elif defined(__linux__)
	return std::make_unique<V4L2Source>(width, height);
#else
	return nullptr;
#endif
}
