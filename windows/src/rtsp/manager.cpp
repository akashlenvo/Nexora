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

	void Manager::Rotate(uint8_t degrees)
	{
		const unsigned char bytes[2] = { Command::ROTATION, degrees };
		server.Send(streamingDevice, bytes, 2);
	}

	void Manager::ApplyCorrectionFilter(std::string filterName, int value)
	{
		uint8_t nameLen = static_cast<uint8_t>(filterName.size());

		std::vector<uint8_t> packet;
		packet.reserve(3 + nameLen);

		// byte 0 -> Command::Filter
		// byte 1 -> Name string length (UTF8)
		// byte 2.. -> Name string
		// byte n -> Value
		packet.push_back(Command::CORRECTION_FILTER);
		packet.push_back(nameLen);
		packet.insert(packet.end(), filterName.begin(), filterName.end());
		packet.push_back(static_cast<uint8_t>(value));

		server.Send(streamingDevice, packet.data(), packet.size());
	}

	void Manager::ApplyEffectFilter(std::string filterName)
	{
		uint8_t nameLen = static_cast<uint8_t>(filterName.size());

		std::vector<uint8_t> packet;
		packet.reserve(3 + nameLen);

		// byte 0 -> Command::Filter
		// byte 1 -> Name string length (UTF8)
		// byte 2.. -> Name string
		packet.push_back(Command::EFFECT_FILTER);
		packet.push_back(nameLen);
		packet.insert(packet.end(), filterName.begin(), filterName.end());

		server.Send(streamingDevice, packet.data(), packet.size());
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