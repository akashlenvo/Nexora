#pragma once

#include "framescaler.h"

namespace Video
{
	/*
		1:1 Scaler
		Scales frames to maintain original dimensions with 1:1 pixel mapping
	*/
	class _1_1Scaler : public FrameScaler
	{
	public:
		_1_1Scaler(AVPixelFormat format);
		
	protected:
		TargetDimensions CalculateTargetDimensions(int width, int height) override;
	};
};