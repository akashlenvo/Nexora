#pragma once

#include "framescaler.h"
#include "ffmpeginterrupt.h"

#include <memory>
#include <thread>
#include <atomic>
#include <functional>

namespace RTSP
{
    class Receiver 
    {
    public:

        /// <summary>
        /// Callback for when the full frame is received. 
        /// Received frame will be in RGB 24bit format.
        /// </summary>
        /// <param name="bytes">Raw RGB data</param>
        using OnFrameReceivedCallback = std::function<void(const uint8_t* bytes, int width, int height)>;

        Receiver(OnFrameReceivedCallback frameReceivedListener);
        virtual ~Receiver();

        void Start(std::string url, std::string protocol, int width, int height);
        void Stop();

    private:

        /// <summary>
        /// RTSP receving function that runs in its own thread 
        /// </summary>
        void Loop(int width, int height);
        void WorkerFunc();

        bool OpenConnection(AVFormatContext** ctx);
        bool FindVideoStream(AVFormatContext* ctx, int& streamIdx, AVCodecContext** codecCtx, int width, int height);

        FFmpegInterrupt::State state;

        std::string rtspUrl;
        std::string rtspProtocol;

        OnFrameReceivedCallback onFrameReceivedCallback;

        std::thread workerThread;
        std::atomic<bool> isRunning;

        FrameScaler scaler;
    };
};
