#include "rtsp/manager.h"

#include "logger.h"
#include "net/serializer.h"

namespace RTSP
{
	Manager::Manager(const Server& server, OnFrameReceivedCallback onFrameReceivedCallback, OnStatsReceivedCallback onStatsReceivedCallback) 
		: Receiver(onFrameReceivedCallback, onStatsReceivedCallback),
		server(server)
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

	void Manager::Connect2Stream(int descriptorId, const StreamOptions& options)
	{
		this->streamingDevice = descriptorId;

		auto& descriptor = descriptors[descriptorId];
		auto& url = descriptor.url();

		logger << "[RTSP Manager] Connecting to stream " << url << "\n";
		
		Stop();
		
		auto serializedOptions = Serializer::SerializeStreamOptions(options);
		serializedOptions[0] = Command::ACTIVATION;

		server.Send(streamingDevice, serializedOptions.data(), serializedOptions.size());

		Start(url, descriptor.protocol(), options.resolution.first, options.resolution.second);
	}

	void Manager::SetResolution(unsigned short width, unsigned short height)
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

	void Manager::SetFPS(int fps)
	{
		const unsigned char bytes[2] = { Command::FPS, (uint8_t)fps };
		server.Send(streamingDevice, bytes, 2);
	}

	void Manager::SetBitrate(int bitrate)
	{
		const unsigned char bytes[3] = { 
			Command::BITRATE, 
			static_cast<unsigned char>(bitrate & 0xFF),		// Low byte of bitrate
			static_cast<unsigned char>(bitrate >> 8),         // High byte of bitrate
		};
		server.Send(streamingDevice, bytes, 3);
	}

	void Manager::SetAdaptiveBitrate(int min, int max)
	{
		const unsigned char bytes[5] = {
			Command::ADAPTIVE_BITRATE,
			static_cast<unsigned char>(min & 0xFF),		// Low byte of bitrate
			static_cast<unsigned char>(min >> 8),         // High byte of bitrate
			static_cast<unsigned char>(max & 0xFF),		// Low byte of bitrate
			static_cast<unsigned char>(max >> 8),         // High byte of bitrate
		};
		server.Send(streamingDevice, bytes, 5);
	}

	void Manager::SetStabilization(bool enabled)
	{
		const unsigned char bytes[2] = { Command::STABILIZATION, enabled ? 1 : 0 };
		server.Send(streamingDevice, bytes, 2);
	}

	void Manager::SetFlash(bool enabled)
	{
		const unsigned char bytes[2] = { Command::FLASH, enabled ? 1 : 0 };
		server.Send(streamingDevice, bytes, 2);
	}

	void Manager::SetFocusMode(int mode)
	{
		const unsigned char bytes[2] = { Command::FOCUS, (uint8_t)mode };
		server.Send(streamingDevice, bytes, 2);
	}

	void Manager::SetH265Codec(bool enabled)
	{
		const unsigned char bytes[2] = { Command::CODEC, enabled ? 1 : 0 };
		server.Send(streamingDevice, bytes, 2);
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

	void Manager::Zoom(float factor)
	{
		int bits;
		std::memcpy(&bits, &factor, sizeof(float));

		const unsigned char bytes[5] = {
			Command::ZOOM,
			static_cast<unsigned char>(bits & 0xFF),
			static_cast<unsigned char>((bits >> 8) & 0xFF),
			static_cast<unsigned char>((bits >> 16) & 0xFF),
			static_cast<unsigned char>((bits >> 24) & 0xFF),
		};

		server.Send(streamingDevice, bytes, 5);
	}

	void Manager::FlipHorizontally()
	{
		const unsigned char bytes[2] = { Command::FLIP, 1 };
		server.Send(streamingDevice, bytes, 2);
	}

	void Manager::FlipVertically()
	{
		const unsigned char bytes[2] = { Command::FLIP, 0 };
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