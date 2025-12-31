#include "rtsp/manager.h"

#include "logger.h"

namespace RTSP
{
	Manager::Manager(const Server& server, OnFrameReceivedCallback onFrameReceivedCallback) : server(server), Receiver(onFrameReceivedCallback)
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
		this->streamingDevice = descriptorId;

		auto& descriptor = descriptors[descriptorId];
		auto& url = descriptor.url();
		auto& resolution = descriptor.backResolutions()[resolutionId];

		logger << "[RTSP Manager] Connecting to stream " << url << "\n";
		
		Stop();
		
		Start(url, descriptor.protocol(), resolution.first, resolution.second);
	}

	void Manager::SetStreamResolution(unsigned short width, unsigned short height)
	{
		const unsigned char bytes[5] = {
			Command::RESOLUTION,							// First byte is the packet type
			static_cast<unsigned char>(width & 0xFF),		// Low byte of width
			static_cast<unsigned char>(width >> 8),			// High byte of width
			static_cast<unsigned char>(height & 0xFF),		// Low byte of height
			static_cast<unsigned char>(height >> 8),		// High byte of height
		};

		server.Send(streamingDevice, bytes, 5);
	}

	void Manager::SwapCamera()
	{
		const unsigned char bytes[1] = { Command::CAMERA };
		server.Send(streamingDevice, bytes, 1);
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