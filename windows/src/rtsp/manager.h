#pragma once

#include "receiver.h"
#include "net/devicedescriptor.h"
#include "net/server.h"
#include "streamoptions.h"
#include "constants.h"

#include <vector>

namespace RTSP
{
	class Manager : private Receiver
	{
	public:
		struct Command
		{
			static const int FRAME = 0x00;
			static const int RESOLUTION = 0x01;
			static const int ACTIVATION = 0x02;
			static const int CAMERA = 0x03;
			static const int QUALITY = 0x04;
			static const int CORRECTION_FILTER = 0x05;
			static const int EFFECT_FILTER = 0x06;
			static const int ROTATION = 0x07;
			static const int BITRATE = 0x08;
			static const int ADAPTIVE_BITRATE = 0x09;
			static const int STABILIZATION = 0xA;
			static const int FLASH = 0xB;
			static const int FOCUS = 0xC;
			static const int CODEC = 0xD;
			static const int FPS = 0xE;
			static const int ZOOM = 0xF;
			static const int FLIP = 0x10;
		};

		Manager(const Server& server, OnFrameReceivedCallback onFrameReceivedCallback, OnStatsReceivedCallback onStatsReceivedCallback);

		void AddDescriptor(DeviceDescriptor& descriptor);
		void RemoveDescriptor(DeviceDescriptor& descriptor);

		void Connect2Stream(int descriptorId, const StreamOptions& options);

		const std::vector<DeviceDescriptor>& GetDescriptors() const;
		const int& GetStreamingDevice() const;

		void SetResolution(unsigned short width, unsigned short height);
		void SetFPS(int fps);
		void SetBitrate(int bitrate);
		void SetAdaptiveBitrate(int min, int max);
		void SetStabilization(bool enabled);
		void SetFlash(bool enabled);
		void SetFocusMode(int mode);
		void SetH265Codec(bool enabled);
		void SwapCamera();
		void Rotate(uint8_t degrees);
		void Zoom(float factor);
		void FlipHorizontally();
		void FlipVertically();

		void ApplyCorrectionFilter(std::string filterName, int value = 0);
		void ApplyEffectFilter(std::string filterName);

	private:

		const Server& server;

		std::vector<DeviceDescriptor> descriptors;
		int streamingDevice;
	};
};