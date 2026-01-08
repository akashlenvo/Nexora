#pragma once

#include "framescaler.h"

namespace Video
{
	class DirectShowScaler : public FrameScaler
	{
	public:
		DirectShowScaler(int width, int height);

	protected:
		TargetDimensions CalculateTargetDimensions(int width, int height) override;

	private:
		int width;
		int height;
	};
};