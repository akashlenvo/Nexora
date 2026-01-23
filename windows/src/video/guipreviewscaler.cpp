#include "video/guipreviewscaler.h"

namespace Video
{
	GuiPreviewScaler::GuiPreviewScaler(int containerWidth, int containerHeight)
		: FrameScaler(AV_PIX_FMT_BGR24),
        containerWidth(containerWidth),
		containerHeight(containerHeight)
	{
	}

	FrameScaler::TargetDimensions GuiPreviewScaler::CalculateTargetDimensions(int width, int height)
	{
        TargetDimensions dims;
        double containerRatio = static_cast<double>(containerWidth) / containerHeight;
        double srcRatio = static_cast<double>(width) / height;

        // Scale Image to FIT inside the box
        if (srcRatio > containerRatio) 
        {
            // Image is wider -> Use full width but lower height to maintain aspect ratio
            dims.width = containerWidth;
            dims.height = static_cast<int>(containerWidth / srcRatio);
        }
        else 
        {
            // Image is taller -> Use full height but lower width to maintain aspect ratio
            dims.width = static_cast<int>(containerHeight * srcRatio);
            dims.height = containerHeight;
        }

        // For the GUI preview we don't want any black bars
        // therefor the container should match the frame
        // and drawing starts at the origin
        dims.containerWidth = dims.width;
        dims.containerHeight = dims.height;
        dims.xOffset = 0;
        dims.yOffset = 0;
        dims.hasPadding = false;

        return dims;
	}
};