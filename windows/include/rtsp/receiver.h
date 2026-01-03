#pragma once

#include "ffmpeginterrupt.h"
#include "ffmpeg.h"

#include <memory>
#include <thread>
#include <atomic>
#include <functional>

namespace RTSP
{
    class Receiver 
    {
    public:

        struct Stats
        {
            int width, height;
            int fps;
            double bitrate;
        };

        /// <summary>
        /// Callback for when the full frame is received. 
        /// Received frame will be in RGB 24bit format.
        /// </summary>
        /// <param name="bytes">Raw RGB data</param>
        using OnFrameReceivedCallback = std::function<void(AVFrame* frame)>;

		/// <summary>
		/// Callback for when new stats are available. 
		/// Called approximately once per second.
		/// </summary>
		using OnStatsReceivedCallback = std::function<void(const Stats& stats)>;

        Receiver(OnFrameReceivedCallback frameReceivedListener, OnStatsReceivedCallback statsReceivedCallback);
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
		OnStatsReceivedCallback onStatsReceivedCallback;

        std::thread workerThread;
        std::atomic<bool> isRunning;
    };
};
