#include "video/directshowscaler.h"

namespace Video
{
    DirectShowScaler::DirectShowScaler(int width, int height) 
        : FrameScaler(AV_PIX_FMT_BGR24),
        width(width),
        height(height)
    {
    }

    FrameScaler::TargetDimensions DirectShowScaler::CalculateTargetDimensions(int width, int height)
    {
        TargetDimensions dims;

        // Container size is always fixed and managed by the direct show filter
        dims.containerWidth = this->width;
        dims.containerHeight = this->height;
        dims.hasPadding = true;

        double containerRatio = static_cast<double>(this->width) / this->height;
        double srcRatio = static_cast<double>(width) / height;
    
        // Scale Image to FIT inside the box
        if (srcRatio > containerRatio) 
        {
            // Image is wider -> Use full width but lower height to maintain aspect ratio
            dims.width = this->width;
            dims.height = static_cast<int>(this->width / srcRatio);
        }
        else 
        {
            // Image is taller -> Use full height but lower width to maintain aspect ratio
            dims.width = static_cast<int>(this->height * srcRatio);
            dims.height = this->height;
        }

        // Calculate offsets for centering the image in the middle
        // of the container 
        dims.xOffset = (this->width - dims.width) / 2;
        dims.yOffset = (this->height - dims.height) / 2;

        return dims;
    }
};