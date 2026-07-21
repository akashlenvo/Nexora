#include "net/serializer.h"

#include <stdexcept>
#include <string>

namespace Serializer
{
	namespace
	{
		class Reader
		{
		public:
			Reader(const uint8_t* bytes, size_t size) : bytes(bytes), size(size) {}

			uint8_t ReadUInt8()
			{
				Require(1);
				return bytes[offset++];
			}

			uint16_t ReadUInt16()
			{
				Require(2);
				uint16_t value = (static_cast<uint16_t>(bytes[offset]) << 8)
					| static_cast<uint16_t>(bytes[offset + 1]);
				offset += 2;
				return value;
			}

			std::string ReadString(size_t maximumLength)
			{
				size_t length = ReadUInt16();
				if (length > maximumLength)
					throw std::invalid_argument("Network string exceeds the allowed length");
				Require(length);
				std::string value(reinterpret_cast<const char*>(bytes + offset), length);
				offset += length;
				return value;
			}

		private:
			const uint8_t* bytes;
			size_t size;
			size_t offset = 0;

			void Require(size_t count) const
			{
				if (!bytes || count > size - offset)
					throw IncompletePacket("Incomplete network packet");
			}
		};
	}

	static void WriteInt16(std::vector<uint8_t>& buffer, uint16_t value)
	{
		buffer.push_back((value >> 8) & 0xFF);
		buffer.push_back(value & 0xFF);
	}

	static void WriteInt32(std::vector<uint8_t>& buffer, uint32_t value)
	{
		buffer.push_back((value >> 24) & 0xFF);
		buffer.push_back((value >> 16) & 0xFF);
		buffer.push_back((value >> 8) & 0xFF);
		buffer.push_back(value & 0xFF);
	}

	static void WriteBool(std::vector<uint8_t>& buffer, bool value)
	{
		WriteInt32(buffer, value);
	}

	static void WriteString(std::vector<uint8_t>& buffer, std::string value)
	{
		WriteInt16(buffer, static_cast<uint16_t>(value.size()));
		buffer.insert(buffer.end(), value.begin(), value.end());
	}

	DeviceDescriptor DeserializeDeviceDescriptor(const uint8_t* bytes, size_t size)
	{
		Reader reader(bytes, size);

		// Each string is preceded by a two-byte big-endian length.
		auto name = reader.ReadString(256);
		auto url = reader.ReadString(2048);

		// Default protocol is udp. If it is a usb connection (via adb) switch to tcp
		std::string protocol = "udp";
		if (url.find("127.0.0.1") != std::string::npos)
		{
			protocol = "tcp";
		}

		uint16_t frontResolutionCount = reader.ReadUInt16();
		if (frontResolutionCount > 64)
			throw std::invalid_argument("Invalid front-camera resolution count");

		std::vector<DeviceDescriptor::Resolution> frontResolutions;
		frontResolutions.reserve(frontResolutionCount);
		for (int i = 0; i < frontResolutionCount; i++)
		{
			auto w = reader.ReadUInt16();
			auto h = reader.ReadUInt16();
			frontResolutions.push_back(DeviceDescriptor::Resolution(w, h));
		}

		uint16_t backResolutionCount = reader.ReadUInt16();
		if (backResolutionCount > 64)
			throw std::invalid_argument("Invalid back-camera resolution count");

		std::vector<DeviceDescriptor::Resolution> backResolutions;
		backResolutions.reserve(backResolutionCount);
		for (int i = 0; i < backResolutionCount; i++)
		{
			auto w = reader.ReadUInt16();
			auto h = reader.ReadUInt16();
			backResolutions.push_back(DeviceDescriptor::Resolution(w, h));
		}

		uint16_t filterCount = reader.ReadUInt16();
		if (filterCount > 128)
			throw std::invalid_argument("Invalid filter count");

		Video::Filter::Registry filters;
		for (int i = 0; i < filterCount; i++)
		{
			auto filterName = reader.ReadString(256);
			auto categoryValue = reader.ReadUInt8();
			if (categoryValue > static_cast<uint8_t>(Video::Filter::Category::ARTISTIC))
				throw std::invalid_argument("Invalid filter category");

			filters[static_cast<Video::Filter::Category>(categoryValue)].push_back(filterName);
		}

		return DeviceDescriptor(name, url, protocol, frontResolutions, backResolutions, filters);
	}

	std::vector<uint8_t> SerializeStreamOptions(const StreamOptions& state)
	{
		std::vector<uint8_t> buffer;
		buffer.reserve(512);

		// First byte of the buffer is reserved
		buffer.push_back(0);

		WriteInt32(buffer, state.fps);
		WriteInt32(buffer, state.resolution.first);
		WriteInt32(buffer, state.resolution.second);

		WriteBool(buffer, state.backCameraActive);
		
		WriteBool(buffer, state.adaptiveBitrate);
		WriteInt32(buffer, state.bitrate);
		WriteInt32(buffer, state.minBitrate);
		WriteInt32(buffer, state.maxBitrate);

		WriteBool(buffer, state.stabilizationEnabled);
		WriteBool(buffer, state.flashEnabled);
		WriteBool(buffer, state.h265Enabled);

		WriteInt16(buffer, static_cast<uint16_t>(state.filterSliderValues.size()));
		for (const auto& [name, value] : state.filterSliderValues)
		{
			WriteString(buffer, name);
			WriteInt32(buffer, value);
		}

		WriteString(buffer, state.activeEffectFilter);

		return buffer;
	}

	Connection::ErrorReport DeserializeErrorReport(const uint8_t* bytes, size_t size)
	{
		Reader reader(bytes, size);
		Connection::ErrorReport report;
		report.severity = reader.ReadUInt8();
		report.error = reader.ReadString(1024);
		report.description = reader.ReadString(8192);
		
		return report;
	}
}
