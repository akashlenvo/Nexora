#include "rtsp/framescaler.h"

FrameScaler::FrameScaler() {}

FrameScaler::~FrameScaler() 
{
    Cleanup();
}

void FrameScaler::Cleanup() 
{
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if (buffer) {
        av_free(buffer);
        buffer = nullptr;
    }
}

bool FrameScaler::Reinitialize(int w, int h, int format) 
{
    // If dimensions match current state, no need to re-init
    if (sws_ctx && w == currentWidth && h == currentHeight && format == currentFormat) 
        return true;

    Cleanup(); // Clear old buffers if resolution changed

    // Alloc buffer for RGB24
    // Align = 1 is crucial for wxWidgets/GUI compatibility (no padding)
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, w, h, 1);
    buffer = (uint8_t*)av_malloc(numBytes);

    if (!buffer) 
        return false;

    sws_ctx = sws_getContext(
        w, h, (AVPixelFormat)format,
        w, h, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!sws_ctx) 
        return false;

    currentWidth = w;
    currentHeight = h;
    currentFormat = format;

    return true;
}

bool FrameScaler::Convert(AVFrame* srcFrame, AVFrame* dstFrame) 
{
    if (!Reinitialize(srcFrame->width, srcFrame->height, srcFrame->format))
        return false;

    // Prepare destination frame structure
    dstFrame->width = srcFrame->width;
    dstFrame->height = srcFrame->height;
    dstFrame->format = AV_PIX_FMT_RGB24;

    av_image_fill_arrays(dstFrame->data, dstFrame->linesize, buffer,
        AV_PIX_FMT_RGB24, dstFrame->width, dstFrame->height, 1);

    sws_scale(sws_ctx, (const uint8_t* const*)srcFrame->data,
        srcFrame->linesize, 0, srcFrame->height,
        dstFrame->data, dstFrame->linesize);

    return true;
}