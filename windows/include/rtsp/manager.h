#pragma once

#include "receiver.h"
#include "net/devicedescriptor.h"
#include "net/server.h"

#include <vector>

namespace RTSP
{
	class Manager : private Receiver
	{
	public:

		static const int DEFAULT_STATIC_BITRATE = 4000;
		static const int MAX_BITRATE = 25000;
		static const int MIN_BITRATE = 500;
		static const int DEFAULT_MAX_BITRATE = 12000;
		static const int DEFAULT_MIN_BITRATE = 2000;

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
		};

		Manager(const Server& server, OnFrameReceivedCallback onFrameReceivedCallback);

		void AddDescriptor(DeviceDescriptor& descriptor);
		void RemoveDescriptor(DeviceDescriptor& descriptor);

		void Connect2Stream(int descriptorId, int resolutionId);

		const std::vector<DeviceDescriptor>& GetDescriptors() const;
		const int& GetStreamingDevice() const;

		void SetStreamResolution(unsigned short width, unsigned short height);
		void SwapCamera();
		void Rotate(uint8_t degrees);
		
		void ApplyCorrectionFilter(std::string filterName, int value = 0);
		void ApplyEffectFilter(std::string filterName);

	private:

		const Server& server;

		std::vector<DeviceDescriptor> descriptors;
		int streamingDevice;
	};
};