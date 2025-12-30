#include "rtsp/manager.h"

#include "logger.h"

namespace RTSP
{
	Manager::Manager(const FrameReceivedListener& frameReceivedListener) : Receiver(frameReceivedListener)
	{
	}

	void Manager::AddDescriptor(DeviceDescriptor& descriptor)
	{
		descriptors.push_back(std::move(descriptor));
	}

	void Manager::RemoveDescriptor(DeviceDescriptor& descriptor)
	{
		descriptors.erase(std::remove(descriptors.begin(), descriptors.end(), descriptor));
	}

	void Manager::Connect2Stream(int descriptorId, int resolutionId)
	{
		auto descriptor = descriptors[descriptorId];
		auto url = descriptor.url();
		auto resolution = descriptor.resolutions()[resolutionId];

		logger << "[RTSP Manager] Connecting to stream " << url << "\n";
		
		Stop();
		
		Start(url, descriptor.protocol(), resolution.first, resolution.second);
	}

	const std::vector<DeviceDescriptor>& Manager::GetDescriptors() const
	{
		return descriptors;
	}

	const int& Manager::GetStreamingDevice() const
	{
		return streamingDevice;
	}
};