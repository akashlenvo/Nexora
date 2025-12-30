#pragma once

#include "receiver.h"
#include "net/devicedescriptor.h"

#include <vector>

namespace RTSP
{
	class Manager : public Receiver
	{
	public:
		Manager(const FrameReceivedListener& frameReceivedListener);
		~Manager();

		void AddDescriptor(DeviceDescriptor& descriptor);
		void RemoveDescriptor(DeviceDescriptor& descriptor);

		void SetStreamingDevice(int device);

		const std::vector<DeviceDescriptor>& GetDescriptors() const;
		const int& GetStreamingDevice() const;

	private:

		std::vector<DeviceDescriptor> descriptors;
		int streamingDevice;
	};
};