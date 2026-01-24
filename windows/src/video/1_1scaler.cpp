#include "1_1scaler.h"

namespace Video
{
	_1_1Scaler::_1_1Scaler(AVPixelFormat format) : FrameScaler(format)
	{
	}

	FrameScaler::TargetDimensions _1_1Scaler::CalculateTargetDimensions(int width, int height)
	{
		// Keep original frame dimensions
		// 1:1 pixel mapping
		TargetDimensions dims;
		dims.width = width;
		dims.height = height;
		dims.containerWidth = width;
		dims.containerHeight = height;
		dims.xOffset = 0;
		dims.yOffset = 0;
		dims.hasPadding = false;
		return dims;
	}
}