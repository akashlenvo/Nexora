#pragma once

#include <atomic>
#include <chrono>

namespace FFmpegInterrupt
{
    const int64_t TIMEOUT_MS = 3000;

    struct State
    {
        std::atomic<int64_t> lastActivityTime;
        std::atomic<bool> stopRequested;
    };

    static int64_t GetTimeMs() 
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    /*
    * FFmpeg interrupt callback that is called internally by the library during blocking calls
    * If return value is 0 => continue waiting, otherwise ABORT
    */
    static int Callback(void* ctx)
    {
        State* state = static_cast<State*>(ctx);

        // Abort if a stop is requested
        if (state->stopRequested) 
        {
            return 1;
        }

        // Abort if timeout duration is exceeded
        if (GetTimeMs() - state->lastActivityTime > TIMEOUT_MS)
        {
            return 1;
        }

        return 0;
    }
}