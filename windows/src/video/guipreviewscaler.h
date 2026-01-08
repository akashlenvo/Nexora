#pragma once

#include "framescaler.h"

namespace Video
{
	class GuiPreviewScaler : public FrameScaler
	{
	public:
		GuiPreviewScaler(int containerWidth, int containerHeight);

	protected:
		TargetDimensions CalculateTargetDimensions(int width, int height) override;

	private:
		int containerWidth;
		int containerHeight;
	};
};