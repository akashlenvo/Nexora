#pragma once

#include "ffmpeg.h"

namespace RTSP
{
    class FrameScaler 
    {
    public:
        FrameScaler();
        ~FrameScaler();

        // Returns true if conversion was successful
        bool Convert(AVFrame* srcFrame, AVFrame* dstFrame);

    private:
        SwsContext* sws_ctx = nullptr;
        uint8_t* buffer = nullptr;

        int currentWidth = 0;
        int currentHeight = 0;
        int currentFormat = -1;

        void Cleanup();
        bool Reinitialize(int w, int h, int format);
    };
};
