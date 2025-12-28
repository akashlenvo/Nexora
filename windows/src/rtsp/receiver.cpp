#include "rtsp/receiver.h"
#include "logger.h"

Receiver::Receiver(std::string url, const FrameReceivedListener& listener)
	: rtspUrl(url),
	frameReceivedListener(listener),
	isRunning(false)
{
	avformat_network_init();
}

Receiver::~Receiver() 
{
	Stop();
}

void Receiver::Start() 
{
	if (isRunning)
		return;
	
	isRunning = true;
	workerThread = std::thread(&Receiver::Loop, this);
}

void Receiver::Stop() {
	isRunning = false;

	if (workerThread.joinable()) 
		workerThread.join();
}

bool Receiver::OpenConnection(AVFormatContext** ctx) {
	AVDictionary* options = nullptr;
	
	// Low latency settings
	av_dict_set(&options, "rtsp_transport", "udp", 0);
	av_dict_set(&options, "fflags", "nobuffer", 0);
	av_dict_set(&options, "flags", "low_delay", 0);

	if (avformat_open_input(ctx, rtspUrl.c_str(), nullptr, &options) != 0) 
	{
		logger << "[RTSP] Error: Could not open stream\n";
		return false;
	}

	if (avformat_find_stream_info(*ctx, nullptr) < 0) 
	{
		logger << "[RTSP] Error: Could not find stream info\n";
		return false;
	}

	return true;
}

bool Receiver::FindVideoStream(AVFormatContext* ctx, int& streamIdx, AVCodecContext** codecCtx) {
	streamIdx = -1;
	const AVCodec* codec = nullptr;

	for (unsigned int i = 0; i < ctx->nb_streams; i++) 
	{
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
		{
			streamIdx = i;
			AVCodecParameters* params = ctx->streams[i]->codecpar;
			codec = avcodec_find_decoder(params->codec_id);

			*codecCtx = avcodec_alloc_context3(codec);
			avcodec_parameters_to_context(*codecCtx, params);
			break;
		}
	}

	if (streamIdx == -1 || !codec) 
	{
		logger << "[RTSP] Error: No video stream found\n";
		return false;
	}

	// Enable multithreading for decoding
	(*codecCtx)->thread_count = 4;
	(*codecCtx)->thread_type = FF_THREAD_SLICE;

	if (avcodec_open2(*codecCtx, codec, nullptr) < 0) 
	{
		logger << "[RTSP] Error: Could not open codec\n";
		return false;
	}
	return true;
}

void Receiver::Loop() 
{
	AVFormatContext* formatCtx = nullptr;
	AVCodecContext* codecCtx = nullptr;
	int videoStreamIdx = -1;

	// Open connection to the RTSP stream
	if (!OpenConnection(&formatCtx))
		return;

	if (!FindVideoStream(formatCtx, videoStreamIdx, &codecCtx)) 
	{
		avformat_close_input(&formatCtx);
		return;
	}

	// Allocate memory for the incoming frames
	AVFrame* pFrame = av_frame_alloc();
	AVFrame* pFrameRGB = av_frame_alloc();
	AVPacket* pPacket = av_packet_alloc();

	logger << "[RTSP] Decoding started...\n";

	while (isRunning && av_read_frame(formatCtx, pPacket) >= 0) 
	{
		if (pPacket->stream_index != videoStreamIdx)
			continue;

		if (avcodec_send_packet(codecCtx, pPacket) != 0)
			continue;

		while (avcodec_receive_frame(codecCtx, pFrame) == 0) 
		{
			// Convert from YUV to RGB
			// If conversion is successfull notify the listener the complete frame is received
			if (scaler.Convert(pFrame, pFrameRGB)) 
			{
				frameReceivedListener.OnFrameReceived(pFrameRGB->data[0], pFrameRGB->width, pFrameRGB->height);
			}
		}
		av_packet_unref(pPacket);
	}

	logger << "[RTSP] Thread stopping, cleaning up...\n";

	av_frame_free(&pFrame);
	av_frame_free(&pFrameRGB);
	av_packet_free(&pPacket);
	avcodec_free_context(&codecCtx);
	avformat_close_input(&formatCtx);
	// Frame scaler cleans itself up in its destructor automatically
}
