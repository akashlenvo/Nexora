#pragma once

#include "framescaler.h"

#include <memory>
#include <thread>
#include <atomic>

class Receiver 
{
public:

    struct FrameReceivedListener
    {
        /// <summary>
        /// Callback for when the full frame is received. 
        /// Received frame will be in RGB 24bit format.
        /// </summary>
        /// <param name="bytes">Raw RGB data</param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        virtual void OnFrameReceived(unsigned char* bytes, int width, int height) const = 0;
    };
    
    Receiver(std::string url, const FrameReceivedListener& frameReceivedListener);
    ~Receiver();

    void Start();
    void Stop();

private:

    /// <summary>
    /// RTSP receving function that runs in its own thread 
    /// </summary>
    void Loop();

    bool OpenConnection(AVFormatContext** ctx);
    bool FindVideoStream(AVFormatContext* ctx, int& streamIdx, AVCodecContext** codecCtx);

    std::string rtspUrl;
    const FrameReceivedListener& frameReceivedListener;

    std::thread workerThread;
    std::atomic<bool> isRunning;

    FrameScaler scaler;
};