#pragma once

#include "video/framescaler.h"

namespace Video
{
    FrameScaler::FrameScaler(int format) : format(format)
    {
    }

    FrameScaler::~FrameScaler()
	{
		Cleanup();
	}

	const uint8_t* FrameScaler::Process(const AVFrame* srcFrame, int& width, int& height)
	{
        if (!PrepareContext(srcFrame->width, srcFrame->height, srcFrame->format))
            return nullptr;

        int dstStride = cachedTargetDims.containerWidth * 3;
        uint8_t* dstSlice[4] = { nullptr };
        int dstStrideArr[4] = { 0 };

        dstSlice[0] = buffer.data() + (cachedTargetDims.yOffset * dstStride) + (cachedTargetDims.xOffset * 3);
        dstStrideArr[0] = dstStride;

        sws_scale(
            sws_ctx,
            (const uint8_t* const*)srcFrame->data,
            srcFrame->linesize,
            0,
            srcFrame->height,
            dstSlice,
            dstStrideArr
        );

        width = cachedTargetDims.containerWidth;
        height = cachedTargetDims.containerHeight;

        return buffer.data();
	}

    bool FrameScaler::PrepareContext(int srcWidth, int srcHeight, int srcFormat)
    {
        // Check if sizes changed from the last received frame. Return if there is no change
        if (sws_ctx && srcWidth == lastSrcWidth && srcHeight == lastSrcHeight && srcFormat == lastSrcFormat)
            return true;

        // Recalculate target dimensions and recreate the context
        // based on the new values
        cachedTargetDims = CalculateTargetDimensions(srcWidth, srcHeight);

        if (sws_ctx) 
            sws_freeContext(sws_ctx);

        sws_ctx = sws_getContext(
            srcWidth, 
            srcHeight, 
            (AVPixelFormat)srcFormat,
            cachedTargetDims.width, 
            cachedTargetDims.height, 
            (AVPixelFormat)this->format,
            SWS_BILINEAR, 
            nullptr, 
            nullptr,
            nullptr
        );

        if (!sws_ctx) 
            return false;

        int stride = cachedTargetDims.containerWidth * 3;
        size_t neededSize = cachedTargetDims.containerHeight * stride;

        // Reallocate the buffer if it is not sufficient
        // and clear everything inside (black background)
        if (buffer.size() < neededSize) 
        {
            buffer.resize(neededSize);
            std::fill(buffer.begin(), buffer.end(), 0);
        }

        // If we have padding (black bars), we must clear the buffer every time 
        // the resolution changes to remove pixels from the previous size.
        if (cachedTargetDims.hasPadding) 
        {
            std::memset(buffer.data(), 0, buffer.size());
        }

        lastSrcWidth = srcWidth; 
        lastSrcHeight = srcHeight; 
        lastSrcFormat = srcFormat;

        return true;
    }

    void FrameScaler::Cleanup()
    {
        if (sws_ctx)
        {
            sws_freeContext(sws_ctx);
            sws_ctx = nullptr;
        }
        buffer.clear();
    }
};