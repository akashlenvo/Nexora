#include "rtsp/manager.h"

#include "logger.h"

namespace RTSP
{
	Manager::Manager(const FrameReceivedListener& frameReceivedListener) : Receiver(frameReceivedListener)
	{
	}

	Manager::~Manager()
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

	void Manager::SetStreamingDevice(int device)
	{
		logger << "[RTSP Manager] Set streaming device to " << descriptors[device].url() << "\n";
		Stop();
		Start(descriptors[device].url());
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